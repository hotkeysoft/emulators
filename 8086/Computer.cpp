#include "Common.h"
#include "Computer.h"
#include "Config.h"
#include "IO/DeviceKeyboard.h"
#include "Storage/DeviceHardDrive.h"
#include "Video/VideoCGA.h"
#include "Video/VideoMDA.h"
#include "Video/VideoHGC.h"
#include "Video/VideoPCjr.h"
#include "Video/VideoTandy.h"
#include "Video/VideoEga.h"

#include <assert.h>
#include <fstream>

using cfg::Config;

namespace emul
{
	Computer::Computer(Memory& memory, MemoryMap& mmap) :
		Logger("PC"),
		CPU8086(m_memory, m_map),
		m_memory(emul::CPU8086_ADDRESS_BITS),
		m_hddROM("HDD", 8192, MemoryType::ROM)
	{
	}

	Computer::~Computer()
	{
		delete m_floppy;
		delete m_hardDrive;
		delete m_joystick;
		delete m_video;
		delete m_pit;
		delete m_pic;
		delete m_ppi;
		delete m_dma;
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
			Reset();
			m_memory.Clear();
		}
		else
		{
			LogPrintf(LOG_WARNING, "SOFT Reset (CTRL+ALT+DEL)");
			GetKeyboard().InputKey(0x1D); // CTRL
			GetKeyboard().InputKey(0x38); // ALT
			GetKeyboard().InputKey(0x53); // DELETE
		}
	}

	void Computer::InitVideo(const std::string& defaultMode, const VideoModes& supported)
	{
		assert(m_video == nullptr);
		std::string mode = Config::Instance().GetValueStr("video", "mode");

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
			// TODO: RAM size from config
			m_video = new video::VideoEGA(memory_ega::RAMSIZE::EGA_256K, 0x3C0, 0x3B0, 0x3D0);
		}
		else
		{
			throw std::exception("Invalid video mode mode");
		}
		assert(m_video);
		m_video->EnableLog(Config::Instance().GetLogLevel("video"));

		LogPrintf(LOG_INFO, "Video mode: [%s]", mode.c_str());

		BYTE border = std::min(255, Config::Instance().GetValueInt32("video", "border", 10));
		LogPrintf(LOG_INFO, "Border: [%d]", border);

		std::string charROM = Config::Instance().GetValueStr("video", "charrom", "data/XT/CGA_CHAR.BIN");
		// Check for override
		std::string overrideKey = "charrom." + mode;
		charROM = Config::Instance().GetValueStr("video", overrideKey.c_str(), charROM.c_str());

		LogPrintf(LOG_INFO, "Character ROM: [%s]", charROM.c_str());

		m_video->Init(&m_memory, charROM.c_str());
		m_video->SetBorder(border);
	}

	void Computer::InitSound()
	{
		m_pcSpeaker.EnableLog(Config::Instance().GetLogLevel("sound"));
		m_pcSpeaker.Init(m_ppi, m_pit);
		m_pcSpeaker.SetMute(Config::Instance().GetValueBool("sound", "mute"));
		std::string audioStream = Config::Instance().GetValueStr("sound", "raw");
		if (audioStream.size())
		{
			m_pcSpeaker.StreamToFile(true, audioStream.c_str());
		}
	}

	void Computer::InitPIT(pit::Device8254* pit)
	{
		assert(pit);
		m_pit = pit;
		m_pit->EnableLog(Config::Instance().GetLogLevel("pit"));
		m_pit->Init();
	}
	void Computer::InitPIC(pic::Device8259* pic)
	{
		assert(pic);
		m_pic = pic;
		m_pic->EnableLog(Config::Instance().GetLogLevel("pic"));
		m_pic->Init();
	}
	void Computer::InitPPI(ppi::Device8255* ppi)
	{
		assert(ppi);
		m_ppi = ppi;
		m_ppi->EnableLog(Config::Instance().GetLogLevel("ppi"));
		m_ppi->Init();
	}

	void Computer::InitDMA(dma::Device8237* dma)
	{
		assert(dma);
		m_dma = dma;
		m_dma->EnableLog(Config::Instance().GetLogLevel("dma"));
		m_dma->Init();
	}

	void Computer::InitJoystick(WORD baseAddress, size_t baseClock)
	{
		if (Config::Instance().GetValueBool("joystick", "enable"))
		{
			m_joystick = new joy::DeviceJoystick(baseAddress, baseClock);
			m_joystick->EnableLog(Config::Instance().GetLogLevel("joystick"));
			m_joystick->Init();
		}
	}

	void Computer::InitInputs(size_t clockSpeedHz)
	{
		m_inputs = new events::InputEvents(clockSpeedHz);
		m_inputs->EnableLog(Config::Instance().GetLogLevel("inputs"));
	}

	void Computer::InitFloppy(fdc::DeviceFloppy* fdd, BYTE irq, BYTE dma)
	{
		assert(fdd);
		m_floppy = fdd;
		m_floppyIRQ = irq;
		m_floppyDMA = dma;

		m_floppy->EnableLog(Config::Instance().GetLogLevel("floppy"));
		m_floppy->Init();

		std::string floppy = Config::Instance().GetValueStr("floppy", "floppy.1");
		if (floppy.size())
		{
			m_floppy->LoadDiskImage(0, floppy.c_str());
		}

		floppy = Config::Instance().GetValueStr("floppy", "floppy.2");
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

		m_hardDrive->EnableLog(Config::Instance().GetLogLevel("hdd"));
		m_hardDrive->Init();

		for (int i = 0; i < 2; i++)
		{
			const HardDriveImageInfo& img = GetHardDriveImageInfo(i + 1);
			if (img.set)
			{
				m_hardDrive->LoadDiskImage(i, img.type, img.file.c_str());
			}
		}

		std::string romFile = Config::Instance().GetValueStr("hdd", "rom");
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

		std::string imageStr = Config::Instance().GetValueStr("hdd", key);
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
			m_dma->DMARequest(m_hddDMA, true);
		}

		if (m_dma->DMAAcknowledged(m_hddDMA))
		{
			m_dma->DMARequest(m_hddDMA, false);

			// Do it manually
			m_hardDrive->DMAAcknowledge();

			dma::DMAChannel& channel = m_dma->GetChannel(m_hddDMA);
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

			if (m_dma->GetTerminalCount(m_hddDMA))
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
			m_dma->DMARequest(m_floppyDMA, true);
		}

		if (m_dma->DMAAcknowledged(m_floppyDMA))
		{
			m_dma->DMARequest(m_floppyDMA, false);

			// Do it manually
			m_floppy->DMAAcknowledge();

			dma::DMAChannel& channel = m_dma->GetChannel(m_floppyDMA);
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

			if (m_dma->GetTerminalCount(m_floppyDMA))
			{
				m_floppy->DMATerminalCount();
			}
		}
	}

	void Computer::Serialize(json& to)
	{
		CPU8086::Serialize(to["cpu"]);
		m_memory.Serialize(to["memory"]);
		m_pit->Serialize(to["pit"]);
		m_pic->Serialize(to["pic"]);
		m_video->Serialize(to["video"]);
	}

	void Computer::Deserialize(json& from)
	{
		CPU8086::Deserialize(from["cpu"]);
		m_memory.Deserialize(from["memory"]);
		m_pit->Deserialize(from["pit"]);
		m_pic->Deserialize(from["pic"]);
		m_video->Deserialize(from["video"]);
	}
}
