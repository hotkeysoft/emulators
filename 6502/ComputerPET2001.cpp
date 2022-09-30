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
		m_romF000("ROMF0000", 0x1000, emul::MemoryType::ROM),
		m_videoRAM("VID", 0x0400),
		m_ioE800("IO", 0x0100),
		m_pia1("PIA1"),
		m_pia2("PIA2")
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

		m_memory.Allocate(&m_videoRAM, 0x8000);
		m_memory.MapWindow(0x8000, 0x8400, 0x0400);
		m_memory.MapWindow(0x8000, 0x8800, 0x0400);
		m_memory.MapWindow(0x8000, 0x8C00, 0x0400);

		m_memory.Allocate(&m_ioE800, 0xE800);
		// TODO: No copy on board #4 to leave room at 0xE900-0xEFFF for nationalized keyboard mappings
		for (WORD b = 0xE900; b < 0xF000; b += 0x100)
		{
			m_memory.MapWindow(0xE800, b, 0x0100);
		}

		//m_ioE800.EnableLog(LOG_DEBUG);

		// PIA1 @ E8[10]
		m_pia1.Init();
		// Incomplete decoding, will also select at 3x, 5x, 7x etc
		m_ioE800.AddDevice(m_pia1, 0x10);

		// PIA2 @ E8[20]
		m_pia2.Init();
		// Incomplete decoding, will also select at 3x, 6x, 7x, Ax, Bx, etc
		m_ioE800.AddDevice(m_pia2, 0x20);

		// VIA @ E8[40]
		m_via.Init();
		// Incomplete decoding, will also select at 5x-7x, Cx-Fx
		m_ioE800.AddDevice(m_via, 0x40);

		InitInputs(6000000);
	}

	void ComputerPET2001::Reset()
	{
		Computer::Reset();
		m_pia1.Reset();
		m_pia2.Reset();
		m_via.Reset();
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

		// Temporary fake screen retrace
		const int refreshCounterInterval = 100000;
		static int refreshCounter = refreshCounterInterval;

		--refreshCounter;
		if (refreshCounter == 1000)
		{
			LogPrintf(LOG_WARNING, "Fake VSYNC BEGIN");
			m_pia1.GetPortB().SetC1(true);
			m_via.SetRetraceIn(true);
		}
		else if (refreshCounter == 0)
		{
			LogPrintf(LOG_WARNING, "Fake VSYNC END");
			refreshCounter = refreshCounterInterval;
			m_pia1.GetPortB().SetC1(false);
			m_via.SetRetraceIn(false);
		}

		return true;
	}
}
