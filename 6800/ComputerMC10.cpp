#include "stdafx.h"

#include "ComputerMC10.h"
#include <Config.h>
#include <Sound/Sound.h>
#include "IO/Console.h"
#include "CPU/CPU6803.h"
#include <Video/VideoMC10.h>

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	ComputerMC10::ComputerMC10() :
		Logger("ComputerMC10"),
		ComputerBase(m_memory, 128),
		m_ram("RAM", RAM_SIZE, emul::MemoryType::RAM),
		m_rom("ROM", ROM_SIZE, emul::MemoryType::ROM)
	{
	}

	void ComputerMC10::Reset()
	{
		ComputerBase::Reset();
	}

	void ComputerMC10::Init(WORD baseRAM)
	{
		ComputerBase::Init(emul::CPUID_6803, baseRAM);

		InitROM();
		InitRAM(baseRAM);
		InitInputs(CPU_CLOCK, INPUT_REFRESH_PERIOD);
		InitKeyboard();
		InitVideo();
		InitIO();
		SOUND().SetBaseClock(CPU_CLOCK);
		InitVideo();
	}

	void ComputerMC10::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_6803) m_cpu = new CPU6803(m_memory, true);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerMC10::InitROM()
	{
		m_rom.LoadFromFile("./data/Tandy/trs80.mc10.bin");
		//m_rom.LoadFromFile("./data/Matra/alice.bin");
		m_memory.Allocate(&m_rom, ROM_BASE);
		m_memory.MapWindow(ROM_BASE, ROM_BASE + ROM_SIZE, ROM_SIZE);
	}

	void ComputerMC10::InitRAM(WORD baseRAM)
	{
		// TODO: 16K Extension
		// $4000 - $4FFF - 4kB RAM
		// $5000 - $8FFF - 16KB RAM expansion
		// $9000 - $BFFF - Minimally decoded single byte I/O slot for keyboard, VDG control and sound out

		m_memory.Allocate(&m_ram, RAM_BASE);

		m_memory.MapWindow(RAM_BASE, RAM_BASE + (RAM_SIZE * 1), RAM_SIZE, AllocateMode::READ); // Read-only image
		m_memory.MapWindow(RAM_BASE, RAM_BASE + (RAM_SIZE * 2), RAM_SIZE);
		m_memory.MapWindow(RAM_BASE, RAM_BASE + (RAM_SIZE * 3), RAM_SIZE, AllocateMode::READ); // Read-only image
	}

	void ComputerMC10::InitKeyboard()
	{
		GetInputs().InitKeyboard(&m_keyboard);
	}

	void ComputerMC10::InitVideo()
	{
		// Dummy video card
		video::VideoMC10* video = new video::VideoMC10();
		video->EnableLog(CONFIG().GetLogLevel("video"));
		video->Init(&m_memory, "./data/Tandy/trs80.mc10.char.bin");
		m_video = video;
	}

	void ComputerMC10::InitIO()
	{
		m_io.Init(&m_soundData, m_video, &m_keyboard);
		m_io.EnableLog(CONFIG().GetLogLevel("io"));
		m_memory.Allocate(&m_io, IO_BASE);

		GetCPU().SetIOEventHandler(this);
	}

	void ComputerMC10::OnWritePort1(BYTE value)
	{
		m_io.SetKeyboardScanRow(emul::LogBase2(~value));
	}

	bool ComputerMC10::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = GetCPU().GetInstructionTicks();

		for (uint32_t i = 0; i < cpuTicks; ++i)
		{
			++g_ticks;

			if (!m_turbo)
			{
				SOUND().PlayMono(m_soundData * 4000);
			}

			m_video->Tick();

			GetInputs().Tick();
			if (GetInputs().IsQuit())
			{
				return false;
			}
		}

		return true;
	}
}
