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
	// Reverse lookup table for PWM table value -> speed
	constexpr BYTE PWMReverseLookup[] = {
		0x00, 0x00, 0x3A, 0x01, 0x3B, 0x27, 0x35, 0x02,
		0x3C, 0x1F, 0x30, 0x28, 0x36, 0x12, 0x22, 0x03,
		0x3D, 0x33, 0x1D, 0x20, 0x31, 0x0B, 0x0D, 0x29,
		0x37, 0x0F, 0x1A, 0x13, 0x23, 0x16, 0x2B, 0x04,
		0x3E, 0x39, 0x26, 0x34, 0x1E, 0x2F, 0x11, 0x21,
		0x32, 0x1C, 0x0A, 0x0C, 0x0E, 0x19, 0x15, 0x2A,
		0x38, 0x25, 0x2E, 0x10, 0x1B, 0x09, 0x18, 0x14,
		0x24, 0x2D, 0x08, 0x17, 0x2C, 0x07, 0x06, 0x05,
	};

	constexpr int MAIN_CLK = 15667200; // 15.7 MHz Main crystal
	constexpr int PIXEL_CLK = MAIN_CLK;
	constexpr int CPU_CLK = PIXEL_CLK / 2;
	constexpr int VIA_CLK = CPU_CLK / 10;

	ComputerMacintosh::ComputerMacintosh() :
		Logger("ComputerMac"),
		ComputerBase(m_memory, 4096),
		m_baseRAM("RAM", RAM_SIZE, emul::MemoryType::RAM),
		m_rom("ROM", ROM_SIZE, emul::MemoryType::ROM),
		m_sound(m_memory),
		m_floppyInternal(CPU_CLK),
		m_floppyExternal(CPU_CLK)
	{
	}

	void ComputerMacintosh::Init(WORD baseRAM)
	{
		ComputerBase::Init(cpu68k::CPUID_68000, baseRAM);

		GetCPU().SetTrapList(cpu68k::mac::s_trapsMac128k);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("./data/Macintosh/mac.128k.bin");

		// Uncomment to make patches in the ROM
		// NOP = 0x4E71
		// Bypass checksum verification
		//m_rom.Fill(0xE2, { 0x4E, 0x71, 0x4E, 0x71, 0x4E, 0x71 });

		// Original low speed  00 80
		// Original high speed 01 00
		
		//m_rom.Fill(0x1e86, { 0x02, 0x00 });
		//m_rom.Fill(0x1e90, { 0x04, 0x00 });

		InitInputs(CPU_CLK);
		InitVideo();
		InitVIA();
		InitSCC();
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
		m_scc.Reset();
		m_sound.ResetBufferPos();
		m_sound.Enable(true);
		m_floppyInternal.Reset();
		m_floppyExternal.Reset();
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
		m_floppyController.EnableLog(CONFIG().GetLogLevel("floppy.iwm"));
		m_floppyController.Init(&m_floppyInternal, &m_floppyExternal);

		m_floppyInternal.Init(true);
		m_floppyInternal.EnableLog(CONFIG().GetLogLevel("floppy"));
		m_floppyInternal.SetTrackCount(80);
		m_floppyInternal.SetStepDelay(10);

		m_floppyExternal.Init(true);
		m_floppyExternal.EnableLog(CONFIG().GetLogLevel("floppy"));
		m_floppyExternal.SetTrackCount(80);
		m_floppyExternal.SetStepDelay(10);

		m_ioIWM.Init(&m_floppyController);
		m_memory.Allocate(&m_ioIWM, 0xD00000);

		std::string image = CONFIG().GetValueStr("floppy", "floppy.1");
		if (image.size())
		{
			if (!m_floppyInternal.LoadDiskImage(image.c_str()))
			{
				LogPrintf(LOG_ERROR, "Error loading image file: %s", image.c_str());
			}
		}

		image = CONFIG().GetValueStr("floppy", "floppy.2");
		if (image.size())
		{
			if (!m_floppyExternal.LoadDiskImage(image.c_str()))
			{
				LogPrintf(LOG_ERROR, "Error loading image file: %s", image.c_str());
			}
		}
	}

	void ComputerMacintosh::InitVIA()
	{
		m_via.EnableLog(CONFIG().GetLogLevel("via"));
		m_via.Init();
		m_via.SetEventHandler(this);
		m_ioVIA.Init(&m_via);
		m_memory.Allocate(&m_ioVIA, 0xE80000);
	}

	void ComputerMacintosh::InitSCC()
	{
		m_ioSCC.EnableLog(CONFIG().GetLogLevel("scc"));
		m_scc.EnableLog(CONFIG().GetLogLevel("scc"));
		m_scc.Init();
		m_ioSCC.Init(&m_scc);
		m_memory.Allocate(&m_ioSCC, 0x800000);
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

		// Update speed every 10 PWM values
		constexpr int PWM_SAMPLES = 10;
		static WORD pwmData = 0;
		static int pwmSamples = PWM_SAMPLES;

		pwmData += PWMReverseLookup[GetLByte(m_sound.GetBufferWord()) & 63];

		if (--pwmSamples == 0)
		{
			pwmSamples = PWM_SAMPLES;
			SetFloppySpeed(pwmData);
			pwmData = 0;
		}
	}

	void ComputerMacintosh::SetFloppySpeed(int pwmSpeed)
	{
		// Speed is out of ~400 (10 samples from 0..39)
		// PWM = 0% should be ~300 RPM
		// PWM = 100% should be ~700 RPM
		// 
		// Computing RPM = (4 * PWM%) + 300
		const int rpm = pwmSpeed + 300;

		m_floppyInternal.SetMotorSpeed(rpm);
		m_floppyExternal.SetMotorSpeed(rpm);
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
		LogPrintf(LOG_DEBUG, "Set Drive SEL line: %d", selectedHead);
		m_floppyController.SetSel(selectedHead);
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
				WORD sound = m_sound.IsEnabled() ? GetHByte(m_sound.GetBufferWord()) * m_via.GetSoundVolume() : 0;
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

			m_floppyInternal.Tick();
			m_floppyExternal.Tick();

			GetCPU().SetInterruptLevel(m_via.GetIRQ() ? 1 : 0);
		}
		return true;
	}
}
