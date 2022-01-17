#include "Common.h"
#include "ComputerTandy.h"
#include "Config.h"
#include "Hardware/Device8255Tandy.h"
#include "Storage/DeviceHardDrive.h"
#include "Video/VideoTandy.h"

#include <thread>

using cfg::Config;

const BYTE IRQ_FLOPPY = 6;
const BYTE DMA_FLOPPY = 2;

const BYTE IRQ_HDD = 2;
const BYTE DMA_HDD = 3;

namespace emul
{
	static const size_t MAIN_CLK = 14318180;

	static const size_t UART_CLK_DIVIDER = 8;
	static const size_t PIT_CLK_DIVIDER = 12;
	static const size_t SOUND_CLK_DIVIDER = 4;

	static const size_t UART_CLK = MAIN_CLK / UART_CLK_DIVIDER;
	static const size_t PIT_CLK = MAIN_CLK / PIT_CLK_DIVIDER;
	static const size_t SOUND_CLK = MAIN_CLK / SOUND_CLK_DIVIDER;

	static class DummyPortTandy : public PortConnector
	{
	public:
		DummyPortTandy() : Logger("DUMMY")
		{
			//EGA
			for (WORD w = 0x3C0; w < 0x3D0; ++w)
			{
				Connect(w, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			}

			// MPU-401
			Connect(0x330, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			Connect(0x330, static_cast<PortConnector::INFunction>(&DummyPortTandy::ReadData));
			Connect(0x331, static_cast<PortConnector::OUTFunction>(&DummyPortTandy::WriteData));
			Connect(0x331, static_cast<PortConnector::INFunction>(&DummyPortTandy::ReadData));
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
		m_base128K("SYSRAM", 0x20000, emul::MemoryType::RAM),
		m_ramExtension("EXTRAM", emul::MemoryType::RAM),
		m_biosFC00("BIOS", 0x4000, emul::MemoryType::ROM),
		m_dma(0x00, m_memory),
		m_floppy(0x3F0, PIT_CLK),
		m_uart(0x2F8, UART_CLK),
		m_inputs(PIT_CLK),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerTandy::Init(WORD baseRAM)
	{
		AddCPUSpeed(CPUSpeed(PIT_CLK, 4));
		AddCPUSpeed(CPUSpeed(PIT_CLK, 8));
		AddCPUSpeed(CPUSpeed(PIT_CLK, 12));

		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(Config::Instance().GetLogLevel("memory"));
		m_mmap.EnableLog(Config::Instance().GetLogLevel("mmap"));

		InitRAM(baseRAM);
		InitPIT(new pit::Device8254(0x40, PIT_CLK));
		InitPIC(new pic::Device8259(0x20));
		InitPPI(new ppi::Device8255Tandy(0x60));
		InitSound();

		m_soundModule.EnableLog(Config::Instance().GetLogLevel("sound.76489"));
		m_soundModule.Init();

		InitVideo("tga");
		
		m_dma.EnableLog(Config::Instance().GetLogLevel("dma"));
		m_dma.Init();

		m_biosFC00.LoadFromFile("data/Tandy/BIOS_Tandy1000A_FC00.BIN");
		m_memory.Allocate(&m_biosFC00, emul::S2A(0xFC00));

		m_keyboard.EnableLog(Config::Instance().GetLogLevel("keyboard"));
		m_keyboard.Init(m_ppi, m_pic);

		m_floppy.EnableLog(Config::Instance().GetLogLevel("floppy"));
		m_floppy.Init();

		std::string floppy = Config::Instance().GetValueStr("floppy", "floppy.1");
		if (floppy.size())
		{ 
			m_floppy.LoadDiskImage(0, floppy.c_str());
		}

		floppy = Config::Instance().GetValueStr("floppy", "floppy.2");
		if (floppy.size())
		{
			m_floppy.LoadDiskImage(1, floppy.c_str());
		}
		
		m_uart.EnableLog(Config::Instance().GetLogLevel("uart"));
		m_uart.Init();

		InitJoystick(0x201, PIT_CLK);

		m_inputs.EnableLog(Config::Instance().GetLogLevel("inputs"));
		m_inputs.InitKeyboard(&m_keyboard, events::KBDMapping::TANDY);
		m_inputs.InitJoystick(m_joystick);

		Connect(0xA0, static_cast<PortConnector::OUTFunction>(&ComputerTandy::SetRAMPage));

		InitHardDrive(new hdd::DeviceHardDrive(0x320, PIT_CLK));

		AddDevice(*m_pic);
		AddDevice(*m_pit);
		AddDevice(*m_ppi);
		AddDevice(m_dma);
		AddDevice(*m_video);
		AddDevice(m_floppy);
		AddDevice(*m_hardDrive);
		//AddDevice(m_uart);
		AddDevice(m_soundModule);
		AddDevice(*m_joystick);
		AddDevice(dummyPortTandy);
		AddDevice(*this);
	}

	void ComputerTandy::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 128)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 128KB", baseRAM);
			baseRAM = 128;
		}

		// 128KB Base memory
		m_base128K.Clear(0x5A);
		LogPrintf(LOG_INFO, "Allocating base 128KB block");
		// Memory block is moved to the appropriate location via SetRAMPage, no need to allocate it here

		// Extra RAM
		if (baseRAM > 128)
		{
			if (baseRAM > 768)
			{
				baseRAM = 768;
				LogPrintf(LOG_WARNING, "Setting maximum memory size to 768KB");
			}
			DWORD extraRAM = (baseRAM - 128);
			LogPrintf(LOG_INFO, "Allocating %dKB extra RAM", extraRAM);

			m_ramExtension.Alloc(extraRAM * 1024);
			m_ramExtension.Clear(0xA5);
			m_memory.Allocate(&m_ramExtension, 0);
		}
	}

	void ComputerTandy::SetRAMPage(BYTE value)
	{
		value >>= 1;
		value &= 0b111;

		ADDRESS ramBase = value * 0x20000;

		LogPrintf(LOG_WARNING, "Set RAM Page: [%d][%05Xh]", value, ramBase);

		// Remove base ram and extension
		m_memory.Free(&m_base128K);

		if (m_ramExtension.GetSize())
		{
			m_memory.Free(&m_ramExtension);
			// Put memory extension at 0
			m_memory.Allocate(&m_ramExtension, 0);
		}

		// Put base/video mem on top at proper offset
		m_memory.Allocate(&m_base128K, ramBase);
		// Notify video module
		video::VideoTandy* video = (video::VideoTandy*)m_video;
		video->SetRAMBase(ramBase);
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

		if (m_pic->InterruptPending() && CanInterrupt())
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

		ppi::Device8255Tandy* ppi = (ppi::Device8255Tandy*)m_ppi;
		video::VideoTandy* video = (video::VideoTandy*)m_video;

		for (uint32_t i = 0; i < cpuTicks / GetCPUSpeedRatio(); ++i)
		{
			++g_ticks;

			m_inputs.Tick();
			if (m_inputs.IsQuit())
			{
				return false;
			}

			m_keyboard.Tick();
			m_joystick->Tick();

			m_pit->GetCounter(0).Tick();

			pit::Counter& timer2 = m_pit->GetCounter(2);
			timer2.SetGate(ppi->GetTimer2Gate());
			timer2.Tick();

			ppi->SetTimer2Output(timer2.GetOutput());

			m_pic->InterruptRequest(0, m_pit->GetCounter(0).GetOutput());

			// SN76489 clock is 3x base clock
			m_soundModule.Tick(); m_soundModule.Tick(); m_soundModule.Tick();

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			if (!m_turbo) m_pcSpeaker.Tick(m_soundModule.GetOutput());

			m_dma.Tick();

			m_floppy.Tick();
			m_pic->InterruptRequest(IRQ_FLOPPY, m_floppy.IsInterruptPending());

			// TODO: duplication with HDD
			if (m_floppy.IsDMAPending())
			{
				m_dma.DMARequest(DMA_FLOPPY, true);
			}

			if (m_dma.DMAAcknowledged(DMA_FLOPPY))
			{
				m_dma.DMARequest(DMA_FLOPPY, false);

				// Do it manually
				m_floppy.DMAAcknowledge();

				dma::DMAChannel& channel = m_dma.GetChannel(DMA_FLOPPY);
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
				case dma::OPERATION::VERIFY:
					channel.DMAOperation(value);
					break;
				default:
					throw std::exception("DMAOperation: Operation not supported");
				}

				if (m_dma.GetTerminalCount(DMA_FLOPPY))
				{
					m_floppy.DMATerminalCount();
				}
			}
			m_hardDrive->Tick();
			m_pic->InterruptRequest(IRQ_HDD, m_hardDrive->IsInterruptPending());

			if (m_hardDrive->IsDMAPending())
			{
				m_dma.DMARequest(DMA_HDD, true);
			}

			if (m_dma.DMAAcknowledged(DMA_HDD))
			{
				m_dma.DMARequest(DMA_HDD, false);

				// Do it manually
				m_hardDrive->DMAAcknowledge();

				dma::DMAChannel& channel = m_dma.GetChannel(DMA_HDD);
				dma::OPERATION op = channel.GetOperation();
				BYTE value;
				switch (op)
				{
				case dma::OPERATION::READ:
					channel.DMAOperation(value);
					m_hardDrive->WriteDataFIFO(value);
					break;
				case dma::OPERATION::WRITE:
					value = m_hardDrive->ReadDataFIFO();
					channel.DMAOperation(value);
					break;
				case dma::OPERATION::VERIFY:
					channel.DMAOperation(value);
					break;
				default:
					throw std::exception("DMAOperation: Operation not supported");
				}

				if (m_dma.GetTerminalCount(DMA_HDD))
				{
					m_hardDrive->DMATerminalCount();
				}
			}

			// Skip one in four video ticks to sync up with pit timing
			if ((syncTicks & 3) != 3)
			{
				video->Tick();
				m_pic->InterruptRequest(5, (video->IsVSync()));
			}

			//m_uart.Tick();
			//// UART clock is 1.5x base clock
			//if (syncTicks & 1)
			//{
			//	m_uart.Tick();
			//}
			//m_pic.InterruptRequest(3, m_uart.IsInterrupt());

			++syncTicks;
		}
		cpuTicks %= GetCPUSpeedRatio();

		return true;
	}
}
