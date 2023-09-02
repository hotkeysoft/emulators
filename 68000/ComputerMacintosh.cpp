#include "stdafx.h"

#include "ComputerMacintosh.h"
#include <Config.h>
#include <Sound/Sound.h>
#include "IO/Console.h"
#include "CPU/CPU68000.h"
#include "CPU/TrapsMac.h"

using cfg::CONFIG;
using sound::SOUND;

namespace emul
{
	const size_t MAIN_CLK = 15667200; // 15.7 MHz Main crystal
	const size_t PIXEL_CLK = MAIN_CLK;
	const size_t CPU_CLK = PIXEL_CLK / 2;

	ComputerMacintosh::ComputerMacintosh() :
		Logger("ComputerMac"),
		ComputerBase(m_memory, 4096),
		m_baseRAM("RAM", RAM_SIZE, emul::MemoryType::RAM),
		m_rom("ROM", ROM_SIZE, emul::MemoryType::ROM),
		m_sound(m_memory)
	{
	}

	void ComputerMacintosh::Init(WORD baseRAM)
	{
		ComputerBase::Init(cpu68k::CPUID_68000, baseRAM);

		GetCPU().SetTrapList(cpu68k::mac::s_trapsMac128k);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("./data/Macintosh/mac.128k.bin");

		// Temp RAM blocks instead of io to check instructions
		MemoryBlock* SCCr = new MemoryBlock("SCCr", 0x100000);
		SCCr->Clear(0x90);
		m_memory.Allocate(SCCr, 0x900000);

		InitInputs(CPU_CLK);
		InitVideo();
		InitVIA();
		InitFloppy();

		SOUND().SetBaseClock(CPU_CLK);
		m_sound.SetBufferBase(0x600000 + 0x1FD00);
		m_sound.ResetBufferPos();
		m_sound.Enable(true);
	}

	void ComputerMacintosh::Reset()
	{
		// Neds to be done before CPU reset so that memory is in the right place
		SetROMOverlayMode(true);

		ComputerBase::Reset();

		m_via.Reset();
		m_sound.ResetBufferPos();
		m_sound.Enable(true);
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

	void ComputerMacintosh::InitFloppy()
	{
		m_floppy.EnableLog(CONFIG().GetLogLevel("floppy"));
		m_ioIWM.Init(&m_floppy);
		m_memory.Allocate(&m_ioIWM, 0xD00000);
	}

	void ComputerMacintosh::InitVIA()
	{
		m_via.EnableLog(CONFIG().GetLogLevel("via"));
		m_via.Init();
		m_via.SetEventHandler(this);
		m_ioVIA.Init(&m_via);
		m_memory.Allocate(&m_ioVIA, 0xE80000);
	}

	void ComputerMacintosh::InitVideo()
	{
		m_video = new video::mac::VideoMac();
		m_video->EnableLog(CONFIG().GetLogLevel("video"));
		m_video->Init(&m_memory, nullptr);
		GetVideo().SetEventHandler(this);
	}

	void ComputerMacintosh::SetROMOverlayMode(bool overlay)
	{
		LogPrintf(LOG_INFO, "Set ROM Overlay mode: [%s]", overlay ? "OVERLAY" : "NORMAL");
		m_ramBaseAddress = overlay ? RAM_BASE_OVERLAY : RAM_BASE;

		LogPrintf(LOG_DEBUG, "Deallocate RAM and ROM blocks");
		m_memory.Free(&m_baseRAM);
		m_memory.Free(&m_rom);

		// Image at 400000 is always present
		LogPrintf(LOG_DEBUG, "Allocate ROM at [%06X]", ROM_BASE);
		m_memory.Allocate(&m_rom, ROM_BASE);

		// 15 mirror images right above
		for (int i = 1; i < 16; ++i)
		{
			m_memory.MapWindow(ROM_BASE, ROM_BASE + (i * ROM_SIZE), ROM_SIZE);
		}

		if (overlay)
		{
			LogPrintf(LOG_DEBUG, "Overlay ROM at [%06X]", ROM_BASE_OVERLAY);
			// Overlay 16 images starting at bottom of memory
			for (int i = 0; i < 16; ++i)
			{
				m_memory.MapWindow(ROM_BASE, ROM_BASE_OVERLAY + (i * ROM_SIZE), ROM_SIZE);
			}
		}

		// Another 16 shadow images of the ROM, at 200000 or 600000
		ADDRESS shadowROMBase = overlay ? ROM_MIRROR_BASE_OVERLAY : ROM_MIRROR_BASE;
		LogPrintf(LOG_DEBUG, "Shadow ROM at [%06X]", shadowROMBase);
		for (int i = 0; i < 16; ++i)
		{
			m_memory.MapWindow(ROM_BASE, shadowROMBase + (i * ROM_SIZE), ROM_SIZE);
		}

		ADDRESS ramBase = overlay ? RAM_BASE_OVERLAY : RAM_BASE;
		LogPrintf(LOG_DEBUG, "Allocate RAM at [%06X]", ramBase);
		m_memory.Allocate(&m_baseRAM, ramBase, RAM_SIZE);
		// 15 mirror images right above
		// TODO: Number will probably change on 512K config
		for (int i = 1; i < 16; ++i)
		{
			m_memory.MapWindow(ramBase, ramBase + (i * RAM_SIZE), RAM_SIZE);
		}

		// Adjust base offsets for sound + video buffers
		GetVideo().SetBaseAddress(m_ramBaseAddress);
		m_sound.SetBufferBase(m_ramBaseAddress);
	}

	void ComputerMacintosh::OnHBlankStart()
	{
		m_via.SetHBlank(false);
		m_via.SetHBlank(true);
		m_sound.BufferWord();
	}

	void ComputerMacintosh::OnVBlankStart()
	{
		m_sound.ResetBufferPos();
		m_via.SetVBlank(false);
		m_via.SetVBlank(true);
	}

	void ComputerMacintosh::OnSoundResetChange(bool reset)
	{
		LogPrintf(LOG_INFO, "Set Sound Reset: %d", reset);
		m_sound.Enable(!reset);
	}
	void ComputerMacintosh::OnSoundBufferChange(bool mainBuffer)
	{
		LogPrintf(LOG_INFO, "Set Sound Buffer: [%s]", mainBuffer ? "MAIN" : "ALT");
		m_sound.SetBufferOffset(mainBuffer ? RAM_SOUND_PAGE1_OFFSET : RAM_SOUND_PAGE2_OFFSET);
	}
	void ComputerMacintosh::OnVideoPageChange(bool mainBuffer)
	{
		LogPrintf(LOG_INFO, "Set Video Buffer : [%s]", mainBuffer ? "MAIN" : "ALT");
		GetVideo().SetPageOffset(mainBuffer ? RAM_VIDEO_PAGE1_OFFSET : RAM_VIDEO_PAGE2_OFFSET);
	}
	void ComputerMacintosh::OnHeadSelChange(bool selectedHead)
	{
		LogPrintf(LOG_INFO, "Set Selected Head: %d", selectedHead);
	}
	void ComputerMacintosh::OnROMOverlayModeChange(bool overlay)
	{
		SetROMOverlayMode(overlay);
	}

	bool ComputerMacintosh::Step()
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
				WORD sound = GetHByte(m_sound.GetBufferWord()) * m_via.GetSoundVolume();
				//sound = 0; //TODO: TEMP
				SOUND().PlayMono(sound * 2);
			}

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

			m_via.Tick();

			GetCPU().SetInterruptLevel(m_via.GetIRQ() ? 1 : 0);
		}
		return true;
	}
}
