#include "stdafx.h"

#include "ComputerZXSpectrum.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZX80.h"

using cfg::CONFIG;

namespace emul
{
	ComputerZXSpectrum::ComputerZXSpectrum() :
		Logger("ZXSpectrum"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x4000, emul::MemoryType::RAM),
		m_rom("ROM", 0x4000, emul::MemoryType::ROM)
	{
	}

	void ComputerZXSpectrum::Init(WORD baseRAM)
	{
		Computer::Init(CPUID_Z80, baseRAM);

		EnableLog(LOG_DEBUG);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/z80/zxspectrum48.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x4000);
		//m_memory.MapWindow(0x4000, 0xC000, 0x4000);

		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZXSpectrum::ReadKeyboard));

		const char* arch = "spectrum";

		video::VideoZX80* video = new video::VideoZX80();

		//DWORD backgroundRGB = CONFIG().GetValueDWORD(arch, "video.bg", video->GetDefaultBackground());
		//DWORD foregroundRGB = CONFIG().GetValueDWORD(arch, "video.fg", video->GetDefaultForeground());

		//video->SetBackground(backgroundRGB);
		//video->SetForeground(foregroundRGB);
		InitVideo(video); // Takes ownership

		// TODO
		const size_t MAIN_CLK = 14000000; // Main crystal
		const size_t PIXEL_CLK = MAIN_CLK / 2;
		const size_t CPU_CLK = PIXEL_CLK / 2;

		InitInputs(CPU_CLK, 50);
		GetInputs().InitKeyboard(&m_keyboard);
	}

	bool ComputerZXSpectrum::Step()
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

		GetVideo().Tick();

		return true;
	}

	BYTE ComputerZXSpectrum::ReadKeyboard()
	{
		BYTE row = GetCPU().GetIOHighAddress();
		LogPrintf(LOG_DEBUG, "ReadKeyboard, row = %02X", row);

		BYTE data = ~m_keyboard.GetRowData(row);

		return data;
	}
}
