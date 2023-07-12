#include "stdafx.h"

#include "ComputerCPC464.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoCPC464.h"
#include <Sound/Sound.h>

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	// TODO
	const size_t MAIN_CLK = 16000000; // 16 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	const size_t RTC_CLK = 50; // 50Hz
	const size_t RTC_RATE = CPU_CLK / RTC_CLK;

	ComputerCPC464::ComputerCPC464() :
		Logger("ZXSpectrum"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x10000, emul::MemoryType::RAM),
		m_romLow("ROML", ROM_SIZE, emul::MemoryType::ROM),
		m_romHigh("ROMH", ROM_SIZE, emul::MemoryType::ROM)
	{
	}

	void ComputerCPC464::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_HI);
		ComputerBase::Init(CPUID_Z80, baseRAM);

		EnableLog(LOG_DEBUG);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_memory.Allocate(&m_baseRAM, 0);

		m_romLow.LoadFromFile("data/z80/amstrad.cpc464.os.bin");
		m_romHigh.LoadFromFile("data/z80/amstrad.cpc464.basic.bin");

		// Only enable low rom (os) at boot
		OnLowROMChange(true); // Load low ROM on top of RAM
		OnHighROMChange(false);

		// Upper ROM Bank Number, not present on 464, shut it down
		Connect(0xDF, static_cast<PortConnector::OUTFunction>(&ComputerCPC464::NullWrite));

		m_pio.EnableLog(CONFIG().GetLogLevel("pio"));
		m_pio.Init("xxxx0xxx");

		InitInputs(CPU_CLK, RTC_CLK);
		GetInputs().InitKeyboard(&m_keyboard);

		SOUND().SetBaseClock(CPU_CLK);
		InitVideo();
	}

	void ComputerCPC464::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerCPC464::InitVideo()
	{
		// Gate array always read directly from base ram block
		vid464::VideoCPC464* video = new vid464::VideoCPC464(&m_baseRAM);
		m_video = video;

		GetVideo().SetEventHandler(this);

		m_video->EnableLog(CONFIG().GetLogLevel("video"));
		m_video->Init(&m_memory, nullptr);
	}

	void ComputerCPC464::OnLowROMChange(bool load)
	{
		LoadROM(load, m_romLow, ROM_LOW);
	}

	void ComputerCPC464::OnHighROMChange(bool load)
	{
		LoadROM(load, m_romHigh, ROM_HIGH);
	}

	void ComputerCPC464::LoadROM(bool load, MemoryBlock& rom, ADDRESS base)
	{
		LogPrintf(LOG_INFO, "%sLOAD ROM [%s]", (load ? "" : "UN"), rom.GetId().c_str());

		if (load)
		{
			m_memory.Allocate(&rom, base, -1, emul::AllocateMode::READ);
		}
		else
		{
			m_memory.Restore(base, ROM_SIZE, emul::AllocateMode::READ);
		}
	}

	bool ComputerCPC464::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		if (GetCPU().IsInterruptAcknowledge())
		{
			GetVideo().InterruptAcknowledge();
		}

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			GetVideo().Tick();

			if (!m_turbo)
			{
				SOUND().PlayMono(0);
			}

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}

			GetCPU().SetINT(GetVideo().IsInterrupt());
		}

		return true;
	}
}
