#include "stdafx.h"

#include "Computer68000.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU68000.h"
#include "Video/Video68000.h"

using cfg::CONFIG;

namespace emul
{
	const size_t MAIN_CLK = 14000000; // 14 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	Computer68000::Computer68000() :
		Logger("Computer68000"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x8000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void Computer68000::Init(WORD baseRAM)
	{
		ComputerBase::Init(CPUID_68000, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		std::vector<BYTE> bootstrap = { 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x01, 0x00 };

		static MemoryBlock bootstrapROM("bootstrap", bootstrap, MemoryType::ROM);
		m_memory.Allocate(&bootstrapROM, 0);

		m_baseRAM.LoadFromFile("P:/Projects/z80/z80test/src/z80doc.out");
		m_memory.Allocate(&m_baseRAM, 32768);

		InitInputs(CPU_CLK);
		InitVideo();
	}

	void Computer68000::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_68000) m_cpu = new CPU68000(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void Computer68000::InitVideo()
	{
		m_video = new video::Video68000();
		m_video->Init(&m_memory, nullptr);
	}

	void Computer68000::PrintChar(BYTE value)
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

	bool Computer68000::Step()
	{
		if (!ComputerBase::Step())
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
