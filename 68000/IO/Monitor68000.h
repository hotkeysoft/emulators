#pragma once

#include <IO/Console.h>
#include <CPU/Memory.h>
#include <CPU/CPUInfo.h>
#include "CPU/CPU68000.h"

namespace emul::cpu68k
{
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

		struct Instruction
		{
			void AddRaw(BYTE b);
			void AddRaw(WORD w);
			void AddRaw(DWORD dw);

			ADDRESS address;
			WORD offset;
			static constexpr size_t RAW_LEN = 24;
			static constexpr size_t TEXT_LEN = 64;

			BYTE rawLen = 0;
			BYTE raw[RAW_LEN];
			char text[TEXT_LEN];
		};
		virtual ADDRESS Disassemble(ADDRESS address, Monitor68000::Instruction& decoded);

		void SetCustomMemoryView(ADDRESS address) { m_customMemView = address; }
		void SetBreakpoint(ADDRESS address) { m_breakpoint = address; m_breakpointEnabled = true; }
		void ClearBreakpoint() { m_breakpointEnabled = false; }
		bool IsBreakpoint() const { return m_breakpointEnabled && m_cpu->GetCurrentAddress() == m_breakpoint; }

		void Show();
		MonitorState Run();
		void SetStepMode() { m_runMode = RUNMode::STEP; }
		void Update();

	protected:
		std::string DecodeRegisterBitmask(WORD mask, bool isPredecrement);
		std::string DecodeRegisters(BYTE mask, char prefix);

		enum class EASize
		{
			Byte  = 0b00,
			Word  = 0b01,
			Long  = 0b10,
			Undef = 0b11
		};

		class EffectiveAddress
		{
		public:
			EffectiveAddress(Memory& mem, Instruction& instr, ADDRESS currAddress) :
				m_currInstruction(instr),
				m_memory(mem),
				m_currAddress(currAddress)
			{}

			void ComputeEA(WORD data);
			const char* BuildText();

			EAMode GetMode() const { return m_mode; }
			EASize GetSize() const { return m_size; }
			void SetSize(EASize size) { m_size = size; }
			BYTE GetRegister() const { return m_regNumber; }
			const char* GetText() { return m_text; }
			ADDRESS GetCurrAddress() const { return m_currAddress; }
		private:
			const char* GetExtensionWord(BYTE reg);
			SWORD GetDisplacementWord();
			void BuildImmediate();

			EASize m_size = EASize::Undef;
			BYTE m_regNumber = 0; // [0..7]|PC
			static constexpr BYTE PC = BYTE(-1);
			EAMode m_mode = EAMode::Invalid;

			ADDRESS m_address = 0xDEADC0DE;

			char m_text[64] = "";
			char m_extWord[16] = "";

			Instruction& m_currInstruction;
			ADDRESS m_currAddress = 0;
			emul::Memory& m_memory;
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
		void WriteValueHex(DWORD value, const cpuInfo::Coord& coord, WORD attr = 15);
		void WriteValueHex24(DWORD value, const cpuInfo::Coord& coord, WORD attr = 15);

		virtual void UpdateRegisters();
		void UpdateTicks();
		void UpdateFlags();

		virtual ADDRESS GetRAMBase() const;
		void UpdateRAM();

		void ToggleCodeMode();
		void PrintInstruction(short y, Instruction& instr);
		void UpdateCode();
		void ClearCode();

		static bool Replace(std::string& str, const std::string& from, const std::string& to);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { A0, A1, A2, A3, A4, A5, A6, USP, SSP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::SSP;

		enum class CODEMode { TEXT_ONLY, RAW_AND_TEXT };
		CODEMode m_codeMode = CODEMode::TEXT_ONLY;

		cpu68k::CPU68000* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
