#include "stdafx.h"

#include "ComputerZX80.h"
#include <Config.h>
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZX80.h"

using cfg::CONFIG;

namespace emul
{
	ComputerZX80::ComputerZX80() :
		Logger("ZX80"),
		ComputerBase(m_memory),
		m_baseRAM("RAM", 0x4000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void ComputerZX80::Init(WORD baseRAM)
	{
		PortConnector::Init(PortConnectorMode::BYTE_LOW);
		ComputerBase::Init(CPUID_Z80, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/z80/zx80rom.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x4000);
		m_memory.MapWindow(0x4000, 0xC000, 0x4000);

		// TODO: Should be any output port
		Connect(0xFF, static_cast<PortConnector::OUTFunction>(&ComputerZX80::EndVSync));

		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));

		InitInputs(1000000);
		GetInputs().InitKeyboard(&m_keyboard);
		InitVideo();
	}

	void ComputerZX80::InitCPU(const char* cpuid)
	{
		if (cpuid == CPUID_Z80) m_cpu = new CPUZ80(m_memory);
		else
		{
			LogPrintf(LOG_ERROR, "CPUType not supported: [%s]", cpuid);
			throw std::exception("CPUType not supported");
		}
	}

	void ComputerZX80::InitVideo()
	{
		video::VideoZX80* video = new video::VideoZX80();

		// TODO, depends if code is shared with zx81
		const char* arch = "zx80";

		DWORD backgroundRGB = CONFIG().GetValueDWORD(arch, "video.bg", video->GetDefaultBackground());
		DWORD foregroundRGB = CONFIG().GetValueDWORD(arch, "video.fg", video->GetDefaultForeground());

		video->SetBackground(backgroundRGB);
		video->SetForeground(foregroundRGB);

		m_video = video;
		m_video->Init(&m_memory, nullptr);
	}

	bool ComputerZX80::Step()
	{
		if (!ComputerBase::Step())
		{
			return false;
		}

		static uint32_t cpuTicks = 0;
		cpuTicks += GetCPU().GetInstructionTicks();

		// Interrupt when A6 = 0
		//
		// Interrupts are only enabled during Display File processing.
		// An interrupt signals the end of a row
		//
		// We check the Refresh Counter value directly here because, unlike
		// on a real Z80, its value never appears on GetCurrentAddress()
		// (somewhat equivalent of the address bus)
		GetCPU().SetINT(!GetBit(GetCPU().GetRefreshCounter(), 6));

		// HSync starts on interrupt acknowledge (or io request, on the cpu they are on the same pin)
		if (GetCPU().IsInterruptAcknowledge() || GetCPU().IsIORequest())
		{
			GetVideo().HSync();
		}

		++g_ticks;

		GetInputs().Tick();
		if (GetInputs().IsQuit())
		{
			return false;
		}

		ADDRESS curr = GetCPU().GetCurrentAddress();

		// Currently "executing" the Display File area
		if (GetBit(curr, 15) && (GetCPU().GetState() != CPUState::HALT))
		{
			BYTE data = m_memory.Read8(curr);
			// Character data (bit 6 = 0) are shown as NOPs to the CPU
			if (!GetBit(data, 6))
			{
				GetCPU().EnableDataBus(false);
				GetVideo().LatchCurrentChar(data);
			}
			else
			{
				GetCPU().EnableDataBus(true);
				GetVideo().LatchCurrentChar(0);
			}
		}

		GetVideo().Tick();

		return true;
	}

	void ComputerZX80::EndVSync(BYTE)
	{
		GetVideo().VSync(false);
	}

	BYTE ComputerZX80::ReadKeyboard()
	{
		BYTE row = GetHByte(GetCurrentPort());
		BYTE data = ~m_keyboard.GetRowData(row);

		GetVideo().VSync(true);

		return data;
	}
}
