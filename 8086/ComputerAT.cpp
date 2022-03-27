#include "stdafx.h"

#include "ComputerAT.h"
#include "Config.h"
#include "IO/Console.h"
#include "Hardware/Device8255XT.h"
#include "Storage/DeviceFloppyXT.h"
#include "Storage/DeviceHardDrive.h"

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

	enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
	const SCREENWIDTH screenWidth = COLS80;

	static const BYTE IRQ_FLOPPY = 6;
	static const BYTE DMA_FLOPPY = 2;

	static const BYTE IRQ_HDD = 5;
	static const BYTE DMA_HDD = 3;

	static const BYTE IRQ_PIC2 = 2;

	ComputerAT::ComputerAT() :
		Logger("AT"),
		Computer(m_memory),
		m_baseRAM("RAM", emul::MemoryType::RAM),
		m_biosF000("BIOS", 0x10000, emul::MemoryType::ROM),
		m_picSecondary("pic2", 0xA0, false),
		m_rtc(0x70),
		m_dmaSecondary("dma2", 0xC0, m_memory),
		m_post(0x80)
	{
	}

	void ComputerAT::Reset()
	{
		Computer::Reset();
		m_nmiEnabled = true;
		m_picSecondary.Reset();
	}

	void ComputerAT::Init(WORD baseRAM)
	{
		Computer::Init(CPUType::i80286, baseRAM);

		AddCPUSpeed(CPUSpeed(PIT_CLK, 4));

		LogPrintf(LOG_INFO, "PIT Clock:  [%zu]", PIT_CLK);
		LogPrintf(LOG_INFO, "UART Clock: [%zu]", UART_CLK);

		m_memory.EnableLog(CONFIG().GetLogLevel("memory"));

		InitRAM(baseRAM);
		InitPIT(new pit::Device8254(0x40, PIT_CLK));
		InitPIC(new pic::Device8259(0x20));
		InitPPI(new ppi::Device8255XT(0x60));
		InitDMA(new dma::Device8237(0x00, m_memory));
		InitSound();

		m_picSecondary.EnableLog(CONFIG().GetLogLevel("pic"));
		m_picSecondary.Init();
		m_pic->AttachSecondaryDevice(IRQ_PIC2, &m_picSecondary);

		// TODO: Page registers

		// Secondary DMA Controller works on 16 bit transfers:
		// All bits are shifted to the left (including port addresses)
		m_dmaSecondary.EnableLog(CONFIG().GetLogLevel("dma"));
		m_dmaSecondary.Init(1);

		m_rtc.EnableLog(CONFIG().GetLogLevel("rtc"));
		m_rtc.Init();

		Connect(0x70, PortConnector::OUTFunction(&ComputerAT::WriteNMIMask), true);

		InitVideo("cga", { "cga", "ega", "vga" });

		m_biosF000.LoadOddEven("data/AT/BIOS_5170_V3_F000_ODD.BIN", OddEven::ODD);
		m_biosF000.LoadOddEven("data/AT/BIOS_5170_V3_F000_EVEN.BIN", OddEven::EVEN);
		m_memory.Allocate(&m_biosF000, emul::S2A(0xF000));

		m_keyboard.EnableLog(CONFIG().GetLogLevel("keyboard"));
		m_keyboard.Init(m_ppi, m_pic);

		m_post.Init();

		InitJoystick(0x201, PIT_CLK);

		InitInputs(PIT_CLK);
		GetInputs().InitKeyboard(&m_keyboard);
		GetInputs().InitJoystick(m_joystick);

		int floppyCount = 0;
		if (CONFIG().GetValueBool("floppy", "enable"))
		{
			InitFloppy(new fdc::DeviceFloppyXT(0x03F0, PIT_CLK), IRQ_FLOPPY, DMA_FLOPPY);
			floppyCount = 2;
		}

		if (CONFIG().GetValueBool("hdd", "enable"))
		{
			InitHardDrive(new hdd::DeviceHardDrive(0x320, PIT_CLK), IRQ_HDD, DMA_HDD);
		}

		// Configuration switches
		//{
		//	ppi::Device8255XT* ppi = (ppi::Device8255XT*)m_ppi;
		//	ppi->SetPOSTLoop(false);
		//	ppi->SetMathCoprocessor(false);
		//	ppi->SetRAMConfig(ppi::RAMSIZE::RAM_256K);
		//	if (m_video->IsMonoAdapter())
		//	{
		//		ppi->SetDisplayConfig(ppi::DISPLAY::MONO_80x25);
		//	}
		//	else if (m_video->GetID() == "ega")
		//	{
		//		ppi->SetDisplayConfig(ppi::DISPLAY::NONE);
		//	}
		//	else
		//	{
		//		ppi->SetDisplayConfig(screenWidth == COLS80 ? ppi::DISPLAY::COLOR_80x25 : ppi::DISPLAY::COLOR_40x25);
		//	}
		//	ppi->SetFloppyCount(floppyCount);
		//}
	}

	void ComputerAT::InitRAM(emul::WORD baseRAM)
	{
		LogPrintf(LOG_INFO, "Requested base RAM: %dKB", baseRAM);

		if (baseRAM < 512)
		{
			LogPrintf(LOG_WARNING, "Requested base RAM too low (%dKB), using 64KB", baseRAM);
			baseRAM = 64;
		}
		else if (baseRAM > 640)
		{
			baseRAM = 640;
			LogPrintf(LOG_WARNING, "Setting maximum memory size to 768KB");
		}

		m_baseRAM.Alloc(baseRAM * 1024);
		m_baseRAM.Clear();
		m_memory.Allocate(&m_baseRAM, 0);
	}

	void ComputerAT::WriteNMIMask(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteNMIMask, value=%02x", value);

		bool enabled = !GetBit(value, 7);
		if (m_nmiEnabled != enabled)
		{
			LogPrintf(LOG_WARNING, "NMI is %s", m_nmiEnabled ? "Enabled" : "Disabled");
		}
		m_nmiEnabled = enabled;
	}

	bool ComputerAT::Step()
	{
		static auto lastTick = std::chrono::high_resolution_clock::now();
		static int64_t syncTicks = 0;

		if (m_pic->InterruptPending() && GetCPU().CanInterrupt())
		{
			m_pic->InterruptAcknowledge();
			LogPrintf(LOG_DEBUG, "[%zu] IRQ %d", g_ticks, m_pic->GetPendingInterrupt() - 8);
			GetCPU().Interrupt(m_pic->GetPendingInterrupt());
			return true;
		}
		else if (!Computer::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU().GetInstructionTicks();

		ppi::Device8255XT* ppi = (ppi::Device8255XT*)m_ppi;

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

			pit::Counter& timer2 = m_pit->GetCounter(2);
			timer2.SetGate(ppi->GetTimer2Gate());

			m_pit->Tick();
			ppi->SetTimer2Output(timer2.GetOutput());

			m_pic->InterruptRequest(0, m_pit->GetCounter(0).GetOutput());

			// TODO: Temporary, pcSpeaker handles the audio, so add to mix
			if (!m_turbo) m_pcSpeaker.Tick();

			m_dma->Tick();
			m_dmaSecondary.Tick();

			if (syncTicks & 1)
			{
				m_video->Tick();
			}
			m_video->Tick();

			TickFloppy();
			TickHardDrive();

			++syncTicks;
		}
		cpuTicks %= GetCPUSpeedRatio();

		return true;
	}
}
