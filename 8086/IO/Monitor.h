#pragma once

#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>
#include "../CPU/CPU8086.h"

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

		void Init(CPU8086* cpu, Memory& memory);

		void SetCustomMemoryView(RawSegmentOffset segoff) { m_customMemView = segoff; }

		void Show();
		MonitorState Run();
		void Update();

	protected:
		RawSegmentOffset m_customMemView{ 0, 0 };

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
		void UpdateCPUType();

		void WriteValueHex(BYTE value, const cpuInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(WORD value, const cpuInfo::Coord& coord, WORD attr = 15);

		void UpdateRegisters();
		void UpdateTicks();
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
