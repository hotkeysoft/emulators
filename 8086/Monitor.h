#pragma once

#include "Console.h"
#include "Memory.h"
#include "CPU8086.h"
#include "CPUInfo.h"

namespace emul
{
	enum class MonitorState { RUN, WAIT, EXIT, SWITCH_MODE };
	class Monitor
	{
	public:
		Monitor(Console& console);

		Monitor() = delete;
		Monitor(const Monitor&) = delete;
		Monitor& operator=(const Monitor&) = delete;
		Monitor(Monitor&&) = delete;
		Monitor& operator=(Monitor&&) = delete;

		void Init(CPU8086& cpu, Memory& memory);

		void SetCustomMemoryView(WORD segment, WORD offset) { m_customSegment = segment; m_customOffset = offset; }

		void Show();
		MonitorState Run();
		void Update();

	protected:
		WORD m_customSegment = 0;
		WORD m_customOffset = 0;

		struct Instruction
		{
			void AddRaw(BYTE b);
			void AddRaw(WORD w);

			SegmentOffset address;
			WORD offset;
			BYTE len = 0;
			BYTE raw[16];
			char text[32];
		};

		MonitorState ProcessKey();

		void ToggleRunMode();
		void UpdateRunMode();
		void ToggleRAMMode();
		void UpdateRAMMode();

		void WriteValueHex(BYTE value, const CPUInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(WORD value, const CPUInfo::Coord& coord, WORD attr = 15);

		void UpdateRegisters();
		void UpdateFlags();
		void UpdateRAM();
		void PrintInstruction(short y, Instruction& instr);
		void UpdateCode();

		SegmentOffset Disassemble(SegmentOffset address, Monitor::Instruction& decoded);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { DSSI, ESDI, STACK, CUSTOM };
		RAMMode m_ramMode = RAMMode::DSSI;

		CPU8086* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
