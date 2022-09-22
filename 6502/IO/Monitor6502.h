#pragma once

#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>

namespace emul
{
	class CPU6502;

	enum class MonitorState { RUN, WAIT, EXIT, SWITCH_MODE };
	class Monitor6502
	{
	public:
		Monitor6502(Console& console);
		virtual ~Monitor6502() {}

		Monitor6502() = delete;
		Monitor6502(const Monitor6502&) = delete;
		Monitor6502& operator=(const Monitor6502&) = delete;
		Monitor6502(Monitor6502&&) = delete;
		Monitor6502& operator=(Monitor6502&&) = delete;

		virtual void Init(CPU6502& cpu, Memory& memory);

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

		static bool Replace(std::string& str, const std::string& from, const std::string& to);

		virtual ADDRESS Disassemble(ADDRESS address, Monitor6502::Instruction& decoded);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { X, Y, SP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::PC;

		CPU6502* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
