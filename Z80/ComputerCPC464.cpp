#include "stdafx.h"

#include "ComputerCPC464.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZXSpectrum.h"
#include <Sound/Sound.h>

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	// TODO
	const size_t MAIN_CLK = 14000000; // 14 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	// TODO: ULA
	// Every 20ms, generate interrupt
	// For now base on CPU clock
	const size_t RTC_CLK = 50; // 50Hz
	const size_t RTC_RATE = CPU_CLK / RTC_CLK;


	ComputerCPC464::ComputerCPC464() :
		Logger("ZXSpectrum"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x10000, emul::MemoryType::RAM),
		m_romLow("ROML", 0x4000, emul::MemoryType::ROM),
		m_romHigh("ROMH", 0x4000, emul::MemoryType::ROM)
	{
	}

	void ComputerCPC464::Init(WORD baseRAM)
	{
		ComputerBase::Init(CPUID_Z80, baseRAM);

		EnableLog(LOG_DEBUG);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_memory.Allocate(&m_baseRAM, 0);

		// TODO
		// Put low ROM on top of RAM
		m_romLow.LoadFromFile("data/z80/amstrad.cpc464.os.bin");
		m_memory.Allocate(&m_romLow, 0);

		// TODO
		// Put high ROM on top of RAM
		m_romHigh.LoadFromFile("data/z80/amstrad.cpc464.basic.bin");
		m_memory.Allocate(&m_romHigh, 0xC000);

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

	// TODO
	void ComputerCPC464::InitVideo()
	{
		video::VideoZXSpectrum* video = new video::VideoZXSpectrum();

		m_video = video;
		m_video->Init(&m_memory, nullptr);
	}

	bool ComputerCPC464::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		uint32_t cpuTicks = GetCPU().GetInstructionTicks();

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
		}

		return true;
	}
}
