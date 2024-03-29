#include "stdafx.h"
#include "Computer.h"
#include <Config.h>
#include "CPU/CPU80186.h"
#include "CPU/CPU80286.h"
#include "Hardware/Device8167.h"
#include "Hardware/Device8237.h"
#include "Hardware/Device8254.h"
#include "Hardware/Device8259.h"
#include "Hardware/DevicePPI.h"
#include "IO/DeviceJoystick.h"
#include "IO/DeviceKeyboard.h"
#include "IO/DeviceSerialMouse.h"
#include "IO/InputEvents.h"
#include "Storage/DeviceFloppy.h"
#include "Storage/DeviceHardDrive.h"
#include "Video/VideoCGA.h"
#include "Video/VideoMDA.h"
#include "Video/VideoHGC.h"
#include "Video/VideoPCjr.h"
#include "Video/VideoTandy.h"
#include "Video/VideoEga.h"
#include "Video/VideoVga.h"

#include <assert.h>
#include <fstream>

using memory_ega::RAMSIZE;
using cfg::CONFIG;

namespace emul
{
	Computer::Computer() :
		ComputerBase(m_memory, 4096),
		m_hddROM("HDD", 8192, MemoryType::ROM),
		m_dmaPageRegister(0x80)
	{
	}

	Computer::~Computer()
	{
		delete m_floppy;
		delete m_hardDrive;
		delete m_joystick;
		delete m_pit;
		delete m_pic;
		delete m_ppi;
		delete m_dma1;
		delete m_dma2;
		delete m_mouse;
		delete m_rtc;
	}

	void Computer::AddCPUSpeed(const CPUSpeed& speed)
	{
		LogPrintf(LOG_INFO, "Adding CPU Speed ratio: [%2d][%5.2f MHz]", speed.GetRatio(), (float)speed.GetSpeed()/1000000.0f);
		m_cpuSpeeds.insert(speed);
	}

	void Computer::SetCPUSpeed(const CPUSpeed& speed)
	{
		LogPrintf(LOG_INFO, "Set CPU Speed ratio: [%2d][%5.2f MHz]", speed.GetRatio(), (float)speed.GetSpeed() / 1000000.0f);
		m_cpuSpeed = speed;
	}

	void Computer::Reboot(bool hard)
	{
		if (hard)
		{
			LogPrintf(LOG_WARNING, "HARD Reset");
			m_cpu->Reset();
			m_memory.Clear();

			// TODO: Reset all components
			m_video->Reset();
		}
		else
		{
			LogPrintf(LOG_WARNING, "SOFT Reset (CTRL+ALT+DEL)");
			GetKeyboard().InputKey(0x1D); // CTRL
			GetKeyboard().InputKey(0x38); // ALT
			GetKeyboard().InputKey(0x53); // DELETE
		}
	}

	void Computer::Init(const char* cpuid, WORD baseram)
	{
		int cpuOverride = CONFIG().GetValueInt32("core", "cpu");
		switch (cpuOverride)
		{
		case 0:
			// No override
			break;
		case 86:
		case 8086:
			LogPrintf(LOG_WARNING, "CPU override [8086]");
			cpuid = CPUID_8086;
			break;
		case 186:
		case 80186:
			LogPrintf(LOG_WARNING, "CPU override [80186]");
			cpuid = CPUID_80186;
			break;
		case 286:
		case 80286:
			LogPrintf(LOG_WARNING, "CPU override [80286]");
			cpuid = CPUID_80286;
			break;
		default:
			LogPrintf(LOG_WARNING, "Unknown cpu override [%d]", cpuOverride);
		}

		PortConnector::Init(PortConnectorMode::WORD);
		ComputerBase::Init(cpuid, baseram);
	}

	void Computer::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_8086) m_cpu = new CPU8086(m_memory);
		else if (cpuid == CPUID_80186) m_cpu = new CPU80186(m_memory);
		else if (cpuid == CPUID_80286) m_cpu = new CPU80286(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void Computer::InitVideo(const std::string& defaultMode, const VideoModes& supported)
	{
		assert(m_video == nullptr);
		std::string mode = CONFIG().GetValueStr("video", "mode");

		m_videoModes = supported;
		if (supported.empty())
		{
			mode = defaultMode;
			m_videoModes.insert(defaultMode);
		}
		else if (supported.find(mode) == supported.end())
		{
			LogPrintf(LOG_WARNING, "Invalid or unsupported video mode [%s], using default [%s]", mode.c_str(), defaultMode.c_str());
			mode = defaultMode;
		}

		if (mode == "cga")
		{
			m_video = new video::VideoCGA(0x3D0);
		}
		else if (mode == "mda")
		{
			m_video = new video::VideoMDA(0x3B0);
		}
		else if (mode == "hgc")
		{
			m_video = new video::VideoHGC(0x3B0);
		}
		else if (mode == "pcjr")
		{
			m_video = new video::VideoPCjr(0x3D0);
		}
		else if (mode == "tga")
		{
			m_video = new video::VideoTandy(0x3D0);
		}
		else if (mode == "ega")
		{
			std::string ramSizeStr = CONFIG().GetValueStr("video.ega", "ram", "256");
			std::string egaMode = CONFIG().GetValueStr("video.ega", "mode", "ega");
			if (egaMode.empty())
			{
				egaMode = "ega";
			}

			RAMSIZE ramSize;
			if (ramSizeStr == "64")
			{
				ramSize = RAMSIZE::EGA_64K;
			}
			else if (ramSizeStr == "128")
			{
				ramSize = RAMSIZE::EGA_128K;
			}
			else if (ramSizeStr == "256")
			{
				ramSize = RAMSIZE::EGA_256K;
			}
			else
			{
				LogPrintf(LOG_WARNING, "Invalid EGA ram size [%s], using 256K", ramSizeStr.c_str());
				ramSize = RAMSIZE::EGA_256K;
			}

			m_video = new video::VideoEGA(egaMode.c_str(), ramSize, 0x3C0, 0x3B0, 0x3D0);
		}
		else if (mode == "vga")
		{
			m_video = new video::VideoVGA(0x3C0, 0x3B0, 0x3D0);
		}
		else
		{
			throw std::exception("Invalid video mode");
		}
		assert(m_video);
		m_video->EnableLog(CONFIG().GetLogLevel("video"));

		LogPrintf(LOG_INFO, "Video mode: [%s]", mode.c_str());

		BYTE border = std::min(255, CONFIG().GetValueInt32("video", "border", 10));
		LogPrintf(LOG_INFO, "Border: [%d]", border);

		std::string charROM = CONFIG().GetValueStr("video", "charrom", "data/XT/CGA_CHAR.BIN");
		// Check for override
		std::string overrideKey = "charrom." + mode;
		charROM = CONFIG().GetValueStr("video", overrideKey.c_str(), charROM.c_str());

		LogPrintf(LOG_INFO, "Character ROM: [%s]", charROM.c_str());

		m_video->Init(&m_memory, charROM.c_str());
		m_video->SetBorder(border);
	}

	void Computer::InitSound()
	{
		m_pcSpeaker.EnableLog(CONFIG().GetLogLevel("sound.pc"));
		m_pcSpeaker.Init(m_ppi, m_pit);
	}

	void Computer::InitPIT(pit::Device8254* pit)
	{
		assert(pit);
		m_pit = pit;
		m_pit->EnableLog(CONFIG().GetLogLevel("pit"));
		m_pit->Init();
	}
	void Computer::InitPIC(pic::Device8259* pic)
	{
		assert(pic);
		m_pic = pic;
		m_pic->EnableLog(CONFIG().GetLogLevel("pic"));
		m_pic->Init();
	}
	void Computer::InitPPI(ppi::DevicePPI* ppi)
	{
		assert(ppi);
		m_ppi = ppi;
		m_ppi->EnableLog(CONFIG().GetLogLevel("ppi"));
		m_ppi->Init();
	}

	void Computer::InitDMA(dma::Device8237* dma1, dma::Device8237* dma2)
	{
		assert(dma1);
		m_dma1 = dma1;
		m_dma1->EnableLog(CONFIG().GetLogLevel("dma"));
		m_dma1->Init();

		if (dma2)
		{
			m_dma2 = dma2;
			m_dma2->EnableLog(CONFIG().GetLogLevel("dma"));

			// Secondary DMA Controller works on 16 bit transfers:
			// All bits are shifted to the left (including port addresses)
			m_dma2->Init(1);
		}

		m_dmaPageRegister.EnableLog(CONFIG().GetLogLevel("dma.page"));
		m_dmaPageRegister.Init(dma1, dma2);
	}

	void Computer::InitJoystick(WORD baseAddress, size_t baseClock)
	{
		if (CONFIG().GetValueBool("joystick", "enable"))
		{
			m_joystick = new joy::DeviceJoystick(baseAddress, baseClock);
			m_joystick->EnableLog(CONFIG().GetLogLevel("joystick"));
			m_joystick->Init();
		}
	}

	void Computer::InitMouse(size_t baseClock)
	{
		if (CONFIG().GetValueBool("joystick", "enable"))
		{
			BYTE comPort = CONFIG().GetValueInt32("mouse", "com", 1);
			if (comPort < 1 || comPort > 4)
			{
				comPort = 1;
				LogPrintf(LOG_WARNING, "InitMouse: Invalid COM port, using default (COM1)");
			}
			WORD port = COM_PORT[comPort];
			BYTE irq = COM_IRQ[comPort];

			WORD overridePort = CONFIG().GetValueInt32("mouse", "port");
			BYTE overrideIRQ = CONFIG().GetValueInt32("mouse", "irq");

			if (overridePort)
			{
				LogPrintf(LOG_INFO, "InitMouse: Using override port [%04x]", overridePort);
				port = overridePort;
			}

			if (overrideIRQ && overrideIRQ < 8)
			{
				LogPrintf(LOG_INFO, "InitMouse: Using override IRQ [%d]", overrideIRQ);
				irq = overrideIRQ;
			}

			LogPrintf(LOG_INFO, "InitMouse: Initializing mouse on port[%04x], irq[%d]", port, irq);

			m_mouse = new mouse::DeviceSerialMouse(port, irq);
			m_mouse->EnableLog(CONFIG().GetLogLevel("mouse"));
			m_mouse->Init();
		}
	}

	void Computer::InitRTC()
	{
		if (CONFIG().GetValueBool("rtc", "enable"))
		{
			const WORD defaultPort = 0x240;
			WORD port = CONFIG().GetValueInt32("rtc", "port", 0x240);
			if (port == 0)
			{
				port = defaultPort;
			}

			m_rtc = new rtc::Device8167(port);
			m_rtc->EnableLog(CONFIG().GetLogLevel("rtc"));
			m_rtc->Init();
		}
	}

	void Computer::InitFloppy(fdc::DeviceFloppy* fdd, BYTE irq, BYTE dma)
	{
		assert(fdd);
		m_floppy = fdd;
		m_floppyIRQ = irq;
		m_floppyDMA = dma;

		m_floppy->EnableLog(CONFIG().GetLogLevel("floppy"));
		m_floppy->Init();

		std::string floppy = CONFIG().GetValueStr("floppy", "floppy.1");
		if (floppy.size())
		{
			m_floppy->LoadDiskImage(0, floppy.c_str());
		}

		floppy = CONFIG().GetValueStr("floppy", "floppy.2");
		if (floppy.size())
		{
			m_floppy->LoadDiskImage(1, floppy.c_str());
		}
	}
	void Computer::InitHardDrive(hdd::DeviceHardDrive* hdd, BYTE irq, BYTE dma)
	{
		assert(hdd);
		m_hardDrive = hdd;
		m_hddIRQ = irq;
		m_hddDMA = dma;

		m_hardDrive->EnableLog(CONFIG().GetLogLevel("hdd"));
		m_hardDrive->Init();

		for (int i = 0; i < 2; i++)
		{
			const HardDriveImageInfo& img = GetHardDriveImageInfo(i + 1);
			if (img.set)
			{
				m_hardDrive->LoadDiskImage(i, img.type, img.file.c_str());
			}
		}

		std::string romFile = CONFIG().GetValueStr("hdd", "rom");
		if (romFile.empty())
		{
			romFile = "data/hdd/WD1002S-WX2_62-000042-11.bin";
		}
		m_hddROM.LoadFromFile(romFile.c_str());
		m_memory.Allocate(&m_hddROM, 0xC8000);
	}

	Computer::HardDriveImageInfo Computer::GetHardDriveImageInfo(int id)
	{
		char key[16];
		sprintf(key, "hdd.%d", id);

		std::string imageStr = CONFIG().GetValueStr("hdd", key);
		HardDriveImageInfo info;
		if (imageStr.size())
		{
			std::stringstream ss(imageStr);
			std::string type;
			std::getline(ss, type, ',');
			if (type.size())
			{
				info.type = atoi(type.c_str());
			}
			else
			{
				LogPrintf(LOG_ERROR, "GetHDDImageConfig: Error parsing hdd image type for key [hdd][%s]", key);
			}
			std::getline(ss, info.file);
			if (info.file.size())
			{
				info.set = true;
			}
			else
			{
				LogPrintf(LOG_ERROR, "GetHDDImageConfig: Error parsing hdd image file for key [hdd][%s]", key);
			}
		}

		return info;
	}

	void Computer::TickHardDrive()
	{
		if (!m_hardDrive)
		{
			return;
		}

		m_hardDrive->Tick();
		m_pic->InterruptRequest(m_hddIRQ, m_hardDrive->IsInterruptPending());

		if (!m_hddDMA)
		{
			return;
		}

		if (m_hardDrive->IsDMAPending())
		{
			m_dma1->DMARequest(m_hddDMA, true);
		}

		if (m_dma1->DMAAcknowledged(m_hddDMA))
		{
			m_dma1->DMARequest(m_hddDMA, false);

			// Do it manually
			m_hardDrive->DMAAcknowledge();

			dma::DMAChannel& channel = m_dma1->GetChannel(m_hddDMA);
			dma::OPERATION op = channel.GetOperation();
			BYTE value;
			switch (op)
			{
			case dma::OPERATION::READ:
				channel.DMAOperation(value);
				m_hardDrive->WriteDataFIFO(value);
				break;
			case dma::OPERATION::WRITE:
				value = m_hardDrive->ReadDataFIFO();
				channel.DMAOperation(value);
				break;
			case dma::OPERATION::VERIFY:
				channel.DMAOperation(value);
				break;
			default:
				throw std::exception("DMAOperation: Operation not supported");
			}

			if (m_dma1->GetTerminalCount(m_hddDMA))
			{
				m_hardDrive->DMATerminalCount();
			}
		}
	}
	void Computer::TickFloppy()
	{
		if (!m_floppy)
		{
			return;
		}

		m_floppy->Tick();
		m_pic->InterruptRequest(m_floppyIRQ, m_floppy->IsInterruptPending());

		if (!m_floppyDMA)
		{
			return;
		}

		// TODO: duplication with HDD
		if (m_floppy->IsDMAPending())
		{
			m_dma1->DMARequest(m_floppyDMA, true);
		}

		if (m_dma1->DMAAcknowledged(m_floppyDMA))
		{
			m_dma1->DMARequest(m_floppyDMA, false);

			// Do it manually
			m_floppy->DMAAcknowledge();

			dma::DMAChannel& channel = m_dma1->GetChannel(m_floppyDMA);
			dma::OPERATION op = channel.GetOperation();
			BYTE value;
			switch (op)
			{
			case dma::OPERATION::READ:
				channel.DMAOperation(value);
				m_floppy->WriteDataFIFO(value);
				break;
			case dma::OPERATION::WRITE:
				value = m_floppy->ReadDataFIFO();
				channel.DMAOperation(value);
				break;
			case dma::OPERATION::VERIFY:
				channel.DMAOperation(value);
				break;
			default:
				throw std::exception("DMAOperation: Operation not supported");
			}

			if (m_dma1->GetTerminalCount(m_floppyDMA))
			{
				m_floppy->DMATerminalCount();
			}
		}
	}

	void Computer::Serialize(json& to)
	{
		ComputerBase::Serialize(to);

		m_cpu->Serialize(to["cpu"]);
		m_memory.Serialize(to["memory"]);
		m_pit->Serialize(to["pit"]);
		m_pic->Serialize(to["pic"]);
		m_video->Serialize(to["video"]);
		m_cpuSpeed.Serialize(to["speed"]);

		if (m_floppy)
		{
			m_floppy->Serialize(to["floppy"]);
		}

		if (m_hardDrive)
		{
			m_hardDrive->Serialize(to["hardDrive"]);
		}

		if (m_mouse)
		{
			m_mouse->Serialize(to["mouse"]);
		}
	}

	void Computer::Deserialize(const json& from)
	{
		ComputerBase::Deserialize(from);

		m_pit->Deserialize(from["pit"]);
		m_pic->Deserialize(from["pic"]);
		m_cpuSpeed.Deserialize(from["speed"]);

		if ((from.contains("floppy") && !m_floppy) ||
			(!from.contains("floppy") && m_floppy))
		{
			throw SerializableException("Computer: Floppy configuration is not compatible", SerializationError::COMPAT);
		}

		if (m_floppy)
		{
			m_floppy->Deserialize(from["floppy"]);
		}

		if ((from.contains("hardDrive") && !m_hardDrive) ||
			(!from.contains("hardDrive") && m_hardDrive))
		{
			throw SerializableException("Computer: Hard drive configuration is not compatible", SerializationError::COMPAT);
		}

		if (m_hardDrive)
		{
			m_hardDrive->Deserialize(from["hardDrive"]);
		}

		if (!from.contains("mouse") && m_mouse)
		{
			LogPrintf(LOG_WARNING, "Snapshot doesn't contain mouse data");
		}

		if (from.contains("mouse") && !m_mouse)
		{
			throw SerializableException("Computer: Mouse configuration is not compatible", SerializationError::COMPAT);
		}

		if (m_mouse && from.contains("mouse"))
		{
			m_mouse->Deserialize(from["mouse"]);
		}
	}
}
