#include "stdafx.h"

#include "ComputerZ80.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZX80.h"

using cfg::CONFIG;

namespace emul
{
	const size_t MAIN_CLK = 14000000; // 14 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK / 2;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	ComputerZ80::ComputerZ80() :
		Logger("ComputerZ80"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x8000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void ComputerZ80::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_LOW);
		ComputerBase::Init(CPUID_Z80, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		std::vector<BYTE> bootstrap = { 0x31, 0xff, 0xff, 0xc3, 0x00, 0x80 };

		static MemoryBlock bootstrapROM("bootstrap", bootstrap, MemoryType::ROM);
		m_memory.Allocate(&bootstrapROM, 0);

		m_baseRAM.LoadFromFile("P:/Projects/z80/z80test/src/z80doc.out");
		m_memory.Allocate(&m_baseRAM, 32768);

		Connect(0xFE, static_cast<PortConnector::OUTFunction>(&ComputerZ80::DummyOut));
		Connect(0xFF, static_cast<PortConnector::OUTFunction>(&ComputerZ80::PrintChar));

		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZ80::DummyIn));

		InitInputs(CPU_CLK);
		InitVideo();
	}

	void ComputerZ80::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerZ80::InitVideo()
	{
		m_video = new video::VideoZX80();
		m_video->Init(&m_memory, nullptr);
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
