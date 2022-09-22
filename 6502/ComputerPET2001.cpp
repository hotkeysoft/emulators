#include "stdafx.h"

#include "ComputerPET2001.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"

using cfg::CONFIG;

namespace emul
{
	ComputerPET2001::ComputerPET2001() :
		Logger("PET2001"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x1000, emul::MemoryType::RAM),
		m_romC000("ROMC0000", 0x1000, emul::MemoryType::ROM),
		m_romD000("ROMD0000", 0x1000, emul::MemoryType::ROM),
		m_romE000("ROME0000", 0x0800, emul::MemoryType::ROM),
		m_romF000("ROMF0000", 0x1000, emul::MemoryType::ROM)
	{
	}

	void ComputerPET2001::Init(WORD baseRAM)
	{
		Computer::Init(CPUID_6502, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_romC000.LoadFromFile("data/PET/PET2001/BASIC1-C000-p.bin");
		m_romD000.LoadFromFile("data/PET/PET2001/BASIC1-D000.bin");
		m_romE000.LoadFromFile("data/PET/PET2001/EDIT1-E000.bin");
		m_romF000.LoadFromFile("data/PET/PET2001/KERNAL1-F000.bin");

		m_memory.Allocate(&m_romC000, 0xC000);
		m_memory.Allocate(&m_romD000, 0xD000);
		m_memory.Allocate(&m_romE000, 0xE000);
		m_memory.Allocate(&m_romF000, 0xF000);

		m_memory.Allocate(&m_baseRAM, 0);

		InitInputs(6000000);
	}

	bool ComputerPET2001::Step()
	{
		if (!Computer::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU().GetInstructionTicks();

		++g_ticks;

		GetInputs().Tick();
		if (GetInputs().IsQuit())
		{
			return false;
		}

		return true;
	}
}
