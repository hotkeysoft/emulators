#include "Common.h"
#include "ComputerXT.h"
#include "Console.h"
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

	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	enum class VIDEO { CGA, MDA };
	static const VIDEO s_video = VIDEO::MDA;

	static class DummyPort : public PortConnector
	{
	public:
		DummyPort() : Logger("DUMMY")
		{
			// Joystick
			Connect(0x201, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteData));

			//EGA
			for (WORD w = 0x3C0; w < 0x3D0; ++w)
			{
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteData));
			}
		}

		void WriteData(BYTE value)
		{
		}
	} dummyPort;

	ComputerXT::ComputerXT() :
		Logger("XT"),
		Computer(m_memory, m_map),
		m_base64K("RAM0", 0xA0000, emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_pit(0x40, 1193182),
		m_pic(0x20),
		m_ppi(0x60),
		m_dma(0x00, m_memory),
		m_videoMDA(0x3B0),
		m_videoCGA(0x3D0),
		m_floppy(0x03F0, PIT_CLK),
		m_inputs(PIT_CLK),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerXT::Init()
	{
		m_memory.EnableLog(true, Logger::LOG_ERROR);
		m_mmap.EnableLog(true, Logger::LOG_WARNING);

		m_base64K.Clear(0xA5);
		m_memory.Allocate(&m_base64K, 0);

		m_pit.Init();
		m_pit.EnableLog(true, Logger::LOG_WARNING);
		//m_pit.GetCounter(2).EnableLog(true, LOG_INFO);
		//m_pit.GetCounter(0).EnableLog(true, LOG_INFO);

		m_pic.Init();
		m_pic.EnableLog(true, Logger::LOG_WARNING);

		m_ppi.Init();
		m_ppi.EnableLog(true, Logger::LOG_WARNING);

		// Configuration switches
		{
			m_ppi.SetPOSTLoop(false);
			m_ppi.SetMathCoprocessor(false);
			m_ppi.SetRAMConfig(ppi::RAMSIZE::RAM_256K);
			if (s_video == VIDEO::CGA)
			{
				m_ppi.SetDisplayConfig(screenWidth == COLS80 ? ppi::DISPLAY::COLOR_80x25 : ppi::DISPLAY::COLOR_40x25);
			}
			else
			{
				m_ppi.SetDisplayConfig(ppi::DISPLAY::MONO_80x25);
			}
			m_ppi.SetFloppyCount(2);
		}

		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.EnableLog(true, Logger::LOG_WARNING);
		
		m_pcSpeaker.SetMute(true); // MUTE HERE
		//m_pcSpeaker.StreamToFile(true, "dump/audio.bin");

		m_soundModule.Init();
		m_soundModule.EnableLog(true, Logger::LOG_INFO);

		m_dma.Init();
		m_dma.EnableLog(false);
		m_dma.EnableLog(true, Logger::LOG_WARNING);

		// TODO: Clean this, have one active video object instead of if/else
		if (s_video == VIDEO::CGA)
		{
			m_videoCGA.EnableLog(true, Logger::LOG_WARNING);
			m_videoCGA.Init("data/XT/CGA_CHAR.BIN");
			//m_video.SetComposite(true);

			m_memory.Allocate(&m_videoCGA.GetVideoRAM(), emul::S2A(0xB800));
			m_memory.Allocate(&m_videoCGA.GetVideoRAM(), emul::S2A(0xBC00));
		}
		else if (s_video == VIDEO::MDA)
		{
			m_videoMDA.EnableLog(true, Logger::LOG_INFO);
			m_videoMDA.Init("data/XT/CGA_CHAR.BIN");

			for (int i = 0; i < 8; ++i)
			{
				m_memory.Allocate(&m_videoMDA.GetVideoRAM(), emul::S2A(0xB000 + (i * 0x100)));
			}
		}

		m_biosF000.LoadFromFile("data/XT/BIOS_5160_V3_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadFromFile("data/XT/BIOS_5160_V3_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		m_keyboard.Init(&m_ppi, &m_pic);

		m_floppy.Init();
		m_floppy.EnableLog(true, Logger::LOG_WARNING);
		//m_floppy.LoadDiskImage(0, "data/floppy/PC-DOS-1.10.img");
		//m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PC\boot games\img\000310_montezumas_revenge\disk1.img)");
		// 
		m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PC\Dos3.3.img)");
		m_floppy.LoadDiskImage(1, R"(P:\floppy\kq1.img)");

		m_inputs.EnableLog(true, Logger::LOG_WARNING);
		m_inputs.Init(&m_keyboard);

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_dma);
		if (s_video == VIDEO::CGA)
		{
			AddDevice(m_videoCGA);
		}
		else 
		{
			AddDevice(m_videoMDA);
		}
		AddDevice(m_floppy);
		AddDevice(m_soundModule);
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

	bool ComputerXT::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		if (m_pic.InterruptPending() && CanInterrupt())
		{
			m_pic.InterruptAcknowledge();
			LogPrintf(LOG_DEBUG, "IRQ %d", m_pic.GetPendingInterrupt()-8);
			Interrupt(m_pic.GetPendingInterrupt());
			return true;
		}
		else if (m_soundModule.IsReady())
		{
			if (!CPU8086::Step())
			{
				return false;
			}
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

			pit::Counter& timer2 = m_pit.GetCounter(2);
			timer2.SetGate(m_ppi.GetTimer2Gate());

			m_pit.Tick();
			m_ppi.SetTimer2Output(timer2.GetOutput());

			m_pic.InterruptRequest(0, m_pit.GetCounter(0).GetOutput());

			// SN76489 clock is 3x base clock
			m_soundModule.Tick(); m_soundModule.Tick(); m_soundModule.Tick();

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			if (!m_turbo) m_pcSpeaker.Tick(m_soundModule.GetOutput());

			m_dma.Tick();
			if (s_video == VIDEO::CGA)
			{
				m_videoCGA.Tick();
			}
			else
			{
				m_videoMDA.Tick();
			}
			m_floppy.Tick();
			m_pic.InterruptRequest(6, m_floppy.IsInterruptPending());

			// TODO: faking it
			if (m_floppy.IsDMAPending())
			{
				m_dma.DMARequest(2, true);
			}

			if (m_dma.DMAAcknowledged(2))
			{
				m_dma.DMARequest(2, false);

				// Do it manually
				m_floppy.DMAAcknowledge();

				dma::DMAChannel& channel = m_dma.GetChannel(2);
				dma::OPERATION op = channel.GetOperation();
				BYTE value;
				switch (op)
				{
				case dma::OPERATION::READ:
					channel.DMAOperation(value);
					m_floppy.WriteDataFIFO(value);
					break;
				case dma::OPERATION::WRITE:
					value = m_floppy.ReadDataFIFO();
					channel.DMAOperation(value);
					break;
				default:
					throw std::exception("DMAOperation: Operation not supported");
				}

				if (m_dma.GetTerminalCount(2))
				{
					m_floppy.DMATerminalCount();
				}
			}

			//++syncTicks;
			//// Every 11932 ticks (~10ms) make an adjustment
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
