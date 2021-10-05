#include "Common.h"
#include "Computer.h"
#include "Console.h"
#include <thread>

namespace emul
{
	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	class DummyPort : public PortConnector
	{
	public:
		DummyPort() : Logger("DUMMY")
		{
			//SN746496 programmable tone/noise generator PCjr
			Connect(0xC0, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteData));

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

	Computer::Computer() : 
		m_memory(emul::CPU8086_ADDRESS_BITS),
		m_base64K("RAM0", 0x80000, emul::MemoryType::RAM),
		Logger("PC"), 
		CPU8086(m_memory, m_map),
		m_pit(0x40, 1193182),
		m_pic(0x20),
		m_ppi(0x60),
		m_dma(0x00, m_memory),
		m_cga(0x3D0),
		m_floppy(0x03F0, 1193182)
	{
	}

	void Computer::Init()
	{
		m_memory.EnableLog(true, Logger::LOG_ERROR);
		m_mmap.EnableLog(true, Logger::LOG_ERROR);

		m_base64K.Clear(0xA5);
		m_memory.Allocate(&m_base64K, 0);

		m_pit.Init();
		m_pit.EnableLog(true, Logger::LOG_WARNING);

		m_pic.Init();
		m_pic.EnableLog(true, Logger::LOG_WARNING);

		m_ppi.Init();
		m_ppi.EnableLog(true, Logger::LOG_WARNING);

		// Configuration switches
		{
			m_ppi.SetPOSTLoop(false);
			m_ppi.SetMathCoprocessor(false);
			m_ppi.SetRAMConfig(ppi::RAMSIZE::RAM_256K);
			m_ppi.SetDisplayConfig(screenWidth == COLS80 ? ppi::DISPLAY::COLOR_80x25 : ppi::DISPLAY::COLOR_40x25);
			m_ppi.SetFloppyCount(2);
		}

		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.EnableLog(true, Logger::LOG_WARNING);

		m_dma.Init();
		m_dma.EnableLog(false);
		m_dma.EnableLog(true, Logger::LOG_INFO);

		m_cga.EnableLog(true, Logger::LOG_WARNING);
		m_cga.Init("data/CGA_CHAR.BIN");
		//m_cga.SetComposite(true);

		m_memory.Allocate(&m_cga.GetVideoRAM(), emul::S2A(0xB800));
		m_memory.Allocate(&m_cga.GetVideoRAM(), emul::S2A(0xBC00));

		m_floppy.Init();
		m_floppy.EnableLog(true, Logger::LOG_INFO);
		m_floppy.LoadDiskImage(0, "data/PC-DOS-1.10.img");
		//m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PC\boot games\img\000310_montezumas_revenge\disk1.img)");
		// 
		//m_floppy.LoadDiskImage(0, R"(D:\Dloads\Emulation\PC\Dos3.3.img)");
		//m_floppy.LoadDiskImage(1, R"(P:\floppy\kq1.img)");

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_dma);
		AddDevice(m_cga);
		AddDevice(m_floppy);
		AddDevice(dummyPort);
	}

	void little_sleep(std::chrono::microseconds us)
	{
		auto start = std::chrono::high_resolution_clock::now();
		auto end = start + us;
		do 
		{
			std::this_thread::yield();
		} while (std::chrono::high_resolution_clock::now() < end);
	}

	bool Computer::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		++m_ticks; // TODO: Coordinate with CPU

		// TODO: Temporary, need dynamic connections
		// Timer 0: Time of day
		// Timer 1: DMA RAM refresh
		// Timer 2: Speaker
		static bool timer0Out = false;

		if (m_pic.InterruptPending() && CanInterrupt())
		{
			m_pic.InterruptAcknowledge();
			Interrupt(m_pic.GetPendingInterrupt());
		}
		else if (!CPU8086::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetInstructionTicks();
		if (cpuTicks == 0)
		{
			LogPrintf(LOG_ERROR, "Operand %02Xh has no timing info", m_lastOp);
			throw std::exception("op with no timing");
		}

		for (int i = 0; i < cpuTicks / 4; ++i)
		{
			static size_t m_lastKbd = 0;
			if (m_keyBufRead != m_keyBufWrite && (m_ticks-m_lastKbd) > 10000)
			{
				m_lastKbd = m_ticks;
				m_ppi.SetCurrentKeyCode(m_keyBuf[m_keyBufRead++]);
				m_pic.InterruptRequest(1);
			}

			m_pit.Tick();
			bool out = m_pit.GetCounter(0).GetOutput();
			if (out != timer0Out)
			{
				if (out)
				{
					m_pic.InterruptRequest(0);
				}
				timer0Out = out;
			}

			m_pcSpeaker.Tick();

			if (m_floppy.IsInterruptPending() && CanInterrupt())
			{
				m_pic.InterruptRequest(6); // TEMP
				m_floppy.ClearInterrupt();
			}

			m_dma.Tick();
			m_cga.Tick();
			m_floppy.Tick();

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

		return true;
	}
}
