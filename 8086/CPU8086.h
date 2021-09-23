#pragma once

#include "CPU.h"
#include "PortConnector.h"
#include "PortAggregator.h"
#include <tuple>
#include <assert.h>

namespace emul
{
	static const size_t CPU8086_ADDRESS_BITS = 20;

	#pragma pack(push, 1)
	union Register
	{
		WORD x = 0;
		struct {
			BYTE l;
			BYTE h;
		} hl;
	};
	#pragma pack(pop)

	struct SourceDest8
	{
		SourceDest8() {}
		SourceDest8(BYTE* d, BYTE* s) : dest(d), source(s) {}
		BYTE* source = nullptr;
		BYTE* dest = nullptr;
	};

	struct SourceDest16
	{
		SourceDest16() {}
		SourceDest16(WORD* d, WORD* s) : dest(d), source(s) {}
		WORD* source = nullptr;
		WORD* dest = nullptr;
	};

	typedef WORD(*RawOpFunc8)(BYTE& dest, const BYTE& src, bool);
	typedef DWORD(*RawOpFunc16)(WORD& dest, const WORD& src, bool);

	typedef std::tuple<WORD, WORD> SegmentOffset;

	class CPU8086 : public CPU
	{
	public:
		CPU8086(Memory& memory, MemoryMap& mmap);
		virtual ~CPU8086();

		virtual size_t GetAddressBits() const { return CPU8086_ADDRESS_BITS; }
		virtual ADDRESS GetCurrentAddress() const { return S2A(regCS, regIP); }

		virtual bool Step() override;

		virtual void Exec(BYTE opcode);

		void AddDevice(PortConnector& ports);

		void Dump();
		void DumpInterruptTable();

		virtual void Reset();
		virtual void Reset(WORD segment, WORD offset);

		void Interrupt(BYTE interrupt) { assert(!inSegOverride); TICK(61); INT(interrupt); }
		bool CanInterrupt() 
		{ 
			return GetFlag(FLAG::FLAG_I) && (m_lastOp != 0xFB);
		}

		// General Registers
		Register regA; // Accumulator
		Register regB; // Base
		Register regC; // Count
		Register regD; // Data

		WORD regSP = 0; // Stack Pointer
		WORD regBP = 0; // Base Pointer
		WORD regSI = 0; // Source Index
		WORD regDI = 0; // Destination Index

		// Segment Registers
		WORD regCS = 0; // Code Segment
		WORD regDS = 0; // Data Segment
		WORD regSS = 0; // Stack Segment
		WORD regES = 0; // Extra Segment

		WORD regIP = 0; // Instruction Pointer

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
			FLAG_R12 = 0x1000, // Reserved, 0
			FLAG_R13 = 0x2000, // Reserved, 0
			FLAG_R14 = 0x4000, // Reserved, 0
			FLAG_R15 = 0x8000, // Reserved, 0
		};

		WORD flags = 0;

		void ClearFlags();
		bool GetFlag(FLAG f) { return (flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { if (v) flags |= f; else flags &= ~f; };

		static const char* GetReg8Str(BYTE reg);
		static const char* GetReg16Str(BYTE reg, bool segReg = false);
		static std::string GetModRMStr(BYTE modrm, bool wide, BYTE& disp);

	protected:
		inline void TICKRM(uint32_t r, uint32_t m) { m_opTicks += (m_regMem == REGMEM::REG) ? r : m; };

		BYTE m_lastOp = 0;

		PortAggregator m_ports;

		// Pseudo flags
		// ----------
		bool GetFlagNotAbove() { return GetFlag(FLAG_C) || GetFlag(FLAG_Z); }
		bool GetFlagNotLess() { return GetFlag(FLAG_S) == GetFlag(FLAG_O); }
		bool GetFlagGreater() { return (GetFlag(FLAG_S) == GetFlag(FLAG_O)) && !GetFlag(FLAG_Z); }

		// REP flags
		bool inRep = false;
		WORD repIP = 0;
		bool repZ = false;

		// segment Override
		bool inSegOverride = false;
		WORD segOverride = 0;

		// Helper functions

		void AdjustParity(BYTE data);
		void AdjustSign(BYTE data);
		void AdjustZero(BYTE data);

		void AdjustParity(WORD data);
		void AdjustSign(WORD data);
		void AdjustZero(WORD data);

		void IndexIncDec(WORD& idx) { GetFlag(FLAG_D) ? --idx : ++idx; }

		BYTE FetchByte();
		WORD FetchWord();

		BYTE* GetModRM8(BYTE modrm);
		WORD* GetModRM16(BYTE modrm);
		enum class REGMEM { REG, MEM } m_regMem;

		SourceDest8 GetModRegRM8(BYTE modregrm, bool toReg = true);
		SourceDest16 GetModRegRM16(BYTE modregrm, bool toReg = true, bool segReg = false);

		SegmentOffset GetEA(BYTE modregrm, bool direct);
		static const char* GetEAStr(BYTE modregrm, bool direct);

		BYTE* GetReg8(BYTE reg);
		WORD* GetReg16(BYTE reg, bool segReg = false);

		// Opcodes

		void CALLfar();
		void CALLNear(WORD offset);
		void CALLIntra(WORD address);
		void CALLInter(WORD* destPtr);

		void JMPfar();
		void JMPNear(BYTE offset);
		void JMPNear(WORD offset);
		void JMPIntra(WORD address);
		void JMPInter(WORD* destPtr);

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
		void INC8(BYTE&);
		void DEC8(BYTE&);

		void INC16(WORD&);
		void DEC16(WORD&);

		void MOV8(BYTE* d, BYTE s);
		void MOV16(WORD* d, WORD s);

		void MOV8(SourceDest8 sd);
		void MOV16(SourceDest16 sd);

		void MOVIMM8(BYTE* dest);
		void MOVIMM16(WORD* dest);

		void SAHF();
		void LAHF();

		void JMPif(bool cond);

		void SHIFTROT8(BYTE op2, BYTE count);
		void SHIFTROT16(BYTE op2, BYTE count);

		void Arithmetic8(SourceDest8 sd, RawOpFunc8 func);
		void Arithmetic16(SourceDest16 sd, RawOpFunc16 func);
		void ArithmeticImm8(BYTE& dest, BYTE imm, RawOpFunc8 func);
		void ArithmeticImm16(WORD& dest, WORD imm, RawOpFunc16 func);

		void ArithmeticImm8(BYTE op2);
		void ArithmeticImm16(BYTE op2, bool signExtend);

		void ArithmeticMulti8(BYTE op2);
		void ArithmeticMulti16(BYTE op2);

		void IN8(WORD port);
		void OUT8(WORD port);
		void OUT16(WORD port);

		void LOOP(BYTE offset, bool cond = true);

		void RETNear(bool pop = false, WORD value = 0);
		void RETFar(bool pop = false, WORD value = 0);

		void XCHG8(SourceDest8 sd);
		void XCHG8(BYTE& b1, BYTE& b2);

		void XCHG16(SourceDest16 sd);
		void XCHG16(WORD& w1, WORD& w2);

		void PUSH(WORD& w);
		void POP(WORD& w);

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

		void SEGOVERRIDE(WORD);

		void INT(BYTE interrupt);

		void IRET();

		void MultiFunc(BYTE op2);

		void LoadPTR(WORD& destSegment, SourceDest16 modRegRm);

		void XLAT();

		void AAA();
		void AAS();
		void AAM(BYTE base);
		void AAD(BYTE base);
		void DAA();
		void DAS();

		void LEA(BYTE op2);

		void NotImplemented(BYTE);
	};
}
