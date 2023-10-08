#pragma once

#include "CPU/CPU6800.h"
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

namespace emul
{
	static const size_t CPU6809_ADDRESS_BITS = 16;
	static const char* CPUID_6809 = "6809";

	class CPU6809 : public CPU6800
	{
	public:
		enum class RegCode {
			// 16 bit registers
			D  = 0b0000,
			X  = 0b0001,
			Y  = 0b0010,
			U  = 0b0011,
			S  = 0b0100,
			PC = 0b0101,

			// 8 bit register
			A  = 0b1000,
			B  = 0b1001,
			CC = 0b1010,
			DP = 0b1011,
		};

		enum class RegSize {
			BB = 0b10001000,
			BW = 0b10000000,
			WB = 0b00001000,
			WW = 0b00000000,

			_MASK = 0b10001000
		};

		enum FLAG6809 : BYTE
		{
			FLAG_E = 128, // 1 when Entire machine state was stacked
			FLAG_F = 64,  // 1 with FIRQ line is ignored
			FLAG_H = 32,  // 1 on half carry (4 bit)
			FLAG_I = 16,  // 1 when IRQ line is ignored
			FLAG_N = 8,   // 1 when result is negative
			FLAG_Z = 4,   // 1 when result is 0
			FLAG_V = 2,   // 1 on signed overflow
			FLAG_C = 1    // 1 on unsigned overflow
		};

		CPU6809(Memory& memory);
		virtual ~CPU6809() {};

		virtual void Init() override;

		virtual void Dump() override;

		virtual void Reset() override;

		WORD GetReg(RegCode reg) const;

		// Interrupts
		void SetFIRQ(bool firq) { m_firq = firq; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		void InitPage2();
		void InitPage3();

		inline void TICK1() { ++m_opTicks; }
		inline void TICKn(int n) { m_opTicks += n; }

		// Vectors
		static const ADDRESS _ADDR_RSV  = 0xFFF0; // Reserved by Motorola
		static const ADDRESS ADDR_SWI3  = 0xFFF2; // SWI3 instruction interrupt vector
		static const ADDRESS ADDR_SWI2  = 0xFFF4; // SWI2 instruction interrupt vector
		static const ADDRESS ADDR_FIRQ  = 0xFFF6; // Fast hardware interrupt vector (FIRQ)
		static const ADDRESS ADDR_SWI   = 0xFFFA; // SWI instruction interrupt vector

		OpcodeTable m_opcodesPage2;
		OpcodeTable m_opcodesPage3;

		// NMI is disabled on Reset until the S stack pointer is set
		bool m_nmiEnabled = false;

		// FIRQ is level sensitive
		bool m_firq = false;

		virtual void Interrupt() override;

		enum class STACK
		{
			S, U
		};

		struct Registers
		{
#pragma pack(push, 1)
			union
			{
				WORD D = 0;	// Accumulators
				struct {
					BYTE B;
					BYTE A;
				} ab;
			};
#pragma pack(pop)

			WORD S = 0; // Hardware Stack Pointer
			WORD U = 0; // User Stack Pointer
			WORD X = 0; // Index Register
			WORD Y = 0; // Index Register

			BYTE DP = 0; // Direct Page Register

			// For invalid destinations;
			BYTE void8;
			WORD void16;
		} m_reg;

		bool IsStackRegister(const WORD& reg) const { return &reg == &m_reg.S; }

		// Flags
		bool GetFlag(FLAG6809 f) { return (m_flags & f) ? true : false; };
		void SetFlag(FLAG6809 f, bool v) { SetBitMask(m_flags, f, v); };

		// Sub Opcode Tables
		void ExecPage2(BYTE opcode);
		void ExecPage3(BYTE opcode);
		void exec(OpcodeTable& table, BYTE opcode);

		// Misc helpers
		virtual ADDRESS GetDirect() override { return MakeWord(m_reg.DP, FetchByte()); }
		virtual ADDRESS GetIndexed() override;
		WORD& GetIndexedRegister(BYTE idx);

		// Register helpers
		//
		// sd (source/dest):
		// [b7..b4] | [b3..b0]
		// r0 (src) | r1 (dst)

		static RegCode GetSourceRegCode(BYTE sd) { return (RegCode)GetHNibble(sd); }
		static RegCode GetDestRegCode(BYTE sd) { return (RegCode)GetLNibble(sd); }
		static RegSize GetRegsSize(BYTE sd) { return (RegSize)(sd & (BYTE)RegSize::_MASK); }

		static bool isSourceRegWide(BYTE sd) { return !GetBit(sd, 7); }
		static bool isDestRegWide(BYTE sd) { return !GetBit(sd, 3); }

		BYTE& GetReg8(RegCode reg);
		WORD& GetReg16(RegCode reg);

		void MEMDirectOp(std::function<void(CPU6809*, BYTE&)> func);
		void MEMIndexedOp(std::function<void(CPU6809*, BYTE&)> func);
		void MEMExtendedOp(std::function<void(CPU6809*, BYTE&)> func);

		// Opcodes

		void LEA(WORD& dest, bool setZero);

		// Branching
		void LBRA(bool condition);

		void BSR();
		void LBSR();
		void JSR(ADDRESS dest);
		void RTS();

		void SWI(BYTE swi);
		void RTI();

		// Transfer register to register
		void TFR(BYTE sd);

		// Exchange Registers
		void EXG(BYTE sd);

		// Stack operations
		static const BYTE REGS_ALL = 0b11111111; // PC, U/S, Y, X, DP, B, A, CC
		static const BYTE REGS_RTI = 0b10000001; // PC, CC
		static const BYTE REGS_PC  = 0b10000000; // PC
		static const BYTE REGS_CC  = 0b00000001; // CC

		// Push/pull one or more registers on s stack (S or U)
		void PSH(STACK s, BYTE regs);
		void PUL(STACK s, BYTE regs);

		// raw push/pop, used by PSH/PUL
		void push(BYTE value);
		BYTE pop();

		// Active stack, used by push/pull
		// (always set before call, no need to serialize)
		WORD* m_currStack = &m_reg.S;
		void SetStack(STACK s) { m_currStack = (s == STACK::S) ? &m_reg.S : &m_reg.U; }

		void SEX(); // Sign Extend B to D

		// Arithmetic
		void MUL(); // D' = A * B

		// Undocumented
		void XNC(BYTE& dest) { GetFlag(FLAG_C) ? COM(dest) : NEG(dest); }
		void XCLR(BYTE& dest);
		void XDEC(BYTE& dest);
		void XRES();

		friend class Monitor6809;
	};
}
