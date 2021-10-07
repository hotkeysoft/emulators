#include "Common.h"
#include "ComputerPCjr.h"
#include <thread>

namespace emul
{
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

			Connect(0x10, static_cast<PortConnector::OUTFunction>(&DummyPort::WriteMfgTest));
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
		CPU8086(m_memory, m_map),
		m_memory(emul::CPU8086_ADDRESS_BITS),
		m_base64K("RAM0", 0x10000, emul::MemoryType::RAM),
		m_biosF000("BIOS0", 0x8000, emul::MemoryType::ROM),
		m_biosF800("BIOS1", 0x8000, emul::MemoryType::ROM),
		m_pit(0x40, 1193182),
		m_pic(0x20),
		m_ppi(0x60),
		m_video(0x3D0)
	{
	}

	void ComputerPCjr::Init()
	{
		m_memory.EnableLog(true, Logger::LOG_INFO);
		m_mmap.EnableLog(true, Logger::LOG_ERROR);

		m_base64K.Clear(0xA5);
		m_memory.Allocate(&m_base64K, 0);
		m_memory.Allocate(&m_base64K, 0x10000);

		m_pit.Init();
		m_pit.EnableLog(true, Logger::LOG_INFO);

		m_pic.Init();
		m_pic.EnableLog(true, Logger::LOG_WARNING);

		m_ppi.Init();
		m_ppi.EnableLog(true, Logger::LOG_DEBUG);

		m_pcSpeaker.Init(&m_ppi, &m_pit);
		m_pcSpeaker.EnableLog(true, Logger::LOG_WARNING);

		m_video.EnableLog(true, Logger::LOG_WARNING);
		m_video.Init("data/XT/CGA_CHAR.BIN");
		//m_video.SetComposite(true);

		m_biosF000.LoadBinary("data/PCjr/BIOS_4860_1504036_F000.BIN");
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_biosF800.LoadBinary("data/PCjr/BIOS_4860_1504037_F800.BIN");
		m_memory.Allocate(&m_biosF800, emul::S2A(0xF800));

		AddDevice(m_pic);
		AddDevice(m_pit);
		AddDevice(m_ppi);
		AddDevice(m_video);
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
				//m_ppi.SetCurrentKeyCode(m_keyBuf[m_keyBufRead++]);
				//m_pic.InterruptRequest(1);
			}

			m_pit.Tick();
			bool out = m_pit.GetCounter(0).GetOutput();
			if (out != timer0Out)
			{
				if (out)
				{
					//m_pic.InterruptRequest(0);
				}
				timer0Out = out;
			}

			m_pcSpeaker.Tick();

			m_video.Tick();

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
