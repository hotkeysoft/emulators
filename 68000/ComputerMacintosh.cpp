#include "stdafx.h"

#include "ComputerMacintosh.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPU68000.h"

using cfg::CONFIG;

namespace emul
{
	const size_t MAIN_CLK = 14000000; // 14 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	ComputerMacintosh::ComputerMacintosh() :
		Logger("ComputerMac"),
		ComputerBase(m_memory, 4096),
		m_baseRAM("RAM", 0x20000, emul::MemoryType::RAM),
		m_rom("ROM", 0x10000, emul::MemoryType::ROM)
	{
	}

	void ComputerMacintosh::Init(WORD baseRAM)
	{
		ComputerBase::Init(cpu68k::CPUID_68000, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("./data/Macintosh/mac.128k.bin");
		m_memory.Allocate(&m_rom, 0x400000);
		m_memory.MapWindow(0x400000, 0, 0x10000);

		m_memory.Allocate(&m_baseRAM, 0x600000);

		// Temp RAM blocks instead of io to check instructions
		MemoryBlock* SCCr = new MemoryBlock("SCCr", 0x100000);
		SCCr->Clear(0x90);
		m_memory.Allocate(SCCr, 0x900000);

		m_via.EnableLog(CONFIG().GetLogLevel("via"));
		m_via.Init();
		m_ioVIA.Init(&m_via);
		m_memory.Allocate(&m_ioVIA, 0xE80000);

		InitInputs(CPU_CLK);
		InitVideo();
	}

	void ComputerMacintosh::InitCPU(const char* cpuid)
	{
		if (cpuid == cpu68k::CPUID_68000) m_cpu = new cpu68k::CPU68000(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerMacintosh::InitVideo()
	{
		m_video = new video::VideoNull();
		m_video->Init(&m_memory, nullptr);
	}

	bool ComputerMacintosh::Step()
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
