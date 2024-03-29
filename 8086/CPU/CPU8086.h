#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/CPUInfo.h>
#include <CPU/PortConnector.h>
#include "CPUException.h"
#include <tuple>
#include <assert.h>

namespace emul
{
	static const size_t CPU8086_ADDRESS_BITS = 20;

	class CPU8086;

	enum class REG16
	{
		// Indices in WORD array of memblock
		INVALID = 0,

		AX = 1,       // Accumulator
		BX = 2,       // Base
		CX = 3,       // Count
		DX = 4,       // Data

		SP = 5,       // Stack Pointer
		BP = 6,       // Base Pointer
		SI = 7,       // Source Index
		DI = 8,       // Destination Index

		IP = 9,       // Instruction Pointer

		FLAGS = 10,   // Flags

		// Pseudo-Registers
		_REP_IP = 11, // IP in REP
		_T0     = 12, // Temp register

		// Segment Registers
		// We leave ample space (16 bytes - 8 words)
		// in between for 80286 register cache data
		// This way, the segment descriptors match with
		// the legacy segment value
		CS = 16,      // Code Segment
		DS = 24,      // Data Segment
		SS = 32,      // Stack Segment
		ES = 40,      // Extra Segment
	};

	enum class SEGREG
	{
		CS = (int)REG16::CS,
		DS = (int)REG16::DS,
		SS = (int)REG16::SS,
		ES = (int)REG16::ES
	};

	enum class REG8
	{
		// Indices in BYTE array of memblock (to align, AL must be 2*AX)
		INVALID = 0,

		AL = ((int)REG16::AX * 2), AH, // Accumulator
		BL = ((int)REG16::BX * 2), BH, // Base
		CL = ((int)REG16::CX * 2), CH, // Count
		DL = ((int)REG16::DX * 2), DH, // Data

		_T0 = ((int)REG16::_T0 * 2), // Temp register
	};

	class Registers : public MemoryBlock
	{
	public:
		Registers() : MemoryBlock("REG", 128) { Clear(0xFF); }

		BYTE& Get8(REG8 r8) { return m_data[(int)r8]; }
		BYTE Read8(REG8 r8) const { return m_data[(int)r8]; }
		void Write8(REG8 r8, BYTE value) { m_data[(int)r8] = value; }

		WORD& Get16(REG16 r16) { return ((WORD*)m_data)[(int)r16]; }
		WORD Read16(REG16 r16) const { return ((WORD*)m_data)[(int)r16]; }
		void Write16(REG16 r16, WORD value) { ((WORD*)m_data)[(int)r16] = value; }

		BYTE operator[](REG8 r8) const { return Read8(r8); }
		BYTE& operator[](REG8 r8) { return Get8(r8); }

		WORD operator[](REG16 r8) const { return Read16(r8); }
		WORD& operator[](REG16 r8) { return Get16(r8); }

		void* GetRawPtr16(size_t offset) const { return (WORD*)m_data + offset; }
	};

	struct RawSegmentOffset
	{
		WORD segment = 0;
		WORD offset = 0;

		// Decodes from "xxxx:yyyy" string
		bool FromString(const char*);
		const char* ToString() const;
	};

	struct SegmentOffset
	{
		SegmentOffset() {}
		SegmentOffset(SEGREG seg, WORD off = 0) : segment(seg), offset(off) {}

		SEGREG segment = SEGREG::CS;
		WORD offset = 0;
	};

	class Mem8
	{
	public:
		Mem8() {}
		Mem8(const SegmentOffset& segoff, bool high = false) : m_segOff(segoff) { if (high) Increment(); }
		Mem8(REG8 r8) : m_reg8(r8) {}

		BYTE Read() const;
		void Write(BYTE value);

		static void Init(CPU8086* cpu, Memory* m, Registers* r) { m_cpu = cpu; m_memory = m; m_registers = r; }

		bool IsRegister() const
		{
			return m_reg8 != REG8::INVALID;
		}

		void Increment(int i = 1)
		{
			m_segOff.offset += i;
		}

		WORD GetOffset() const
		{
			if (!IsRegister())
			{
				return m_segOff.offset;
			}
			else
			{
				// We shouldn't get there. This means that the operation needs a
				// memory operand instead of a register, so this is an invalid opcode
				throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
			}
		}

	protected:
		static CPU8086* m_cpu;
		static Memory* m_memory;
		static Registers* m_registers;

		SegmentOffset m_segOff;
		REG8 m_reg8 = REG8::INVALID;
	};

	struct SourceDest8
	{
		Mem8 source;
		Mem8 dest;
	};

	class Mem16
	{
	public:
		Mem16() {}
		Mem16(const SegmentOffset& segoff) : l(segoff), h(segoff, true)
		{
			if (m_checkAlignment && (segoff.offset == 0xFFFF))
			{
				throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
			}
		}
		Mem16(REG16 r16) : m_reg16(r16), l((REG8)((int)r16 * 2)), h((REG8)((int)r16 * 2 + 1)) {}

		WORD GetOffset() const
		{
			return l.GetOffset();
		}

		bool IsRegister() const { return m_reg16 != REG16::INVALID; }
		REG16 GetRegister() const { return m_reg16; }

		void Increment()
		{
			if (!IsRegister())
			{
				l.Increment(2);
				h.Increment(2);
			}
			else
			{
				// We shouldn't get there. This means that the operation needs a
				// memory operand instead of a register, so this is an invalid opcode
				throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
			}
		}

		WORD Read() const { return MakeWord(h.Read(), l.Read()); }
		void Write(WORD value) { l.Write(GetLByte(value)); h.Write(GetHByte(value)); }

		static void Init(Memory* m, Registers* r) { m_memory = m; m_registers = r; m_checkAlignment; }
		static void CheckAlignment(bool check) { m_checkAlignment = check; }

	protected:
		static Memory* m_memory;
		static Registers* m_registers;
		static bool m_checkAlignment;

		REG16 m_reg16 = REG16::INVALID;
		Mem8 h;
		Mem8 l;
	};

	struct SourceDest16
	{
		Mem16 source;
		Mem16 dest;
	};

	enum class MemAccess { NONE, READ, WRITE };

	typedef WORD(*RawOpFunc8)(BYTE& dest, const BYTE src, bool);
	typedef DWORD(*RawOpFunc16)(WORD& dest, const WORD src, bool);

	class CPU8086 : public CPU, public PortConnector
	{
	public:
		CPU8086(Memory& memory);
		virtual ~CPU8086();

		virtual void Init();

		virtual const std::string GetID() const override { return m_info.GetId(); }
		virtual size_t GetAddressBits() const { return CPU8086_ADDRESS_BITS; }
		virtual ADDRESS GetAddress(SegmentOffset segoff, MemAccess = MemAccess::NONE) const { return S2A(GetRegValue(segoff.segment), segoff.offset); }
		virtual ADDRESS GetCurrentAddress() const { return S2A(m_reg[REG16::CS], m_reg[REG16::IP]); }

		virtual bool Step() override;

		virtual void Exec(BYTE opcode);

		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

		virtual void Reset();
		virtual void Reset(WORD segment, WORD offset);

		void Interrupt(BYTE irq) { m_irqPending = irq; }
		bool CanInterrupt()
		{
			if (m_opcode == 0x17)
			{
				// POP SS inhibits all interrupts, including NMI, until after
				// the execution of the next instruction. This permits a POP SP
				// instruction to be performed first.
				// (Possibly 286+ but can't harm)
				return false;
			}

			// TODO: last check only if interrupts were disabled previously
			return GetFlag(FLAG::FLAG_I) && (m_opcode != 0xFB);
		}

		Registers m_reg;

		enum FLAG : WORD
		{
			FLAG_C = 0x0001, // Carry
			FLAG_P = 0x0004, // Parity
			FLAG_A = 0x0010, // Auxiliary Carry
			FLAG_Z = 0x0040, // Zero
			FLAG_S = 0x0080, // Sign
			FLAG_T = 0x0100, // Trap
			FLAG_I = 0x0200, // Interrupt Enable
			FLAG_D = 0x0400, // Direction
			FLAG_O = 0x0800, // Overflow

			FLAG_R1 = 0x0002,  // Reserved, 1
			FLAG_R3 = 0x0008,  // Reserved, 0
			FLAG_R5 = 0x0020,  // Reserved, 0
			FLAG_R12 = 0x1000, // Reserved, 1
			FLAG_R13 = 0x2000, // Reserved, 1
			FLAG_R14 = 0x4000, // Reserved, 1
			FLAG_R15 = 0x8000, // Reserved, 1
		};

		FLAG FLAG_RESERVED_ON = FLAG(FLAG_R1 | FLAG_R12 | FLAG_R13 | FLAG_R14 | FLAG_R15);
		FLAG FLAG_RESERVED_OFF = FLAG(FLAG_R3 | FLAG_R5);

		virtual void ClearFlags();
		virtual void SetFlags(WORD flags);

		bool GetFlag(FLAG f) { return (m_reg[REG16::FLAGS] & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg[REG16::FLAGS], f, v); };

		static const char* GetReg8Str(BYTE reg);
		static const char* GetReg16Str(BYTE reg, bool segReg = false);
		static std::string GetModRMStr(BYTE modrm, bool wide, BYTE& disp);

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		virtual WORD GetRegValue(SEGREG segreg) const { return m_reg[(REG16)segreg]; }
		WORD GetRegValue(REG16 reg) const { return m_reg[reg]; }
		BYTE GetRegValue(REG8 reg) const { return m_reg[reg]; }

	protected:
		CPU8086(const char* cpuid, Memory& memory);

		// Fetch the timing for the reg (base) or mem variant.
		// If not applicable, this has no impact since timing[base] == timing[mem]
		// (This happens when the timing values are loaded: if no mem timing, we copy the base timing)
		inline void TICK() { m_opTicks += (*m_currTiming)[(int)m_regMem]; };
		inline void TICKMISC(cpuInfo::MiscTiming misc) { CPU::TICK(m_info.GetMiscTiming(misc)[0]); }
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }
		inline void TICKT4() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T4]); }

		std::vector<std::function<void()>> m_opcodes;

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		BYTE m_opcode = 0;
		int m_irqPending = -1;

		// Pseudo flags
		// ----------
		bool GetFlagNotAbove() { return GetFlag(FLAG_C) || GetFlag(FLAG_Z); }
		bool GetFlagNotLess() { return GetFlag(FLAG_S) == GetFlag(FLAG_O); }
		bool GetFlagGreater() { return (GetFlag(FLAG_S) == GetFlag(FLAG_O)) && !GetFlag(FLAG_Z); }

		// REP flags
		bool inRep = false;
		bool repZ = false;

		// segment Override
		bool inSegOverride = false;
		SEGREG segOverride = SEGREG::DS;

		// Helper functions

		void AdjustParity(BYTE data);
		void AdjustSign(BYTE data);
		void AdjustZero(BYTE data);

		void AdjustParity(WORD data);
		void AdjustSign(WORD data);
		void AdjustZero(WORD data);

		void IndexIncDec(WORD& idx) { GetFlag(FLAG_D) ? --idx : ++idx; }

		virtual BYTE FetchByte() override;
		virtual WORD FetchWord() override;

		Mem8 GetModRM8(BYTE modrm);
		Mem16 GetModRM16(BYTE modrm);
		enum class REGMEM { REG = 0, MEM = 1} m_regMem = REGMEM::REG;

		SourceDest8 GetModRegRM8(BYTE modregrm, bool toReg = true);
		SourceDest16 GetModRegRM16(BYTE modregrm, bool toReg = true, bool segReg = false);

		SegmentOffset GetEA(BYTE modregrm, bool direct);
		static const char* GetEAStr(BYTE modregrm, bool direct);

		Mem8 GetReg8(BYTE reg);
		Mem16 GetReg16(BYTE reg, bool segReg = false);

		// Exceptions
		virtual void CPUExceptionHandler(CPUException e);

		// Opcodes
		void NotImplemented();
		virtual void InvalidOpcode();

		virtual void CALLfar();
		void CALLNear(WORD offset);
		void CALLIntra(WORD address);
		virtual void CALLInter(Mem16 destPtr);

		virtual void JMPfar();
		void JMPNear(BYTE offset);
		void JMPNear(WORD offset);
		void JMPIntra(WORD address);
		virtual void JMPInter(Mem16 destPtr);

		void CLC();
		void CMC();
		void STC();
		void CLD();
		void STD();
		void CLI();
		void STI();

		void CBW();
		void CWD();

		void HLT();

		void INCDEC8(BYTE op2);
		void INC8(Mem8);
		void DEC8(Mem8);

		void INC16(WORD&);
		void DEC16(WORD&);

		void MOV8(Mem8 d, BYTE s);
		void MOV16(Mem16 d, WORD s);

		void MOV8(SourceDest8 sd);
		void MOV16(SourceDest16 sd);

		void MOVIMM8(Mem8 dest);
		void MOVIMM16(Mem16 dest);

		void SAHF();
		void LAHF();

		void JMPif(bool cond);

		BYTE _SHIFTROT8(BYTE in, BYTE op2, BYTE count);
		WORD _SHIFTROT16(WORD in, BYTE op2, BYTE count);

		void SHIFTROT8One(BYTE op2);
		void SHIFTROT16One(BYTE op2);

		void SHIFTROT8Multi(BYTE op2, BYTE mask = 0xFF);
		void SHIFTROT16Multi(BYTE op2, BYTE mask = 0xFF);

		void Arithmetic8(SourceDest8 sd, RawOpFunc8 func);
		void Arithmetic16(SourceDest16 sd, RawOpFunc16 func);
		void ArithmeticImm8(Mem8 dest, BYTE imm, RawOpFunc8 func);
		void ArithmeticImm16(Mem16 dest, WORD imm, RawOpFunc16 func);

		void ArithmeticMulti8Imm(BYTE op2);
		void ArithmeticMulti16Imm(BYTE op2, bool signExtend);

		void ArithmeticMulti8(BYTE op2);
		void ArithmeticMulti16(BYTE op2);

		void IN8(WORD port);
		void IN16(WORD port);
		void OUT8(WORD port);
		void OUT16(WORD port);

		void LOOP(BYTE offset, bool cond = true);

		void RETNear(bool pop = false, WORD value = 0);
		virtual void RETFar(bool pop = false, WORD value = 0);

		void XCHG8(SourceDest8 sd);
		void XCHG8(BYTE& b1, BYTE& b2);

		void XCHG16(SourceDest16 sd);
		void XCHG16(WORD& w1, WORD& w2);

		void PUSH(WORD w);
		void PUSH(Mem16 w);
		void POP(Mem16 w);
		WORD POP();

		void PUSHF();
		void POPF();

		void LODS8();
		void LODS16();

		void STOS8();
		void STOS16();

		void SCAS8();
		void SCAS16();

		void MOVS8();
		void MOVS16();

		void CMPS8();
		void CMPS16();

		void REP(bool z);
		bool PreREP();
		void PostREP(bool checkZ);

		void SEGOVERRIDE(SEGREG);

		virtual void INT(BYTE interrupt);
		virtual void IRET();

		void MultiFunc(BYTE op2);

		virtual void LoadPTR(SEGREG dest, SourceDest16 modRegRm);

		void XLAT();

		void AAA();
		void AAS();
		void AAM(BYTE base);
		void AAD(BYTE base);
		void DAA();
		void DAS();

		void LEA(BYTE op2);

		void SALC();
	};
}
