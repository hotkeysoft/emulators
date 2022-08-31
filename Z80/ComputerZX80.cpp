#include "stdafx.h"

#include "ComputerZX80.h"
#include "Config.h"
#include "IO/Console.h"
#include "CPU/CPUZ80.h"
#include "Video/VideoZX80.h"

using cfg::CONFIG;
using cpuInfo::CPUType;

namespace emul
{
	ComputerZX80::ComputerZX80() :
		Logger("ZX80"),
		Computer(m_memory),
		m_baseRAM("RAM", 0x4000, emul::MemoryType::RAM),
		m_rom("ROM", 0x1000, emul::MemoryType::ROM)
	{
	}

	void ComputerZX80::Init(WORD baseRAM)
	{
		Computer::Init(CPUType::z80, baseRAM);

		GetMemory().EnableLog(CONFIG().GetLogLevel("memory"));

		m_rom.LoadFromFile("data/z80/zx80rom.bin");
		m_memory.Allocate(&m_rom, 0);

		m_memory.Allocate(&m_baseRAM, 0x4000);
		m_memory.MapWindow(0x4000, 0xC000, 0x4000);

		// TODO: Should be any output port
		Connect(0xFF, static_cast<PortConnector::OUTFunction>(&ComputerZX80::EndVSync));

		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));
		Connect(0xFE, static_cast<PortConnector::INFunction>(&ComputerZX80::ReadKeyboard));

		InitVideo(new video::VideoZX80());

		InitInputs(6000000);
		GetInputs().InitKeyboard(&m_keyboard);
	}

	bool ComputerZX80::Step()
	{
		if (!Computer::Step())
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
			// Character data (bit 6 = 0) are shown as to NOPs to the CPU
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
		BYTE row = GetCPU().GetIOHighAddress();
		BYTE data = ~m_keyboard.GetRowData(row);

		// debug, force shift + V
		//if (row == 0xFE)
		//{
		//	if (g_ticks > 1000000 && g_ticks < 2000000) {
		//					SetBit(data, 0, false);
		//					SetBit(data, 4, false);
		//	}
		//}
		//LogPrintf(LOG_WARNING, "Read Keyboard row %02x ["PRINTF_BIN_PATTERN_INT8"]", row, PRINTF_BYTE_TO_BIN_INT8(data));


		GetVideo().VSync(true);

		return data;
	}
}
