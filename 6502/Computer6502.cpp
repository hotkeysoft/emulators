#include "stdafx.h"

#include "Computer6502.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"

using cfg::CONFIG;

namespace emul
{
	Computer6502::Computer6502() :
		Logger("Computer6502"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x10000, emul::MemoryType::RAM)
	{
	}

	void Computer6502::Reset()
	{
		Computer::Reset();
		GetCPU().Reset(0x400); // Override reset vector for tests
	}

	void Computer6502::Init(WORD baseRAM)
	{
		Computer::Init(CPUID_6502, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		std::vector<BYTE> bootstrap;
		bootstrap.resize(0x100);
		// = { 0x31, 0xff, 0xff, 0xc3, 0x00, 0x80 };

		//static MemoryBlock bootstrapROM("bootstrap", bootstrap, MemoryType::ROM);
		//m_memory.Allocate(&bootstrapROM, 0xFF00);

		m_baseRAM.LoadFromFile("P:/Projects/6502/6502_65C02_functional_tests/bin_files/6502_functional_test.bin");
		m_memory.Allocate(&m_baseRAM, 0);

		InitInputs(6000000);
	}

	bool Computer6502::Step()
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
