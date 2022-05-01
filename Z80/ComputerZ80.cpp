#include "stdafx.h"

#include "ComputerZ80.h"
#include "Config.h"
#include "IO/Console.h"

using cfg::CONFIG;
using cpuInfo::CPUType;

namespace emul
{
	ComputerZ80::ComputerZ80() :
		Logger("Z80"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x4000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void ComputerZ80::Init(WORD baseRAM)
	{
		Computer::Init(CPUType::z80, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/z80/zx80rom.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x4000);
		m_memory.MapWindow(0x4000, 0xC000, 0x4000);
	}

	bool ComputerZ80::Step()
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
