#pragma once

#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>
#include "CPU/CPU6800.h"

namespace emul
{
	class CPU;

	enum class MonitorState { RUN, WAIT, EXIT, SWITCH_MODE };
	class Monitor6800
	{
	public:
		Monitor6800(Console& console);
		virtual ~Monitor6800() {}

		Monitor6800() = delete;
		Monitor6800(const Monitor6800&) = delete;
		Monitor6800& operator=(const Monitor6800&) = delete;
		Monitor6800(Monitor6800&&) = delete;
		Monitor6800& operator=(Monitor6800&&) = delete;

		virtual void Init(CPU* cpu, Memory& memory);

		void SetCustomMemoryView(ADDRESS address) { m_customMemView = address; }
		void SetBreakpoint(ADDRESS address) { m_breakpoint = address; m_breakpointEnabled = true; }
		void ClearBreakpoint() { m_breakpointEnabled = false; }
		bool IsBreakpoint() const { return m_breakpointEnabled && m_cpu->GetCurrentAddress() == m_breakpoint; }

		void Show();
		MonitorState Run();
		void SetStepMode() { m_runMode = RUNMode::STEP; }
		void Update();

	protected:
		struct Instruction
		{
			void AddRaw(BYTE b);
			void AddRaw(WORD w);

			ADDRESS address;
			WORD offset;
			BYTE len = 0;
			BYTE raw[16];
			char text[32];
		};

		ADDRESS m_customMemView = 0;
		ADDRESS m_breakpoint = 0;
		bool m_breakpointEnabled = false;

		MonitorState ProcessKey();

		void ToggleRunMode();
		void UpdateRunMode();
		virtual void ToggleRAMMode();
		virtual void UpdateRAMMode();
		void UpdateCPUType();

		void WriteValueNibble(BYTE value, const cpuInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(BYTE value, const cpuInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex(WORD value, const cpuInfo::Coord& coord, WORD attr = 15);

		virtual void UpdateRegisters();
		void UpdateTicks();
		void UpdateFlags();
		virtual ADDRESS GetRAMBase() const;
		void UpdateRAM();
		void PrintInstruction(short y, Instruction& instr);
		void UpdateCode();
		void ClearCode();

		static bool Replace(std::string& str, const std::string& from, const std::string& to);

		virtual ADDRESS Disassemble(ADDRESS address, Monitor6800::Instruction& decoded);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { ZP, IX, SP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::IX;

		CPU6800* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
