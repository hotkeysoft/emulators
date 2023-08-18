#include "stdafx.h"

#include "ComputerAT.h"
#include <Config.h>
#include "Hardware/Device8042AT.h"
#include "Hardware/Device8237.h"
#include "Hardware/Device8254.h"
#include "Hardware/Device8259.h"
#include "IO/Console.h"
#include "IO/DeviceJoystick.h"
#include "IO/DeviceSerialMouse.h"
#include "Storage/DeviceFloppyXT.h"
#include "Storage/DeviceHardDrive.h"
#include <Sound/Sound.h>

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	static const size_t MAIN_CLK = 14318180;

	static const size_t UART_CLK_DIVIDER = 8;
	static const size_t PIT_CLK_DIVIDER = 12;

	static const size_t UART_CLK = MAIN_CLK / UART_CLK_DIVIDER;
	static const size_t PIT_CLK = MAIN_CLK / PIT_CLK_DIVIDER;

	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	static const BYTE IRQ_FLOPPY = 6;
	static const BYTE DMA_FLOPPY = 2;

	static const BYTE IRQ_HDD = 5;
	static const BYTE DMA_HDD = 3;

	static const BYTE IRQ_SECONDARY = 2; // Secondary PIC is connected to IRQ2 of primary
	static const BYTE DMA_CASCADE = 0;  // First dma controller is cascaded to DMA0 of second controller

	ComputerAT::ComputerAT() :
		Logger("AT"),
		m_baseRAM("RAM", emul::MemoryType::RAM),
		m_highRAM("HIGH", emul::MemoryType::RAM),
		m_biosF000("BIOS", 0x10000, emul::MemoryType::ROM),
		m_picSecondary("pic2", 0xA0, false),
		m_rtc(0x70),
		m_post(0x80),
		m_gameBlaster(0x220),
		m_soundDSS(0x378, PIT_CLK)
	{
	}

	void ComputerAT::Reset()
	{
		Computer::Reset();
		m_nmiEnabled = true;
		m_picSecondary.Reset();
	}

	void ComputerAT::Init(WORD baseRAM)
	{
		Computer::Init(CPUID_80286, baseRAM);

		AddCPUSpeed(CPUSpeed(PIT_CLK, 7));

		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(CONFIG().GetLogLevel("memory"));

		InitRAM(baseRAM);
		InitPIT(new pit::Device8254(0x40, PIT_CLK));
		InitPIC(new pic::Device8259(0x20));
		InitPPI(new ppi::Device8042AT(0x60));
		InitDMA(new dma::Device8237("dma1", 0x00, m_memory), new dma::Device8237("dma2", 0xC0, m_memory));
		InitSound();
		SOUND().SetBaseClock(PIT_CLK);

		m_picSecondary.EnableLog(CONFIG().GetLogLevel("pic"));
		m_picSecondary.Init();
		m_pic->AttachSecondaryDevice(IRQ_SECONDARY, &m_picSecondary);

		// Cascade DMA controllers
		m_dma2->SetCascadedDevice(DMA_CASCADE, m_dma1);

		m_rtc.EnableLog(CONFIG().GetLogLevel("rtc"));
		m_rtc.Init();

		Connect(0x70, PortConnector::OUTFunction(&ComputerAT::WriteNMIMask), true);

		// Temporary, hdd controller must report non busy
		Connect(0x1F7, PortConnector::INFunction(&ComputerAT::ReadHDDStatus));
		Connect(0x1F4, PortConnector::INFunction(&ComputerAT::ReadHDDReg));
		Connect(0x1F4, PortConnector::OUTFunction(&ComputerAT::WriteHDDReg));

		InitVideo("cga", { "cga", "ega", "vga" });

		m_biosF000.LoadOddEven("data/AT/BIOS_5170_V3_F000_ODD.BIN", OddEven::ODD);
		m_biosF000.LoadOddEven("data/AT/BIOS_5170_V3_F000_EVEN.BIN", OddEven::EVEN);
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
		m_keyboard.Init(m_ppi, m_pic);

		InitJoystick(0x201, PIT_CLK);
		InitMouse(UART_CLK);

		InitInputs(PIT_CLK);
		GetInputs().InitKeyboard(&m_keyboard);
		GetInputs().InitJoystick(m_joystick);
		GetInputs().InitMouse(m_mouse);

		std::string soundModule = CONFIG().GetValueStr("sound", "soundcard");
		if (soundModule == "cms" || soundModule == "gb")
		{
			m_gameBlaster.EnableLog(CONFIG().GetLogLevel("sound.cms"));
			m_gameBlaster.Init();
			isSoundGameBlaster = true;
		}
		else if (soundModule == "dss" || soundModule == "covox")
		{
			m_soundDSS.EnableLog(CONFIG().GetLogLevel("sound.dss"));
			m_soundDSS.Init();
			isSoundDSS= true;
		}

		int floppyCount = 0;
		if (CONFIG().GetValueBool("floppy", "enable"))
		{
			InitFloppy(new fdc::DeviceFloppyXT(0x03F0, PIT_CLK), IRQ_FLOPPY, DMA_FLOPPY);
			floppyCount = 2;
		}

		if (CONFIG().GetValueBool("hdd", "enable"))
		{
			InitHardDrive(new hdd::DeviceHardDrive(0x320, PIT_CLK), IRQ_HDD, DMA_HDD);
		}

		m_post.Init(GetCPU());

		{
			ppi::Device8042AT* ppi = (ppi::Device8042AT*)m_ppi;
			ppi->SetCPU(GetCPU());
		}

		// Configuration switches
		{
			ppi::Device8042AT* ppi = (ppi::Device8042AT*)m_ppi;

			ppi->SetPOSTLoop(false);
			ppi->SetKeyLock(true);

			if (m_video->GetID() == "cga")
			{
				ppi->SetDisplayConfig(ppi::DISPLAY::CGA);
			}
			else if ((m_video->GetID() == "mda") || (m_video->GetID() == "hgc"))
			{
				ppi->SetDisplayConfig(ppi::DISPLAY::MDA);
			}
			else
			{
				ppi->SetDisplayConfig(ppi::DISPLAY::OTHER);
			}
		}
	}

	void ComputerAT::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 512)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 64KB", baseRAM);
			baseRAM = 64;
		}
		else if (baseRAM > 2048)
		{
			// TODO: Max block size = 1M.
			// Expand block size or allocate multi blocks
			baseRAM = 2048;
			LogPrintf(LOG_WARNING, "Setting maximum memory size to 2MB");
		}
		m_baseRAMSize = baseRAM;

		// TODO.
		// If requested <= 1M, allocate 640 + extra (above 1M)
		// If not, allocate first meg low (640K allocated) + other megs high
		WORD highRAM = 0;
		if (baseRAM <= 640)
		{
			// Leave as-is
		}
		else if (baseRAM <= 1024)
		{
			highRAM = baseRAM - 640;
			baseRAM = 640;
		}
		else // > 1M
		{
			highRAM = baseRAM - 1024;
			baseRAM = 640;
		}

		m_baseRAM.Alloc(baseRAM * 1024);
		m_baseRAM.Clear();
		m_memory.Allocate(&m_baseRAM, 0);

		if (highRAM)
		{
			m_highRAM.Alloc(highRAM * 1024);
			m_highRAM.Clear();
			m_memory.Allocate(&m_highRAM, 0x100000);
		}
	}

	void ComputerAT::WriteNMIMask(BYTE value)
	{
		LogPrintf(LOG_TRACE, "WriteNMIMask, value=%02x", value);

		bool enabled = !GetBit(value, 7);
		if (m_nmiEnabled != enabled)
		{
			LogPrintf(LOG_DEBUG, "NMI is %s", m_nmiEnabled ? "Enabled" : "Disabled");
		}
		m_nmiEnabled = enabled;
	}

	bool ComputerAT::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		if (m_pic->InterruptPending() && GetCPU()->CanInterrupt())
		{
			m_pic->InterruptAcknowledge();
			LogPrintf(LOG_DEBUG, "[%zu] IRQ %d", g_ticks, m_pic->GetPendingInterrupt() - 8);
			GetCPU()->Interrupt(m_pic->GetPendingInterrupt());
			return true;
		}
		else if (!Computer::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU()->GetInstructionTicks();

		ppi::Device8042AT* ppi = (ppi::Device8042AT*)m_ppi;

		for (uint32_t i = 0; i < cpuTicks / GetCPUSpeedRatio(); ++i)
		{
			++g_ticks;

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}

			m_keyboard.Tick();

			if (m_joystick)
			{
				m_joystick->Tick();
			}

			pit::Counter& timer2 = m_pit->GetCounter(2);
			timer2.SetGate(ppi->GetTimer2Gate());

			ppi->SetRefresh(m_pit->GetCounter(1).GetOutput());
			ppi->Tick();
			m_pic->InterruptRequest(1, ppi->IsInterruptPending());

			m_pit->Tick();
			ppi->SetTimer2Output(timer2.GetOutput());

			m_pic->InterruptRequest(0, m_pit->GetCounter(0).GetOutput());

			if (isSoundGameBlaster)
			{
				// TODO: Ugly
				// Runs as 7.159MHz, 6x base frequency
				for (int i = 0; i < 6; ++i)
				{
					m_gameBlaster.Tick();
				}

				saa1099::OutputData out = m_gameBlaster.GetOutput();
				if (!m_turbo) m_pcSpeaker.Tick(out.left * 10, out.right * 10);
			}
			else if (isSoundDSS)
			{
				m_soundDSS.Tick();
				if (!m_turbo) m_pcSpeaker.Tick(m_soundDSS.GetOutput() * 40);
			}
			else // PC Speaker only
			{
				if (!m_turbo) m_pcSpeaker.Tick();
			}

			if (syncTicks & 1)
			{
				m_video->Tick();
			}
			m_video->Tick();

			TickFloppy();
			TickHardDrive();

			m_mouse->Tick();
			// UART clock is 1.5x base clock
			if (syncTicks & 1)
			{
				m_mouse->Tick();
			}
			m_pic->InterruptRequest(m_mouse->GetIRQ(), m_mouse->IsInterrupt());

			++syncTicks;
		}
		cpuTicks %= GetCPUSpeedRatio();

		return true;
	}
}
