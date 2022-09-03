#include "stdafx.h"

#include "Computer8080.h"
#include <Config.h>
#include "IO/Console.h"

using cfg::CONFIG;
using cpuInfo::CPUType;

namespace emul
{
	Computer8080::Computer8080() :
		Logger("8080"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x8000, emul::MemoryType::RAM),
		m_rom("ROM", 0x8000, emul::MemoryType::ROM)
	{
	}

	void Computer8080::Init(WORD baseRAM)
	{
		Computer::Init(CPUType::i8080, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/8080/basic.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x8000);
	}

	bool Computer8080::Step()
	{
		if (!Computer::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU().GetInstructionTicks();

		++g_ticks;

		return true;
	}
}
