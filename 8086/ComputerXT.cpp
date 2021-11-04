#include "Common.h"
#include "ComputerXT.h"
#include "Config.h"
#include "Console.h"
#include "VideoCGA.h"
#include "VideoMDA.h"
#include "VideoHGC.h"

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

	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	enum class VIDEO { CGA, MDA, HGC };
	static const VIDEO s_video = VIDEO::CGA;

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
		m_baseRAM("RAM", emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_pit(0x40, 1193182),
		m_pic(0x20),
		m_ppi(0x60),
		m_dma(0x00, m_memory),
		m_floppy(0x03F0, PIT_CLK),
		m_inputs(PIT_CLK),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerXT::Init(WORD baseRAM)
	{
		m_memory.EnableLog(Config::Instance().GetLogLevel("memory"));
		m_mmap.EnableLog(Config::Instance().GetLogLevel("mmap"));

		InitRAM(baseRAM);

		m_pit.EnableLog(Config::Instance().GetLogLevel("pit"));
		m_pit.Init();

		m_pic.EnableLog(Config::Instance().GetLogLevel("pic"));
		m_pic.Init();

		m_ppi.EnableLog(Config::Instance().GetLogLevel("ppi"));
		m_ppi.Init();

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

		m_pcSpeaker.EnableLog(Config::Instance().GetLogLevel("sound"));
		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.SetMute(false); // MUTE HERE
		//m_pcSpeaker.StreamToFile(true, "dump/audio.bin");

		m_soundModule.EnableLog(Config::Instance().GetLogLevel("sound.76489"));
		m_soundModule.Init();

		m_dma.EnableLog(Config::Instance().GetLogLevel("dma"));
		m_dma.Init();

		// TODO: Clean this, have one active video object instead of if/else
		if (s_video == VIDEO::CGA)
		{
			video::VideoCGA* cga = new video::VideoCGA(0x3D0);
			m_video = cga;
			m_video->EnableLog(Config::Instance().GetLogLevel("video"));
			//cga->SetComposite(true);
			cga->Init(m_memory, "data/XT/CGA_CHAR.BIN");

		}
		else if (s_video == VIDEO::MDA)
		{
			video::VideoMDA* mda = new video::VideoMDA(0x3B0);
			m_video = mda;
			m_video->EnableLog(Config::Instance().GetLogLevel("video"));
			mda->Init(m_memory, "data/XT/CGA_CHAR.BIN");
		}
		else if (s_video == VIDEO::HGC)
		{
			video::VideoHGC* hgc = new video::VideoHGC(0x3B0);
			m_video = hgc;
			m_video->EnableLog(Config::Instance().GetLogLevel("video"));
			hgc->Init(m_memory, "data/XT/CGA_CHAR.BIN");
		}
		else
		{
			throw std::exception("Invalid mode");
		}
		assert(m_video);

		m_biosF000.LoadFromFile("data/XT/BIOS_5160_V3_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadFromFile("data/XT/BIOS_5160_V3_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		m_keyboard.EnableLog(Config::Instance().GetLogLevel("keyboard"));
		m_keyboard.Init(&m_ppi, &m_pic);

		m_floppy.EnableLog(Config::Instance().GetLogLevel("floppy"));
		m_floppy.Init();
		//m_floppy.LoadDiskImage(0, "data/floppy/PC-DOS-1.10.img");
		//m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PC\boot games\img\000310_montezumas_revenge\disk1.img)");
		// 
		m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PC\Dos3.3.img)");
		m_floppy.LoadDiskImage(1, R"(P:\floppy\kq1.img)");

		m_inputs.EnableLog(Config::Instance().GetLogLevel("inputs"));
		m_inputs.Init(&m_keyboard);

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_dma);
		AddDevice(*m_video);
		AddDevice(m_floppy);
		AddDevice(m_soundModule);
		AddDevice(dummyPort);
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
		m_baseRAM.Clear(0xA5);
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

		for (uint32_t i = 0; i < cpuTicks / 10; ++i)
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
			m_video->Tick();
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
		cpuTicks %= 10;

		return true;
	}
}
