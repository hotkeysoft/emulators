#pragma once

#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>

namespace emul
{
	class CPU;
	class CPU68000;

	enum class MonitorState { RUN, WAIT, EXIT, SWITCH_MODE };
	class Monitor68000
	{
	public:
		Monitor68000(Console& console);
		virtual ~Monitor68000() {}

		Monitor68000() = delete;
		Monitor68000(const Monitor68000&) = delete;
		Monitor68000& operator=(const Monitor68000&) = delete;
		Monitor68000(Monitor68000&&) = delete;
		Monitor68000& operator=(Monitor68000&&) = delete;

		virtual void Init(CPU* cpu, Memory& memory);

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

		virtual ADDRESS Disassemble(ADDRESS address, Monitor68000::Instruction& decoded);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { ZP, SP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::ZP;

		CPU68000* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
