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

		struct Instruction
		{
			void AddRaw(BYTE b);
			void AddRaw(WORD w);
			void AddRaw(DWORD dw);

			ADDRESS address;
			WORD offset;
			BYTE len = 0;
			BYTE raw[16];
			char text[32];
		};
		virtual ADDRESS Disassemble(ADDRESS address, Monitor68000::Instruction& decoded);

		void SetCustomMemoryView(ADDRESS address) { m_customMemView = address; }

		void Show();
		MonitorState Run();
		void Update();

	protected:
		enum class EASize
		{
			Byte  = 0b00,
			Word  = 0b01,
			Long  = 0b10,
			Undef = 0b11
		};

		enum class EAMode
		{
			// Register Direct modes
			DataRegDirect = 0b000000,
			AddrRegDirect = 0b001000,

			// Memory Address modes
			AddrRegIndirect = 0b010000,
			AddrRegIndirectPostIncrement = 0b011000,
			AddrRegIndirectPreDecrement = 0b100000,
			AddrRegIndirectDisplacement = 0b101000,
			AddrRegIndirectIndex = 0b110000,

			// Special Address Modes (Mode=111)
			// (need reg# for complete decoding)
			AbsoluteShort = 0b111000,
			AbsoluteLong = 0b111001,
			ProgramCounterDisplacement = 0b111010,
			ProgramCounterIndex = 0b111011,
			Immediate = 0b111100,

			Invalid = 0b111111
		};

		class EffectiveAddress
		{
		public:
			EffectiveAddress() {}

			ADDRESS ComputeEA(Memory& memory, WORD data, ADDRESS currAddress);

			EAMode GetMode() const { return m_mode; }
			BYTE GetRegister() const { return m_regNumber; }
			const char* GetText() const { return m_text[0] ? m_text : BuildText(); }
		private:
			const char* BuildText() const;

			EASize m_size = EASize::Undef;
			BYTE m_regNumber = 0;
			EAMode m_mode = EAMode::Invalid;

			ADDRESS m_address = 0xDEADC0DE;

			mutable char m_text[32] = ""; // Mutable because lazy evaluation
		};

		ADDRESS m_customMemView = 0;

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

		virtual void UpdateRegisters();
		void UpdateTicks();
		void UpdateFlags();
		virtual ADDRESS GetRAMBase() const;
		void UpdateRAM();
		void PrintInstruction(short y, Instruction& instr);
		void UpdateCode();

		static bool Replace(std::string& str, const std::string& from, const std::string& to);

		enum class RUNMode { STEP, RUN };
		RUNMode m_runMode = RUNMode::STEP;

		enum class RAMMode { ZP, SP, PC, CUSTOM };
		RAMMode m_ramMode = RAMMode::ZP;

		CPU68000* m_cpu = nullptr;
		Memory* m_memory = nullptr;
		Console& m_console;
	};
}
