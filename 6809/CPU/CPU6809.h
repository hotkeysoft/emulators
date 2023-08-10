#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

namespace emul
{
	static const size_t CPU6809_ADDRESS_BITS = 16;
	static const char* CPUID_6809 = "6809";

	class CPU6809 : public CPU, public PortConnector
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

		enum FLAG : BYTE
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

		virtual void Init();

		void Dump();

		virtual void Reset();
		virtual void Reset(ADDRESS overrideAddress);

		virtual bool Step();

		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU6809_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		WORD GetReg(RegCode reg) const;
		bool GetFlag(FLAG f) { return (m_reg.flags & f) ? true : false; }

		// Interrupts
		void SetNMI(bool nmi) { m_nmi.Set(nmi); }
		void SetFIRQ(bool firq) { m_firq = firq; }
		void SetIRQ(bool irq) { m_irq = irq; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		CPU6809(const char* cpuid, Memory& memory);

		void InitPage2();
		void InitPage3();

		inline void TICK1() { ++m_opTicks; }
		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		// Use third timing conditional penalty (2nd value not used)
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }
		inline void TICKINT() { CPU::TICK(m_info.GetMiscTiming(cpuInfo::MiscTiming::TRAP)[0]); }

		// Vectors
		static const ADDRESS _ADDR_RSV  = 0xFFF0; // Reserved by Motorola
		static const ADDRESS ADDR_SWI3  = 0xFFF2; // SWI3 instruction interrupt vector
		static const ADDRESS ADDR_SWI2  = 0xFFF4; // SWI2 instruction interrupt vector
		static const ADDRESS ADDR_FIRQ  = 0xFFF6; // Fast hardware interrupt vector (FIRQ)
		static const ADDRESS ADDR_IRQ   = 0xFFF8; // Hardware interrupt vector (IRQ)
		static const ADDRESS ADDR_SWI   = 0xFFFA; // SWI instruction interrupt vector
		static const ADDRESS ADDR_NMI   = 0xFFFC; // Non-maskable interrupt vector (NMI)
		static const ADDRESS ADDR_RESET = 0xFFFE; // Reset vector

		using OpcodeTable = std::vector<std::function<void()>>;
		OpcodeTable m_opcodes;
		OpcodeTable m_opcodesPage2;
		OpcodeTable m_opcodesPage3;
		void UnknownOpcode();

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		BYTE m_opcode = 0;

		// NMI is edge sensitive
		hscommon::EdgeDetectLatch m_nmi;

		// NMI is disabled on Reset until the S stack pointer is set
		bool m_nmiEnabled = false;

		// IRQ and FIRQ are level sensitive
		bool m_firq = false;
		bool m_irq = false;

		virtual void Interrupt();

		enum class STACK
		{
			S, U
		};

		ADDRESS m_programCounter = 0;

		// Alias when we need a WORD version of the program counter
		WORD& m_PC = *((WORD*)&m_programCounter);

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
			BYTE flags = 0;

			// For invalid destinations;
			BYTE void8;
			WORD void16;
		} m_reg;

		bool IsStackRegister(const WORD& reg) const { return &reg == &m_reg.S; }

		// Flags
		void ClearFlags(BYTE& flags) { flags = 0; }
		void SetFlags(BYTE f) { m_reg.flags = f; }

		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); }
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		// Adjust negative and zero flag
		void AdjustNZ(BYTE val);
		void AdjustNZ(WORD val);

		// Sub Opcode Tables
		void ExecPage2(BYTE opcode);
		void ExecPage3(BYTE opcode);
		void exec(OpcodeTable& table, BYTE opcode);

		// Misc helpers
		ADDRESS GetDirect(BYTE low) { return MakeWord(m_reg.DP, low); }
		ADDRESS GetIndexed(BYTE idx);
		ADDRESS GetExtended() { return FetchWord(); }
		WORD& GetIndexedRegister(BYTE idx);

		virtual BYTE FetchByte() override;
		virtual WORD FetchWord() override;

		SBYTE FetchSignedByte() { return (SBYTE)FetchByte(); }
		SWORD FetchSignedWord() { return (SWORD)FetchWord(); }

		BYTE GetMemDirectByte();
		WORD GetMemDirectWord();

		BYTE GetMemIndexedByte();
		WORD GetMemIndexedWord();

		BYTE GetMemExtendedByte();
		WORD GetMemExtendedWord();

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

		// Opcodes

		void LEA(WORD& dest, bool setZero);

		// Branching
		void BRA(bool condition);
		void LBRA(bool condition);

		void JMP(ADDRESS dest) { m_programCounter = dest; }
		void BSR();
		void LBSR();
		void JSR(ADDRESS dest);
		void RTS();

		void SWI(BYTE swi);
		void RTI();

		// Load
		void LD8(BYTE& dest, BYTE src);
		void LD16(WORD& dest, WORD src);

		// Store
		void ST8(ADDRESS dest, BYTE src);
		void ST16(ADDRESS dest, WORD src);

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

		// Logical
		void ASL(BYTE& dest); // Arithmetic Shift Left
		void ASLm(ADDRESS dest);

		void ASR(BYTE& dest); // Arithmetic Shift Right
		void ASRm(ADDRESS dest);

		void LSR(BYTE& dest); // Logical Shift Right
		void LSRm(ADDRESS dest);

		void ROL(BYTE& dest); // Rotate Left Through Carry
		void ROLm(ADDRESS dest);

		void ROR(BYTE& dest); // Rotate Right Through Carry
		void RORm(ADDRESS dest);

		void EOR(BYTE& dest, BYTE src); // Logical XOR
		void OR(BYTE& dest, BYTE src); // Logical OR
		void AND(BYTE& dest, BYTE src); // Logical AND

		// dest by value so it's not modified
		void BIT(BYTE dest, BYTE src) { return AND(dest, src); }

		void CLR(BYTE& dest); // Clear
		void CLRm(ADDRESS dest);

		void COM(BYTE& dest); // Complement
		void COMm(ADDRESS dest);

		void INC(BYTE& dest); // Increment
		void INCm(ADDRESS dest);

		void DEC(BYTE& dest); // Decrement
		void DECm(ADDRESS dest);

		void TST(const BYTE dest); // Sets N & Z flags

		void SEX(); // Sign Extend B to D

		// Arithmetic

		// dest' <- dest + src (+ carry)
		void ADD8(BYTE& dest, BYTE src, bool carry = false);
		void ADD16(WORD& dest, WORD src, bool carry = false);

		// dest' <- dest - src (- borrow)
		void SUB8(BYTE& dest, BYTE src, bool borrow = false);
		void SUB16(WORD& dest, WORD src, bool borrow = false);

		// dest by value so it's not modified
		// (void) <- dest - src
		void CMP8(BYTE dest, BYTE src) { return SUB8(dest, src); }
		void CMP16(WORD dest, WORD src) { return SUB16(dest, src); }

		// Negate operand, dest' <- (0 - dest)
		void NEG(BYTE& dest);
		void NEGm(ADDRESS dest);

		void MUL(); // D' = A * B

		friend class Monitor6809;
	};
}
