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
		m_baseRAM("RAM", 0x8000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void Computer6502::Init(WORD baseRAM)
	{
		Computer::Init(CPUID_6502, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		std::vector<BYTE> bootstrap = { 0x31, 0xff, 0xff, 0xc3, 0x00, 0x80 };

		static MemoryBlock bootstrapROM("bootstrap", bootstrap, MemoryType::ROM);
		m_memory.Allocate(&bootstrapROM, 0);

		m_baseRAM.LoadFromFile("P:/Projects/6502/6502test/src/6502doc.out");
		m_memory.Allocate(&m_baseRAM, 32768);

		Connect(0xFE, static_cast<PortConnector::OUTFunction>(&Computer6502::DummyOut));

		Connect(0xFE, static_cast<PortConnector::INFunction>(&Computer6502::DummyIn));

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

		GetInputs().Tick();
		if (GetInputs().IsQuit())
		{
			return false;
		}

		return true;
	}
}
