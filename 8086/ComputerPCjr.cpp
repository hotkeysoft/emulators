#include "Common.h"
#include "ComputerPCjr.h"
#include "Config.h"

#include <thread>

using cfg::Config;

namespace emul
{
	static const size_t MAIN_CLK = 14318180;

	static const size_t CPU_CLK_DIVIDER = 3;
	static const size_t UART_CLK_DIVIDER = 8;
	static const size_t PIT_CLK_DIVIDER = 12;
	static const size_t SOUND_CLK_DIVIDER = 4;

	static const size_t CPU_CLK = MAIN_CLK / CPU_CLK_DIVIDER;
	static const size_t UART_CLK = MAIN_CLK / UART_CLK_DIVIDER;
	static const size_t PIT_CLK = MAIN_CLK / PIT_CLK_DIVIDER;
	static const size_t SOUND_CLK = MAIN_CLK / SOUND_CLK_DIVIDER;

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
		m_ext64K("RAM1", emul::MemoryType::RAM),
		m_extraRAM("EXTRAM", emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_pit(0x40, PIT_CLK),
		m_pic(0x20),
		m_ppi(0x60),
		m_video(0x3D0),
		m_keyboard(0xA0),
		m_floppy(0xF0, PIT_CLK),
		m_uart(0x2F8, UART_CLK),
		m_inputs(PIT_CLK),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerPCjr::Init(WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "CPU Clock:  [%zu]", CPU_CLK);
		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(Config::Instance().GetLogLevel("memory"));
		m_mmap.EnableLog(Config::Instance().GetLogLevel("mmap"));

		InitRAM(baseRAM);

		m_pit.EnableLog(Config::Instance().GetLogLevel("pit"));
		m_pit.Init();

		m_pic.EnableLog(Config::Instance().GetLogLevel("pic"));
		m_pic.Init();

		m_ppi.EnableLog(Config::Instance().GetLogLevel("ppi"));
		m_ppi.Init();
		{
			m_ppi.SetKeyboardConnected(true);
			m_ppi.SetRAMExpansion(baseRAM > 64);
			m_ppi.SetDisketteCard(true);
			m_ppi.SetModemCard(false);
		}

		m_pcSpeaker.EnableLog(Config::Instance().GetLogLevel("sound"));
		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.SetMute(false); // MUTE HERE

		m_soundModule.EnableLog(Config::Instance().GetLogLevel("sound.76489"));
		m_soundModule.Init();

		m_video.EnableLog(Config::Instance().GetLogLevel("video"));
		m_video.Init(&m_memory, "data/XT/CGA_CHAR.BIN");
		
		m_biosF000.LoadFromFile("data/PCjr/BIOS_4860_1504036_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadFromFile("data/PCjr/BIOS_4860_1504037_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		m_keyboard.EnableLog(Config::Instance().GetLogLevel("keyboard"));
		m_keyboard.Init(&m_ppi, &m_pic);

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

		m_floppy.EnableLog(Config::Instance().GetLogLevel("floppy"));
		m_floppy.Init();
		//m_floppy.LoadDiskImage(0, "data/floppy/PC-DOS-2.10d1.img");
		m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PCjr\Games\KQ1PCJR.IMG)");
		
		m_uart.EnableLog(Config::Instance().GetLogLevel("uart"));
		m_uart.Init();

		m_inputs.Init(&m_keyboard);
		m_inputs.EnableLog(Config::Instance().GetLogLevel("inputs"));

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_video);
		AddDevice(m_keyboard);
		AddDevice(m_floppy);
		AddDevice(m_uart);
		AddDevice(m_soundModule);
		AddDevice(dummyPort);
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
		m_base64K.Clear(0xA5);
		LogPrintf(LOG_INFO, "Allocating base 64KB block");
		m_memory.Allocate(&m_base64K, 0);

		// 64KB Memory extension
		if (baseRAM > 64)
		{
			m_ext64K.Alloc(0x10000);
			m_ext64K.Clear(0x5A);
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
			m_extraRAM.Clear(0xA5);
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
		else if (m_pic.InterruptPending() && CanInterrupt())
		{
			m_pic.InterruptAcknowledge();
			Interrupt(m_pic.GetPendingInterrupt());
			return true;
		}
		else if (!CPU8086::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		assert(GetInstructionTicks());
		cpuTicks += GetInstructionTicks();

		for (uint32_t i = 0; i < cpuTicks / 4; ++i)
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

			// SN76489 clock is 3x base clock
			m_soundModule.Tick(); m_soundModule.Tick(); m_soundModule.Tick();

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			m_pcSpeaker.Tick(m_soundModule.GetOutput());

			m_floppy.Tick();
			m_pic.InterruptRequest(6, m_floppy.IsWatchdogInterrupt());

			// Skip one in four video ticks to sync up with pit timing
			if ((syncTicks & 3) != 3)
			{
				m_video.Tick();
				m_pic.InterruptRequest(5, (m_video.IsVSync()));
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
			//if (!m_turbo && (syncTicks >= 11931))
			//{
			//	auto delta = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - lastTick);

			//	if (delta < std::chrono::microseconds(10000))
			//	{
			//		little_sleep(std::chrono::microseconds(10000)-delta);
			//	}

			//	syncTicks = 0;
			//	lastTick = std::chrono::high_resolution_clock::now();
			//}
		}
		cpuTicks %= 4;

		return true;
	}
}
