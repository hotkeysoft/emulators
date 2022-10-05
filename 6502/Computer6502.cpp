#include "stdafx.h"

#include "Computer6502.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU6502.h"
#include "Video/VideoPET2001.h"

using cfg::CONFIG;

namespace emul
{
	Computer6502::Computer6502() :
		Logger("Computer6502"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x10000, emul::MemoryType::RAM)
	{
	}

	void Computer6502::Reset()
	{
		ComputerBase::Reset();
		GetCPU().Reset(0x400); // Override reset vector for tests
	}

	void Computer6502::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6502, baseRAM);

		std::vector<BYTE> bootstrap;
		bootstrap.resize(0x100);
		// = { 0x31, 0xff, 0xff, 0xc3, 0x00, 0x80 };

		//static MemoryBlock bootstrapROM("bootstrap", bootstrap, MemoryType::ROM);
		//m_memory.Allocate(&bootstrapROM, 0xFF00);

		m_baseRAM.LoadFromFile("P:/Projects/6502/6502_65C02_functional_tests/6502_functional_test.bin");
		m_memory.Allocate(&m_baseRAM, 0);

		InitInputs(1000000, 100000);
		InitVideo();
	}

	void Computer6502::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_6502) m_cpu = new CPU6502(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void Computer6502::InitVideo()
	{
		// Dummy video card
		video::VideoPET2001* video = new video::VideoPET2001();
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, "");
		m_video = video;
	}

	bool Computer6502::Step()
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
