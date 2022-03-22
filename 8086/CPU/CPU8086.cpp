#include "stdafx.h"

#include "CPU8086.h"

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


	CPU8086::CPU8086(Memory& memory, MemoryMap& mmap)
		: CPU(CPU8086_ADDRESS_BITS, memory, mmap), Logger("CPU8086")
	{
		Mem8::Init(&memory, &m_reg);
		Mem16::Init(&memory, &m_reg);
	}

	CPU8086::~CPU8086()
	{

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
			TICK(1);
			ret = true;
		}

		if (m_irqPending != -1)
		{
			assert(!inSegOverride);
			TICK(61);
			INT(m_irqPending);
			m_irqPending = -1;
			m_state = CPUState::RUN;
		}

		return ret;
	}

	void CPU8086::Exec(BYTE opcode)
	{
		//if (m_reg[REG16::CS] == 0x0100 && m_reg[REG16::IP] == 0x6F8D)
		//{
		//	__debugbreak();
		//}
		m_lastOp = opcode;

		bool trap = GetFlag(FLAG_T);

		++m_reg[REG16::IP];

		// Disable override after next instruction
		bool clearSegOverride = inSegOverride;

		switch(opcode)
		{
		// ADD
		// --------
		// REG8/MEM8, REG8
		case 0x00: Arithmetic8(GetModRegRM8(FetchByte(), false), rawAdd8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x01: Arithmetic16(GetModRegRM16(FetchByte(), false), rawAdd16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x02: Arithmetic8(GetModRegRM8(FetchByte(), true), rawAdd8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x03: Arithmetic16(GetModRegRM16(FetchByte(), true), rawAdd16); TICKRM(3, 9); break;

		// ADD
		// --------
		// AL, IMMED8
		case 0x04: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawAdd8); break;
		// AX, IMMED16
		case 0x05: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawAdd16); break;

		// PUSH ES (1)
		case 0x06: TICK(10); PUSH(REG16::ES); break;
		// POP ES (1)
		case 0x07: TICK(8); POP(REG16::ES); break;

		// OR
		// ----------
		// REG8/MEM8, REG8
		case 0x08: Arithmetic8(GetModRegRM8(FetchByte(), false), rawOr8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x09: Arithmetic16(GetModRegRM16(FetchByte(), false), rawOr16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x0A: Arithmetic8(GetModRegRM8(FetchByte(), true), rawOr8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x0B: Arithmetic16(GetModRegRM16(FetchByte(), true), rawOr16); TICKRM(3, 9); break;

		// OR
		// ----------
		// AL, IMMED8
		case 0x0C: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawOr8); break;
		// AX, IMMED16
		case 0x0D: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawOr16); break;

		// PUSH CS
		case 0x0E: TICK(10); PUSH(REG16::CS); break;
		// POP CS // Undocumented, 8086 only
		case 0x0F: TICK(8); POP(REG16::CS); break;

		// ADC
		// ----------
		// REG8/MEM8, REG8
		case 0x10: Arithmetic8(GetModRegRM8(FetchByte(), false), rawAdc8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x11: Arithmetic16(GetModRegRM16(FetchByte(), false), rawAdc16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x12: Arithmetic8(GetModRegRM8(FetchByte(), true), rawAdc8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x13: Arithmetic16(GetModRegRM16(FetchByte(), true), rawAdc16); TICKRM(3, 9); break;


		// ADC
		// ----------
		// AL, IMMED8
		case 0x14: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawAdc8); break;
		// AX, IMMED16
		case 0x15: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawAdc16); break;

		// PUSH SS
		case 0x16: TICK(10); PUSH(REG16::SS); break;
		// POP SS
		case 0x17: TICK(8); POP(REG16::SS); break;

		// SBB
		// ----------
		// REG8/MEM8, REG8
		case 0x18: Arithmetic8(GetModRegRM8(FetchByte(), false), rawSbb8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x19: Arithmetic16(GetModRegRM16(FetchByte(), false), rawSbb16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x1A: Arithmetic8(GetModRegRM8(FetchByte(), true), rawSbb8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x1B: Arithmetic16(GetModRegRM16(FetchByte(), true), rawSbb16); TICKRM(3, 9); break;

		// SBB
		// ----------
		// AL, IMMED8
		case 0x1C: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawSbb8); break;
		// AX, IMMED16
		case 0x1D: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawSbb16); break;

		// PUSH DS (1)
		case 0x1E: TICK(10); PUSH(REG16::DS); break;
		// POP DS
		case 0x1F: TICK(8); POP(REG16::DS); break;

		// AND
		// ----------
		// REG8/MEM8, REG8
		case 0x20: Arithmetic8(GetModRegRM8(FetchByte(), false), rawAnd8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x21: Arithmetic16(GetModRegRM16(FetchByte(), false), rawAnd16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x22: Arithmetic8(GetModRegRM8(FetchByte(), true), rawAnd8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x23: Arithmetic16(GetModRegRM16(FetchByte(), true), rawAnd16); TICKRM(3, 9); break;

		// AND
		// ----------
		// AL, IMMED8
		case 0x24: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawAnd8); break;
		// AX, IMMED16
		case 0x25: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawAnd16); break;

		// ES Segment Override
		case 0x26: TICK(2); SEGOVERRIDE(m_reg[REG16::ES]); break;

		// DAA
		case 0x27: TICK(4); DAA(); break;

		// SUB
		// ----------
		// REG8/MEM8, REG8
		case 0x28: Arithmetic8(GetModRegRM8(FetchByte(), false), rawSub8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x29: Arithmetic16(GetModRegRM16(FetchByte(), false), rawSub16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x2A: Arithmetic8(GetModRegRM8(FetchByte(), true), rawSub8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x2B: Arithmetic16(GetModRegRM16(FetchByte(), true), rawSub16); TICKRM(3, 9); break;

		// SUB
		// ----------
		// AL, IMMED8
		case 0x2C: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawSub8); break;
		// AX, IMMED16
		case 0x2D: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawSub16); break;

		// CS Segment Override
		case 0x2E: TICK(2); SEGOVERRIDE(m_reg[REG16::CS]); break;

		// DAS
		case 0x2F: TICK(4); DAS(); break;

		// XOR
		// ----------
		// REG8/MEM8, REG8
		case 0x30: Arithmetic8(GetModRegRM8(FetchByte(), false), rawXor8); TICKRM(3, 16); break;
		// REG16/MEM16, REG16
		case 0x31: Arithmetic16(GetModRegRM16(FetchByte(), false), rawXor16); TICKRM(3, 16); break;
		// REG8, REG8/MEM8
		case 0x32: Arithmetic8(GetModRegRM8(FetchByte(), true), rawXor8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x33: Arithmetic16(GetModRegRM16(FetchByte(), true), rawXor16); TICKRM(3, 9); break;

		// XOR
		// ----------
		// AL, IMMED8
		case 0x34: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawXor8); break;
		// AX, IMMED16
		case 0x35: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawXor16); break;

		// SS Segment Override
		case 0x36: TICK(2); SEGOVERRIDE(m_reg[REG16::SS]); break;

		// AAA
		case 0x37: TICK(4);  AAA(); break;

		// CMP
		// ----------
		// REG8/MEM8, REG8
		case 0x38: Arithmetic8(GetModRegRM8(FetchByte(), false), rawCmp8); TICKRM(3, 9); break;
		// REG16/MEM16, REG16
		case 0x39: Arithmetic16(GetModRegRM16(FetchByte(), false), rawCmp16); TICKRM(3, 9); break;
		// REG8, REG8/MEM8
		case 0x3A: Arithmetic8(GetModRegRM8(FetchByte(), true), rawCmp8); TICKRM(3, 9); break;
		// REG16, REG16/MEM16
		case 0x3B: Arithmetic16(GetModRegRM16(FetchByte(), true), rawCmp16); TICKRM(3, 9); break;

		// CMP
		// ----------
		// AL, IMMED8
		case 0x3C: TICK(4); ArithmeticImm8(REG8::AL, FetchByte(), rawCmp8); break;
		// AX, IMMED16
		case 0x3D: TICK(4); ArithmeticImm16(REG16::AX, FetchWord(), rawCmp16); break;

		// DS Segment Override
		case 0x3E: TICK(2); SEGOVERRIDE(m_reg[REG16::DS]); break;

		// AAS
		case 0x3F: TICK(4); AAS(); break;

		// INC
		// ----------
		// INC AX
		case 0x40: TICK(2); INC16(m_reg[REG16::AX]); break;
		// INC CX
		case 0x41: TICK(2); INC16(m_reg[REG16::CX]); break;
		// INC DX
		case 0x42: TICK(2); INC16(m_reg[REG16::DX]); break;
		// INC BX
		case 0x43: TICK(2); INC16(m_reg[REG16::BX]); break;
		// INC SP
		case 0x44: TICK(2); INC16(m_reg[REG16::SP]); break;
		// INC BP
		case 0x45: TICK(2); INC16(m_reg[REG16::BP]); break;
		// INC SI
		case 0x46: TICK(2); INC16(m_reg[REG16::SI]); break;
		// INC DI
		case 0x47: TICK(2); INC16(m_reg[REG16::DI]); break;

		// DEC
		// ----------
		// DEC AX
		case 0x48: TICK(2); DEC16(m_reg[REG16::AX]); break;
		// DEC CX
		case 0x49: TICK(2); DEC16(m_reg[REG16::CX]); break;
		// DEC DX
		case 0x4A: TICK(2); DEC16(m_reg[REG16::DX]); break;
		// DEC BX
		case 0x4B: TICK(2); DEC16(m_reg[REG16::BX]); break;
		// DEC SP
		case 0x4C: TICK(2); DEC16(m_reg[REG16::SP]); break;
		// DEC BP
		case 0x4D: TICK(2); DEC16(m_reg[REG16::BP]); break;
		// DEC SI
		case 0x4E: TICK(2); DEC16(m_reg[REG16::SI]); break;
		// DEC DI
		case 0x4F: TICK(2); DEC16(m_reg[REG16::DI]); break;

		// PUSH
		// ----------
		// PUSH AX
		case 0x50: TICK(11); PUSH(REG16::AX); break;
		// PUSH CX
		case 0x51: TICK(11); PUSH(REG16::CX); break;
		// PUSH DX
		case 0x52: TICK(11); PUSH(REG16::DX); break;
		// PUSH BX
		case 0x53: TICK(11); PUSH(REG16::BX); break;
		// PUSH SP
		// "Bug" on 8086/80186 where push sp pushes an already-decremented value
		case 0x54: TICK(11); PUSH(m_reg[REG16::SP] - 2); break;
		// PUSH BP
		case 0x55: TICK(11); PUSH(REG16::BP); break;
		// PUSH SI
		case 0x56: TICK(11); PUSH(REG16::SI); break;
		// PUSH DI
		case 0x57: TICK(11); PUSH(REG16::DI); break;

		// POP
		// ----------
		// POP AX
		case 0x58: TICK(8); POP(REG16::AX); break;
		// POP CX
		case 0x59: TICK(8); POP(REG16::CX); break;
		// POP DX
		case 0x5A: TICK(8); POP(REG16::DX); break;
		// POP BX
		case 0x5B: TICK(8); POP(REG16::BX); break;
		// POP SP
		case 0x5C: TICK(8); POP(REG16::SP); break;
		// POP BP
		case 0x5D: TICK(8); POP(REG16::BP); break;
		// POP SI
		case 0x5E: TICK(8); POP(REG16::SI); break;
		// POP DI
		case 0x5F: TICK(8); POP(REG16::DI); break;

		// Undocumented: 0x60-0x6F maps to 0x70-0x7F on 8086 only
		// JO
		case 0x60:
		case 0x70: JMPif(GetFlag(FLAG_O)); break;
		// JNO
		case 0x61:
		case 0x71: JMPif(!GetFlag(FLAG_O)); break;
		// JB/JNAE/JC
		case 0x62:
		case 0x72: JMPif(GetFlag(FLAG_C)); break;
		// JNB/JAE/JNC
		case 0x63:
		case 0x73: JMPif(!GetFlag(FLAG_C)); break;
		// JE/JZ
		case 0x64:
		case 0x74: JMPif(GetFlag(FLAG_Z)); break;
		// JNE/JNZ
		case 0x65:
		case 0x75: JMPif(!GetFlag(FLAG_Z)); break;
		// JBE/JNA
		case 0x66:
		case 0x76: JMPif(GetFlagNotAbove()); break;
		// JNBE/JA
		case 0x67:
		case 0x77: JMPif(!GetFlagNotAbove()); break;
		// JS
		case 0x68:
		case 0x78: JMPif(GetFlag(FLAG_S)); break;
		// JNS
		case 0x69:
		case 0x79: JMPif(!GetFlag(FLAG_S)); break;
		// JP/JPE
		case 0x6A:
		case 0x7A: JMPif(GetFlag(FLAG_P)); break;
		// JNP/JPO
		case 0x6B:
		case 0x7B: JMPif(!GetFlag(FLAG_P)); break;
		// JL/JNGE
		case 0x6C:
		case 0x7C: JMPif(!GetFlagNotLess()); break;
		// JNL/JGE
		case 0x6D:
		case 0x7D: JMPif(GetFlagNotLess()); break;
		// JLE/JNG
		case 0x6E:
		case 0x7E: JMPif(!GetFlagGreater()); break;
		// JNLE/JG
		case 0x6F:
		case 0x7F: JMPif(GetFlagGreater()); break;

		//----------
		// ADD/OR/ADC/SBB/AND/SUB/XOR/CMP
		// ----------
		// REG8/MEM8, IMM8
		case 0x80: ArithmeticImm8(FetchByte()); TICKRM(4, 17); break; // TODO: CMP is 10
		// REG16/MEM16, IMM16
		case 0x81: ArithmeticImm16(FetchByte(), false); TICKRM(4, 17); break; // imm data = word

		// ADD/--/ADC/SBB/---/SUB/---/CMP w/sign Extension
		// ----------
		// REG8/MEM8, IMM8 (same as 0x80)
		case 0x82: ArithmeticImm8(FetchByte()); TICKRM(4, 17); break; // TODO: CMP is 10
		// REG16/MEM16, IMM8 (sign-extend to 16)
		case 0x83: ArithmeticImm16(FetchByte(), true); TICKRM(4, 17); break; // imm data = sign-extended byte

		// TEST
		// ----------
		// REG8/MEM8, REG8
		case 0x84: Arithmetic8(GetModRegRM8(FetchByte(), true), rawTest8); TICKRM(3, 9); break;
		// REG16/MEM16, REG16
		case 0x85: Arithmetic16(GetModRegRM16(FetchByte(), true), rawTest16); TICKRM(3, 9); break;

		// XCHG
		// ----------
		// REG8/MEM8, REG8
		case 0x86: XCHG8(GetModRegRM8(FetchByte())); TICKRM(4, 17); break;
		// REG16/MEM16, REG16
		case 0x87: XCHG16(GetModRegRM16(FetchByte())); TICKRM(4, 17); break;

		// MOV
		// ----------
		// REG8/MEM8, REG8
		case 0x88: MOV8(GetModRegRM8(FetchByte(), false)); TICKRM(2, 9); break;
		// REG16/MEM16, REG16
		case 0x89: MOV16(GetModRegRM16(FetchByte(), false)); TICKRM(2, 9); break;
		// REG8, REG8/MEM8
		case 0x8A: MOV8(GetModRegRM8(FetchByte(), true)); TICKRM(2, 8); break;
		// REG16, REG16/MEM16
		case 0x8B: MOV16(GetModRegRM16(FetchByte(), true)); TICKRM(2, 8); break;

		// MOV
		// ----------
		// MOV REG16/MEM16, SEGREG
		case 0x8C: MOV16(GetModRegRM16(FetchByte(), false, true)); TICKRM(2, 9); break;

		// LEA
		// ----------
		// REG16, MEM16
		case 0x8D: TICK(2); LEA(FetchByte()); break;

		// MOV
		// ----------
		// MOV SEGREG, REG16/MEM16
		case 0x8E: MOV16(GetModRegRM16(FetchByte(), true, true)); TICKRM(2, 8); break;

		// POP
		// ----------
		// POP REG16/MEM16
		case 0x8F: POP(GetModRM16(FetchByte())); TICKRM(8, 17); break;

		// XCHG
		// ----------
		// XCHG AX, AX (NOP)
		case 0x90: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::AX]); break;
		// XCHG AX, CX
		case 0x91: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::CX]); break;
		// XCHG AX, DX
		case 0x92: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::DX]); break;
		// XCHG AX, BX
		case 0x93: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::BX]); break;
		// XCHG AX, SP
		case 0x94: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::SP]); break;
		// XCHG AX, BP
		case 0x95: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::BP]); break;
		// XCHG AX, SI
		case 0x96: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::SI]); break;
		// XCHG AX, DI
		case 0x97: TICK(3); XCHG16(m_reg[REG16::AX], m_reg[REG16::DI]); break;

		// CBW
		case 0x98: TICK(2); CBW(); break;
		// CWD
		case 0x99: TICK(5); CWD(); break;

		// CALL Far
		case 0x9A: TICK(28); CALLfar(); break;

		// WAIT
		case 0x9B: NotImplemented(opcode); break;

		// PUSHF
		case 0x9C: TICK(10); PUSHF(); break;
		// POPF
		case 0x9D: TICK(8); POPF(); break;
		// SAHF
		case 0x9E: TICK(4); SAHF(); break;
		// LAHF
		case 0x9F: TICK(4); LAHF(); break;

		// MOV
		// ----------
		// MOV AL, MEM8
		case 0xA0: TICK(10); MOV8(REG8::AL, m_memory.Read8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord()))); break;
		// MOV AX, MEM16
		case 0xA1: TICK(10); MOV16(REG16::AX, m_memory.Read16(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord()))); break;

		// MOV
		// ----------
		// MOV MEM8, AL
		case 0xA2: TICK(10); MOV8(Mem8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord())), m_reg[REG8::AL]); break;
		// MOV MEM16, AX
		case 0xA3: TICK(10); MOV16(Mem16(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], FetchWord())), m_reg[REG16::AX]); break;

		// MOVS
		// ----------
		// MOVS DEST-STR8, SRC-STR8
		case 0xA4: TICK(17); MOVS8(); break;
		// MOVS DEST-STR16, SRC-STR16
		case 0xA5: TICK(17); MOVS16(); break;

		// CMPS
		// ----------
		// CMPS DEST-STR8, SRC-STR8
		case 0xA6: TICK(22); CMPS8(); break;
		// CMPS DEST-STR16, SRC-STR16
		case 0xA7: TICK(22); CMPS16(); break;

		// TEST
		// ----------
		// TEST AL, IMM8
		case 0xA8: TICK(5); ArithmeticImm8(REG8::AL, FetchByte(), rawTest8); break;
		// TEST AX, IMM16
		case 0xA9: TICK(5); ArithmeticImm16(REG16::AX, FetchWord(), rawTest16); break;

		// STOS
		// ----------
		// STOS DEST-STR8
		case 0xAA: TICK(10); STOS8(); break;
		// STOS DEST-STR16
		case 0xAB: TICK(10); STOS16(); break;

		// LODS
		// ----------
		// LODS SRC-STR8
		case 0xAC: TICK(12); LODS8(); break;
		// LODS SRC-STR16
		case 0xAD: TICK(12); LODS16(); break;

		// SCAS
		// ----------
		// SCAS DEST-STR8
		case 0xAE: TICK(15); SCAS8(); break;
		// SCAS DEST-STR16
		case 0xAF: TICK(15); SCAS16(); break;

		// MOV
		// ----------
		// MOV AL, IMM8
		case 0xB0: TICK(4); MOV8(REG8::AL, FetchByte()); break;
		// MOV CL, IMM8
		case 0xB1: TICK(4); MOV8(REG8::CL, FetchByte()); break;
		// MOV DL, IMM8
		case 0xB2: TICK(4); MOV8(REG8::DL, FetchByte()); break;
		// MOV BL, IMM8
		case 0xB3: TICK(4); MOV8(REG8::BL, FetchByte()); break;
		// MOV AH, IMM8
		case 0xB4: TICK(4); MOV8(REG8::AH, FetchByte()); break;
		// MOV CH, IMM8
		case 0xB5: TICK(4); MOV8(REG8::CH, FetchByte()); break;
		// MOV DH, IMM8
		case 0xB6: TICK(4); MOV8(REG8::DH, FetchByte()); break;
		// MOV BH, IMM8
		case 0xB7: TICK(4); MOV8(REG8::BH, FetchByte()); break;

		// MOV AX, IMM16
		case 0xB8: TICK(4); MOV16(REG16::AX, FetchWord()); break;
		// MOV CX, IMM16
		case 0xB9: TICK(4); MOV16(REG16::CX, FetchWord()); break;
		// MOV DX, IMM16
		case 0xBA: TICK(4); MOV16(REG16::DX, FetchWord()); break;
		// MOV BX, IMM16
		case 0xBB: TICK(4); MOV16(REG16::BX, FetchWord()); break;
		// MOV SP, IMM16
		case 0xBC: TICK(4); MOV16(REG16::SP, FetchWord()); break;
		// MOV BP, IMM16
		case 0xBD: TICK(4); MOV16(REG16::BP, FetchWord()); break;
		// MOV SI, IMM16
		case 0xBE: TICK(4); MOV16(REG16::SI, FetchWord()); break;
		// MOV DI, IMM16
		case 0xBF: TICK(4); MOV16(REG16::DI, FetchWord()); break;

		// RET SP+IMM16
		case 0xC0: // Undocumented, 8086 only
		case 0xC2: TICK(12); RETNear(true, FetchWord()); break;
		// RET Near
		case 0xC1: // Undocumented, 8086 only
		case 0xC3: TICK(8); RETNear(); break;

		// LES REG16, MEM16
		case 0xC4: TICK(16); LoadPTR(m_reg[REG16::ES], GetModRegRM16(FetchByte(), true)); break;
		// LDS REG16, MEM16
		case 0xC5: TICK(16); LoadPTR(m_reg[REG16::DS], GetModRegRM16(FetchByte(), true)); break;

		// MOV
		// ----------
		// MOV MEM8, IMM8
		case 0xC6: TICK(10); MOVIMM8(GetModRM8(FetchByte())); break;
		// MOV MEM16, IMM16
		case 0xC7: TICK(10); MOVIMM16(GetModRM16(FetchByte())); break;

		// RET Far SP+IMM16
		case 0xC8: //Undocumented, 8086 only
		case 0xCA: TICK(18); RETFar(true, FetchWord()); break;
		// RET Far
		case 0xC9: //Undocumented, 8086 only
		case 0xCB: TICK(17); RETFar(); break;

		// INT3
		case 0xCC: TICK(52); INT(3); break;
		// INT IMM8
		case 0xCD: TICK(51); INT(FetchByte()); break;
		// INTO
		case 0xCE: TICK(4);  if (GetFlag(FLAG_O)) { TICK(49); INT(4); } break;
		// IRET
		case 0xCF: TICK(24); IRET(); break;

		// ROL/ROR/RCL/RCR/SAL|SHL/SHR/---/SAR
		// ----------
		// REG8/MEM8, 1
		case 0xD0: SHIFTROT8(FetchByte(), 1); TICKRM(5, 15); break;
		// REG16/MEM16, 1
		case 0xD1: SHIFTROT16(FetchByte(), 1); TICKRM(5, 15); break;
		// REG8/MEM8, CL
		case 0xD2: SHIFTROT8(FetchByte(), m_reg[REG8::CL]); TICKRM(4 * m_reg[REG8::CL] + 8, 4 * m_reg[REG8::CL] + 20); break;
		// REG16/MEM16, CL
		case 0xD3: SHIFTROT16(FetchByte(), m_reg[REG8::CL]); TICKRM(4 * m_reg[REG8::CL] + 8, 4 * m_reg[REG8::CL] + 20); break;

		// AAM
		case 0xD4: TICK(83); AAM(FetchByte()); break;
		// AAD
		case 0xD5: TICK(60); AAD(FetchByte()); break;

		// Undocumented, Performs an operation equivalent to SBB AL,AL, but without modifying any flags. 
		// In other words, AL will be set to 0xFF or 0x00, depending on whether CF is set or clear.
		case 0xD6: TICK(3); SALC(); break;

		// XLAT
		case 0xD7: TICK(11); XLAT(); break;

		// ESC
		case 0xD8: 
		case 0xD9: 
		case 0xDA: 
		case 0xDB: 
		case 0xDC: 
		case 0xDD: 
		case 0xDE: 
		case 0xDF: GetModRegRM16(FetchByte()); break;

		// LOOPNZ/LOOPNE
		case 0xE0: TICK(1); LOOP(FetchByte(), GetFlag(FLAG_Z) == false); break;
		// LOOPZ/LOOPE
		case 0xE1: TICK(1); LOOP(FetchByte(), GetFlag(FLAG_Z) == true); break;
		// LOOP
		case 0xE2: LOOP(FetchByte()); break;
		// JCXZ
		case 0xE3: TICK(2); JMPif(m_reg[REG16::CX] == 0); break;

		// IN fixed
		// --------
		// IN AL, IMM8
		case 0xE4: TICK(10); IN8(FetchByte()); break;
		// IN AX, IMM16
		case 0xE5: TICK(10); IN16(FetchByte()); break;

		// OUT fixed
		// --------
		// OUT PORT8, AL
		case 0xE6: TICK(10); OUT8(FetchByte()); break;
		// OUT PORT8, AX
		case 0xE7: TICK(10); OUT16(FetchByte()); break;

		// CALL Near
		case 0xE8: TICK(19); CALLNear(FetchWord()); break;
		// JUMP Near
		case 0xE9: TICK(15); JMPNear(FetchWord()); break;
		// JUMP Far
		case 0xEA: TICK(15); JMPfar(); break;
		// JUMP Near Short
		case 0xEB: TICK(15); JMPNear(FetchByte()); break;

		// IN variable
		// --------
		// IN AL, DX
		case 0xEC: TICK(8); IN8(m_reg[REG16::DX]); break;
		// IN AX, DX
		case 0xED: TICK(8); IN16(m_reg[REG16::DX]); break;

		// OUT variable
		// --------
		// OUT DX, AL
		case 0xEE: TICK(8); OUT8(m_reg[REG16::DX]); break;
		// OUT DX, AX
		case 0xEF: TICK(8); OUT16(m_reg[REG16::DX]); break;

		// LOCK
		case 0xF0: TICK(2); NotImplemented(opcode); break;

		// REPNZ/REPNE
		case 0xF2: TICK(2); REP(false); break;
		// REPZ/REPE
		case 0xF3: TICK(2); REP(true); break;

		// HLT
		case 0xF4: HLT(); break;
		// CMC
		case 0xF5: TICK(2); CMC(); break;

		// TEST/---/NOT/NEG/MUL/IMUL/DIV/IDIV
		// --------
		// REG8/MEM8 (, IMM8 {TEST})
		case 0xF6: ArithmeticMulti8(FetchByte()); break;
		// REG16/MEM16 (, IMM16 {TEST})
		case 0xF7: ArithmeticMulti16(FetchByte()); break;

		case 0xF8: TICK(2); CLC(); break;
		// STC (1)
		case 0xF9: TICK(2); STC(); break;
		// CLI (1)
		case 0xFA: TICK(2); CLI(); break;
		// STI (1)
		case 0xFB: TICK(2); STI(); break;
		// CLD (1)
		case 0xFC: TICK(2); CLD(); break;
		// STD (1)
		case 0xFD: TICK(2); STD(); break;

		// INC/DEC/---/---/---/---/---/---
		// --------
		// REG8/MEM8
		case 0xFE: TICK(3); INCDEC8(FetchByte()); break;

		// INC/DEC/CALL/CALL/JMP/JMP/PUSH/---
		case 0xFF: MultiFunc(FetchByte()); break;

		default: UnknownOpcode(opcode);
		}

		// Disable override after next instruction
		if (clearSegOverride)
		{
			inSegOverride = false;
		}

		// Check for trap
		if (trap && GetFlag(FLAG_T))
		{
			LogPrintf(LOG_INFO, "TRAP AT CS=%04X, IP=%04X", m_reg[REG16::CS], m_reg[REG16::IP]);
			TICK(50);
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
			TICK(1);
			return SegmentOffset{ m_reg[REG16::DS], 0 };
		}

		switch (modregrm & 7)
		{
		case 0: TICK(1); return SegmentOffset{ m_reg[REG16::DS], (WORD)(m_reg[REG16::BX] + m_reg[REG16::SI]) };
		case 1: TICK(2); return SegmentOffset{ m_reg[REG16::DS], (WORD)(m_reg[REG16::BX] + m_reg[REG16::DI]) };
		case 2: TICK(2); return SegmentOffset{ m_reg[REG16::SS], (WORD)(m_reg[REG16::BP] + m_reg[REG16::SI]) };
		case 3: TICK(1); return SegmentOffset{ m_reg[REG16::SS], (WORD)(m_reg[REG16::BP] + m_reg[REG16::DI]) };

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
		TICK(5);
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
			TICK(4);
			break;
		case 0x80:
			displacement = FetchWord();
			TICK(4);
			break;
		default:
			throw std::exception("GetModRM8: not implemented");
		}
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
		TICK(5);
		switch (modrm & 0xC0)
		{
		case 0xC0: // REG
			m_regMem = REGMEM::REG;
			LogPrintf(LOG_DEBUG, "GetModRM16: REG %s", GetReg16Str(modrm));
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
			TICK(4);
			break;
		case 0x80:
			displacement = FetchWord();
			TICK(4);
			break;
		default:
			throw std::exception("GetModRM16: not implemented");
		}
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
		destPtr.Increment(m_memory);
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
		destPtr.Increment(m_memory);
		m_reg[REG16::CS] = destPtr.Read();
		LogPrintf(LOG_DEBUG, "JMPInter newCS=%04X, newIP=%04X", m_reg[REG16::CS], m_reg[REG16::IP]);
	}

	void CPU8086::NotImplemented(BYTE op)
	{
		EnableLog(LOG_DEBUG);
		LogPrintf(LOG_ERROR, "Not implemented op=%x", op);
		m_state = CPUState::STOP;
	}

	void CPU8086::INCDEC8(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "INCDEC8");

		Mem8 dest = GetModRM8(op2);

		switch (op2 & 0x38)
		{
		case 0: // INC
			INC8(dest);
			break;
		case 8: // DEC
			DEC8(dest);
			break;
		default:
			throw std::exception("INCDEC8: invalid op2");
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
		TICK(4);
		LogPrintf(LOG_DEBUG, "JMPif %d", cond);
		BYTE offset = FetchByte();
		if (cond)
		{
			TICK(12);
			m_reg[REG16::IP] += Widen(offset);
		}
	}

	// Rotate left: n = (n << d)|(n >> (BITS - d))
	// Rotate right: n = (n >> d)|(n << (BITS - d))

	void CPU8086::SHIFTROT8(BYTE op2, BYTE count)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT8 op2=" PRINTF_BIN_PATTERN_INT8 ", count=%d", PRINTF_BYTE_TO_BIN_INT8(op2), count);

		Mem8 dest = GetModRM8(op2);
		BYTE work = dest.Read();

		op2 &= 0x38;

		if (count == 0)
		{
			return;
		}

		// TODO: Ugly but approximates what i8086 does
		LogPrintf(LOG_DEBUG, "SHIFTROT8 before=" PRINTF_BIN_PATTERN_INT8 " (%02X)", PRINTF_BYTE_TO_BIN_INT8(work), work);
		for (BYTE i = 0; i < count; ++i)
		{
			BYTE before = work;
			BYTE sign;
			bool carry;
			switch (op2 & 0x38)
			{
			case 0x00: // ROL
				LogPrintf(LOG_DEBUG, "SHIFTROT8 ROL");
				SetFlag(FLAG_C, GetMSB(work));
				work = (work << 1) | (work >> 7);
				break;
			case 0x08: // ROR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 ROR");
				SetFlag(FLAG_C, GetLSB(work));
				work = (work >> 1) | (work << 7);
				break;
			case 0x10: // RCL
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCL");
				carry = GetFlag(FLAG_C);
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				work |= (carry ? 1 : 0);
				break;
			case 0x18: // RCR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCR");
				carry = GetLSB(work);
				work >>= 1;
				work |= (GetFlag(FLAG_C) ? 128 : 0);
				SetFlag(FLAG_C, carry);
				break;
			case 0x20: // SHL/SAL
			case 0x30: // Undocumented 
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SHL");
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				break;
			case 0x28: // SHR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SHR");
				SetFlag(FLAG_C, GetLSB(work));
				work >>= 1;
				break;
			case 0x38: // SAR
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
		}
		dest.Write(work);
		LogPrintf(LOG_DEBUG, "SHIFTROT8 after=" PRINTF_BIN_PATTERN_INT8 " (%02X)", PRINTF_BYTE_TO_BIN_INT8(work), work);

		if (op2 >= 0x20) // Only shift operation adjusts SZP flags
		{
			AdjustSign(work);
			AdjustZero(work);
			AdjustParity(work);
		}
	}

	void CPU8086::SHIFTROT16(BYTE op2, BYTE count)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT16 op2=" PRINTF_BIN_PATTERN_INT8 ", count=%d", PRINTF_BYTE_TO_BIN_INT8(op2), count);

		Mem16 dest = GetModRM16(op2);
		WORD work = dest.Read();

		op2 &= 0x38;

		if (count == 0)
		{
			return;
		}

		// TODO: Ugly but approximates what i8086 does
		LogPrintf(LOG_DEBUG, "SHIFTROT16 before=" PRINTF_BIN_PATTERN_INT16 " (%04X)", PRINTF_BYTE_TO_BIN_INT16(work), work);
		for (BYTE i = 0; i < count; ++i)
		{
			WORD before = work;
			WORD sign;
			bool carry;
			switch (op2)
			{
			case 0x00: // ROL
				LogPrintf(LOG_DEBUG, "SHIFTROT16 ROL");
				SetFlag(FLAG_C, GetMSB(work));
				work = (work << 1) | (work >> 15);
				break;
			case 0x08: // ROR
				LogPrintf(LOG_DEBUG, "SHIFTROT16 ROR");
				SetFlag(FLAG_C, GetLSB(work));
				work = (work >> 1) | (work << 15);
				break;
			case 0x10: // RCL
				LogPrintf(LOG_DEBUG, "SHIFTROT16 RCL");
				carry = GetFlag(FLAG_C);
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				work |= (carry ? 1 : 0);
				break;
			case 0x18: // RCR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCR");
				carry = GetLSB(work);
				work >>= 1;
				work |= (GetFlag(FLAG_C) ? 32768 : 0);
				SetFlag(FLAG_C, carry);
				break;
			case 0x20: // SHL/SAL
			case 0x30: // Undocumented 
				LogPrintf(LOG_DEBUG, "SHIFTROT16 SHL");
				SetFlag(FLAG_C, GetMSB(work));
				work <<= 1;
				break;
			case 0x28: // SHR
				LogPrintf(LOG_DEBUG, "SHIFTROT16 SHR");
				SetFlag(FLAG_C, GetLSB(work));
				work >>= 1;
				break;
			case 0x38: // SAR
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
		}
		dest.Write(work);
		LogPrintf(LOG_DEBUG, "SHIFTROT16 after=" PRINTF_BIN_PATTERN_INT16 " (%04X)", PRINTF_BYTE_TO_BIN_INT16(work), work);

		if (op2 >= 0x20) // Only shift operation adjusts flags
		{
			AdjustSign(work);
			AdjustZero(work);
			AdjustParity(work);
		}
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
		TICK(5);
		--m_reg[REG16::CX];
		LogPrintf(LOG_DEBUG, "LOOP, CX=%04X", m_reg[REG16::CX]);
		if (m_reg[REG16::CX] && cond)
		{
			TICK(12);
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

	void CPU8086::ArithmeticImm8(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticImm8");

		RawOpFunc8 func;

		switch (op2 & 0x38)
		{
		case 0x00: func = rawAdd8; LogPrintf(LOG_DEBUG, "add"); break;
		case 0x08: func = rawOr8;  LogPrintf(LOG_DEBUG, "or");  break;
		case 0x10: func = rawAdc8; LogPrintf(LOG_DEBUG, "adc"); break;
		case 0x18: func = rawSbb8; LogPrintf(LOG_DEBUG, "sbb"); break;
		case 0x20: func = rawAnd8; LogPrintf(LOG_DEBUG, "and"); break;
		case 0x28: func = rawSub8; LogPrintf(LOG_DEBUG, "sub"); break;
		case 0x30: func = rawXor8; LogPrintf(LOG_DEBUG, "xor"); break;
		case 0x38: func = rawCmp8; LogPrintf(LOG_DEBUG, "cmp"); break;
		default:
			throw(std::exception("not possible"));
		}

		SourceDest8 sd;
		sd.dest = GetModRM8(op2);

		m_reg[REG8::_T0] = FetchByte();
		sd.source = REG8::_T0;

		Arithmetic8(sd, func);
	}
	void CPU8086::ArithmeticImm16(BYTE op2, bool signExtend)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticImm16");

		RawOpFunc16 func;

		switch (op2 & 0x38)
		{
		case 0x00: func = rawAdd16; LogPrintf(LOG_DEBUG, "add"); break;
		case 0x08: func = rawOr16;  LogPrintf(LOG_DEBUG, "or");  break;
		case 0x10: func = rawAdc16; LogPrintf(LOG_DEBUG, "adc"); break;
		case 0x18: func = rawSbb16; LogPrintf(LOG_DEBUG, "sbb"); break;
		case 0x20: func = rawAnd16; LogPrintf(LOG_DEBUG, "and"); break;
		case 0x28: func = rawSub16; LogPrintf(LOG_DEBUG, "sub"); break;
		case 0x30: func = rawXor16; LogPrintf(LOG_DEBUG, "xor"); break;
		case 0x38: func = rawCmp16; LogPrintf(LOG_DEBUG, "cmp"); break;
		default:
			throw(std::exception("not possible"));
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

		switch (op2 & 0x38)
		{
		case 0x00: // TEST
		case 0x08: // Undocumented
		{
			LogPrintf(LOG_DEBUG, "TEST8");
			TICKRM(4, 17);
			BYTE imm = FetchByte();
			BYTE after = (BYTE)rawTest8(val, imm, false);
			SetFlag(FLAG_O, false);
			SetFlag(FLAG_C, false);
			AdjustSign(after);
			AdjustZero(after);
			AdjustParity(after);
			break;
		}
		case 0x10: // NOT
		{
			LogPrintf(LOG_DEBUG, "NOT16");
			TICKRM(3, 16);
			modrm.Write(~val);
			break;
		}
		case 0x18: // NEG
		{
			TICKRM(3, 16);
			SourceDest8 sd;
			m_reg[REG8::_T0] = 0;
			sd.dest = REG8::_T0;
			sd.source = modrm;
			Arithmetic8(sd, rawSub8);
			LogPrintf(LOG_DEBUG, "NEG8, -(%02X) = %02X", val, m_reg[REG8::_T0]);
			modrm.Write(m_reg[REG8::_T0]);
			break;
		}
		case 0x20: // MUL
		{
			TICKRM(72, 80); // Varies
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
		case 0x28: // IMUL
		{
			TICKRM(82, 90); // Varies
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
		case 0x30:
		{
			LogPrintf(LOG_DEBUG, "DIV8");
			TICKRM(82, 90); // Varies
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
		case 0x38:
		{
			LogPrintf(LOG_DEBUG, "IDIV8");
			TICKRM(104, 110); // Varies
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
		default:
			throw(std::exception("not possible"));
		}
	}

	void CPU8086::ArithmeticMulti16(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticMulti16");

		Mem16 modrm = GetModRM16(op2);
		WORD val = modrm.Read();

		switch (op2 & 0x38)
		{
		case 0x00: // TEST
		case 0x08:
		{
			LogPrintf(LOG_DEBUG, "TEST16");
			TICKRM(5, 11); // Varies
			WORD imm = FetchWord();
			WORD after = rawTest16(val, imm, false);
			SetFlag(FLAG_O, false);
			SetFlag(FLAG_C, false);
			AdjustSign(after);
			AdjustZero(after);
			AdjustParity(after);
			break;
		}
		case 0x10: // NOT
		{
			LogPrintf(LOG_DEBUG, "NOT16");
			TICKRM(3, 16);
			modrm.Write(~val);
			break;
		}
		case 0x18: // NEG
		{
			TICKRM(3, 16);
			SourceDest16 sd;
			m_reg[REG16::_T0] = 0;
			sd.dest = REG16::_T0;
			sd.source = modrm;
			Arithmetic16(sd, rawSub16);
			LogPrintf(LOG_DEBUG, "NEG16, -(%04X) = %04X", val, m_reg[REG16::_T0]);
			modrm.Write(m_reg[REG16::_T0]);
			break;
		}
		case 0x20: // MUL
		{
			TICKRM(120, 128); // Varies
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
		case 0x28: // IMUL
		{
			TICKRM(132, 140); // Varies
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
		case 0x30:
		{
			LogPrintf(LOG_DEBUG, "DIV16");
			TICKRM(150, 158); // Varies
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
		case 0x38:
		{
			LogPrintf(LOG_DEBUG, "IDIV16");
			TICKRM(170, 178); // Varies
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
		default:
			throw(std::exception("not possible"));
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
		LogPrintf(LOG_DEBUG, "PUSH %04X", w);
		m_memory.Write8(S2A(m_reg[REG16::SS], --m_reg[REG16::SP]), GetHByte(w));
		m_memory.Write8(S2A(m_reg[REG16::SS], --m_reg[REG16::SP]), GetLByte(w));
	}

	void CPU8086::POP(Mem16 dest)
	{
		BYTE lo = m_memory.Read8(S2A(m_reg[REG16::SS], m_reg[REG16::SP]++));
		BYTE hi = m_memory.Read8(S2A(m_reg[REG16::SS], m_reg[REG16::SP]++));
		WORD w = MakeWord(hi, lo);
		dest.Write(w);
		LogPrintf(LOG_DEBUG, "POP %04X", w);
	}

	void CPU8086::PUSHF()
	{
		PUSH(REG16::FLAGS);
	}

	void CPU8086::POPF()
	{
		POP(REG16::FLAGS); 
		SetBitMask(m_reg[REG16::FLAGS], FLAG_RESERVED_OFF, false); 
		SetBitMask(m_reg[REG16::FLAGS], FLAG_RESERVED_ON, true);
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
			return true;

		if (m_reg[REG16::CX] == 0)
		{
			LogPrintf(LOG_DEBUG, "PreRep, end loop");
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
			TICK(9);
		}
		else
		{
			LogPrintf(LOG_DEBUG, "PostRep, end loop");
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

		switch (op2 & 0x38)
		{
		// INC/DEC MEM16
		case 0x00: TICK(15); INC16(val); dest.Write(val);  break;
		case 0x08: TICK(15); DEC16(val); dest.Write(val); break;
		// CALL RM16(intra) / CALL MEM16(intersegment)
		case 0x10: TICK(21); CALLIntra(val); break;
		case 0x18: TICK(37); CALLInter(dest); break;
		// JMP RM16(intra) // JMP MEM16(intersegment)
		case 0x20: TICK(18); JMPIntra(val); break;
		case 0x28: TICK(24); JMPInter(dest); break;
		// PUSH MEM16
		case 0x30: TICK(16); PUSH(dest); break;
		// not used
		case 0x38: LogPrintf(LOG_WARNING, "Multifunc(0x28): not used"); break;

		default:
			throw(std::exception("not possible"));
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
		regMem.source.Increment(m_memory);
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

		to["lastOp"] = m_lastOp;
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

		m_lastOp = from["lastOp"];
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
