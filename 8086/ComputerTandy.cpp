#include "Common.h"
#include "ComputerTandy.h"
#include <thread>

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

	static class DummyPortTandy : public PortConnector
	{
	public:
		DummyPortTandy() : Logger("DUMMY")
		{
			// Joystick
			Connect(0x201, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			Connect(0x201, static_cast<PortConnector::INFunction>(&DummyPortTandy::ReadData));

			//EGA
			for (WORD w = 0x3C0; w < 0x3D0; ++w)
			{
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			}

			// Optional DMA Controller
			for (WORD w = 0; w < 0xF; ++w)
			{
				Connect(w, static_cast<PortConnector::INFunction>(&DummyPortTandy::ReadData));
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			}

			// Optional DMA Controller (Page Register)
			for (WORD w = 0x80; w < 0x84; ++w)
			{
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			}
		}

		BYTE ReadData()
		{
			return 0xFF;
		}

		void WriteData(BYTE value)
		{
		}
	} dummyPortTandy;

	ComputerTandy::ComputerTandy() :
		Logger("Tandy"),
		Computer(m_memory, m_map),
		m_base128K("RAM0", 0x20000, emul::MemoryType::RAM),
		m_biosFC00("BIOS", 0x4000, emul::MemoryType::ROM),
		m_pit(0x40, PIT_CLK),
		m_pic(0x20),
		m_ppi(0x60),
		m_video(0x3D0),
		m_floppy(0x3F0, PIT_CLK),
		m_uart(0x2F8, UART_CLK),
		m_inputs(PIT_CLK),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerTandy::Init()
	{
		bool RAM128K = true;

		LogPrintf(LOG_INFO, "CPU Clock:  [%zu]", CPU_CLK);
		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(true, Logger::LOG_WARNING);
		m_mmap.EnableLog(true, Logger::LOG_ERROR);

		m_base128K.Clear(0xA5);
		m_memory.Allocate(&m_base128K, 0);

		m_pit.Init();
		m_pit.EnableLog(true, Logger::LOG_INFO);

		m_pic.Init();
		m_pic.EnableLog(true, Logger::LOG_WARNING);

		m_ppi.Init();
		m_ppi.EnableLog(true, Logger::LOG_INFO);

		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.EnableLog(true, Logger::LOG_WARNING);

		m_soundModule.Init();
		m_soundModule.EnableLog(true, Logger::LOG_WARNING);

		m_video.EnableLog(true, Logger::LOG_INFO);
		m_video.Init(&m_memory, "data/XT/CGA_CHAR.BIN");
		
		m_biosFC00.LoadFromFile("data/Tandy/BIOS_Tandy1000A_FC00.BIN");
		m_memory.Allocate(&m_biosFC00, emul::S2A(0xFC00));

		m_keyboard.Init(&m_ppi, &m_pic);
		m_keyboard.EnableLog(true, Logger::LOG_WARNING);

		m_floppy.Init();
		m_floppy.EnableLog(true, Logger::LOG_INFO);
		m_floppy.LoadDiskImage(0, "data/floppy/TANDY-MS-DOS-2.11.22.img");
		m_floppy.LoadDiskImage(1, "data/floppy/TANDY-DESKMATE-1.01.00.img");
		
		m_uart.Init();
		m_uart.EnableLog(true, Logger::LOG_WARNING);

		m_inputs.EnableLog(true, Logger::LOG_WARNING);
		m_inputs.Init(&m_keyboard, events::KBDMapping::TANDY);

		Connect(0xA0, static_cast<PortConnector::OUTFunction>(&ComputerTandy::SetRAMPage));

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_video);
		AddDevice(m_floppy);
		//AddDevice(m_uart);
		AddDevice(m_soundModule);
		AddDevice(dummyPortTandy);
		AddDevice(*this);
	}

	void ComputerTandy::SetRAMPage(BYTE value)
	{
		value >>= 1;
		value &= 0b111;

		ADDRESS ramBase = value * 0x20000;

		LogPrintf(LOG_WARNING, "Set RAM Page: [%d][%05Xh]", value, ramBase);

		m_memory.EnableLog(true, LOG_INFO);
		m_memory.Free(&m_base128K);
		m_memory.Allocate(&m_base128K, ramBase);
		m_video.SetRAMBase(ramBase);
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

	bool ComputerTandy::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		static bool timer0Out = false;

		if (m_pic.InterruptPending() && CanInterrupt())
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

			pit::Counter& timer2 = m_pit.GetCounter(2);
			timer2.SetGate(m_ppi.GetTimer2Gate());
			timer2.Tick();

			m_ppi.SetTimer2Output(timer2.GetOutput());

			m_pic.InterruptRequest(0, m_pit.GetCounter(0).GetOutput());

			// SN76489 clock is 3x base clock
			m_soundModule.Tick(); m_soundModule.Tick(); m_soundModule.Tick();

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			m_pcSpeaker.Tick(m_soundModule.GetOutput());

			m_floppy.Tick();
			m_pic.InterruptRequest(6, m_floppy.IsInterruptPending());

			// Skip one in four video ticks to sync up with pit timing
			if ((syncTicks & 3) != 3)
			{
				m_video.Tick();
				m_pic.InterruptRequest(5, (m_video.IsVSync()));
			}

			//m_uart.Tick();
			//// UART clock is 1.5x base clock
			//if (syncTicks & 1)
			//{
			//	m_uart.Tick();
			//}
			//m_pic.InterruptRequest(3, m_uart.IsInterrupt());

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
