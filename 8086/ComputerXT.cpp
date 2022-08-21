#include "stdafx.h"

#include "ComputerXT.h"
#include "Config.h"
#include "IO/Console.h"
#include "Hardware/Device8237.h"
#include "Hardware/Device8254.h"
#include "Hardware/Device8255XT.h"
#include "IO/DeviceJoystick.h"
#include "IO/DeviceSerialMouse.h"
#include "Storage/DeviceFloppyXT.h"
#include "Storage/DeviceHardDrive.h"

#include <thread>

using cfg::CONFIG;
using cpuInfo::CPUType;

namespace emul
{
	static const size_t MAIN_CLK = 14318180;

	static const size_t UART_CLK_DIVIDER = 8;
	static const size_t PIT_CLK_DIVIDER = 12;
	static const size_t SOUND_CLK_DIVIDER = 4;

	static const size_t UART_CLK = MAIN_CLK / UART_CLK_DIVIDER;
	static const size_t PIT_CLK = MAIN_CLK / PIT_CLK_DIVIDER;
	static const size_t SOUND_CLK = MAIN_CLK / SOUND_CLK_DIVIDER;

	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	static const BYTE IRQ_FLOPPY = 6;
	static const BYTE DMA_FLOPPY = 2;

	static const BYTE IRQ_HDD = 5;
	static const BYTE DMA_HDD = 3;

	ComputerXT::ComputerXT() :
		Logger("XT"),
		Computer(m_memory),
		m_baseRAM("RAM", emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_soundPCjr(0xC0, SOUND_CLK),
		m_gameBlaster(0x220),
		m_soundDSS(0x378, PIT_CLK)
	{
	}

	void ComputerXT::Init(WORD baseRAM)
	{
		Computer::Init(CPUType::i8086, baseRAM);

		AddCPUSpeed(CPUSpeed(PIT_CLK, 4));
		AddCPUSpeed(CPUSpeed(PIT_CLK, 8));
		AddCPUSpeed(CPUSpeed(PIT_CLK, 12));

		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		InitRAM(baseRAM);
		InitPIT(new pit::Device8254(0x40, PIT_CLK));
		InitPIC(new pic::Device8259(0x20));
		InitPPI(new ppi::Device8255XT(0x60));
		InitDMA(new dma::Device8237(0x00, m_memory));
		InitSound();
		InitRTC();

		m_dma1->GetChannel(0).EnableLog(LOG_OFF);

		InitVideo("cga", { "cga", "mda", "hgc", "ega", "vga"});

		m_biosF000.LoadFromFile("data/XT/BIOS_5160_V3_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadFromFile("data/XT/BIOS_5160_V3_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
		m_keyboard.Init(m_ppi, m_pic);

		InitJoystick(0x201, PIT_CLK);
		InitMouse(UART_CLK);

		InitInputs(PIT_CLK);
		GetInputs().InitKeyboard(&m_keyboard);
		GetInputs().InitJoystick(m_joystick);
		GetInputs().InitMouse(m_mouse);

		std::string soundModule = CONFIG().GetValueStr("sound", "soundcard");
		if (soundModule == "pcjr" || soundModule == "tandy")
		{
			m_soundPCjr.EnableLog(CONFIG().GetLogLevel("sound.pcjr"));
			m_soundPCjr.Init();
			isSoundPCjr = true;
		}
		else if (soundModule == "cms" || soundModule == "gb")
		{
			m_gameBlaster.EnableLog(CONFIG().GetLogLevel("sound.cms"));
			m_gameBlaster.Init();
			isSoundGameBlaster = true;
		}
		else if (soundModule == "dss" || soundModule == "covox")
		{
			m_soundDSS.EnableLog(CONFIG().GetLogLevel("sound.dss"));
			m_soundDSS.Init();
			isSoundDSS = true;
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

		// Configuration switches
		{
			ppi::Device8255XT* ppi = (ppi::Device8255XT*)m_ppi;
			ppi->SetPOSTLoop(false);
			ppi->SetMathCoprocessor(false);
			ppi->SetRAMConfig(ppi::RAMSIZE::RAM_256K);
			if (m_video->IsMonoAdapter())
			{
				ppi->SetDisplayConfig(ppi::DISPLAY::MONO_80x25);
			}
			else if ((m_video->GetID() == "ega") || (m_video->GetID() == "vga"))
			{
				// TODO: Put flag in video card
				ppi->SetDisplayConfig(ppi::DISPLAY::NONE);
			}
			else
			{
				ppi->SetDisplayConfig(screenWidth == COLS80 ? ppi::DISPLAY::COLOR_80x25 : ppi::DISPLAY::COLOR_40x25);
			}
			ppi->SetFloppyCount(floppyCount);
		}
	}

	void ComputerXT::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 64)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 64KB", baseRAM);
			baseRAM = 64;
		}
		else if (baseRAM > 768)
		{
			baseRAM = 768;
			LogPrintf(LOG_WARNING, "Setting maximum memory size to 768KB");
		}

		m_baseRAM.Alloc(baseRAM * 1024);
		m_baseRAM.Clear();
		m_memory.Allocate(&m_baseRAM, 0);
	}

	static void little_sleep(std::chrono::microseconds us)
	{
		auto start = std::chrono::high_resolution_clock::now();
		auto end = start + us;
		do 
		{
			std::this_thread::yield();
		} while (std::chrono::high_resolution_clock::now() < end);
	}

	bool ComputerXT::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		if (m_pic->InterruptPending() && GetCPU().CanInterrupt())
		{
			m_pic->InterruptAcknowledge(); 
			LogPrintf(LOG_DEBUG, "[%zu] IRQ %d", g_ticks, m_pic->GetPendingInterrupt() - 8);
			GetCPU().Interrupt(m_pic->GetPendingInterrupt());
			return true;
		}
		else if (m_soundPCjr.IsReady())
		{
			if (!Computer::Step())
			{
				return false;
			}
		}

		static uint32_t cpuTicks = 0;	
		cpuTicks += GetCPU().GetInstructionTicks();

		ppi::Device8255XT* ppi = (ppi::Device8255XT*)m_ppi;

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

			m_pit->Tick();
			ppi->SetTimer2Output(timer2.GetOutput());

			m_pic->InterruptRequest(0, m_pit->GetCounter(0).GetOutput());

			if (isSoundPCjr)
			{
				// SN76489 clock is 3x base clock
				m_soundPCjr.Tick(); m_soundPCjr.Tick(); m_soundPCjr.Tick();
				if (!m_turbo) m_pcSpeaker.Tick(m_soundPCjr.GetOutput());
			}
			else if (isSoundGameBlaster)
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
				if (!m_turbo) m_pcSpeaker.Tick(m_soundDSS.GetOutput());
			}
			else // PC Speaker only
			{
				if (!m_turbo) m_pcSpeaker.Tick();
			}

			// Fake DMA Channel 0 memory refresh to shut up POST
			{
				static size_t dma0Div = 0;
				if (dma0Div++ == 15)
				{
					dma0Div = 0;
					m_dma1->GetChannel(0).Tick();
				}
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
