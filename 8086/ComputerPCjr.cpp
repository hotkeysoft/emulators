#include "stdafx.h"

#include "ComputerPCjr.h"
#include "Config.h"
#include "Hardware/Device8255PCjr.h"
#include "Video/VideoPCjr.h"
#include "Storage/DeviceFloppyPCjr.h"

#include <thread>

using cfg::CONFIG;

namespace emul
{
	static const size_t MAIN_CLK = 14318180;

	static const size_t UART_CLK_DIVIDER = 8;
	static const size_t PIT_CLK_DIVIDER = 12;
	static const size_t SOUND_CLK_DIVIDER = 4;

	static const size_t UART_CLK = MAIN_CLK / UART_CLK_DIVIDER;
	static const size_t PIT_CLK = MAIN_CLK / PIT_CLK_DIVIDER;
	static const size_t SOUND_CLK = MAIN_CLK / SOUND_CLK_DIVIDER;

	static const BYTE IRQ_FLOPPY = 6;
	static const BYTE IRQ_UART = 3;
	static const BYTE IRQ_VSYNC = 5;

	ComputerPCjr::ComputerPCjr() :
		Logger("PCjr"),
		Computer(m_memory, m_map),
		m_base64K("RAM0", 0x10000, emul::MemoryType::RAM),
		m_ext64K("RAM1", emul::MemoryType::RAM),
		m_extraRAM("EXTRAM", emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_keyboard(0xA0),
		m_uart(0x2F8, IRQ_UART, UART_CLK),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerPCjr::Init(WORD baseRAM)
	{
		Computer::Init(baseRAM);

		AddCPUSpeed(CPUSpeed(PIT_CLK, 3));
		AddCPUSpeed(CPUSpeed(PIT_CLK, 4));

		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(CONFIG().GetLogLevel("memory"));
		m_mmap.EnableLog(CONFIG().GetLogLevel("mmap"));

		InitRAM(baseRAM);
		InitPIT(new pit::Device8254(0x40, PIT_CLK));
		InitPIC(new pic::Device8259(0x20));
		InitPPI(new ppi::Device8255PCjr(0x60));

		{
			ppi::Device8255PCjr* ppi = (ppi::Device8255PCjr*)m_ppi;
			ppi->SetKeyboardConnected(true);
			ppi->SetRAMExpansion(baseRAM > 64);
			ppi->SetDisketteCard(true);
			ppi->SetModemCard(false);
		}

		InitSound();

		m_soundModule.EnableLog(CONFIG().GetLogLevel("sound.76489"));
		m_soundModule.Init();

		InitVideo("pcjr");
		
		m_biosF000.LoadFromFile("data/PCjr/BIOS_4860_1504036_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadFromFile("data/PCjr/BIOS_4860_1504037_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
		m_keyboard.Init(m_ppi, m_pic);

		// TODO: Make this dynamic
		// Cartridges
		if (m_cart1.LoadFromFile("data/PCjr/CartridgeBASIC_E800.jrc"))
		{
			m_memory.Allocate(&m_cart1, m_cart1.GetBaseAddress());
		}

		// Cartridges
		//if (m_cart2.LoadFromFile(R"()"))
		//{
		//	m_memory.Allocate(&m_cart2, m_cart2.GetBaseAddress());
		//}

		if (CONFIG().GetValueBool("floppy", "enable"))
		{
			InitFloppy(new fdc::DeviceFloppyPCjr(0xF0, PIT_CLK));
		}
		
		m_uart.EnableLog(CONFIG().GetLogLevel("uart"));
		m_uart.Init();

		InitJoystick(0x201, PIT_CLK);

		InitInputs(PIT_CLK);
		GetInputs().InitKeyboard(&m_keyboard);
		GetInputs().InitJoystick(m_joystick);
	}

	void ComputerPCjr::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 64)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 64KB", baseRAM);
			baseRAM = 64;
		}

		// 64KB Base memory
		m_base64K.Clear();
		LogPrintf(LOG_INFO, "Allocating base 64KB block");
		m_memory.Allocate(&m_base64K, 0);

		// 64KB Memory extension
		if (baseRAM > 64)
		{
			m_ext64K.Alloc(0x10000);
			m_ext64K.Clear();
			LogPrintf(LOG_INFO, "Allocating 64KB block extension");
			m_memory.Allocate(&m_ext64K, 0x10000);
		}
		else
		{
			LogPrintf(LOG_INFO, "Duplicating base 64K at 0x10000");
			m_memory.Allocate(&m_base64K, 0x10000);
		}

		// Extra RAM
		if (baseRAM > 128)
		{
			if (baseRAM > 768)
			{
				baseRAM = 768;
				LogPrintf(LOG_WARNING, "Setting maximum memory size to 768KB");
			}
			DWORD extraRAM = (baseRAM - 128);
			LogPrintf(LOG_INFO, "Allocating %dKB extra RAM at address 0", extraRAM);

			m_extraRAM.Alloc(extraRAM * 1024);
			m_extraRAM.Clear();
			m_memory.Allocate(&m_extraRAM, 0x20000);
		}
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

	bool ComputerPCjr::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		static bool timer0Out = false;

		if (m_keyboard.NMIPending())
		{
			Interrupt(2);
			return true;
		}
		else if (m_pic->InterruptPending() && CanInterrupt())
		{
			m_pic->InterruptAcknowledge();
			Interrupt(m_pic->GetPendingInterrupt());
			return true;
		}
		else if (!CPU8086::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		assert(GetInstructionTicks());
		cpuTicks += GetInstructionTicks();

		ppi::Device8255PCjr* ppi = (ppi::Device8255PCjr*)m_ppi;
		video::VideoPCjr* video = (video::VideoPCjr*)m_video;

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

			m_pit->GetCounter(0).Tick();
			if (m_keyboard.GetTimer1Source() == kbd::CLK1::MAIN_CLK)
			{
				m_pit->GetCounter(1).Tick();
			}

			pit::Counter& timer2 = m_pit->GetCounter(2);
			timer2.SetGate(ppi->GetTimer2Gate());
			timer2.Tick();

			ppi->SetTimer2Output(timer2.GetOutput());

			bool out = m_pit->GetCounter(0).GetOutput();
			m_pic->InterruptRequest(0, out);
			if (out != timer0Out)
			{
				if (out && m_keyboard.GetTimer1Source() == kbd::CLK1::TIMER0_OUT)
				{
					m_pit->GetCounter(1).Tick();
				}
				timer0Out = out;
			}

			// SN76489 clock is 3x base clock
			m_soundModule.Tick(); m_soundModule.Tick(); m_soundModule.Tick();

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			if (!m_turbo) m_pcSpeaker.Tick(m_soundModule.GetOutput());

			TickFloppy();

			if ((syncTicks & 1))
			{
				video->Tick();
				m_pic->InterruptRequest(IRQ_VSYNC, (video->IsVSync()));
			}
			video->Tick();
			m_pic->InterruptRequest(IRQ_VSYNC, (video->IsVSync()));

			m_uart.Tick();
			// UART clock is 1.5x base clock
			if (syncTicks & 1)
			{
				m_uart.Tick();
			}
			m_pic->InterruptRequest(m_uart.GetIRQ(), m_uart.IsInterrupt());

			++syncTicks;
		}
		cpuTicks %= GetCPUSpeedRatio();

		return true;
	}
	void ComputerPCjr::TickFloppy()
	{
		if (!m_floppy)
		{
			return;
		}

		fdc::DeviceFloppyPCjr* floppy = (fdc::DeviceFloppyPCjr*)m_floppy;
		floppy->Tick();
		m_pic->InterruptRequest(IRQ_FLOPPY, floppy->IsWatchdogInterrupt());
	}
}
