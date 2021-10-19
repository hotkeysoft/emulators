#include "Common.h"
#include "ComputerPCjr.h"
#include <thread>

namespace emul
{
	static const size_t MAIN_CLK = 14318180;

	static const size_t CPU_CLK_DIVIDER = 3;
	static const size_t UART_CLK_DIVIDER = 8;
	static const size_t PIT_CLK_DIVIDER = 12;

	static const size_t CPU_CLK = MAIN_CLK / CPU_CLK_DIVIDER;
	static const size_t UART_CLK = MAIN_CLK / UART_CLK_DIVIDER;
	static const size_t PIT_CLK = MAIN_CLK / PIT_CLK_DIVIDER;

	static class DummyPort : public PortConnector
	{
	public:
		DummyPort() : Logger("DUMMY")
		{
			// Joystick
			Connect(0x201, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteData));
			Connect(0x201, static_cast<PortConnector::INFunction>(&DummyPort::ReadData));

			//EGA
			for (WORD w = 0x3C0; w < 0x3D0; ++w)
			{
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteData));
			}

			Connect(0x10, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteMfgTest));
			
			// PCjr sound
			Connect(0xC0, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteData));
		}

		BYTE ReadData()
		{
			return 0xFF;
		}

		void WriteData(BYTE value)
		{
		}

		void WriteMfgTest(BYTE value)
		{
			LogPrintf(LOG_ERROR, "MFG TEST: %02Xh", value);
		}
	} dummyPort;

	ComputerPCjr::ComputerPCjr() :
		Logger("PCjr"),
		Computer(m_memory, m_map),
		m_base64K("RAM0", 0x10000, emul::MemoryType::RAM),
		m_ext64K("RAM1", 0x10000, emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_pit(0x40, PIT_CLK),
		m_pic(0x20),
		m_ppi(0x60),
		m_video(0x3D0),
		m_keyboard(0xA0),
		m_floppy(0xF0, PIT_CLK),
		m_uart(0x2F8, UART_CLK),
		m_inputs(1193182)
	{
	}

	void ComputerPCjr::Init()
	{
		bool RAM128K = true;

		LogPrintf(LOG_INFO, "CPU Clock:  [%zu]", CPU_CLK);
		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(true, Logger::LOG_WARNING);
		m_mmap.EnableLog(true, Logger::LOG_ERROR);

		m_base64K.Clear(0xA5);
		m_memory.Allocate(&m_base64K, 0);
		m_memory.Allocate(RAM128K ? &m_ext64K : &m_base64K, 0x10000);

		m_pit.Init();
		m_pit.EnableLog(true, Logger::LOG_WARNING);

		m_pic.Init();
		m_pic.EnableLog(true, Logger::LOG_DEBUG);

		m_ppi.Init();
		m_ppi.EnableLog(true, Logger::LOG_WARNING);
		{
			m_ppi.SetKeyboardConnected(true);
			m_ppi.SetRAMExpansion(RAM128K);
			m_ppi.SetDisketteCard(true);
			m_ppi.SetModemCard(false);
		}

		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.EnableLog(true, Logger::LOG_WARNING);

		m_video.EnableLog(true, Logger::LOG_INFO);
		m_video.Init(&m_memory, "data/XT/CGA_CHAR.BIN");
		
		m_biosF000.LoadFromFile("data/PCjr/BIOS_4860_1504036_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadFromFile("data/PCjr/BIOS_4860_1504037_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		m_keyboard.Init(&m_ppi, &m_pic);
		m_keyboard.EnableLog(true, Logger::LOG_WARNING);

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

		m_floppy.Init();
		m_floppy.EnableLog(true, Logger::LOG_INFO);
		//m_floppy.LoadDiskImage(0, "data/floppy/PC-DOS-2.10d1.img");
		m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PCjr\Games\KQ1PCJR.IMG)");

		m_uart.Init();
		m_uart.EnableLog(true, Logger::LOG_WARNING);

		m_inputs.EnableLog(true, Logger::LOG_INFO);
		m_inputs.Init(&m_keyboard);

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_video);
		AddDevice(m_keyboard);
		AddDevice(m_floppy);
		AddDevice(m_uart);
		AddDevice(dummyPort);
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

		// TODO: Temporary, need dynamic connections
		// Timer 0: Time of day
		// Timer 1: 
		// Timer 2: Speaker
		static bool timer0Out = false;

		if (m_keyboard.NMIPending())
		{
			Interrupt(2);
			return true;
		}
		else if (m_pic.InterruptPending() && CanInterrupt())
		{
			m_pic.InterruptAcknowledge();
			Interrupt(m_pic.GetPendingInterrupt());

			LogPrintf(LOG_WARNING, "[%zu] Processing IRQ: %d", g_ticks, m_pic.GetPendingInterrupt()-8);
			return true;
		}
		else if (!CPU8086::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		assert(GetInstructionTicks());
		cpuTicks += GetInstructionTicks();

		for (int i = 0; i < cpuTicks / 4; ++i)
		{
			++g_ticks;

			m_inputs.Tick();
			if (m_inputs.IsQuit())
			{
				return false;
			}

			m_keyboard.Tick();

			m_pit.GetCounter(0).Tick();
			if (m_keyboard.GetTimer1Source() == kbd::CLK1::MAIN_CLK)
			{
				m_pit.GetCounter(1).Tick();
			}

			pit::Counter& timer2 = m_pit.GetCounter(2);
			timer2.SetGate(m_ppi.GetTimer2Gate());
			timer2.Tick();

			m_ppi.SetTimer2Output(timer2.GetOutput());

			bool out = m_pit.GetCounter(0).GetOutput();
			m_pic.InterruptRequest(0, out);
			if (out != timer0Out)
			{
				if (out && m_keyboard.GetTimer1Source() == kbd::CLK1::TIMER0_OUT)
				{
					m_pit.GetCounter(1).Tick();
				}
				timer0Out = out;
			}

			m_pcSpeaker.Tick();

			m_floppy.Tick();
			m_pic.InterruptRequest(6, m_floppy.IsWatchdogInterrupt());

			// Skip one in four video ticks to sync up with pit timing
			if ((syncTicks & 3) != 3)
			{
				static bool wasVSync = false;
				m_video.Tick();
				m_pic.InterruptRequest(5, (/*!wasVSync && */m_video.IsVSync()));
				if (!wasVSync && m_video.IsVSync())
				{
					LogPrintf(LOG_WARNING, "VSYNC 0->1");
				}
				else if (wasVSync && !m_video.IsVSync())
				{
					LogPrintf(LOG_WARNING, "VSYNC 1->0");
				}
				wasVSync = m_video.IsVSync();
			}

			m_uart.Tick();
			// UART clock is 1.5x base clock
			if (syncTicks & 1)
			{
				m_uart.Tick();
			}
			m_pic.InterruptRequest(3, m_uart.IsInterrupt());

			++syncTicks;
			// Every 11932 ticks (~10ms) make an adjustment
			if (syncTicks >= 11931)
			{
				auto delta = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - lastTick);

				if (delta < std::chrono::microseconds(10000))
				{
					little_sleep(std::chrono::microseconds(10000)-delta);
				}

				syncTicks = 0;
				lastTick = std::chrono::high_resolution_clock::now();
			}
		}
		cpuTicks %= 4;

		return true;
	}
}
