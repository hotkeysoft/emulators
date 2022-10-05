#include "stdafx.h"

#include <CPU/CPU8080.h>
#include "Computer8080.h"
#include <Config.h>
#include <Video/VideoZXSpectrum.h>

using cfg::CONFIG;

namespace emul
{
	const size_t CPU_CLK = 4000000;

	Computer8080::Computer8080() :
		Logger("Computer8080"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x8000, emul::MemoryType::RAM),
		m_rom("ROM", 0x8000, emul::MemoryType::ROM)
	{
	}

	void Computer8080::Init(WORD baseRAM)
	{
		ComputerBase::Init(CPUID_8080, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/8080/basic.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x8000);

		InitInputs(CPU_CLK);
		InitVideo();
	}

	void Computer8080::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_8080) m_cpu = new CPU8080(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void Computer8080::InitVideo()
	{
		// Test computer has no video, use dummy one
		video::VideoZXSpectrum* video = new video::VideoZXSpectrum();

		m_video = video;
		m_video->Init(&m_memory, nullptr);
	}

	bool Computer8080::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU()->GetInstructionTicks();

		++g_ticks;

		return true;
	}
}
