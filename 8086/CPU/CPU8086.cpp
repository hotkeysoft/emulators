#include "stdafx.h"

#include "CPU8086.h"

using cpuInfo::Opcode;
using cpuInfo::MiscTiming;
using cpuInfo::CPUType;
using cpuInfo::OpcodeTimingType;

namespace emul
{
	WORD rawAdd8(BYTE& dest, const BYTE src, bool) { WORD r = dest + src; dest = (BYTE)r; return r; }
	WORD rawSub8(BYTE& dest, const BYTE src, bool) { WORD r = dest - src; dest = (BYTE)r; return r; }
	WORD rawCmp8(BYTE& dest, const BYTE src, bool) { return dest - src; }
	WORD rawAdc8(BYTE& dest, const BYTE src, bool c) { WORD r = dest + src + (BYTE)c; dest = (BYTE)r; return r; }
	WORD rawSbb8(BYTE& dest, const BYTE src, bool b) { WORD r = dest - src - (BYTE)b; dest = (BYTE)r; return r; }

	WORD rawAnd8(BYTE& dest, const BYTE src, bool) { dest &= src; return dest; }
	WORD rawOr8(BYTE& dest, const BYTE src, bool) { dest |= src; return dest; }
	WORD rawXor8(BYTE& dest, const BYTE src, bool) { dest ^= src; return dest; }
	WORD rawTest8(BYTE& dest, const BYTE src, bool) { return dest & src; }

	DWORD rawAdd16(WORD& dest, const WORD src, bool) { DWORD r = dest + src; dest = (WORD)r; return r; }
	DWORD rawSub16(WORD& dest, const WORD src, bool) { DWORD r = dest - src; dest = (WORD)r; return r; }
	DWORD rawCmp16(WORD& dest, const WORD src, bool) { return dest - src; }
	DWORD rawAdc16(WORD& dest, const WORD src, bool c) { DWORD r = dest + src + WORD(c); dest = (WORD)r; return r; }
	DWORD rawSbb16(WORD& dest, const WORD src, bool b) { DWORD r = dest - src - WORD(b); dest = (WORD)r; return r; }

	DWORD rawAnd16(WORD& dest, const WORD src, bool) { dest &= src; return dest; }
	DWORD rawOr16(WORD& dest, const WORD src, bool) { dest |= src; return dest; }
	DWORD rawXor16(WORD& dest, const WORD src, bool) { dest ^= src; return dest; }
	DWORD rawTest16(WORD& dest, const WORD src, bool) { return dest & src; }

	Memory* Mem8::m_memory = nullptr;
	Registers* Mem8::m_registers = nullptr;

	Memory* Mem16::m_memory = nullptr;
	Registers* Mem16::m_registers = nullptr;

	BYTE GetOP2(BYTE op2) { return (op2 >> 3) & 7; }

	CPU8086::CPU8086(Memory& memory) : CPU8086(CPUType::i80186, memory)
	{
	}

	CPU8086::CPU8086(cpuInfo::CPUType type, Memory& memory) : 
		CPU(CPU8086_ADDRESS_BITS, memory), 
		m_info(type),
		Logger("CPU8086")
	{
		Mem8::Init(&memory, &m_reg);
		Mem16::Init(&memory, &m_reg);

		try
		{
			m_info.LoadConfig();
		}
		catch (nlohmann::detail::exception e)
		{
			LogPrintf(LOG_ERROR, "Fatal json error loading config: %s\n", e.what());
			throw;
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "Fatal error loading config: %s\n", e.what());
			throw;
		}
	}

	CPU8086::~CPU8086()
	{

	}

	void CPU8086::Init()
	{
		m_opcodes.resize(256);
		std::fill(m_opcodes.begin(), m_opcodes.end(), [=]() { InvalidOpcode(); });

		// ADD
		// --------
		// REG8/MEM8, REG8
		m_opcodes[0x00] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawAdd8); };
		// REG16/MEM16, REG16
		m_opcodes[0x01] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawAdd16); };
		// REG8, REG8/MEM8
		m_opcodes[0x02] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawAdd8); };
		// REG16, REG16/MEM16
		m_opcodes[0x03] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawAdd16); };

		// ADD
		// --------
		// AL, IMMED8
		m_opcodes[0x04] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawAdd8); };
		// AX, IMMED16
		m_opcodes[0x05] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawAdd16); };

		// PUSH ES (1)
		m_opcodes[0x06] = [=]() { PUSH(REG16::ES); };
		// POP ES (1)
		m_opcodes[0x07] = [=]() { POP(REG16::ES); };

		// OR
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x08] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawOr8); };
		// REG16/MEM16, REG16
		m_opcodes[0x09] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawOr16); };
		// REG8, REG8/MEM8
		m_opcodes[0x0A] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawOr8); };
		// REG16, REG16/MEM16
		m_opcodes[0x0B] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawOr16); };

		// OR
		// ----------
		// AL, IMMED8
		m_opcodes[0x0C] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawOr8); };
		// AX, IMMED16
		m_opcodes[0x0D] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawOr16); };

		// PUSH CS
		m_opcodes[0x0E] = [=]() { PUSH(REG16::CS); };
		// POP CS // Undocumented, 8086 only
		m_opcodes[0x0F] = [=]() { POP(REG16::CS); };

		// ADC
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x10] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawAdc8); };
		// REG16/MEM16, REG16
		m_opcodes[0x11] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawAdc16); };
		// REG8, REG8/MEM8
		m_opcodes[0x12] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawAdc8); };
		// REG16, REG16/MEM16
		m_opcodes[0x13] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawAdc16); };


		// ADC
		// ----------
		// AL, IMMED8
		m_opcodes[0x14] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawAdc8); };
		// AX, IMMED16
		m_opcodes[0x15] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawAdc16); };

		// PUSH SS
		m_opcodes[0x16] = [=]() { PUSH(REG16::SS); };
		// POP SS
		m_opcodes[0x17] = [=]() { POP(REG16::SS); };

		// SBB
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x18] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawSbb8); };
		// REG16/MEM16, REG16
		m_opcodes[0x19] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawSbb16); };
		// REG8, REG8/MEM8
		m_opcodes[0x1A] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawSbb8); };
		// REG16, REG16/MEM16
		m_opcodes[0x1B] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawSbb16); };

		// SBB
		// ----------
		// AL, IMMED8
		m_opcodes[0x1C] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawSbb8); };
		// AX, IMMED16
		m_opcodes[0x1D] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawSbb16); };

		// PUSH DS (1)
		m_opcodes[0x1E] = [=]() { PUSH(REG16::DS); };
		// POP DS
		m_opcodes[0x1F] = [=]() { POP(REG16::DS); };

		// AND
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x20] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawAnd8); };
		// REG16/MEM16, REG16
		m_opcodes[0x21] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawAnd16); };
		// REG8, REG8/MEM8
		m_opcodes[0x22] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawAnd8); };
		// REG16, REG16/MEM16
		m_opcodes[0x23] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawAnd16); };

		// AND
		// ----------
		// AL, IMMED8
		m_opcodes[0x24] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawAnd8); };
		// AX, IMMED16
		m_opcodes[0x25] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawAnd16); };

		// ES Segment Override
		m_opcodes[0x26] = [=]() { SEGOVERRIDE(m_reg[REG16::ES]); };

		// DAA
		m_opcodes[0x27] = [=]() { DAA(); };

		// SUB
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x28] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawSub8); };
		// REG16/MEM16, REG16
		m_opcodes[0x29] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawSub16); };
		// REG8, REG8/MEM8
		m_opcodes[0x2A] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawSub8); };
		// REG16, REG16/MEM16
		m_opcodes[0x2B] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawSub16); };

		// SUB
		// ----------
		// AL, IMMED8
		m_opcodes[0x2C] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawSub8); };
		// AX, IMMED16
		m_opcodes[0x2D] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawSub16); };

		// CS Segment Override
		m_opcodes[0x2E] = [=]() { SEGOVERRIDE(m_reg[REG16::CS]); };

		// DAS
		m_opcodes[0x2F] = [=]() { DAS(); };

		// XOR
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x30] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawXor8); };
		// REG16/MEM16, REG16
		m_opcodes[0x31] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawXor16); };
		// REG8, REG8/MEM8
		m_opcodes[0x32] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawXor8); };
		// REG16, REG16/MEM16
		m_opcodes[0x33] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawXor16); };

		// XOR
		// ----------
		// AL, IMMED8
		m_opcodes[0x34] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawXor8); };
		// AX, IMMED16
		m_opcodes[0x35] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawXor16); };

		// SS Segment Override
		m_opcodes[0x36] = [=]() { SEGOVERRIDE(m_reg[REG16::SS]); };

		// AAA
		m_opcodes[0x37] = [=]() { AAA(); };

		// CMP
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x38] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), false), rawCmp8); };
		// REG16/MEM16, REG16
		m_opcodes[0x39] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), false), rawCmp16); };
		// REG8, REG8/MEM8
		m_opcodes[0x3A] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawCmp8); };
		// REG16, REG16/MEM16
		m_opcodes[0x3B] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawCmp16); };

		// CMP
		// ----------
		// AL, IMMED8
		m_opcodes[0x3C] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawCmp8); };
		// AX, IMMED16
		m_opcodes[0x3D] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawCmp16); };

		// DS Segment Override
		m_opcodes[0x3E] = [=]() { SEGOVERRIDE(m_reg[REG16::DS]); };

		// AAS
		m_opcodes[0x3F] = [=]() { AAS(); };

		// INC
		// ----------
		// INC AX
		m_opcodes[0x40] = [=]() { INC16(m_reg[REG16::AX]); };
		// INC CX
		m_opcodes[0x41] = [=]() { INC16(m_reg[REG16::CX]); };
		// INC DX
		m_opcodes[0x42] = [=]() { INC16(m_reg[REG16::DX]); };
		// INC BX
		m_opcodes[0x43] = [=]() { INC16(m_reg[REG16::BX]); };
		// INC SP
		m_opcodes[0x44] = [=]() { INC16(m_reg[REG16::SP]); };
		// INC BP
		m_opcodes[0x45] = [=]() { INC16(m_reg[REG16::BP]); };
		// INC SI
		m_opcodes[0x46] = [=]() { INC16(m_reg[REG16::SI]); };
		// INC DI
		m_opcodes[0x47] = [=]() { INC16(m_reg[REG16::DI]); };

		// DEC
		// ----------
		// DEC AX
		m_opcodes[0x48] = [=]() { DEC16(m_reg[REG16::AX]); };
		// DEC CX
		m_opcodes[0x49] = [=]() { DEC16(m_reg[REG16::CX]); };
		// DEC DX
		m_opcodes[0x4A] = [=]() { DEC16(m_reg[REG16::DX]); };
		// DEC BX
		m_opcodes[0x4B] = [=]() { DEC16(m_reg[REG16::BX]); };
		// DEC SP
		m_opcodes[0x4C] = [=]() { DEC16(m_reg[REG16::SP]); };
		// DEC BP
		m_opcodes[0x4D] = [=]() { DEC16(m_reg[REG16::BP]); };
		// DEC SI
		m_opcodes[0x4E] = [=]() { DEC16(m_reg[REG16::SI]); };
		// DEC DI
		m_opcodes[0x4F] = [=]() { DEC16(m_reg[REG16::DI]); };

		// PUSH
		// ----------
		// PUSH AX
		m_opcodes[0x50] = [=]() { PUSH(REG16::AX); };
		// PUSH CX
		m_opcodes[0x51] = [=]() { PUSH(REG16::CX); };
		// PUSH DX
		m_opcodes[0x52] = [=]() { PUSH(REG16::DX); };
		// PUSH BX
		m_opcodes[0x53] = [=]() { PUSH(REG16::BX); };
		// PUSH SP
		// "Bug" on 8086/80186 where push sp pushes an already-decremented value
		m_opcodes[0x54] = [=]() { PUSH(m_reg[REG16::SP] - 2); };
		// PUSH BP
		m_opcodes[0x55] = [=]() { PUSH(REG16::BP); };
		// PUSH SI
		m_opcodes[0x56] = [=]() { PUSH(REG16::SI); };
		// PUSH DI
		m_opcodes[0x57] = [=]() { PUSH(REG16::DI); };

		// POP
		// ----------
		// POP AX
		m_opcodes[0x58] = [=]() { POP(REG16::AX); };
		// POP CX
		m_opcodes[0x59] = [=]() { POP(REG16::CX); };
		// POP DX
		m_opcodes[0x5A] = [=]() { POP(REG16::DX); };
		// POP BX
		m_opcodes[0x5B] = [=]() { POP(REG16::BX); };
		// POP SP
		m_opcodes[0x5C] = [=]() { POP(REG16::SP); };
		// POP BP
		m_opcodes[0x5D] = [=]() { POP(REG16::BP); };
		// POP SI
		m_opcodes[0x5E] = [=]() { POP(REG16::SI); };
		// POP DI
		m_opcodes[0x5F] = [=]() { POP(REG16::DI); };

		// Undocumented: 0x60-0x6F maps to 0x70-0x7F on 8086 only
		// JO
		m_opcodes[0x60] = [=]() { JMPif(GetFlag(FLAG_O)); };
		m_opcodes[0x70] = [=]() { JMPif(GetFlag(FLAG_O)); };
		// JNO
		m_opcodes[0x61] = [=]() { JMPif(!GetFlag(FLAG_O)); };
		m_opcodes[0x71] = [=]() { JMPif(!GetFlag(FLAG_O)); };
		// JB/JNAE/JC
		m_opcodes[0x62] = [=]() { JMPif(GetFlag(FLAG_C)); };
		m_opcodes[0x72] = [=]() { JMPif(GetFlag(FLAG_C)); };
		// JNB/JAE/JNC
		m_opcodes[0x63] = [=]() { JMPif(!GetFlag(FLAG_C)); };
		m_opcodes[0x73] = [=]() { JMPif(!GetFlag(FLAG_C)); };
		// JE/JZ
		m_opcodes[0x64] = [=]() { JMPif(GetFlag(FLAG_Z)); };
		m_opcodes[0x74] = [=]() { JMPif(GetFlag(FLAG_Z)); };
		// JNE/JNZ
		m_opcodes[0x65] = [=]() { JMPif(!GetFlag(FLAG_Z)); };
		m_opcodes[0x75] = [=]() { JMPif(!GetFlag(FLAG_Z)); };
		// JBE/JNA
		m_opcodes[0x66] = [=]() { JMPif(GetFlagNotAbove()); };
		m_opcodes[0x76] = [=]() { JMPif(GetFlagNotAbove()); };
		// JNBE/JA
		m_opcodes[0x67] = [=]() { JMPif(!GetFlagNotAbove()); };
		m_opcodes[0x77] = [=]() { JMPif(!GetFlagNotAbove()); };
		// JS
		m_opcodes[0x68] = [=]() { JMPif(GetFlag(FLAG_S)); };
		m_opcodes[0x78] = [=]() { JMPif(GetFlag(FLAG_S)); };
		// JNS
		m_opcodes[0x69] = [=]() { JMPif(!GetFlag(FLAG_S)); };
		m_opcodes[0x79] = [=]() { JMPif(!GetFlag(FLAG_S)); };
		// JP/JPE
		m_opcodes[0x6A] = [=]() { JMPif(GetFlag(FLAG_P)); };
		m_opcodes[0x7A] = [=]() { JMPif(GetFlag(FLAG_P)); };
		// JNP/JPO
		m_opcodes[0x6B] = [=]() { JMPif(!GetFlag(FLAG_P)); };
		m_opcodes[0x7B] = [=]() { JMPif(!GetFlag(FLAG_P)); };
		// JL/JNGE
		m_opcodes[0x6C] = [=]() { JMPif(!GetFlagNotLess()); };
		m_opcodes[0x7C] = [=]() { JMPif(!GetFlagNotLess()); };
		// JNL/JGE
		m_opcodes[0x6D] = [=]() { JMPif(GetFlagNotLess()); };
		m_opcodes[0x7D] = [=]() { JMPif(GetFlagNotLess()); };
		// JLE/JNG
		m_opcodes[0x6E] = [=]() { JMPif(!GetFlagGreater()); };
		m_opcodes[0x7E] = [=]() { JMPif(!GetFlagGreater()); };
		// JNLE/JG
		m_opcodes[0x6F] = [=]() { JMPif(GetFlagGreater()); };
		m_opcodes[0x7F] = [=]() { JMPif(GetFlagGreater()); };

		//----------
		// ADD/OR/ADC/SBB/AND/SUB/XOR/CMP
		// ----------
		// REG8/MEM8, IMM8
		m_opcodes[0x80] = [=]() { ArithmeticMulti8Imm(FetchByte()); };
		// REG16/MEM16, IMM16
		m_opcodes[0x81] = [=]() { ArithmeticMulti16Imm(FetchByte(), false); }; // imm data = word

		// ADD/--/ADC/SBB/---/SUB/---/CMP w/sign Extension
		// ----------
		// REG8/MEM8, IMM8 (same as 0x80)
		m_opcodes[0x82] = [=]() { ArithmeticMulti8Imm(FetchByte()); };
		// REG16/MEM16, IMM8 (sign-extend to 16)
		m_opcodes[0x83] = [=]() { ArithmeticMulti16Imm(FetchByte(), true); }; // imm data = sign-extended byte

		// TEST
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x84] = [=]() { Arithmetic8(GetModRegRM8(FetchByte(), true), rawTest8); };
		// REG16/MEM16, REG16
		m_opcodes[0x85] = [=]() { Arithmetic16(GetModRegRM16(FetchByte(), true), rawTest16); };

		// XCHG
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x86] = [=]() { XCHG8(GetModRegRM8(FetchByte())); };
		// REG16/MEM16, REG16
		m_opcodes[0x87] = [=]() { XCHG16(GetModRegRM16(FetchByte())); };

		// MOV
		// ----------
		// REG8/MEM8, REG8
		m_opcodes[0x88] = [=]() { MOV8(GetModRegRM8(FetchByte(), false)); };
		// REG16/MEM16, REG16
		m_opcodes[0x89] = [=]() { MOV16(GetModRegRM16(FetchByte(), false)); };
		// REG8, REG8/MEM8
		m_opcodes[0x8A] = [=]() { MOV8(GetModRegRM8(FetchByte(), true)); };
		// REG16, REG16/MEM16
		m_opcodes[0x8B] = [=]() { MOV16(GetModRegRM16(FetchByte(), true)); };

		// MOV
		// ----------
		// MOV REG16/MEM16, SEGREG
		m_opcodes[0x8C] = [=]() { MOV16(GetModRegRM16(FetchByte(), false, true)); };

		// LEA
		// ----------
		// REG16, MEM16
		m_opcodes[0x8D] = [=]() { LEA(FetchByte()); };

		// MOV
		// ----------
		// MOV SEGREG, REG16/MEM16
		m_opcodes[0x8E] = [=]() { MOV16(GetModRegRM16(FetchByte(), true, true)); };

		// POP
		// ----------
		// POP REG16/MEM16
		m_opcodes[0x8F] = [=]() { POP(GetModRM16(FetchByte())); };

		// XCHG
		// ----------
		// XCHG AX, AX (NOP)
		m_opcodes[0x90] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::AX]); };
		// XCHG AX, CX
		m_opcodes[0x91] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::CX]); };
		// XCHG AX, DX
		m_opcodes[0x92] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::DX]); };
		// XCHG AX, BX
		m_opcodes[0x93] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::BX]); };
		// XCHG AX, SP
		m_opcodes[0x94] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::SP]); };
		// XCHG AX, BP
		m_opcodes[0x95] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::BP]); };
		// XCHG AX, SI
		m_opcodes[0x96] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::SI]); };
		// XCHG AX, DI
		m_opcodes[0x97] = [=]() { XCHG16(m_reg[REG16::AX], m_reg[REG16::DI]); };

		// CBW
		m_opcodes[0x98] = [=]() { CBW(); };
		// CWD
		m_opcodes[0x99] = [=]() { CWD(); };

		// CALL Far
		m_opcodes[0x9A] = [=]() { CALLfar(); };

		// WAIT
		m_opcodes[0x9B] = [=]() { NotImplemented(); };

		// PUSHF
		m_opcodes[0x9C] = [=]() { PUSHF(); };
		// POPF
		m_opcodes[0x9D] = [=]() { POPF(); };
		// SAHF
		m_opcodes[0x9E] = [=]() { SAHF(); };
		// LAHF
		m_opcodes[0x9F] = [=]() { LAHF(); };

		// MOV
		// ----------
		// MOV AL, MEM8
		m_opcodes[0xA0] = [=]() { MOV8(REG8::AL, m_memory.Read8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord()))); };
		// MOV AX, MEM16
		m_opcodes[0xA1] = [=]() { MOV16(REG16::AX, m_memory.Read16(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord()))); };

		// MOV
		// ----------
		// MOV MEM8, AL
		m_opcodes[0xA2] = [=]() { MOV8(Mem8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord())), m_reg[REG8::AL]); };
		// MOV MEM16, AX
		m_opcodes[0xA3] = [=]() { MOV16(Mem16(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord())), m_reg[REG16::AX]); };

		// MOVS
		// ----------
		// MOVS DEST-STR8, SRC-STR8
		m_opcodes[0xA4] = [=]() { MOVS8(); };
		// MOVS DEST-STR16, SRC-STR16
		m_opcodes[0xA5] = [=]() { MOVS16(); };

		// CMPS
		// ----------
		// CMPS DEST-STR8, SRC-STR8
		m_opcodes[0xA6] = [=]() { CMPS8(); };
		// CMPS DEST-STR16, SRC-STR16
		m_opcodes[0xA7] = [=]() { CMPS16(); };

		// TEST
		// ----------
		// TEST AL, IMM8
		m_opcodes[0xA8] = [=]() { ArithmeticImm8(REG8::AL, FetchByte(), rawTest8); };
		// TEST AX, IMM16
		m_opcodes[0xA9] = [=]() { ArithmeticImm16(REG16::AX, FetchWord(), rawTest16); };

		// STOS
		// ----------
		// STOS DEST-STR8
		m_opcodes[0xAA] = [=]() { STOS8(); };
		// STOS DEST-STR16
		m_opcodes[0xAB] = [=]() { STOS16(); };

		// LODS
		// ----------
		// LODS SRC-STR8
		m_opcodes[0xAC] = [=]() { LODS8(); };
		// LODS SRC-STR16
		m_opcodes[0xAD] = [=]() { LODS16(); };

		// SCAS
		// ----------
		// SCAS DEST-STR8
		m_opcodes[0xAE] = [=]() { SCAS8(); };
		// SCAS DEST-STR16
		m_opcodes[0xAF] = [=]() { SCAS16(); };

		// MOV
		// ----------
		// MOV AL, IMM8
		m_opcodes[0xB0] = [=]() { MOV8(REG8::AL, FetchByte()); };
		// MOV CL, IMM8
		m_opcodes[0xB1] = [=]() { MOV8(REG8::CL, FetchByte()); };
		// MOV DL, IMM8
		m_opcodes[0xB2] = [=]() { MOV8(REG8::DL, FetchByte()); };
		// MOV BL, IMM8
		m_opcodes[0xB3] = [=]() { MOV8(REG8::BL, FetchByte()); };
		// MOV AH, IMM8
		m_opcodes[0xB4] = [=]() { MOV8(REG8::AH, FetchByte()); };
		// MOV CH, IMM8
		m_opcodes[0xB5] = [=]() { MOV8(REG8::CH, FetchByte()); };
		// MOV DH, IMM8
		m_opcodes[0xB6] = [=]() { MOV8(REG8::DH, FetchByte()); };
		// MOV BH, IMM8
		m_opcodes[0xB7] = [=]() { MOV8(REG8::BH, FetchByte()); };

		// MOV AX, IMM16
		m_opcodes[0xB8] = [=]() { MOV16(REG16::AX, FetchWord()); };
		// MOV CX, IMM16
		m_opcodes[0xB9] = [=]() { MOV16(REG16::CX, FetchWord()); };
		// MOV DX, IMM16
		m_opcodes[0xBA] = [=]() { MOV16(REG16::DX, FetchWord()); };
		// MOV BX, IMM16
		m_opcodes[0xBB] = [=]() { MOV16(REG16::BX, FetchWord()); };
		// MOV SP, IMM16
		m_opcodes[0xBC] = [=]() { MOV16(REG16::SP, FetchWord()); };
		// MOV BP, IMM16
		m_opcodes[0xBD] = [=]() { MOV16(REG16::BP, FetchWord()); };
		// MOV SI, IMM16
		m_opcodes[0xBE] = [=]() { MOV16(REG16::SI, FetchWord()); };
		// MOV DI, IMM16
		m_opcodes[0xBF] = [=]() { MOV16(REG16::DI, FetchWord()); };

		// RET SP+IMM16
		m_opcodes[0xC0] = [=]() { RETNear(true, FetchWord()); }; // Undocumented, 8086 only
		m_opcodes[0xC2] = [=]() { RETNear(true, FetchWord()); };
		// RET Near
		m_opcodes[0xC1] = [=]() { RETNear(); }; // Undocumented, 8086 only
		m_opcodes[0xC3] = [=]() { RETNear(); };

		// LES REG16, MEM16
		m_opcodes[0xC4] = [=]() { LoadPTR(m_reg[REG16::ES], GetModRegRM16(FetchByte(), true)); };
		// LDS REG16, MEM16
		m_opcodes[0xC5] = [=]() { LoadPTR(m_reg[REG16::DS], GetModRegRM16(FetchByte(), true)); };

		// MOV
		// ----------
		// MOV MEM8, IMM8
		m_opcodes[0xC6] = [=]() { MOVIMM8(GetModRM8(FetchByte())); };
		// MOV MEM16, IMM16
		m_opcodes[0xC7] = [=]() { MOVIMM16(GetModRM16(FetchByte())); };

		// RET Far SP+IMM16
		m_opcodes[0xC8] = [=]() { RETFar(true, FetchWord()); }; //Undocumented, 8086 only
		m_opcodes[0xCA] = [=]() { RETFar(true, FetchWord()); };
		// RET Far
		m_opcodes[0xC9] = [=]() { RETFar(); }; //Undocumented, 8086 only
		m_opcodes[0xCB] = [=]() { RETFar(); };

		// INT3
		m_opcodes[0xCC] = [=]() { INT(3); };
		// INT IMM8
		m_opcodes[0xCD] = [=]() { INT(FetchByte()); };
		// INTO
		m_opcodes[0xCE] = [=]() { if (GetFlag(FLAG_O)) { INT(4); TICKT3(); } };
		// IRET
		m_opcodes[0xCF] = [=]() { IRET(); };

		// ROL/ROR/RCL/RCR/SAL|SHL/SHR/---/SAR
		// ----------
		// REG8/MEM8, 1
		m_opcodes[0xD0] = [=]() { SHIFTROT8One(FetchByte()); };
		// REG16/MEM16, 1
		m_opcodes[0xD1] = [=]() { SHIFTROT16One(FetchByte()); };
		// REG8/MEM8, CL
		m_opcodes[0xD2] = [=]() { SHIFTROT8Multi(FetchByte()); };
		// REG16/MEM16, CL
		m_opcodes[0xD3] = [=]() { SHIFTROT16Multi(FetchByte()); };

		// AAM
		m_opcodes[0xD4] = [=]() { AAM(FetchByte()); };
		// AAD
		m_opcodes[0xD5] = [=]() { AAD(FetchByte()); };

		// Undocumented, Performs an operation equivalent to SBB AL,AL, but without modifying any flags. 
		// In other words, AL will be set to 0xFF or 0x00, depending on whether CF is set or clear.
		m_opcodes[0xD6] = [=]() { SALC(); };

		// XLAT
		m_opcodes[0xD7] = [=]() { XLAT(); };

		// ESC
		m_opcodes[0xD8] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xD9] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xDA] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xDB] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xDC] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xDD] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xDE] = [=]() { GetModRegRM16(FetchByte()); };
		m_opcodes[0xDF] = [=]() { GetModRegRM16(FetchByte()); };

		// LOOPNZ/LOOPNE
		m_opcodes[0xE0] = [=]() { LOOP(FetchByte(), GetFlag(FLAG_Z) == false); };
		// LOOPZ/LOOPE
		m_opcodes[0xE1] = [=]() { LOOP(FetchByte(), GetFlag(FLAG_Z) == true); };
		// LOOP
		m_opcodes[0xE2] = [=]() { LOOP(FetchByte()); };
		// JCXZ
		m_opcodes[0xE3] = [=]() { JMPif(m_reg[REG16::CX] == 0); };

		// IN fixed
		// --------
		// IN AL, IMM8
		m_opcodes[0xE4] = [=]() { IN8(FetchByte()); };
		// IN AX, IMM16
		m_opcodes[0xE5] = [=]() { IN16(FetchByte()); };

		// OUT fixed
		// --------
		// OUT PORT8, AL
		m_opcodes[0xE6] = [=]() { OUT8(FetchByte()); };
		// OUT PORT8, AX
		m_opcodes[0xE7] = [=]() { OUT16(FetchByte()); };

		// CALL Near
		m_opcodes[0xE8] = [=]() { CALLNear(FetchWord()); };
		// JUMP Near
		m_opcodes[0xE9] = [=]() { JMPNear(FetchWord()); };
		// JUMP Far
		m_opcodes[0xEA] = [=]() { JMPfar(); };
		// JUMP Near Short
		m_opcodes[0xEB] = [=]() { JMPNear(FetchByte()); };

		// IN variable
		// --------
		// IN AL, DX
		m_opcodes[0xEC] = [=]() { IN8(m_reg[REG16::DX]); };
		// IN AX, DX
		m_opcodes[0xED] = [=]() { IN16(m_reg[REG16::DX]); };

		// OUT variable
		// --------
		// OUT DX, AL
		m_opcodes[0xEE] = [=]() { OUT8(m_reg[REG16::DX]); };
		// OUT DX, AX
		m_opcodes[0xEF] = [=]() { OUT16(m_reg[REG16::DX]); };

		// LOCK
		m_opcodes[0xF0] = [=]() { NotImplemented(); };

		// REPNZ/REPNE
		m_opcodes[0xF2] = [=]() { REP(false); };
		// REPZ/REPE
		m_opcodes[0xF3] = [=]() { REP(true); };

		// HLT
		m_opcodes[0xF4] = [=]() { HLT(); };
		// CMC
		m_opcodes[0xF5] = [=]() { CMC(); };

		// TEST/---/NOT/NEG/MUL/IMUL/DIV/IDIV
		// --------
		// REG8/MEM8 (, IMM8 {TEST})
		m_opcodes[0xF6] = [=]() { ArithmeticMulti8(FetchByte()); };
		// REG16/MEM16 (, IMM16 {TEST})
		m_opcodes[0xF7] = [=]() { ArithmeticMulti16(FetchByte()); };

		m_opcodes[0xF8] = [=]() { CLC(); };
		// STC (1)
		m_opcodes[0xF9] = [=]() { STC(); };
		// CLI (1)
		m_opcodes[0xFA] = [=]() { CLI(); };
		// STI (1)
		m_opcodes[0xFB] = [=]() { STI(); };
		// CLD (1)
		m_opcodes[0xFC] = [=]() { CLD(); };
		// STD (1)
		m_opcodes[0xFD] = [=]() { STD(); };

		// INC/DEC/---/---/---/---/---/---
		// --------
		// REG8/MEM8
		m_opcodes[0xFE] = [=]() { INCDEC8(FetchByte()); };

		// INC/DEC/CALL/CALL/JMP/JMP/PUSH/---
		m_opcodes[0xFF] = [=]() { MultiFunc(FetchByte()); };
	}

	bool CPU8086::Step()
	{
		bool ret = CPU::Step();
		if (ret && inSegOverride)
		{
			ret = CPU::Step();
		}

		if (m_state == CPUState::HALT)
		{
			CPU::TICK(1);
			ret = true;
		}

		if (m_irqPending != -1)
		{
			assert(!inSegOverride);
			TICKMISC(MiscTiming::IRQ);
			INT(m_irqPending);
			m_irqPending = -1;
			m_state = CPUState::RUN;
		}

		return ret;
	}

	void CPU8086::Exec(BYTE opcode)
	{
		m_opcode = opcode;

		bool trap = GetFlag(FLAG_T);

		++m_reg[REG16::IP];

		// Disable override after next instruction
		bool clearSegOverride = inSegOverride;

		m_currTiming = &m_info.GetOpcodeTiming(opcode);
		
		// Fetch the function corresponding to the opcode and run it
		{
			auto& opFunc = m_opcodes[opcode];
			opFunc();
		}

		TICK();

		// Disable override after next instruction
		if (clearSegOverride)
		{
			inSegOverride = false;
		}

		// Check for trap
		if (trap && GetFlag(FLAG_T))
		{
			LogPrintf(LOG_INFO, "TRAP AT CS=%04X, IP=%04X", m_reg[REG16::CS], m_reg[REG16::IP]);
			TICKMISC(MiscTiming::TRAP);
			INT(1);
		}
	}

	void CPU8086::Reset()
	{
		CPU::Reset();

		m_reg.Clear();
		m_reg.Write16(REG16::CS, 0xFFFF);
		ClearFlags();
		inRep = false;
		inSegOverride = false;
	}

	void CPU8086::Reset(WORD segment, WORD offset)
	{
		LogPrintf(LOG_DEBUG, "RESET AT CS=%04X, IP=%04X", segment, offset);

		CPU8086::Reset();
		m_reg.Write16(REG16::CS, segment);
		m_reg.Write16(REG16::IP, offset);
	}

	void CPU8086::Dump()
	{
		//	LogPrintf(LOG_DEBUG, "PC = %04X\n", m_programCounter);
		LogPrintf(LOG_DEBUG, "REGISTER DUMP\n"
			"\tAH|AL %02X|%02X\n"
			"\tBH|BL %02X|%02X\n"
			"\tCH|CL %02X|%02X\n"
			"\tDH|DL %02X|%02X\n"
			"\t---------------\n"
			"\tCS|IP %04X|%04X\n"
			"\tDS|SI %04X|%04X\n"
			"\tES|DI %04X|%04X\n"
			"\tSS|SP %04X|%04X\n"
			"\t   BP %04X\n"
			"FLAGS xxxxODITSZxAxPxC\n"
			"      " PRINTF_BIN_PATTERN_INT16
			"\n",
			m_reg[REG8::AH], m_reg[REG8::AL],
			m_reg[REG8::BH], m_reg[REG8::BL],
			m_reg[REG8::CH], m_reg[REG8::CL],
			m_reg[REG8::DH], m_reg[REG8::DL],
			m_reg[REG16::CS], m_reg[REG16::IP],
			m_reg[REG16::DS], m_reg[REG16::SI],
			m_reg[REG16::ES], m_reg[REG16::DI],
			m_reg[REG16::SS], m_reg[REG16::SP],
			m_reg[REG16::BP],
			PRINTF_BYTE_TO_BIN_INT16(m_reg[REG16::FLAGS]));
	}

	void CPU8086::DumpInterruptTable()
	{
		LogPrintf(LOG_ERROR, "INTERRUPT TABLE @ %04X:%04X", m_reg[REG16::CS], m_reg[REG16::IP]);
		for (BYTE interrupt = 0; interrupt <= 0x1F; ++interrupt)
		{
			emul::ADDRESS interruptAddress = interrupt * 4;
			WORD CS = m_memory.Read16(interruptAddress + 2);
			WORD IP = m_memory.Read16(interruptAddress);

			LogPrintf(LOG_ERROR, "\tINT%02X: %04X:%04X", interrupt, CS, IP);
		}
	}

	void CPU8086::ClearFlags()
	{
		m_reg[REG16::FLAGS] = FLAG_RESERVED_ON;
	}

	void CPU8086::SetFlags(WORD flags)
	{
		SetBitMask(flags, FLAG_RESERVED_OFF, false);
		SetBitMask(flags, FLAG_RESERVED_ON, true);
		m_reg[REG16::FLAGS] = flags;
	}

	BYTE CPU8086::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_reg[REG16::IP];
		return b;
	}
	WORD CPU8086::FetchWord()
	{
		BYTE l = m_memory.Read8(GetCurrentAddress());
		++m_reg[REG16::IP];
		BYTE h = m_memory.Read8(GetCurrentAddress());
		++m_reg[REG16::IP];

		return MakeWord(h, l);
	}

	const char* CPU8086::GetReg8Str(BYTE reg)
	{
		switch (reg & 7)
		{
		case 0: return "AL";
		case 1: return "CL";
		case 2: return "DL";
		case 3: return "BL";

		case 4: return "AH";
		case 5: return "CH";
		case 6: return "DH";
		case 7: return "BH";
		}
		throw std::exception("not possible");
	}

	Mem8 CPU8086::GetReg8(BYTE reg)
	{
		switch (reg & 7)
		{
		case 0: return REG8::AL;
		case 1: return REG8::CL;
		case 2: return REG8::DL;
		case 3: return REG8::BL;

		case 4: return REG8::AH;
		case 5: return REG8::CH;
		case 6: return REG8::DH;
		case 7: return REG8::BH;
		}
		throw std::exception("not possible");
	}

	const char* CPU8086::GetReg16Str(BYTE reg, bool segReg)
	{
		switch (reg & 7)
		{
		case 0: return segReg ? "ES" : "AX";
		case 1: return segReg ? "CS" : "CX";
		case 2: return segReg ? "SS" : "DX";
		case 3: return segReg ? "DS" : "BX";
		// Undocumented: 4-7 segment register same as 0-3 on 808x/8018x
		case 4: return segReg ? "ES" : "SP";
		case 5: return segReg ? "CS" : "BP";
		case 6: return segReg ? "SS" : "SI";
		case 7: return segReg ? "DS" : "DI";
		}
		throw std::exception("not possible");
	}

	Mem16 CPU8086::GetReg16(BYTE reg, bool segReg)
	{
		switch (reg & 7)
		{
		case 0: return segReg ? REG16::ES : REG16::AX;
		case 1: return segReg ? REG16::CS : REG16::CX;
		case 2: return segReg ? REG16::SS : REG16::DX;
		case 3: return segReg ? REG16::DS : REG16::BX;
		// Undocumented: 4-7 segment register same as 0-3 on 808x/8018x
		case 4: return segReg ? REG16::ES : REG16::SP;
		case 5: return segReg ? REG16::CS : REG16::BP;
		case 6: return segReg ? REG16::SS : REG16::SI;
		case 7: return segReg ? REG16::DS : REG16::DI;
		}
		throw std::exception("not possible");
	}

	SegmentOffset CPU8086::GetEA(BYTE modregrm, bool direct)
	{
		if (direct)
		{
			TICKMISC(MiscTiming::EA_DIRECT);
			return SegmentOffset{ m_reg[REG16::DS], 0 };
		}

		switch (modregrm & 7)
		{
		case 0: TICKMISC(MiscTiming::EA_INDEX_LO); return SegmentOffset{ m_reg[REG16::DS], (WORD)(m_reg[REG16::BX] + m_reg[REG16::SI]) };
		case 1: TICKMISC(MiscTiming::EA_INDEX_HI); return SegmentOffset{ m_reg[REG16::DS], (WORD)(m_reg[REG16::BX] + m_reg[REG16::DI]) };
		case 2: TICKMISC(MiscTiming::EA_INDEX_HI); return SegmentOffset{ m_reg[REG16::SS], (WORD)(m_reg[REG16::BP] + m_reg[REG16::SI]) };
		case 3: TICKMISC(MiscTiming::EA_INDEX_LO); return SegmentOffset{ m_reg[REG16::SS], (WORD)(m_reg[REG16::BP] + m_reg[REG16::DI]) };

		case 4: return SegmentOffset{ m_reg[REG16::DS], m_reg[REG16::SI] };
		case 5: return SegmentOffset{ m_reg[REG16::DS], m_reg[REG16::DI] };
		case 6: return SegmentOffset{ m_reg[REG16::SS], m_reg[REG16::BP] };
		case 7: return SegmentOffset{ m_reg[REG16::DS], m_reg[REG16::BX] };
		}
		throw std::exception("not possible");
	}

	const char* CPU8086::GetEAStr(BYTE modregrm, bool direct)
	{
		if (direct)
		{
			return "DS:(DIRECT)";
		}
		switch (modregrm & 7)
		{
		case 0: return "DS:BX+SI";
		case 1: return "DS:BX+DI";
		case 2: return "SS:BP+SI";
		case 3: return "SS:BP+DI";
		case 4: return "DS:SI";
		case 5: return "DS:DI";
		case 6: return "SS:BP";
		case 7: return "DS:BX";
		}
		throw std::exception("not possible");
	}

	std::string CPU8086::GetModRMStr(BYTE modrm, bool wide, BYTE& disp)
	{
		disp = 0;
		switch (modrm & 0xC0)
		{
		case 0xC0: return wide ? GetReg16Str(modrm) : GetReg8Str(modrm); // REG
		case 0x40: disp = 8; break;
		case 0x80: disp = 16; break;
		case 0x00: // NO DISP (or DIRECT)
			if ((modrm & 7) == 6) // Direct
			{
				disp = 16;
				return "[{d16}]";
			}
			break;
		default:
			throw std::exception("not possible");
		}

		static char tmp[32];
		switch (modrm & 7)
		{
		case 0: strcpy(tmp, "[BX+SI"); break;
		case 1: strcpy(tmp, "[BX+DI"); break;
		case 2: strcpy(tmp, "[BP+SI"); break;
		case 3: strcpy(tmp, "[BP+DI"); break;
		case 4: strcpy(tmp, "[SI"); break;
		case 5: strcpy(tmp, "[DI"); break;
		case 6: strcpy(tmp, "[BP"); break;
		case 7: strcpy(tmp, "[BX"); break;
		}

		if (disp)
		{
			strcat(tmp, disp == 8 ? "+{d8}" : "+{d16}");
		}
		strcat(tmp, "]");

		return tmp;
	}

	Mem8 CPU8086::GetModRM8(BYTE modrm)
	{
		WORD displacement = 0;
		bool direct = false;

		switch (modrm & 0xC0)
		{
		case 0xC0: // REG
			m_regMem = REGMEM::REG;
			return GetReg8(modrm);
		case 0x00: // NO DISP (or DIRECT)
			if ((modrm & 7) == 6) // Direct
			{
				direct = true;
				displacement = FetchWord();
			}
			break;
		case 0x40:
			displacement = Widen(FetchByte());
			TICKMISC(MiscTiming::EA_DISP);
			break;
		case 0x80:
			displacement = FetchWord();
			TICKMISC(MiscTiming::EA_DISP);
			break;
		default:
			throw std::exception("GetModRM8: not implemented");
		}

		TICKMISC(MiscTiming::EA_BASE);
		m_regMem = REGMEM::MEM;

		SegmentOffset segoff = GetEA(modrm, direct);

		if (inSegOverride)
		{
			segoff.segment = m_reg[REG16::_SEG_O];
		}

		segoff.offset = (direct ? 0 : segoff.offset) + displacement;
		return Mem8(segoff);
	}

	SourceDest8 CPU8086::GetModRegRM8(BYTE modregrm, bool toReg)
	{
		SourceDest8 sd;

		// reg part
		Mem8 reg = GetReg8(modregrm >> 3);
		Mem8 modrm = GetModRM8(modregrm);

		sd.source = toReg ? modrm : reg;
		sd.dest = toReg ? reg : modrm;

		return sd;
	}

	Mem16 CPU8086::GetModRM16(BYTE modrm)
	{
		WORD displacement = 0;
		bool direct = false;

		switch (modrm & 0xC0)
		{
		case 0xC0: // REG
			m_regMem = REGMEM::REG;
			return GetReg16(modrm);
		case 0x00: // NO DISP (or DIRECT)
			if ((modrm & 7) == 6) // Direct 
			{
				direct = true;
				displacement = FetchWord();
			}
			break;
		case 0x40:
			displacement = Widen(FetchByte());
			TICKMISC(MiscTiming::EA_DISP);
			break;
		case 0x80:
			displacement = FetchWord();
			TICKMISC(MiscTiming::EA_DISP);
			break;
		default:
			throw std::exception("GetModRM16: not implemented");
		}

		TICKMISC(MiscTiming::EA_BASE);
		m_regMem = REGMEM::MEM;

		SegmentOffset segoff = GetEA(modrm, direct);

		if (inSegOverride)
		{
			segoff.segment = m_reg[REG16::_SEG_O];
		}

		segoff.offset = (direct ? 0 : segoff.offset) + displacement;
		LogPrintf(LOG_DEBUG, "GetModRM16: MEM %04X:%04X", segoff.segment, segoff.offset);
		return Mem16(segoff);
	}

	SourceDest16 CPU8086::GetModRegRM16(BYTE modregrm, bool toReg, bool segReg)
	{
		LogPrintf(LOG_DEBUG, "GetModRegRM16: modregrm=%d, toReg=%d", modregrm, toReg);

		SourceDest16 sd;

		// reg part
		LogPrintf(LOG_DEBUG, "GetModRegRM16: REG %s", GetReg16Str(modregrm >> 3));
		Mem16 reg = GetReg16(modregrm >> 3, segReg);
		Mem16 modrm = GetModRM16(modregrm);

		sd.source = toReg ? modrm : reg;
		sd.dest = toReg ? reg : modrm;

		return sd;
	}

	void CPU8086::AdjustParity(BYTE data)
	{
		SetFlag(FLAG_P, IsParityEven(data));
	}
	void CPU8086::AdjustSign(BYTE data)
	{
		SetFlag(FLAG_S, (data & 128) ? true : false);
	}
	void CPU8086::AdjustZero(BYTE data)
	{
		SetFlag(FLAG_Z, (data == 0));
	}
	void CPU8086::AdjustParity(WORD data)
	{
		AdjustParity(GetLByte(data));
	}
	void CPU8086::AdjustSign(WORD data)
	{
		SetFlag(FLAG_S, (data & 32768) ? true : false);
	}
	void CPU8086::AdjustZero(WORD data)
	{
		SetFlag(FLAG_Z, (data == 0));
	}

	// =============================================

	void CPU8086::CLC()
	{
		LogPrintf(LOG_DEBUG, "CLC");
		SetFlag(FLAG_C, false);
	}

	void CPU8086::CMC()
	{
		LogPrintf(LOG_DEBUG, "CMC");
		SetFlag(FLAG_C, !GetFlag(FLAG_C));
	}

	void CPU8086::STC()
	{
		LogPrintf(LOG_DEBUG, "STC");
		SetFlag(FLAG_C, true);
	}

	void CPU8086::CLD()
	{
		LogPrintf(LOG_DEBUG, "CLD");
		SetFlag(FLAG_D, false);
	}

	void CPU8086::STD()
	{
		LogPrintf(LOG_DEBUG, "STD");
		SetFlag(FLAG_D, true);
	}

	void CPU8086::CLI()
	{
		LogPrintf(LOG_DEBUG, "CLI");
		SetFlag(FLAG_I, false);
	}

	void CPU8086::STI()
	{
		LogPrintf(LOG_DEBUG, "STI");
		SetFlag(FLAG_I, true);
	}

	void CPU8086::CBW()
	{
		LogPrintf(LOG_DEBUG, "CBW");
		m_reg[REG8::AH] = GetMSB(m_reg[REG8::AL]) ? 0xFF : 0;
	}

	void CPU8086::CWD()
	{
		LogPrintf(LOG_DEBUG, "CWD");
		m_reg[REG16::DX] = GetMSB(m_reg[REG16::AX]) ? 0xFFFF : 0;
	}

	void CPU8086::HLT()
	{
		LogPrintf(LOG_DEBUG, "HLT");
		m_state = CPUState::HALT;
	}
	
	void CPU8086::CALLNear(WORD offset)
	{
		LogPrintf(LOG_DEBUG, "CALLNear Byte offset %02X", offset);
		PUSH(REG16::IP);
		m_reg[REG16::IP] += offset;
	}

	void CPU8086::CALLIntra(WORD address)
	{
		LogPrintf(LOG_DEBUG, "CALLIntra newIP=%04X", address);
		PUSH(REG16::IP);
		m_reg[REG16::IP] = address;
	}

	void CPU8086::CALLfar()
	{
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		LogPrintf(LOG_DEBUG, "CALLfar %02X|%02X", segment, offset);
		PUSH(REG16::CS);
		PUSH(REG16::IP);
		m_reg[REG16::CS] = segment;
		m_reg[REG16::IP] = offset;
	}

	void CPU8086::CALLInter(Mem16 destPtr)
	{
		PUSH(REG16::CS);
		PUSH(REG16::IP);

		m_reg[REG16::IP] = destPtr.Read();
		destPtr.Increment();
		m_reg[REG16::CS] = destPtr.Read();
		LogPrintf(LOG_DEBUG, "CALLInter newCS=%04X, newIP=%04X", m_reg[REG16::CS], m_reg[REG16::IP]);
	}

	void CPU8086::JMPfar()
	{
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		LogPrintf(LOG_DEBUG, "JMPfar %02X|%02X", segment, offset);
		m_reg[REG16::CS] = segment;
		m_reg[REG16::IP] = offset;
	}

	void CPU8086::JMPNear(BYTE offset)
	{
		LogPrintf(LOG_DEBUG, "JMPNear Byte offset %02X", offset);
		m_reg[REG16::IP] += Widen(offset);
	}
	void CPU8086::JMPNear(WORD offset)
	{
		LogPrintf(LOG_DEBUG, "JMPNear Word offset %04X", offset);
		m_reg[REG16::IP] += offset;
	}

	void CPU8086::JMPIntra(WORD address)
	{
		LogPrintf(LOG_DEBUG, "JMPIntra newIP=%04X", address);
		m_reg[REG16::IP] = address;
	}

	void CPU8086::JMPInter(Mem16 destPtr)
	{
		m_reg[REG16::IP] = destPtr.Read();
		destPtr.Increment();
		m_reg[REG16::CS] = destPtr.Read();
		LogPrintf(LOG_DEBUG, "JMPInter newCS=%04X, newIP=%04X", m_reg[REG16::CS], m_reg[REG16::IP]);
	}

	void CPU8086::InvalidOpcode()
	{
		LogPrintf(LOG_ERROR, "TRAP: Invalid opcode [%02x] @ %08x", m_opcode, GetCurrentAddress());
	}

	void CPU8086::NotImplemented()
	{
		LogPrintf(LOG_ERROR, "Not implemented opcode [%02x] @ %08x", m_opcode, GetCurrentAddress());
		m_state = CPUState::STOP;
	}

	void CPU8086::INCDEC8(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "INCDEC8");

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP4, GetOP2(op2));

		Mem8 dest = GetModRM8(op2);

		switch (GetOP2(op2))
		{
		case 0: // INC
			INC8(dest);
			break;
		case 1: // DEC
			DEC8(dest);
			break;
		default:
			InvalidOpcode();
			break;
		}
	}

	void CPU8086::INC8(Mem8 b)
	{
		LogPrintf(LOG_DEBUG, "INC8");
		BYTE before = b.Read();
		BYTE after = before + 1;
		b.Write(after);

		SetFlag(FLAG_O, (!GetMSB(before) && GetMSB(after)));
		AdjustSign(after);
		AdjustZero(after);
		SetFlag(FLAG_A, ((before & 0x0F) == 0x0F));
		AdjustParity(after);

		LogPrintf(LOG_DEBUG, "INC8 %02X->%02X", before, after);
	}

	void CPU8086::DEC8(Mem8 b)
	{
		LogPrintf(LOG_DEBUG, "DEC8");
		BYTE before = b.Read();
		BYTE after = before - 1;
		b.Write(after);

		SetFlag(FLAG_O, (GetMSB(before) && !GetMSB(after)));
		AdjustSign(after);
		AdjustZero(after);
		SetFlag(FLAG_A, ((before & 0x0F) == 0));
		AdjustParity(after);

		LogPrintf(LOG_DEBUG, "DEC8 %02X->%02X", before, after);
	}

	void CPU8086::INC16(WORD& w)
	{
		LogPrintf(LOG_DEBUG, "INC16");
		WORD before = w;

		++w;

		SetFlag(FLAG_O, (!GetMSB(before) && GetMSB(w)));
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, ((before & 0x000F) == 0x0F));
		AdjustParity(w);
		LogPrintf(LOG_DEBUG, "INC16 %04X->%04X", before, w);
	}
	void CPU8086::DEC16(WORD& w)
	{
		LogPrintf(LOG_DEBUG, "DEC16");
		WORD before = w;

		--w;

		SetFlag(FLAG_O, (GetMSB(before) && !GetMSB(w)));
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, ((before & 0x000F) == 0));
		AdjustParity(w);

		LogPrintf(LOG_DEBUG, "DEC16 %04X->%04X", before, w);
	}

	void CPU8086::MOV8(Mem8 d, BYTE s)
	{
		d.Write(s);
	}
	void CPU8086::MOV8(SourceDest8 sd)
	{
		sd.dest.Write(sd.source.Read());
	}
	void CPU8086::MOVIMM8(Mem8 dest)
	{
		dest.Write(FetchByte());
	}

	void CPU8086::MOV16(Mem16 d, WORD s)
	{
		d.Write(s);
	}
	void CPU8086::MOV16(SourceDest16 sd)
	{
		sd.dest.Write(sd.source.Read());
	}
	void CPU8086::MOVIMM16(Mem16 dest)
	{
		dest.Write(FetchWord());
	}

	void CPU8086::SAHF()
	{
		LogPrintf(LOG_DEBUG, "SAHF");

		const WORD mask = FLAG_C | FLAG_P | FLAG_A | FLAG_Z | FLAG_S;

		m_reg[REG16::FLAGS] &= (~mask);
		m_reg[REG16::FLAGS] |= (m_reg[REG8::AH] & mask);
	}
	void CPU8086::LAHF()
	{
		LogPrintf(LOG_DEBUG, "LAHF");

		m_reg[REG8::AH] = (m_reg[REG16::FLAGS] & 0x00FF);
	}

	void CPU8086::JMPif(bool cond)
	{	
		LogPrintf(LOG_DEBUG, "JMPif %d", cond);
		BYTE offset = FetchByte();
		if (cond)
		{
			TICKT3(); // Penalty for jump
			m_reg[REG16::IP] += Widen(offset);
		}
	}

	// Rotate left: n = (n << d)|(n >> (BITS - d))
	// Rotate right: n = (n >> d)|(n << (BITS - d))

	BYTE CPU8086::_SHIFTROT8(BYTE work, BYTE op2, BYTE count)
	{
		for (BYTE i = 0; i < count; ++i)
		{
			BYTE before = work;
			BYTE sign;
			bool carry;
			switch (GetOP2(op2))
			{
			case 0: // ROL
				LogPrintf(LOG_DEBUG, "SHIFTROT8 ROL");
				SetFlag(FLAG_C, GetMSB(work));
				work = (work << 1) | (work >> 7);
				break;
			case 1: // ROR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 ROR");
				SetFlag(FLAG_C, GetLSB(work));
				work = (work >> 1) | (work << 7);
				break;
			case 2: // RCL
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCL");
				carry = GetFlag(FLAG_C);
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				work |= (carry ? 1 : 0);
				break;
			case 3: // RCR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCR");
				carry = GetLSB(work);
				work >>= 1;
				work |= (GetFlag(FLAG_C) ? 128 : 0);
				SetFlag(FLAG_C, carry);
				break;
			case 4: // SHL/SAL
			case 6: // Undocumented 
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SHL");
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				break;
			case 5: // SHR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SHR");
				SetFlag(FLAG_C, GetLSB(work));
				work >>= 1;
				break;
			case 7: // SAR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SAR");
				SetFlag(FLAG_C, GetLSB(work));
				sign = (work & 128);
				work >>= 1;
				work |= sign;
				break;
			default:
				throw(std::exception("not possible"));
			}
			SetFlag(FLAG_O, GetMSB(before) != GetMSB(work));

			TICKT3(); // Add overhead for each shift
		}

		if (GetOP2(op2) >= 4) // Only shift operation adjusts SZP flags
		{
			AdjustSign(work);
			AdjustZero(work);
			AdjustParity(work);
		}

		return work;
	}

	void CPU8086::SHIFTROT8One(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT8One");

		Mem8 dest = GetModRM8(op2);
		BYTE work = dest.Read();
		work = _SHIFTROT8(work, op2, 1);
		dest.Write(work);
	}

	void CPU8086::SHIFTROT8Multi(BYTE op2, BYTE mask)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT8Multi");

		BYTE count = m_reg[REG8::CL] & mask;
		Mem8 dest = GetModRM8(op2);
		BYTE work = dest.Read();
		work = _SHIFTROT8(work, op2, count);
		dest.Write(work);		
	}

	WORD CPU8086::_SHIFTROT16(WORD work, BYTE op2, BYTE count)
	{
		for (BYTE i = 0; i < count; ++i)
		{
			WORD before = work;
			WORD sign;
			bool carry;
			switch (GetOP2(op2))
			{
			case 0: // ROL
				LogPrintf(LOG_DEBUG, "SHIFTROT16 ROL");
				SetFlag(FLAG_C, GetMSB(work));
				work = (work << 1) | (work >> 15);
				break;
			case 1: // ROR
				LogPrintf(LOG_DEBUG, "SHIFTROT16 ROR");
				SetFlag(FLAG_C, GetLSB(work));
				work = (work >> 1) | (work << 15);
				break;
			case 2: // RCL
				LogPrintf(LOG_DEBUG, "SHIFTROT16 RCL");
				carry = GetFlag(FLAG_C);
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				work |= (carry ? 1 : 0);
				break;
			case 3: // RCR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCR");
				carry = GetLSB(work);
				work >>= 1;
				work |= (GetFlag(FLAG_C) ? 32768 : 0);
				SetFlag(FLAG_C, carry);
				break;
			case 4: // SHL/SAL
			case 6: // Undocumented 
				LogPrintf(LOG_DEBUG, "SHIFTROT16 SHL");
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				break;
			case 5: // SHR
				LogPrintf(LOG_DEBUG, "SHIFTROT16 SHR");
				SetFlag(FLAG_C, GetLSB(work));
				work >>= 1;
				break;
			case 7: // SAR
				LogPrintf(LOG_DEBUG, "SHIFTROT16 SAR");
				SetFlag(FLAG_C, GetLSB(work));
				sign = (work & 32768);
				work >>= 1;
				work |= sign;
				break;
			default:
				throw(std::exception("not possible"));
			}
			SetFlag(FLAG_O, GetMSB(before) != GetMSB(work));

			TICKT3(); // Add overhead for each shift
		}

		if (GetOP2(op2) >= 4) // Only shift operation adjusts flags
		{
			AdjustSign(work);
			AdjustZero(work);
			AdjustParity(work);
		}

		return work;
	}

	void CPU8086::SHIFTROT16One(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT16One");

		Mem16 dest = GetModRM16(op2);
		WORD work = dest.Read();
		work = _SHIFTROT16(work, op2, 1);
		dest.Write(work);
	}

	void CPU8086::SHIFTROT16Multi(BYTE op2, BYTE mask)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT16Multi");

		BYTE count = m_reg[REG8::CL] & mask;
		Mem16 dest = GetModRM16(op2);
		WORD work = dest.Read();
		work = _SHIFTROT16(work, op2, count);
		dest.Write(work);
	}

	void CPU8086::Arithmetic8(SourceDest8 sd, RawOpFunc8 func)
	{
		// Aliases
		const BYTE source = sd.source.Read();
		BYTE dest = sd.dest.Read();

		// AC Calculation
		const BYTE source4 = source & 0x0F;
		BYTE dest4 = dest & 0x0F;
		WORD after4 = func(dest4, source4, GetFlag(FLAG_C));
		SetFlag(FLAG_A, after4 > 0x0F);

		BYTE before = dest;
		WORD after = func(dest, source, GetFlag(FLAG_C));
		BYTE afterB = (BYTE)after;
		sd.dest.Write(dest);
		SetFlag(FLAG_C, after > 0xFF);

		// TODO: improve this
		if (func == rawAdd8 || func == rawAdc8)
		{
			// If 2 Two's Complement numbers are added, and they both have the same sign (both positive or both negative), 
			// then overflow occurs if and only if the result has the opposite sign. 
			// Overflow never occurs when adding operands with different signs. 
			SetFlag(FLAG_O, (GetMSB(source) == GetMSB(before)) && (GetMSB(afterB) != GetMSB(source)));
		}
		else if (func == rawSub8 || func == rawCmp8 || func == rawSbb8)
		{
			// If 2 Two's Complement numbers are subtracted, and their signs are different, 
			// then overflow occurs if and only if the result has the same sign as what is being subtracted.
			SetFlag(FLAG_O, (GetMSB(source) != GetMSB(before)) && (GetMSB(afterB) == GetMSB(source)));
		}
		else
		{
			SetFlag(FLAG_O, false);
		}

		AdjustSign(afterB);
		AdjustZero(afterB);
		AdjustParity(afterB);
	}

	void CPU8086::Arithmetic16(SourceDest16 sd, RawOpFunc16 func)
	{
		// Aliases
		const WORD source = sd.source.Read();
		WORD dest = sd.dest.Read();

		// AC Calculations
		const WORD source4 = source & 0x0F;
		WORD dest4 = dest & 0x0F;
		WORD after4 = func(dest4, source4, GetFlag(FLAG_C));
		SetFlag(FLAG_A, after4 > 0x0F);

		WORD before = dest;
		DWORD after = func(dest, source, GetFlag(FLAG_C));
		WORD afterW = (WORD)after;
		sd.dest.Write(dest);
		SetFlag(FLAG_C, after > 65535);

		// TODO: improve this
		if (func == rawAdd16 || func == rawAdc16)
		{
			// If 2 Two's Complement numbers are added, and they both have the same sign (both positive or both negative), 
			// then overflow occurs if and only if the result has the opposite sign. 
			// Overflow never occurs when adding operands with different signs. 
			SetFlag(FLAG_O, (GetMSB(source) == GetMSB(before)) && (GetMSB(before) != GetMSB(afterW)));
		}
		else if (func == rawSub16 || func == rawCmp16 || func == rawSbb16)
		{
			// If 2 Two's Complement numbers are subtracted, and their signs are different, 
			// then overflow occurs if and only if the result has the same sign as what is being subtracted.
			SetFlag(FLAG_O, (GetMSB(source) != GetMSB(before)) && (GetMSB(afterW) == GetMSB(source)));
		}
		else
		{
			SetFlag(FLAG_O, false);
		}

		AdjustSign(afterW);
		AdjustZero(afterW);
		AdjustParity(afterW);
	}

	void CPU8086::ArithmeticImm8(Mem8 dest, BYTE imm, RawOpFunc8 func)
	{
		m_reg.Write8(REG8::_T0, imm);

		SourceDest8 sd;
		sd.source = REG8::_T0;
		sd.dest = dest;
		Arithmetic8(sd, func);
	}
	void CPU8086::ArithmeticImm16(Mem16 dest, WORD imm, RawOpFunc16 func)
	{
		m_reg.Write16(REG16::_T0, imm);

		SourceDest16 sd;
		sd.source = REG16::_T0;
		sd.dest = dest;
		Arithmetic16(sd, func);
	}

	void CPU8086::IN8(WORD port)
	{
		LogPrintf(LOG_DEBUG, "IN8 port %04X", port);
		In(port, m_reg[REG8::AL]);
	}

	void CPU8086::IN16(WORD port)
	{
		LogPrintf(LOG_DEBUG, "IN16 port %04X", port);
		In(port, m_reg[REG8::AL]);
		In(port+1, m_reg[REG8::AH]);
	}

	void CPU8086::OUT8(WORD port)
	{
		Out(port, m_reg[REG8::AL]);
	}

	void CPU8086::OUT16(WORD port)
	{
		Out(port, m_reg[REG8::AL]);
		Out(port+1, m_reg[REG8::AH]);
	}

	void CPU8086::LOOP(BYTE offset, bool cond)
	{
		--m_reg[REG16::CX];
		LogPrintf(LOG_DEBUG, "LOOP, CX=%04X", m_reg[REG16::CX]);
		if (m_reg[REG16::CX] && cond)
		{
			TICKT3(); // Penalty for loop
			m_reg[REG16::IP] += Widen(offset);
		}
	}

	void CPU8086::RETNear(bool pop, WORD value)
	{
		LogPrintf(LOG_DEBUG, "RETNear [%s][%d]", pop?"Pop":"NoPop", value);

		POP(REG16::IP);
		m_reg[REG16::SP] += value;
	}

	void CPU8086::RETFar(bool pop, WORD value)
	{
		LogPrintf(LOG_DEBUG, "RETFar [%s][%d]", pop ? "Pop" : "NoPop", value);

		POP(REG16::IP);
		POP(REG16::CS);
		m_reg[REG16::SP] += value;
	}

	void CPU8086::ArithmeticMulti8Imm(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticImm8");

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP1, GetOP2(op2));

		RawOpFunc8 func;

		switch (GetOP2(op2))
		{
		case 0: func = rawAdd8; break;
		case 1: func = rawOr8;  break;
		case 2: func = rawAdc8; break;
		case 3: func = rawSbb8; break;
		case 4: func = rawAnd8; break;
		case 5: func = rawSub8; break;
		case 6: func = rawXor8; break;
		case 7: func = rawCmp8; break;
		default: throw std::exception("not possible");
		}

		SourceDest8 sd;
		sd.dest = GetModRM8(op2);

		m_reg[REG8::_T0] = FetchByte();
		sd.source = REG8::_T0;

		Arithmetic8(sd, func);
	}
	void CPU8086::ArithmeticMulti16Imm(BYTE op2, bool signExtend)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticImm16");

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP1, GetOP2(op2));

		RawOpFunc16 func;

		switch (GetOP2(op2))
		{
		case 0: func = rawAdd16; break;
		case 1: func = rawOr16;  break;
		case 2: func = rawAdc16; break;
		case 3: func = rawSbb16; break;
		case 4: func = rawAnd16; break;
		case 5: func = rawSub16; break;
		case 6: func = rawXor16; break;
		case 7: func = rawCmp16; break;
		default: throw std::exception("not possible");
		}

		SourceDest16 sd;
		sd.dest = GetModRM16(op2);

		m_reg[REG16::_T0] = signExtend ? Widen(FetchByte()) : FetchWord();
		sd.source = REG16::_T0;

		Arithmetic16(sd, func);
	}

	void CPU8086::ArithmeticMulti8(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticMulti8");

		Mem8 modrm = GetModRM8(op2);
		BYTE val = modrm.Read();

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP3, GetOP2(op2));

		switch (GetOP2(op2))
		{
		case 0: // TEST
		case 1: // Undocumented
		{
			LogPrintf(LOG_DEBUG, "TEST8");
			BYTE imm = FetchByte();
			BYTE after = (BYTE)rawTest8(val, imm, false);
			SetFlag(FLAG_O, false);
			SetFlag(FLAG_C, false);
			AdjustSign(after);
			AdjustZero(after);
			AdjustParity(after);
			break;
		}
		case 2: // NOT
		{
			LogPrintf(LOG_DEBUG, "NOT16");
			modrm.Write(~val);
			break;
		}
		case 3: // NEG
		{
			SourceDest8 sd;
			m_reg[REG8::_T0] = 0;
			sd.dest = REG8::_T0;
			sd.source = modrm;
			Arithmetic8(sd, rawSub8);
			LogPrintf(LOG_DEBUG, "NEG8, -(%02X) = %02X", val, m_reg[REG8::_T0]);
			modrm.Write(m_reg[REG8::_T0]);
			break;
		}
		case 4: // MUL
		{
			WORD result = m_reg[REG8::AL] * val;
			LogPrintf(LOG_DEBUG, "MUL8, %02X * %02X = %04X", m_reg[REG8::AL], val, result);
			m_reg[REG16::AX] = result;
			SetFlag(FLAG_O, m_reg[REG8::AH] != 0);
			SetFlag(FLAG_C, m_reg[REG8::AH] != 0);
			AdjustSign(m_reg[REG8::AL]);
			AdjustZero(m_reg[REG8::AL]);
			AdjustParity(m_reg[REG8::AL]);
			break;
		}
		case 5: // IMUL
		{
			int16_t result = (int8_t)m_reg[REG8::AL] * (int8_t)(val);
			LogPrintf(LOG_DEBUG, "IMUL8, %d * %d = %d", (int8_t)m_reg[REG8::AL], (int8_t)(val), result);
			m_reg[REG16::AX] = (WORD)result;
			WORD tmp = Widen(m_reg[REG8::AL]);
			bool ocFlags = (tmp != (WORD)result);
			SetFlag(FLAG_O, ocFlags);
			SetFlag(FLAG_C, ocFlags);
			AdjustSign(m_reg[REG8::AL]);
			AdjustZero(m_reg[REG8::AL]);
			AdjustParity(m_reg[REG8::AL]);
			break;
		}
		case 6:
		{
			LogPrintf(LOG_DEBUG, "DIV8");
			if (val == 0)
			{
				INT(0);
				return;
			}
			WORD dividend = m_reg[REG16::AX];
			WORD quotient = dividend / val;
			if (quotient > 0xFF)
			{
				INT(0);
				return;
			}
			BYTE remainder = dividend % val;
			LogPrintf(LOG_DEBUG, "DIV8 %04X / %02X = %02X r %02X", dividend, val, quotient, remainder);
			m_reg[REG8::AL] = (BYTE)quotient;
			m_reg[REG8::AH] = remainder;
			break;
		}
		case 7:
		{
			LogPrintf(LOG_DEBUG, "IDIV8");
			if (val == 0)
			{
				INT(0);
				return;
			}
			int16_t dividend = (int16_t)m_reg[REG16::AX];
			int16_t quotient = dividend / int8_t(val);
			if (quotient > 127 || quotient < -127)
			{
				INT(0);
				return;
			}
			int8_t remainder = dividend % int8_t(val);
			LogPrintf(LOG_DEBUG, "IDIV8 %04X / %02X = %02X r %02X", dividend, val, quotient, remainder);
			m_reg[REG8::AL] = (BYTE)quotient;
			m_reg[REG8::AH] = (BYTE)remainder;
			break;
		}
		default: throw std::exception("not possible");
		}
	}

	void CPU8086::ArithmeticMulti16(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticMulti16");

		Mem16 modrm = GetModRM16(op2);
		WORD val = modrm.Read();

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP3, GetOP2(op2));
		TICKT3(); // Add 16-bit operation overhead

		switch (GetOP2(op2))
		{
		case 0: // TEST
		case 1:
		{
			LogPrintf(LOG_DEBUG, "TEST16");
			WORD imm = FetchWord();
			WORD after = rawTest16(val, imm, false);
			SetFlag(FLAG_O, false);
			SetFlag(FLAG_C, false);
			AdjustSign(after);
			AdjustZero(after);
			AdjustParity(after);
			break;
		}
		case 2: // NOT
		{
			LogPrintf(LOG_DEBUG, "NOT16");
			modrm.Write(~val);
			break;
		}
		case 3: // NEG
		{
			SourceDest16 sd;
			m_reg[REG16::_T0] = 0;
			sd.dest = REG16::_T0;
			sd.source = modrm;
			Arithmetic16(sd, rawSub16);
			LogPrintf(LOG_DEBUG, "NEG16, -(%04X) = %04X", val, m_reg[REG16::_T0]);
			modrm.Write(m_reg[REG16::_T0]);
			break;
		}
		case 4: // MUL
		{
			DWORD result = m_reg[REG16::AX] * val;
			LogPrintf(LOG_DEBUG, "MUL16, %04X * %04X = %08X", m_reg[REG16::AX], val, result);
			m_reg[REG16::DX] = GetHWord(result);
			m_reg[REG16::AX] = GetLWord(result);
			SetFlag(FLAG_O, m_reg[REG16::DX] != 0);
			SetFlag(FLAG_C, m_reg[REG16::DX] != 0);
			AdjustSign(m_reg[REG16::AX]);
			AdjustZero(m_reg[REG16::AX]);
			AdjustParity(m_reg[REG16::AX]);
			break;
		}
		case 5: // IMUL
		{
			int32_t result = (int16_t)m_reg[REG16::AX] * (int16_t)(val);
			LogPrintf(LOG_DEBUG, "IMUL16, %d * %d = %d", (int16_t)m_reg[REG16::AX], (int16_t)(val), result);
			m_reg[REG16::DX] = GetHWord(result);
			m_reg[REG16::AX] = GetLWord(result);
			DWORD tmp = Widen(m_reg[REG16::AX]);
			bool ocFlags = (tmp != (DWORD)result);
			SetFlag(FLAG_O, ocFlags);
			SetFlag(FLAG_C, ocFlags);
			AdjustSign(m_reg[REG16::AX]);
			AdjustZero(m_reg[REG16::AX]);
			AdjustParity(m_reg[REG16::AX]);
			break;
		}
		case 6:
		{
			LogPrintf(LOG_DEBUG, "DIV16");
			if (val == 0)
			{
				INT(0);
				return;
			}
			DWORD dividend = MakeDword(m_reg[REG16::DX], m_reg[REG16::AX]);
			DWORD quotient = dividend / val;
			if (quotient > 0xFFFF)
			{
				INT(0);
				return;
			}
			WORD remainder = dividend % val;
			LogPrintf(LOG_DEBUG, "DIV16 %08X / %04X = %04X r %04X", dividend, val, quotient, remainder);
			m_reg[REG16::AX] = (WORD)quotient;
			m_reg[REG16::DX] = remainder;
			break;
		}
		case 7:
		{
			LogPrintf(LOG_DEBUG, "IDIV16");
			if (val == 0)
			{
				INT(0);
				return;
			}
			int32_t dividend = (int32_t)MakeDword(m_reg[REG16::DX], m_reg[REG16::AX]);
			int32_t quotient = dividend / int16_t(val);
			if (quotient > 32767 || quotient < -32767)
			{
				INT(0);
				return;
			}
			int16_t remainder = dividend % int16_t(val);
			LogPrintf(LOG_DEBUG, "IDIV16 %08X / %04X = %04X r %04X", dividend, val, quotient, remainder);
			m_reg[REG16::AX] = (WORD)quotient;
			m_reg[REG16::DX] = (WORD)remainder;
			break;
		}
		default: throw std::exception("not possible");
		}
	}

	void CPU8086::XCHG8(SourceDest8 sd)
	{
		BYTE source = sd.source.Read();
		BYTE dest = sd.dest.Read();

		XCHG8(source, dest);
		sd.source.Write(source);
		sd.dest.Write(dest);
	}

	void CPU8086::XCHG8(BYTE& b1, BYTE& b2)
	{
		LogPrintf(LOG_DEBUG, "XCHG8");
		BYTE temp = b1;
		b1 = b2;
		b2 = temp;
	}

	void CPU8086::XCHG16(SourceDest16 sd)
	{
		WORD source = sd.source.Read();
		WORD dest = sd.dest.Read();

		XCHG16(source, dest);
		sd.source.Write(source);
		sd.dest.Write(dest);
	}

	void CPU8086::XCHG16(WORD& w1, WORD& w2)
	{
		LogPrintf(LOG_DEBUG, "XCHG16 %04X<=>%04X", w1, w2);
		WORD temp = w1;
		w1 = w2;
		w2 = temp;
	}

	void CPU8086::PUSH(Mem16 m)
	{
		PUSH(m.Read());
	}

	void CPU8086::PUSH(WORD w)
	{
		m_memory.Write8(S2A(m_reg[REG16::SS], --m_reg[REG16::SP]), GetHByte(w));
		m_memory.Write8(S2A(m_reg[REG16::SS], --m_reg[REG16::SP]), GetLByte(w));
	}

	void CPU8086::POP(Mem16 dest)
	{
		dest.Write(POP());
	}

	WORD CPU8086::POP()
	{
		BYTE lo = m_memory.Read8(S2A(m_reg[REG16::SS], m_reg[REG16::SP]++));
		BYTE hi = m_memory.Read8(S2A(m_reg[REG16::SS], m_reg[REG16::SP]++));
		return MakeWord(hi, lo);
	}

	void CPU8086::PUSHF()
	{
		PUSH(REG16::FLAGS);
	}

	void CPU8086::POPF()
	{
		WORD flags = POP(); 
		SetFlags(flags);
	}

	void CPU8086::LODS8()
	{
		LogPrintf(LOG_DEBUG, "LODS8, SI=%04X", m_reg[REG16::SI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			m_reg[REG8::AL] = m_memory.Read8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]));
			IndexIncDec(m_reg[REG16::SI]);
		}
		PostREP(false);
	}

	void CPU8086::LODS16()
	{
		LogPrintf(LOG_DEBUG, "LODS16, SI=%04X", m_reg[REG16::SI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			SourceDest16 sd;
			sd.dest = REG16::AX;
			sd.source = S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]);
			sd.dest.Write(sd.source.Read());

			IndexIncDec(m_reg[REG16::SI]);
			IndexIncDec(m_reg[REG16::SI]);
		}
		PostREP(false);
	}

	void CPU8086::STOS8()
	{
		LogPrintf(LOG_DEBUG, "STOS8, DI=%04X", m_reg[REG16::DI]);

		if (PreREP())
		{
			m_memory.Write8(S2A(m_reg[REG16::ES], m_reg[REG16::DI]), m_reg[REG8::AL]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(false);
	}
	void CPU8086::STOS16()
	{
		LogPrintf(LOG_DEBUG, "STOS16, DI=%04X", m_reg[REG16::DI]);

		if (PreREP())
		{
			SourceDest16 sd;
			sd.dest = S2A(m_reg[REG16::ES], m_reg[REG16::DI]);
			sd.source = REG16::AX;
			sd.dest.Write(sd.source.Read());

			IndexIncDec(m_reg[REG16::DI]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(false);
	}

	void CPU8086::SCAS8()
	{
		LogPrintf(LOG_DEBUG, "SCAS8, DI=%04X", m_reg[REG16::DI]);

		if (PreREP())
		{
			SourceDest8 sd;

			sd.source = S2A(m_reg[REG16::ES], m_reg[REG16::DI]);
			sd.dest = REG8::AL;
			Arithmetic8(sd, rawCmp8);

			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(true);
	}
	void CPU8086::SCAS16()
	{
		LogPrintf(LOG_DEBUG, "SCAS16, DI=%04X", m_reg[REG16::DI]);

		if (PreREP())
		{
			SourceDest16 sd;

			sd.source = S2A(m_reg[REG16::ES], m_reg[REG16::DI]);
			sd.dest = REG16::AX;
			Arithmetic16(sd, rawCmp16);

			IndexIncDec(m_reg[REG16::DI]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(true);
	}

	void CPU8086::MOVS8()
	{
		LogPrintf(LOG_DEBUG, "MOVS8, SI=%04X, DI=%04X", m_reg[REG16::SI], m_reg[REG16::DI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			BYTE val = m_memory.Read8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]));
			m_memory.Write8(S2A(m_reg[REG16::ES], m_reg[REG16::DI]), val);

			IndexIncDec(m_reg[REG16::SI]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(false);
	}
	void CPU8086::MOVS16()
	{
		LogPrintf(LOG_DEBUG, "MOVS16, SI=%04X, DI=%04X", m_reg[REG16::SI], m_reg[REG16::DI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			SourceDest16 sd;
			sd.source.SetAddress(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]));
			sd.dest.SetAddress(S2A(m_reg[REG16::ES], m_reg[REG16::DI]));

			sd.dest.Write(sd.source.Read());

			IndexIncDec(m_reg[REG16::SI]);
			IndexIncDec(m_reg[REG16::SI]);

			IndexIncDec(m_reg[REG16::DI]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(false);
	}

	void CPU8086::CMPS8()
	{
		LogPrintf(LOG_DEBUG, "CMPS8, SI=%04X, DI=%04X", m_reg[REG16::SI], m_reg[REG16::DI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			SourceDest8 sd;
			sd.dest = S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]);
			sd.source = S2A(m_reg[REG16::ES], m_reg[REG16::DI]);

			Arithmetic8(sd, rawCmp8);

			IndexIncDec(m_reg[REG16::SI]);

			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(true);
	}
	void CPU8086::CMPS16()
	{
		LogPrintf(LOG_DEBUG, "CMPS16, SI=%04X, DI=%04X", m_reg[REG16::SI], m_reg[REG16::DI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			SourceDest16 sd;
			
			sd.dest = S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]);
			sd.source = S2A(m_reg[REG16::ES], m_reg[REG16::DI]);

			Arithmetic16(sd, rawCmp16);

			IndexIncDec(m_reg[REG16::SI]);
			IndexIncDec(m_reg[REG16::SI]);

			IndexIncDec(m_reg[REG16::DI]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(true);
	}

	void CPU8086::REP(bool z)
	{
		LogPrintf(LOG_DEBUG, "REP, Z=%d, cx=%04X", z, m_reg[REG16::CX]);

		inRep = true;
		repZ = z;
		m_reg[REG16::_REP_IP] = m_reg[REG16::IP] - 1;
	}

	bool CPU8086::PreREP()
	{
		if (!inRep)
		{
			// penalty for no-REP
			TICKT4();
			return true;
		}

		if (m_reg[REG16::CX] == 0)
		{
			LogPrintf(LOG_DEBUG, "PreRep, end loop");
			TICKT3(); // Setup penalty for REP
			inRep = false;
			return false;
		}
		return true;
	}
	void CPU8086::PostREP(bool checkZ)
	{
		if (!inRep)
		{
			return;
		}

		--m_reg[REG16::CX];
		LogPrintf(LOG_DEBUG, "PostRep, cx=%04X", m_reg[REG16::CX]);
		if (!checkZ || (GetFlag(FLAG_Z) == repZ))
		{ 
			m_reg[REG16::IP] = m_reg[REG16::_REP_IP];
		}
		else
		{
			LogPrintf(LOG_DEBUG, "PostRep, end loop");
			TICKT3(); // Setup penalty for REP
			inRep = false;
		}
	}

	void CPU8086::SEGOVERRIDE(WORD val)
	{
		LogPrintf(LOG_DEBUG, "Segment Override, val=%04X", val);

		inSegOverride = true;
		m_reg[REG16::_SEG_O] = val;
	}

	void CPU8086::INT(BYTE interrupt)
	{
		LogPrintf(LOG_DEBUG, "Interrupt, int=%02X", interrupt);

		if (interrupt == 0x16)
		{
			LogPrintf(LOG_DEBUG, "Waiting for keyboard input");
		} 
		else  if (interrupt == 0x10)
		{
			LogPrintf(LOG_DEBUG, "VIDEO");
#if 0
			char ch = (m_reg[REG8::AL] < 32) ? '.' : (char)m_reg[REG8::AL];
			
			switch (m_reg[REG8::AH])
			{
			case 0x00: LogPrintf(LOG_ERROR, "INT10: 0x00 - Set video mode [%02X]", m_reg[REG8::AL]); break;
			case 0x01: LogPrintf(LOG_DEBUG, "INT10: 0x01 - Set text-mode cursor shape [%02X]-[%02X]", m_reg[REG8::CH], m_reg[REG8::CL]); break;
			case 0x02: LogPrintf(LOG_DEBUG, "INT10: 0x02 - Set cursor position p[%d]:[%02d ,%02d]", m_reg[REG8::BH], m_reg[REG8::DL], m_reg[REG8::DH]); break;
			case 0x03: LogPrintf(LOG_DEBUG, "INT10: 0x03 - Get cursor position p[%d]", m_reg[REG8::BH]); break;
			case 0x04: LogPrintf(LOG_DEBUG, "INT10: 0x04 - Read light pen"); break;
			case 0x05: LogPrintf(LOG_ERROR, "INT10: 0x05 - Select page p[%d]", m_reg[REG8::AL]); break;
			case 0x06: LogPrintf(LOG_DEBUG, "INT10: 0x06 - Scroll up"); break;
			case 0x07: LogPrintf(LOG_DEBUG, "INT10: 0x07 - Scroll down"); break;
			case 0x08: LogPrintf(LOG_DEBUG, "INT10: 0x08 - Read char & attr at cursor"); break;
			case 0x09: LogPrintf(LOG_ERROR, "INT10: 0x09 - Write char & attr at cursor ch=[%02d]['%c'], p=[%d], color=[%02d], times=[%d]", m_reg[REG8::AL], ch, m_reg[REG8::BH], m_reg[REG8::BL], m_reg[REG16::CX]); break;
			case 0x0A: LogPrintf(LOG_ERROR, "INT10: 0x0A - Write char at cursor ch=[%02d]['%c'], p=[%d], times=[%d]", m_reg[REG8::AL], ch, m_reg[REG8::BH], m_reg[REG16::CX]); break;
			case 0x0B: LogPrintf(LOG_DEBUG, "INT10: 0x0B - Set background/border color / Set palette"); break;
			case 0x0C: LogPrintf(LOG_DEBUG, "INT10: 0x0C - Write pixel"); break;
			case 0x0D: LogPrintf(LOG_DEBUG, "INT10: 0x0D - Read pixel"); break;
			case 0x0E: LogPrintf(LOG_ERROR, "INT10: 0x0E - Teletype output ch=[%02d]['%c'] p=[%d]", m_reg[REG8::AL], ch, m_reg[REG8::BL]); break;
			case 0x0F: LogPrintf(LOG_DEBUG, "INT10: 0x0F - Get video mode"); break;
			case 0x11: LogPrintf(LOG_DEBUG, "INT10: 0x11 - Change charset"); break;
			default: LogPrintf(LOG_DEBUG, "INT10: Other function ah=%02X", m_reg[REG8::AH]); break;
			}
			//return;
#endif
		}
		else if (interrupt == 0x19)
		{
			LogPrintf(LOG_ERROR, "BOOT LOADER");
		}
		else if (interrupt == 0x18)
		{
			LogPrintf(LOG_ERROR, "BASIC");
		}
		else if (interrupt == 0x13)
		{
			WORD cyl = m_reg[REG8::CH];
			cyl |= ((m_reg[REG8::CL] & 0b11000000) << 2);
			switch (m_reg[REG8::AH])
			{
			case 0x00: LogPrintf(LOG_INFO, "DISK[%02X]: Reset drive", m_reg[REG8::DL]); break;
			case 0x01: LogPrintf(LOG_INFO, "DISK[%02X]: Get Status drive", m_reg[REG8::DL]); break;
			case 0x02: LogPrintf(LOG_INFO, "DISK[%02X]: Read Sectors count=[%d], cyl=[%d] head=[%d] sect=[%d]", m_reg[REG8::DL], m_reg[REG8::AL], cyl, m_reg[REG8::DH], m_reg[REG8::CL] & 63); break;
			case 0x03: LogPrintf(LOG_INFO, "DISK[%02X]: Write Sectors count=[%d], cyl=[%d] head=[%d] sect=[%d]", m_reg[REG8::DL], m_reg[REG8::AL], cyl, m_reg[REG8::DH], m_reg[REG8::CL] & 63); break;
			case 0x04: LogPrintf(LOG_INFO, "DISK[%02X]: Verify Sectors count=[%d], cyl=[%d] head=[%d] sect=[%d]", m_reg[REG8::DL], m_reg[REG8::AL], cyl, m_reg[REG8::DH], m_reg[REG8::CL] & 63); break;

			default: LogPrintf(LOG_INFO, "DISK[%02X], Other function ah=%02X", m_reg[REG8::DL], m_reg[REG8::AH]); break;
			}
		}
		else if (interrupt == 0x21)
		{
			LogPrintf(LOG_DEBUG, "DOS");

#if 0
			switch (m_reg[REG8::AH])
			{				
			case 0x09: LogPrintf(LOG_ERROR, "INT21: 0x09 - Print String @[%04X:%04X]", m_reg[REG16::DS], m_reg[REG16::DX]); break;
			case 0x0A: LogPrintf(LOG_ERROR, "INT21: 0x0A - Buffered input"); break;
			case 0x0E: LogPrintf(LOG_ERROR, "INT21: 0x0E - Select Disk dl=%02X", m_reg[REG8::DL]); break;
			case 0x2B: LogPrintf(LOG_ERROR, "INT21: 0x2B - Set system date[%04d-%02d-%02d]", m_reg[REG16::CX], m_reg[REG8::DH], m_reg[REG8::DL]); break;
			default: LogPrintf(LOG_ERROR, "INT21: Other function ah=%02X", m_reg[REG8::AH]); break;
			}
#endif
		}
#if 0
		else if (interrupt >= 0x09 && interrupt < 0x10)
		{
			LogPrintf(LOG_WARNING, "IRQ(%d)", interrupt - 8);
		}
#endif
		PUSHF();
		PUSH(m_reg[REG16::CS]);
		PUSH(m_reg[inRep ? REG16::_REP_IP : REG16::IP]);
		if (inRep)
		{
			inRep = false;
		}

		SetFlag(FLAG_T, false);
		CLI();
		
		ADDRESS interruptAddress = interrupt * 4;
		m_reg[REG16::CS] = m_memory.Read16(interruptAddress + 2);
		m_reg[REG16::IP] = m_memory.Read16(interruptAddress);
	}

	void CPU8086::IRET()
	{
		LogPrintf(LOG_DEBUG, "IRET");
		POP(REG16::IP);
		POP(REG16::CS);
		POPF();
	}

	void CPU8086::MultiFunc(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "Multifunc, op=%02X", op2);

		Mem16 dest = GetModRM16(op2);
		WORD val = dest.Read();

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP5, GetOP2(op2));

		switch (GetOP2(op2))
		{
		// INC/DEC MEM16
		case 0: INC16(val); dest.Write(val);  break;
		case 1: DEC16(val); dest.Write(val); break;
		// CALL RM16(intra) / CALL MEM16(intersegment)
		case 2: CALLIntra(val); break;
		case 3: CALLInter(dest); break;
		// JMP RM16(intra) // JMP MEM16(intersegment)
		case 4: JMPIntra(val); break;
		case 5: JMPInter(dest); break;
		// PUSH MEM16
		case 6: PUSH(dest); break;
		// not used
		case 7: InvalidOpcode(); break;
		default: throw std::exception("not possible");
		}
	}

	void CPU8086::LoadPTR(WORD& destSegment, SourceDest16 regMem)
	{
		LogPrintf(LOG_DEBUG, "LoadPtr");

		// TODO: This should fail if modrm is register
		// otherwise we will read data in *this instead of memory

		// Target register -> offset
		regMem.dest.Write(regMem.source.Read());
		
		// Read segment
		regMem.source.Increment();
		destSegment = regMem.source.Read();

		LogPrintf(LOG_DEBUG, "LoadPtr Loaded [%04X:%04X]", destSegment, regMem.dest.Read());

	}

	void CPU8086::XLAT()
	{
		LogPrintf(LOG_DEBUG, "XLAT");

		WORD offset = m_reg[REG16::BX] + m_reg[REG8::AL]; // TODO: Wrap around?
		m_reg[REG8::AL] = m_memory.Read8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], offset));
	}

	void CPU8086::AAA()
	{
		LogPrintf(LOG_DEBUG, "AAA");
		
		if (GetFlag(FLAG_A) || ((m_reg[REG8::AL] & 15) > 9))
		{
			++m_reg[REG8::AH];
			m_reg[REG8::AL] += 6;
			SetFlag(FLAG_A, true);
			SetFlag(FLAG_C, true);
		}
		else
		{
			SetFlag(FLAG_A, false);
			SetFlag(FLAG_C, false);
		}
		m_reg[REG8::AL] &= 0x0F;

		AdjustSign(m_reg[REG8::AL]);
		AdjustZero(m_reg[REG8::AL]);
		AdjustParity(m_reg[REG8::AL]);
	}

	void CPU8086::AAS()
	{
		LogPrintf(LOG_DEBUG, "AAS");

		if (GetFlag(FLAG_A) || ((m_reg[REG8::AL] & 15) > 9))
		{
			--m_reg[REG8::AH];
			m_reg[REG8::AL] -= 6;
			SetFlag(FLAG_A, true);
			SetFlag(FLAG_C, true);
		}
		else
		{
			SetFlag(FLAG_A, false);
			SetFlag(FLAG_C, false);
		}
		m_reg[REG8::AL] &= 0x0F;

		AdjustSign(m_reg[REG8::AL]);
		AdjustZero(m_reg[REG8::AL]);
		AdjustParity(m_reg[REG8::AL]);
	}

	void CPU8086::AAM(BYTE base)
	{
		LogPrintf(LOG_DEBUG, "AAM base %d", base);
		if (base == 0)
		{
			INT(0);
			return;
		}

		m_reg[REG8::AH] = m_reg[REG8::AL] / base;
		m_reg[REG8::AL] %= base;

		AdjustSign(m_reg[REG8::AL]);
		AdjustZero(m_reg[REG8::AL]);
		AdjustParity(m_reg[REG8::AL]);
	}

	void CPU8086::AAD(BYTE base)
	{
		LogPrintf(LOG_DEBUG, "AAD base %d", base);

		ArithmeticImm8(REG8::AL, base * m_reg[REG8::AH], rawAdd8);
		m_reg[REG8::AH] = 0;
	}

	void CPU8086::DAA()
	{
		LogPrintf(LOG_DEBUG, "DAA");

		if (GetFlag(FLAG_A) || ((m_reg[REG8::AL] & 15) > 9))
		{
			m_reg[REG8::AL] += 6;
			SetFlag(FLAG_A, true);
		}

		if (GetFlag(FLAG_C) || m_reg[REG8::AL] > 0x9F)
		{
			m_reg[REG8::AL] += 0x60;
			SetFlag(FLAG_C, true);
		}

		AdjustSign(m_reg[REG8::AL]);
		AdjustZero(m_reg[REG8::AL]);
		AdjustParity(m_reg[REG8::AL]);
	}

	void CPU8086::DAS()
	{
		LogPrintf(LOG_DEBUG, "DAS");

		BYTE oldAL = m_reg[REG8::AL];
		bool oldCF = GetFlag(FLAG_C);
		SetFlag(FLAG_C, false);

		if (GetFlag(FLAG_A) || ((m_reg[REG8::AL] & 15) > 9))
		{
			m_reg[REG8::AL] -= 6;
			SetFlag(FLAG_C, oldCF || (oldAL < 6));
			SetFlag(FLAG_A, true);
		}
		else
		{
			SetFlag(FLAG_A, false);
		}

		if (oldCF || oldAL > 0x99)
		{
			m_reg[REG8::AL] -= 0x60;
			SetFlag(FLAG_C, true);
		}

		AdjustSign(m_reg[REG8::AL]);
		AdjustZero(m_reg[REG8::AL]);
		AdjustParity(m_reg[REG8::AL]);
	}

	void CPU8086::LEA(BYTE modregrm)
	{
		LogPrintf(LOG_DEBUG, "LEA");

		SourceDest16 sd;

		// reg part
		LogPrintf(LOG_DEBUG, "LEA Target Register: %s", GetReg16Str(modregrm >> 3));
		Mem16 dest = GetReg16(modregrm >> 3, false);

		// TODO, duplication
		WORD displacement = 0;
		bool direct = false;
		switch (modregrm & 0xC0)
		{
		case 0xC0: // REG
			throw std::exception("register not valid source");
		case 0x00: // NO DISP (or DIRECT)
			if ((modregrm & 7) == 6) // Direct 
			{
				direct = true;
				displacement = FetchWord();
			}
			else
			{
				LogPrintf(LOG_DEBUG, "GetModRM16: MEM disp=0");
			}
			break;
		case 0x40:
			displacement = Widen(FetchByte());
			LogPrintf(LOG_DEBUG, "GetModRM16: MEM disp8=%04X", displacement);
			break;
		case 0x80:
			displacement = FetchWord();
			LogPrintf(LOG_DEBUG, "GetModRM16: MEM disp16=%04X", displacement);
			break;
		default:
			throw std::exception("GetModRM16: not implemented");
		}

		SegmentOffset segoff = GetEA(modregrm, direct);

		segoff.offset = (direct ? 0 : segoff.offset) + displacement;

		dest.Write(segoff.offset);
	}

	void CPU8086::SALC()
	{
		// Undocumented, Performs an operation equivalent to SBB AL,AL, but without modifying any flags. 
		// In other words, AL will be set to 0xFF or 0x00, depending on whether CF is set or clear.
		m_reg[REG8::AL] = GetFlag(FLAG_C) ? 0xFF : 0;
	}

	void CPU8086::Serialize(json& to)
	{
		to["ax"] = m_reg[REG16::AX];
		to["bx"] = m_reg[REG16::BX];
		to["cx"] = m_reg[REG16::CX];
		to["dx"] = m_reg[REG16::DX];

		to["sp"] = m_reg[REG16::SP];
		to["bp"] = m_reg[REG16::BP];
		to["si"] = m_reg[REG16::SI];
		to["di"] = m_reg[REG16::DI];

		to["cs"] = m_reg[REG16::CS];
		to["ds"] = m_reg[REG16::DS];
		to["ss"] = m_reg[REG16::SS];
		to["es"] = m_reg[REG16::ES];

		to["ip"] = m_reg[REG16::IP];
		to["flags"] = m_reg[REG16::FLAGS];

		to["lastOp"] = m_opcode;
		to["irqPending"] = m_irqPending;

		to["inRep"] = inRep;
		to["repIP"] = m_reg[REG16::_REP_IP];
		to["repZ"] = repZ;

		to["inSegOverride"] = inSegOverride;
		to["segOverride"] = m_reg[REG16::_SEG_O];
	}

	void CPU8086::Deserialize(const json& from)
	{
		m_reg[REG16::AX] = from["ax"];
		m_reg[REG16::BX] = from["bx"];
		m_reg[REG16::CX] = from["cx"];
		m_reg[REG16::DX] = from["dx"];

		m_reg[REG16::SP] = from["sp"];
		m_reg[REG16::BP] = from["bp"];
		m_reg[REG16::SI] = from["si"];
		m_reg[REG16::DI] = from["di"];

		m_reg[REG16::CS] = from["cs"];
		m_reg[REG16::DS] = from["ds"];
		m_reg[REG16::SS] = from["ss"];
		m_reg[REG16::ES] = from["es"];

		m_reg[REG16::IP] = from["ip"];
		m_reg[REG16::FLAGS] = from["flags"];

		m_opcode = from["lastOp"];
		m_irqPending = from["irqPending"];

		inRep = from["inRep"];
		m_reg[REG16::_REP_IP] = from["repIP"];
		repZ = from["repZ"];

		inSegOverride = from["inSegOverride"];
		m_reg[REG16::_SEG_O] = from["segOverride"];
	}

	bool SegmentOffset::FromString(const char* str)
	{
		if (!str || !str[0])
			return false;

		int seg;
		int off;
		int ret = sscanf(str, "%04X:%04X", &seg, &off);
		segment = WORD(seg);
		offset = WORD(off);

		return ((ret == 2) && 
			(seg >=0) && 
			(seg <=0xFFFF) && 
			(off >=0) && 
			(off <=0xFFFF));
	}

	const char* SegmentOffset::ToString() const
	{
		static char buf[16];
		sprintf(buf, "%04X:%04X", segment, offset);
		return buf;
	}

}
