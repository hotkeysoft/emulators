#pragma once

#include "Console.h"
#include "../CPU/Memory.h"
#include "../CPU/CPUInfo.h"

namespace emul
{
	class CPU8080;

	enum class MonitorState { RUN, WAIT, EXIT, SWITCH_MODE };
	class Monitor8080
	{
	public:
		Monitor8080(Console& console);

		Monitor8080() = delete;
		Monitor8080(const Monitor8080&) = delete;
		Monitor8080& operator=(const Monitor8080&) = delete;
		Monitor8080(Monitor8080&&) = delete;
		Monitor8080& operator=(Monitor8080&&) = delete;

		virtual void Init(CPU8080& cpu, Memory& memory);

		void SetCustomMemoryView(ADDRESS address) { m_customMemView = address; }

		void Show();
		MonitorState Run();
		void Update();

	protected:
		ADDRESS m_customMemView = 0;

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

		ADDRESS Disassemble(ADDRESS address, Monitor8080::Instruction& decoded);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { HL, SP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::HL;

		CPU8080* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}