#include "stdafx.h"

#include "Computer6800.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6800.h"
#include <Video/VideoNull.h>

using cfg::CONFIG;

namespace emul
{
	Computer6800::Computer6800() :
		Logger("Computer6800"),
		ComputerBase(m_memory, 128),
		m_ram("RAM", 128, emul::MemoryType::RAM),
		m_rom("ROM", 1024, emul::MemoryType::ROM)
	{
	}

	void Computer6800::Reset()
	{
		ComputerBase::Reset();
		GetCPU().Reset(0xE0D0); // Override reset vector
	}

	void Computer6800::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6800, baseRAM);

		m_memory.Allocate(&m_ram, 0xA000);

		m_rom.LoadFromFile("./data/swtbug.bin");
		m_memory.Allocate(&m_rom, 0xE000);

		InitInputs(1000000, 100000);
		InitVideo();
	}

	void Computer6800::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_6800) m_cpu = new CPU6800(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void Computer6800::InitVideo()
	{
		// Dummy video card
		video::VideoNull* video = new video::VideoNull();
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, nullptr);
		m_video = video;
	}

	bool Computer6800::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU().GetInstructionTicks();

		++g_ticks;

		m_video->Tick();

		GetInputs().Tick();
		if (GetInputs().IsQuit())
		{
			return false;
		}

		return true;
	}
}
