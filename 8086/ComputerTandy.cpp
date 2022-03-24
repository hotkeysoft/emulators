#include "stdafx.h"

#include "ComputerTandy.h"
#include "Config.h"
#include "Hardware/Device8255Tandy.h"
#include "Storage/DeviceHardDrive.h"
#include "Storage/DeviceFloppyTandy.h"
#include "Video/VideoTandy.h"

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

	static const BYTE IRQ_FLOPPY = 6;
	static const BYTE DMA_FLOPPY = 2;

	static const BYTE IRQ_HDD = 2;
	static const BYTE DMA_HDD = 3;

	static const BYTE IRQ_VSYNC = 5;

	ComputerTandy::ComputerTandy() :
		Logger("Tandy"),
		Computer(m_memory, m_map),
		m_base128K("SYSRAM", 0x20000, emul::MemoryType::RAM),
		m_ramExtension("EXTRAM", emul::MemoryType::RAM),
		m_biosFC00("BIOS", 0x4000, emul::MemoryType::ROM),
		m_soundModule(0xC0, SOUND_CLK)
	{
	}

	void ComputerTandy::Init(WORD baseRAM)
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
		InitPPI(new ppi::Device8255Tandy(0x60));
		InitDMA(new dma::Device8237(0x00, m_memory));
		InitSound();
		InitRTC();

		m_soundModule.EnableLog(CONFIG().GetLogLevel("sound.76489"));
		m_soundModule.Init();

		InitVideo("tga");

		m_biosFC00.LoadFromFile("data/Tandy/BIOS_Tandy1000A_FC00.BIN");
		m_memory.Allocate(&m_biosFC00, emul::S2A(0xFC00));

		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
		m_keyboard.Init(m_ppi, m_pic);

		//m_uart.EnableLog(CONFIG().GetLogLevel("uart"));
		//m_uart.Init();

		InitJoystick(0x201, PIT_CLK);

		InitInputs(PIT_CLK);
		GetInputs().InitKeyboard(&m_keyboard, events::KBDMapping::TANDY);
		GetInputs().InitJoystick(m_joystick);

		Connect(0xA0, static_cast<PortConnector::OUTFunction>(&ComputerTandy::SetRAMPage));

		if (CONFIG().GetValueBool("floppy", "enable"))
		{
			InitFloppy(new fdc::DeviceFloppyTandy(0x3F0, PIT_CLK), IRQ_FLOPPY, DMA_FLOPPY);
		}

		if (CONFIG().GetValueBool("hdd", "enable"))
		{
			InitHardDrive(new hdd::DeviceHardDrive(0x320, PIT_CLK), IRQ_HDD, DMA_HDD);
		}

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
		m_base128K.Clear();
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
			m_ramExtension.Clear();
			m_memory.Allocate(&m_ramExtension, 0);
		}
	}

	void ComputerTandy::SetRAMPage(BYTE value)
	{
		value >>= 1;
		value &= 0b111;

		ADDRESS ramBase = value * 0x20000;

		LogPrintf(LOG_WARNING, "Set RAM Page: [%d][%05Xh]", value, ramBase);
		SetRAMBase(ramBase);
	}

	void ComputerTandy::SetRAMBase(ADDRESS ramBase)
	{
		m_ramBase = ramBase;
		// Remove base ram and extension
		m_memory.Free(&m_base128K);

		if (m_ramExtension.GetSize())
		{
			m_memory.Free(&m_ramExtension);
			// Put memory extension at 0
			m_memory.Allocate(&m_ramExtension, 0);
		}

		// Put base/video mem on top at proper offset
		m_memory.Allocate(&m_base128K, m_ramBase);
		// Notify video module
		video::VideoTandy* video = (video::VideoTandy*)m_video;
		video->SetRAMBase(m_ramBase);
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

		if (m_pic->InterruptPending() && GetCPU().CanInterrupt())
		{
			m_pic->InterruptAcknowledge();
			GetCPU().Interrupt(m_pic->GetPendingInterrupt());
			return true;
		}
		else if (!Computer::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		assert(GetCPU().GetInstructionTicks());
		cpuTicks += GetCPU().GetInstructionTicks();

		ppi::Device8255Tandy* ppi = (ppi::Device8255Tandy*)m_ppi;
		video::VideoTandy* video = (video::VideoTandy*)m_video;

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

			pit::Counter& timer2 = m_pit->GetCounter(2);
			timer2.SetGate(ppi->GetTimer2Gate());
			timer2.Tick();

			ppi->SetTimer2Output(timer2.GetOutput());

			m_pic->InterruptRequest(0, m_pit->GetCounter(0).GetOutput());

			// SN76489 clock is 3x base clock
			m_soundModule.Tick(); m_soundModule.Tick(); m_soundModule.Tick();

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			if (!m_turbo) m_pcSpeaker.Tick(m_soundModule.GetOutput());

			m_dma->Tick();

			TickFloppy();
			TickHardDrive();

			// Skip one in four video ticks to sync up with pit timing
			if (syncTicks & 1)
			{
				video->Tick();
				m_pic->InterruptRequest(IRQ_VSYNC, (video->IsVSync()));
			}
			video->Tick();
			m_pic->InterruptRequest(IRQ_VSYNC, (video->IsVSync()));

			++syncTicks;
		}
		cpuTicks %= GetCPUSpeedRatio();

		return true;
	}

	void ComputerTandy::Serialize(json& to)
	{
		to["tandy"]["rambase"] = GetRAMBase();
		m_soundModule.Serialize(to["tandy"]["sn76489"]);
		Computer::Serialize(to);
	}

	void ComputerTandy::Deserialize(const json& from)
	{
		Computer::Deserialize(from);
		m_soundModule.Deserialize(from["tandy"]["sn76489"]);
		SetRAMBase(from["tandy"]["rambase"]);
	}

}
