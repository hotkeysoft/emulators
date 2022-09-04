#include "stdafx.h"

#include "ComputerZ80.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZX80.h"

using cfg::CONFIG;

namespace emul
{
	ComputerZ80::ComputerZ80() :
		Logger("ComputerZ80"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x8000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void ComputerZ80::Init(WORD baseRAM)
	{
		Computer::Init(CPUID_Z80, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		std::vector<BYTE> bootstrap = { 0x31, 0xff, 0xff, 0xc3, 0x00, 0x80 };

		static MemoryBlock bootstrapROM("bootstrap", bootstrap, MemoryType::ROM);
		m_memory.Allocate(&bootstrapROM, 0);

		m_baseRAM.LoadFromFile("P:/Projects/z80/z80test/src/z80doc.out");
		m_memory.Allocate(&m_baseRAM, 32768);

		Connect(0xFE, static_cast<PortConnector::OUTFunction>(&ComputerZ80::DummyOut));
		Connect(0xFF, static_cast<PortConnector::OUTFunction>(&ComputerZ80::PrintChar));

		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZ80::DummyIn));

		InitVideo(new video::VideoZX80());

		InitInputs(6000000);
	}

	void ComputerZ80::PrintChar(BYTE value)
	{
		if (value == 13)
		{
			putc('\r', stderr);
			putc('\n', stderr);
		}
		else if (value < 32)
		{
			putc(' ', stderr);
		}
		else
		{
			putc(value, stderr);
		}
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

		GetVideo().Tick();

		return true;
	}
}
