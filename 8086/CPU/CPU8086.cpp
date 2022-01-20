#include "CPU8086.h"

namespace emul
{
	WORD rawAdd8(BYTE& dest, const BYTE& src, bool) { WORD r = dest + src; dest = (BYTE)r; return r; }
	WORD rawSub8(BYTE& dest, const BYTE& src, bool) { WORD r = dest - src; dest = (BYTE)r; return r; }
	WORD rawCmp8(BYTE& dest, const BYTE& src, bool) { return dest - src; }
	WORD rawAdc8(BYTE& dest, const BYTE& src, bool c) { WORD r = dest + src + (BYTE)c; dest = (BYTE)r; return r; }
	WORD rawSbb8(BYTE& dest, const BYTE& src, bool b) { WORD r = dest - src - (BYTE)b; dest = (BYTE)r; return r; }

	WORD rawAnd8(BYTE& dest, const BYTE& src, bool) { dest &= src; return dest; }
	WORD rawOr8(BYTE& dest, const BYTE& src, bool) { dest |= src; return dest; }
	WORD rawXor8(BYTE& dest, const BYTE& src, bool) { dest ^= src; return dest; }
	WORD rawTest8(BYTE& dest, const BYTE& src, bool) { return dest & src; }

	DWORD rawAdd16(WORD& dest, const WORD& src, bool) { DWORD r = dest + src; dest = (WORD)r; return r; }
	DWORD rawSub16(WORD& dest, const WORD& src, bool) { DWORD r = dest - src; dest = (WORD)r; return r; }
	DWORD rawCmp16(WORD& dest, const WORD& src, bool) { return dest - src; }
	DWORD rawAdc16(WORD& dest, const WORD& src, bool c) { DWORD r = dest + src + WORD(c); dest = (WORD)r; return r; }
	DWORD rawSbb16(WORD& dest, const WORD& src, bool b) { DWORD r = dest - src - WORD(b); dest = (WORD)r; return r; }

	DWORD rawAnd16(WORD& dest, const WORD& src, bool) { dest &= src; return dest; }
	DWORD rawOr16(WORD& dest, const WORD& src, bool) { dest |= src; return dest; }
	DWORD rawXor16(WORD& dest, const WORD& src, bool) { dest ^= src; return dest; }
	DWORD rawTest16(WORD& dest, const WORD& src, bool) { return dest & src; }

	CPU8086::CPU8086(Memory& memory, MemoryMap& mmap)
		: CPU(CPU8086_ADDRESS_BITS, memory, mmap), Logger("CPU8086")
	{
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

		if (m_irqPending != -1)
		{
			assert(!inSegOverride);
			TICK(61);
			INT(m_irqPending);
			m_irqPending = -1;
		}

		return ret;
	}

	void CPU8086::Exec(BYTE opcode)
	{
		//if (regCS.x == 0x0100 && regIP.x == 0x6F8D)
		//{
		//	__debugbreak();
		//}
		m_lastOp = opcode;

		bool trap = GetFlag(FLAG_T);

		++regIP.x;

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
		case 0x04: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawAdd8); break;
		// AX, IMMED16
		case 0x05: TICK(4); ArithmeticImm16(regA, FetchWord(), rawAdd16); break;

		// PUSH ES (1)
		case 0x06: TICK(10); PUSH(regES); break;
		// POP ES (1)
		case 0x07: TICK(8); POP(regES); break;

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
		case 0x0C: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawOr8); break;
		// AX, IMMED16
		case 0x0D: TICK(4); ArithmeticImm16(regA, FetchWord(), rawOr16); break;

		// PUSH CS
		case 0x0E: TICK(10); PUSH(regCS); break;
		// POP CS // Undocumented, 8086 only
		case 0x0F: TICK(8); POP(regCS); break;

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
		case 0x14: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawAdc8); break;
		// AX, IMMED16
		case 0x15: TICK(4); ArithmeticImm16(regA, FetchWord(), rawAdc16); break;

		// PUSH SS
		case 0x16: TICK(10); PUSH(regSS); break;
		// POP SS
		case 0x17: TICK(8); POP(regSS); break;

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
		case 0x1C: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawSbb8); break;
		// AX, IMMED16
		case 0x1D: TICK(4); ArithmeticImm16(regA, FetchWord(), rawSbb16); break;

		// PUSH DS (1)
		case 0x1E: TICK(10); PUSH(regDS); break;
		// POP DS
		case 0x1F: TICK(8); POP(regDS); break;

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
		case 0x24: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawAnd8); break;
		// AX, IMMED16
		case 0x25: TICK(4); ArithmeticImm16(regA, FetchWord(), rawAnd16); break;

		// ES Segment Override
		case 0x26: TICK(2); SEGOVERRIDE(regES.x); break;

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
		case 0x2C: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawSub8); break;
		// AX, IMMED16
		case 0x2D: TICK(4); ArithmeticImm16(regA, FetchWord(), rawSub16); break;

		// CS Segment Override
		case 0x2E: TICK(2); SEGOVERRIDE(regCS.x); break;

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
		case 0x34: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawXor8); break;
		// AX, IMMED16
		case 0x35: TICK(4); ArithmeticImm16(regA, FetchWord(), rawXor16); break;

		// SS Segment Override
		case 0x36: TICK(2); SEGOVERRIDE(regSS.x); break;

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
		case 0x3C: TICK(4); ArithmeticImm8(regA.hl.l, FetchByte(), rawCmp8); break;
		// AX, IMMED16
		case 0x3D: TICK(4); ArithmeticImm16(regA, FetchWord(), rawCmp16); break;

		// DS Segment Override
		case 0x3E: TICK(2); SEGOVERRIDE(regDS.x); break;

		// AAS
		case 0x3F: TICK(4); AAS(); break;

		// INC
		// ----------
		// INC AX
		case 0x40: TICK(2); INC16(regA.x); break;
		// INC CX
		case 0x41: TICK(2); INC16(regC.x); break;
		// INC DX
		case 0x42: TICK(2); INC16(regD.x); break;
		// INC BX
		case 0x43: TICK(2); INC16(regB.x); break;
		// INC SP
		case 0x44: TICK(2); INC16(regSP.x); break;
		// INC BP
		case 0x45: TICK(2); INC16(regBP.x); break;
		// INC SI
		case 0x46: TICK(2); INC16(regSI.x); break;
		// INC DI
		case 0x47: TICK(2); INC16(regDI.x); break;

		// DEC
		// ----------
		// DEC AX
		case 0x48: TICK(2); DEC16(regA.x); break;
		// DEC CX
		case 0x49: TICK(2); DEC16(regC.x); break;
		// DEC DX
		case 0x4A: TICK(2); DEC16(regD.x); break;
		// DEC BX
		case 0x4B: TICK(2); DEC16(regB.x); break;
		// DEC SP
		case 0x4C: TICK(2); DEC16(regSP.x); break;
		// DEC BP
		case 0x4D: TICK(2); DEC16(regBP.x); break;
		// DEC SI
		case 0x4E: TICK(2); DEC16(regSI.x); break;
		// DEC DI
		case 0x4F: TICK(2); DEC16(regDI.x); break;

		// PUSH
		// ----------
		// PUSH AX
		case 0x50: TICK(11); PUSH(regA); break;
		// PUSH CX
		case 0x51: TICK(11); PUSH(regC); break;
		// PUSH DX
		case 0x52: TICK(11); PUSH(regD); break;
		// PUSH BX
		case 0x53: TICK(11); PUSH(regB); break;
		// PUSH SP
		case 0x54: TICK(11); PUSH(regSP.x); break;
		// PUSH BP
		case 0x55: TICK(11); PUSH(regBP.x); break;
		// PUSH SI
		case 0x56: TICK(11); PUSH(regSI.x); break;
		// PUSH DI
		case 0x57: TICK(11); PUSH(regDI.x); break;

		// POP
		// ----------
		// POP AX
		case 0x58: TICK(8); POP(regA); break;
		// POP CX
		case 0x59: TICK(8); POP(regC); break;
		// POP DX
		case 0x5A: TICK(8); POP(regD); break;
		// POP BX
		case 0x5B: TICK(8); POP(regB); break;
		// POP SP
		case 0x5C: TICK(8); POP(regSP); break;
		// POP BP
		case 0x5D: TICK(8); POP(regBP); break;
		// POP SI
		case 0x5E: TICK(8); POP(regSI); break;
		// POP DI
		case 0x5F: TICK(8); POP(regDI); break;

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
		case 0x90: TICK(3); XCHG16(regA.x, regA.x); break;
		// XCHG AX, CX
		case 0x91: TICK(3); XCHG16(regA.x, regC.x); break;
		// XCHG AX, DX
		case 0x92: TICK(3); XCHG16(regA.x, regD.x); break;
		// XCHG AX, BX
		case 0x93: TICK(3); XCHG16(regA.x, regB.x); break;
		// XCHG AX, SP
		case 0x94: TICK(3); XCHG16(regA.x, regSP.x); break;
		// XCHG AX, BP
		case 0x95: TICK(3); XCHG16(regA.x, regBP.x); break;
		// XCHG AX, SI
		case 0x96: TICK(3); XCHG16(regA.x, regSI.x); break;
		// XCHG AX, DI
		case 0x97: TICK(3); XCHG16(regA.x, regDI.x); break;

		// CBW
		case 0x98: TICK(2); CBW(); break;
		// CWD
		case 0x99: TICK(5); CWD(); break;

		// CALL Far
		case 0x9A: TICK(28); CALLfar(); break;

		// WAIT
		case 0x9B: NotImplemented(opcode); break;

		// PUSHF
		case 0x9C: TICK(10); PUSH(flags); break;
		// POPF
		case 0x9D: TICK(8); POP(flags); flags.x |= FLAG_R1; flags.x &= ~(FLAG_R3 | FLAG_R5 | FLAG_R12 | FLAG_R13 | FLAG_R14 | FLAG_R15); break;  // TODO: Clean flags in function
		// SAHF
		case 0x9E: TICK(4); SAHF(); break;
		// LAHF
		case 0x9F: TICK(4); LAHF(); break;

		// MOV
		// ----------
		// MOV AL, MEM8
		case 0xA0: TICK(10); MOV8(&regA.hl.l, m_memory.Read8(S2A(inSegOverride ? segOverride : regDS.x, FetchWord()))); break;
		// MOV AX, MEM16
		case 0xA1: TICK(10); MOV16(regA, m_memory.Read16(S2A(inSegOverride ? segOverride : regDS.x, FetchWord()))); break;

		// MOV
		// ----------
		// MOV MEM8, AL
		case 0xA2: TICK(10); MOV8(m_memory.GetPtr8(S2A(inSegOverride ? segOverride : regDS.x, FetchWord())), regA.hl.l); break;
		// MOV MEM16, AX
		case 0xA3: TICK(10); MOV16(Mem16(m_memory, S2A(inSegOverride ? segOverride : regDS.x, FetchWord())), regA.x); break;

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
		case 0xA8: TICK(5); ArithmeticImm8(regA.hl.l, FetchByte(), rawTest8); break;
		// TEST AX, IMM16
		case 0xA9: TICK(5); ArithmeticImm16(regA, FetchWord(), rawTest16); break;

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
		case 0xB0: TICK(4); MOV8(&regA.hl.l, FetchByte()); break;
		// MOV CL, IMM8
		case 0xB1: TICK(4); MOV8(&regC.hl.l, FetchByte()); break;
		// MOV DL, IMM8
		case 0xB2: TICK(4); MOV8(&regD.hl.l, FetchByte()); break;
		// MOV BL, IMM8
		case 0xB3: TICK(4); MOV8(&regB.hl.l, FetchByte()); break;
		// MOV AH, IMM8
		case 0xB4: TICK(4); MOV8(&regA.hl.h, FetchByte()); break;
		// MOV CH, IMM8
		case 0xB5: TICK(4); MOV8(&regC.hl.h, FetchByte()); break;
		// MOV DH, IMM8
		case 0xB6: TICK(4); MOV8(&regD.hl.h, FetchByte()); break;
		// MOV BH, IMM8
		case 0xB7: TICK(4); MOV8(&regB.hl.h, FetchByte()); break;

		// MOV AX, IMM16
		case 0xB8: TICK(4); MOV16(regA, FetchWord()); break;
		// MOV CX, IMM16
		case 0xB9: TICK(4); MOV16(regC, FetchWord()); break;
		// MOV DX, IMM16
		case 0xBA: TICK(4); MOV16(regD, FetchWord()); break;
		// MOV BX, IMM16
		case 0xBB: TICK(4); MOV16(regB, FetchWord()); break;
		// MOV SP, IMM16
		case 0xBC: TICK(4); MOV16(regSP, FetchWord()); break;
		// MOV BP, IMM16
		case 0xBD: TICK(4); MOV16(regBP, FetchWord()); break;
		// MOV SI, IMM16
		case 0xBE: TICK(4); MOV16(regSI, FetchWord()); break;
		// MOV DI, IMM16
		case 0xBF: TICK(4); MOV16(regDI, FetchWord()); break;

		// RET SP+IMM16
		case 0xC0: // Undocumented, 8086 only
		case 0xC2: TICK(12); RETNear(true, FetchWord()); break;
		// RET Near
		case 0xC1: // Undocumented, 8086 only
		case 0xC3: TICK(8); RETNear(); break;

		// LES REG16, MEM16
		case 0xC4: TICK(16); LoadPTR(regES.x, GetModRegRM16(FetchByte(), true)); break;
		// LDS REG16, MEM16
		case 0xC5: TICK(16); LoadPTR(regDS.x, GetModRegRM16(FetchByte(), true)); break;

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
		case 0xD2: SHIFTROT8(FetchByte(), regC.hl.l); TICKRM(4 * regC.hl.l + 8, 4 * regC.hl.l + 20); break;
		// REG16/MEM16, CL
		case 0xD3: SHIFTROT16(FetchByte(), regC.hl.l); TICKRM(4 * regC.hl.l + 8, 4 * regC.hl.l + 20); break;

		// AAM
		case 0xD4: TICK(83); AAM(FetchByte()); break;
		// AAD
		case 0xD5: TICK(60); AAD(FetchByte()); break;

		// TODO: Undocumented, Performs an operation equivalent to SBB AL,AL, but without modifying any flags. 
		// In other words, AL will be set to 0xFF or 0x00, depending on whether CF is set or clear.
		// case 0xD6: SALC(); break

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
		case 0xE3: TICK(2); JMPif(regC.x == 0); break;

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
		case 0xEC: TICK(8); IN8(regD.x); break;
		// IN AX, DX
		case 0xED: TICK(8); IN16(regD.x); break;

		// OUT variable
		// --------
		// OUT DX, AL
		case 0xEE: TICK(8); OUT8(regD.x); break;
		// OUT DX, AX
		case 0xEF: TICK(8); OUT16(regD.x); break;

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
			LogPrintf(LOG_INFO, "TRAP AT CS=%04X, IP=%04X", regCS.x, regIP.x);
			TICK(50);
			INT(1);
		}
	}

	void CPU8086::AddDevice(PortConnector& ports)
	{
		ports.ConnectTo(m_ports);
	}

	void CPU8086::Reset()
	{
		CPU::Reset();

		ClearFlags();
		regIP.x = 0x0000;
		regCS.x = 0xFFFF;
		regDS.x = 0x0000;
		regSS.x = 0x0000;
		regES.x = 0x0000;

		inRep = false;
		repIP = 0x0000;

		inSegOverride = false;
		segOverride = 0x000;
	}

	void CPU8086::Reset(WORD segment, WORD offset)
	{
		CPU8086::Reset();
		regCS.x = segment;
		regIP.x = offset;

		LogPrintf(LOG_DEBUG, "RESET AT CS=%04X, IP=%04X", regCS.x, regIP.x);
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
			regA.hl.h, regA.hl.l,
			regB.hl.h, regB.hl.l,
			regC.hl.h, regC.hl.l,
			regD.hl.h, regD.hl.l,
			regCS.x, regIP.x,
			regDS.x, regSI.x,
			regES.x, regDI.x,
			regSS.x, regSP.x,
			regBP.x,
			PRINTF_BYTE_TO_BIN_INT16(flags.x));
	}

	void CPU8086::DumpInterruptTable()
	{
		LogPrintf(LOG_ERROR, "INTERRUPT TABLE @ %04X:%04X", regCS.x, regIP.x);
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
		flags.x = FLAG_R1; //| FLAG_R12 | FLAG_R13;
	}

	BYTE CPU8086::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++regIP.x;
		return b;
	}
	WORD CPU8086::FetchWord()
	{
		Register r;
		r.hl.l = m_memory.Read8(GetCurrentAddress());
		++regIP.x;
		r.hl.h = m_memory.Read8(GetCurrentAddress());
		++regIP.x;

		return r.x;
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

	BYTE* CPU8086::GetReg8(BYTE reg)
	{
		switch (reg & 7)
		{
		case 0: return &regA.hl.l;
		case 1: return &regC.hl.l;
		case 2: return &regD.hl.l;
		case 3: return &regB.hl.l;

		case 4: return &regA.hl.h;
		case 5: return &regC.hl.h;
		case 6: return &regD.hl.h;
		case 7: return &regB.hl.h;
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
		case 0: return segReg ? regES : regA;
		case 1: return segReg ? regCS : regC;
		case 2: return segReg ? regSS : regD;
		case 3: return segReg ? regDS : regB;
		// Undocumented: 4-7 segment register same as 0-3 on 808x/8018x
		case 4: return segReg ? regES : regSP;
		case 5: return segReg ? regCS : regBP;
		case 6: return segReg ? regSS : regSI;
		case 7: return segReg ? regDS : regDI;
		}
		throw std::exception("not possible");
	}

	SegmentOffset CPU8086::GetEA(BYTE modregrm, bool direct)
	{
		if (direct)
		{
			TICK(1);
			return std::make_tuple(regDS.x, 0);
		}

		switch (modregrm & 7)
		{
		case 0: TICK(1); return std::make_tuple(regDS.x, regB.x + regSI.x);
		case 1: TICK(2); return std::make_tuple(regDS.x, regB.x + regDI.x);
		case 2: TICK(2); return std::make_tuple(regSS.x, regBP.x + regSI.x);
		case 3: TICK(1); return std::make_tuple(regSS.x, regBP.x + regDI.x);

		case 4: return std::make_tuple(regDS.x, regSI.x);
		case 5: return std::make_tuple(regDS.x, regDI.x);
		case 6: return std::make_tuple(regSS.x, regBP.x);
		case 7: return std::make_tuple(regDS.x, regB.x);
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

	BYTE* CPU8086::GetModRM8(BYTE modrm)
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
		WORD& segment = std::get<0>(segoff);
		WORD& offset = std::get<1>(segoff);

		if (inSegOverride)
		{
			segment = segOverride;
		}

		offset = (direct ? 0 : offset) + displacement;
		return m_memory.GetPtr8(S2A(segment, offset));
	}

	SourceDest8 CPU8086::GetModRegRM8(BYTE modregrm, bool toReg)
	{
		SourceDest8 sd;

		// reg part
		BYTE* reg = GetReg8(modregrm >> 3);
		BYTE* modrm = GetModRM8(modregrm);

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
		WORD& segment = std::get<0>(segoff);
		WORD& offset = std::get<1>(segoff);

		if (inSegOverride)
		{
			segment = segOverride;
		}

		offset = (direct ? 0 : offset) + displacement;
		LogPrintf(LOG_DEBUG, "GetModRM16: MEM %04X:%04X", segment, offset);
		return Mem16(m_memory, S2A(segment, offset));
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
		regA.hl.h = GetMSB(regA.hl.l) ? 0xFF : 0;
	}

	void CPU8086::CWD()
	{
		LogPrintf(LOG_DEBUG, "CWD");
		regD.x = GetMSB(regA.x) ? 0xFFFF : 0;
	}

	void CPU8086::HLT()
	{
		EnableLog(LOG_DEBUG);
		LogPrintf(LOG_ERROR, "HALT");
		Dump();
		m_state = CPUState::STOP;
	}
	
	void CPU8086::CALLNear(WORD offset)
	{
		LogPrintf(LOG_DEBUG, "CALLNear Byte offset %02X", offset);
		PUSH(regIP.x);
		regIP.x += offset;
	}

	void CPU8086::CALLIntra(WORD address)
	{
		LogPrintf(LOG_DEBUG, "CALLIntra newIP=%04X", address);
		PUSH(regIP.x);
		regIP.x = address;
	}

	void CPU8086::CALLfar()
	{
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		LogPrintf(LOG_DEBUG, "CALLfar %02X|%02X", segment, offset);
		PUSH(regCS.x);
		PUSH(regIP.x);
		regCS.x = segment;
		regIP.x = offset;
	}

	void CPU8086::CALLInter(Mem16 destPtr)
	{
		PUSH(regCS.x);
		PUSH(regIP.x);

		regIP.x = destPtr.GetValue();
		destPtr.Increment(m_memory);
		regCS.x = destPtr.GetValue();
		LogPrintf(LOG_DEBUG, "CALLInter newCS=%04X, newIP=%04X", regCS.x, regIP.x);
	}

	void CPU8086::JMPfar()
	{
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		LogPrintf(LOG_DEBUG, "JMPfar %02X|%02X", segment, offset);
		regCS.x = segment;
		regIP.x = offset;
	}

	void CPU8086::JMPNear(BYTE offset)
	{
		LogPrintf(LOG_DEBUG, "JMPNear Byte offset %02X", offset);
		regIP.x += Widen(offset);
	}
	void CPU8086::JMPNear(WORD offset)
	{
		LogPrintf(LOG_DEBUG, "JMPNear Word offset %04X", offset);
		regIP.x += offset;
	}

	void CPU8086::JMPIntra(WORD address)
	{
		LogPrintf(LOG_DEBUG, "JMPIntra newIP=%04X", address);
		regIP.x = address;
	}

	void CPU8086::JMPInter(Mem16 destPtr)
	{
		regIP.x = destPtr.GetValue();
		destPtr.Increment(m_memory);
		regCS.x = destPtr.GetValue();
		LogPrintf(LOG_DEBUG, "JMPInter newCS=%04X, newIP=%04X", regCS.x, regIP.x);
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

		BYTE* dest = GetModRM8(op2);

		switch (op2 & 0x38)
		{
		case 0: // INC
			INC8(*dest);
			break;
		case 8: // DEC
			DEC8(*dest);
			break;
		default:
			throw std::exception("INCDEC8: invalid op2");
		}
	}

	void CPU8086::INC8(BYTE& b)
	{
		BYTE before = b;

		++b;

		SetFlag(FLAG_O, (!GetMSB(before) && GetMSB(b)));
		AdjustSign(b);
		AdjustZero(b);
		SetFlag(FLAG_A, ((before & 0x0F) == 0x0F));
		AdjustParity(b);

		LogPrintf(LOG_DEBUG, "INC8 %02X->%02X", before, b);
	}

	void CPU8086::DEC8(BYTE& b)
	{
		LogPrintf(LOG_DEBUG, "DEC8");
		BYTE before = b;

		--b;

		SetFlag(FLAG_O, (GetMSB(before) && !GetMSB(b)));
		AdjustSign(b);
		AdjustZero(b);
		SetFlag(FLAG_A, ((before & 0x0F) == 0));
		AdjustParity(b);

		LogPrintf(LOG_DEBUG, "DEC8 %02X->%02X", before, b);
	}

	void CPU8086::INC16(WORD& w)
	{
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
		WORD before = w;

		--w;

		SetFlag(FLAG_O, (GetMSB(before) && !GetMSB(w)));
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, ((before & 0x000F) == 0));
		AdjustParity(w);

		LogPrintf(LOG_DEBUG, "DEC16 %04X->%04X", before, w);
	}

	void CPU8086::MOV8(BYTE* d, BYTE s)
	{
		*d = s;
	}
	void CPU8086::MOV8(SourceDest8 sd)
	{
		*(sd.dest) = *(sd.source);
	}
	void CPU8086::MOVIMM8(BYTE* dest)
	{
		*(dest) = FetchByte();
	}

	void CPU8086::MOV16(Mem16 d, WORD s)
	{
		d.SetValue(s);
	}
	void CPU8086::MOV16(SourceDest16 sd)
	{
		sd.dest.SetValue(sd.source.GetValue());
	}
	void CPU8086::MOVIMM16(Mem16 dest)
	{
		dest.SetValue(FetchWord());
	}

	void CPU8086::SAHF()
	{
		LogPrintf(LOG_DEBUG, "SAHF");

		const WORD mask = FLAG_C | FLAG_P | FLAG_A | FLAG_Z | FLAG_S;

		flags.x &= (~mask);
		flags.x |= (regA.hl.h & mask);
	}
	void CPU8086::LAHF()
	{
		LogPrintf(LOG_DEBUG, "LAHF");

		regA.hl.h = (flags.x & 0x00FF);
	}

	void CPU8086::JMPif(bool cond)
	{
		TICK(4);
		LogPrintf(LOG_DEBUG, "JMPif %d", cond);
		BYTE offset = FetchByte();
		if (cond)
		{
			TICK(12);
			regIP.x += Widen(offset);
		}
	}

	// Rotate left: n = (n << d)|(n >> (BITS - d))
	// Rotate right: n = (n >> d)|(n << (BITS - d))

	void CPU8086::SHIFTROT8(BYTE op2, BYTE count)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT8 op2=" PRINTF_BIN_PATTERN_INT8 ", count=%d", PRINTF_BYTE_TO_BIN_INT8(op2), count);

		BYTE* b = GetModRM8(op2);
		BYTE& dest = *b;

		op2 &= 0x38;

		count &= 0b11111; // Technically done on 186 and above

		if (count == 0)
		{
			return;
		}

		// TODO: Ugly but approximates what i8086 does
		LogPrintf(LOG_DEBUG, "SHIFTROT8 before=" PRINTF_BIN_PATTERN_INT8 " (%02X)", PRINTF_BYTE_TO_BIN_INT8(dest), dest, dest);
		for (BYTE i = 0; i < count; ++i)
		{
			BYTE before = dest;
			BYTE sign;
			bool carry;
			switch (op2 & 0x38)
			{
			case 0x00: // ROL
				LogPrintf(LOG_DEBUG, "SHIFTROT8 ROL");
				SetFlag(FLAG_C, GetMSB(dest));
				dest = (dest << 1) | (dest >> 7);
				break;
			case 0x08: // ROR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 ROR");
				SetFlag(FLAG_C, GetLSB(dest));
				dest = (dest >> 1) | (dest << 7);
				break;
			case 0x10: // RCL
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCL");
				carry = GetFlag(FLAG_C);
				SetFlag(FLAG_C, GetMSB(dest));
				dest <<= 1;
				dest |= (carry ? 1 : 0);
				break;
			case 0x18: // RCR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 RCR");
				carry = GetLSB(dest);
				dest >>= 1;
				dest |= (GetFlag(FLAG_C) ? 128 : 0);
				SetFlag(FLAG_C, carry);
				break;
			case 0x20: // SHL/SAL
			case 0x30: // Undocumented 
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SHL");
				SetFlag(FLAG_C, GetMSB(dest));
				dest <<= 1;
				break;
			case 0x28: // SHR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SHR");
				SetFlag(FLAG_C, GetLSB(dest));
				dest >>= 1;
				break;
			case 0x38: // SAR
				LogPrintf(LOG_DEBUG, "SHIFTROT8 SAR");
				SetFlag(FLAG_C, GetLSB(dest));
				sign = (dest & 128);
				dest >>= 1;
				dest |= sign;
				break;
			default:
				throw(std::exception("not possible"));
			}

			SetFlag(FLAG_O, GetMSB(before) != GetMSB(dest));
		}
		LogPrintf(LOG_DEBUG, "SHIFTROT8 after=" PRINTF_BIN_PATTERN_INT8 " (%02X)", PRINTF_BYTE_TO_BIN_INT8(dest), dest, dest);

		if (op2 >= 0x20) // Only shift operation adjusts SZP flags
		{
			AdjustSign(dest);
			AdjustZero(dest);
			AdjustParity(dest);
		}
	}

	void CPU8086::SHIFTROT16(BYTE op2, BYTE count)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT16 op2=" PRINTF_BIN_PATTERN_INT8 ", count=%d", PRINTF_BYTE_TO_BIN_INT8(op2), count);

		Mem16 dest = GetModRM16(op2);
		WORD work = dest.GetValue();

		op2 &= 0x38;

		count &= 0b11111; // Technically done on 186 and above
		if (count == 0)
		{
			return;
		}

		// TODO: Ugly but approximates what i8086 does
		LogPrintf(LOG_DEBUG, "SHIFTROT16 before=" PRINTF_BIN_PATTERN_INT16 " (%04X)", PRINTF_BYTE_TO_BIN_INT16(dest.GetValue()), dest.GetValue());
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
		dest.SetValue(work);
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
		const BYTE& source = *(sd.source);
		BYTE& dest = *(sd.dest);

		// AC Calculation
		const BYTE source4 = source & 0x0F;
		BYTE dest4 = dest & 0x0F;
		WORD after4 = func(dest4, source4, GetFlag(FLAG_C));
		SetFlag(FLAG_A, after4 > 0x0F);

		BYTE before = dest;
		WORD after = func(dest, source, GetFlag(FLAG_C));
		BYTE afterB = (BYTE)after;
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
		const WORD source = sd.source.GetValue();
		WORD dest = sd.dest.GetValue();

		// AC Calculations
		const WORD source4 = source & 0x0F;
		WORD dest4 = dest & 0x0F;
		WORD after4 = func(dest4, source4, GetFlag(FLAG_C));
		SetFlag(FLAG_A, after4 > 0x0F);

		WORD before = dest;
		DWORD after = func(dest, source, GetFlag(FLAG_C));
		WORD afterW = (WORD)after;
		if (dest != before)
		{
			sd.dest.SetValue(dest);
		}
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

	void CPU8086::ArithmeticImm8(BYTE& dest, BYTE imm, RawOpFunc8 func)
	{
		SourceDest8 sd(&dest, &imm);
		Arithmetic8(sd, func);
	}
	void CPU8086::ArithmeticImm16(Mem16 dest, WORD imm, RawOpFunc16 func)
	{
		Register source;
		source.x = imm;
		SourceDest16 sd;
		sd.source = source;
		sd.dest = dest;
		Arithmetic16(sd, func);
	}

	void CPU8086::IN8(WORD port)
	{
		LogPrintf(LOG_DEBUG, "IN8 port %04X", port);
		m_ports.In(port, regA.hl.l);
	}

	void CPU8086::IN16(WORD port)
	{
		LogPrintf(LOG_DEBUG, "IN16 port %04X", port);
		m_ports.In(port, regA.hl.l);
		m_ports.In(port+1, regA.hl.h);
	}

	void CPU8086::OUT8(WORD port)
	{
		m_ports.Out(port, regA.hl.l);
	}

	void CPU8086::OUT16(WORD port)
	{
		m_ports.Out(port, regA.hl.l);
		m_ports.Out(port+1, regA.hl.h);
	}

	void CPU8086::LOOP(BYTE offset, bool cond)
	{
		TICK(5);
		--regC.x;
		LogPrintf(LOG_DEBUG, "LOOP, CX=%04X", regC.x);
		if (regC.x && cond)
		{
			TICK(12);
			regIP.x += Widen(offset);
		}
	}

	void CPU8086::RETNear(bool pop, WORD value)
	{
		LogPrintf(LOG_DEBUG, "RETNear [%s][%d]", pop?"Pop":"NoPop", value);

		POP(regIP);
		regSP.x += value;
	}

	void CPU8086::RETFar(bool pop, WORD value)
	{
		LogPrintf(LOG_DEBUG, "RETFar [%s][%d]", pop ? "Pop" : "NoPop", value);

		POP(regIP);
		POP(regCS);
		regSP.x += value;
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

		BYTE imm = FetchByte();
		sd.source = &imm;

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

		Register imm;
		imm.x = signExtend ? Widen(FetchByte()) : FetchWord();
		sd.source = imm;

		Arithmetic16(sd, func);
	}

	void CPU8086::ArithmeticMulti8(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticMulti8");

		BYTE* modrm = GetModRM8(op2);

		switch (op2 & 0x38)
		{
		case 0x00: // TEST
		case 0x08: // Undocumented
		{
			LogPrintf(LOG_DEBUG, "TEST8");
			TICKRM(4, 17);
			BYTE imm = FetchByte();
			BYTE after = (BYTE)rawTest8(*modrm, imm, false);
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
			*modrm = ~(*modrm);
			break;
		}
		case 0x20: // MUL
		{
			TICKRM(72, 80); // Varies
			WORD result = regA.hl.l * (*modrm);
			LogPrintf(LOG_DEBUG, "MUL8, %02X * %02X = %04X", regA.hl.l, *modrm, result);
			regA.x = result;
			SetFlag(FLAG_O, regA.hl.h != 0);
			SetFlag(FLAG_C, regA.hl.h != 0);
			AdjustSign(regA.hl.l);
			AdjustZero(regA.hl.l);
			AdjustParity(regA.hl.l);
			break;
		}
		case 0x18: // NEG
		{
			TICKRM(3, 16);
			BYTE tempDest = 0;
			SourceDest8 sd;
			sd.dest = &tempDest;
			sd.source = modrm;
			Arithmetic8(sd, rawSub8);
			LogPrintf(LOG_DEBUG, "NEG8, -(%02X) = %02X", *modrm, tempDest);
			*modrm = tempDest;
			break;
		}
		case 0x28: // IMUL
		{
			TICKRM(82, 90); // Varies
			int16_t result = (int8_t)regA.hl.l * (int8_t)(*modrm);
			LogPrintf(LOG_DEBUG, "IMUL8, %d * %d = %d", (int8_t)regA.hl.l, (int8_t)(*modrm), result);
			regA.x = (WORD)result;
			WORD tmp = Widen(regA.hl.l);
			bool ocFlags = (tmp != (WORD)result);
			SetFlag(FLAG_O, ocFlags);
			SetFlag(FLAG_C, ocFlags);
			AdjustSign(regA.hl.l);
			AdjustZero(regA.hl.l);
			AdjustParity(regA.hl.l);
			break;
		}
		case 0x30:
		{
			LogPrintf(LOG_DEBUG, "DIV8");
			TICKRM(82, 90); // Varies
			if ((*modrm) == 0)
			{
				INT(0);
				return;
			}
			WORD dividend = regA.x;
			WORD quotient = dividend / (*modrm);
			if (quotient > 0xFF)
			{
				INT(0);
				return;
			}
			BYTE remainder = dividend % (*modrm);
			LogPrintf(LOG_DEBUG, "DIV8 %04X / %02X = %02X r %02X", dividend, (*modrm), quotient, remainder);
			regA.hl.l = (BYTE)quotient;
			regA.hl.h = remainder;
			break;
		}
		case 0x38:
		{
			LogPrintf(LOG_DEBUG, "IDIV8");
			TICKRM(104, 110); // Varies
			if ((*modrm) == 0)
			{
				INT(0);
				return;
			}
			int16_t dividend = (int16_t)regA.x;
			int16_t quotient = dividend / (int8_t)(*modrm);
			if (quotient > 127 || quotient < -127)
			{
				INT(0);
				return;
			}
			int8_t remainder = dividend % (int8_t)(*modrm);
			LogPrintf(LOG_DEBUG, "IDIV8 %04X / %02X = %02X r %02X", dividend, (*modrm), quotient, remainder);
			regA.hl.l = (BYTE)quotient;
			regA.hl.h = (BYTE)remainder;
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
		WORD val = modrm.GetValue();

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
			modrm.SetValue(~val);
			break;
		}
		case 0x18: // NEG
		{
			TICKRM(3, 16);
			Register tempDest;
			SourceDest16 sd;
			sd.dest = tempDest;
			sd.source = modrm;
			Arithmetic16(sd, rawSub16);
			LogPrintf(LOG_DEBUG, "NEG16, -(%04X) = %04X", val, tempDest.x);
			modrm.SetValue(tempDest.x);
			break;
		}
		case 0x20: // MUL
		{
			TICKRM(120, 128); // Varies
			DWORD result = regA.x * val;
			LogPrintf(LOG_DEBUG, "MUL16, %04X * %04X = %08X", regA.x, val, result);
			regD.x = GetHWord(result);
			regA.x = GetLWord(result);
			SetFlag(FLAG_O, regD.x != 0);
			SetFlag(FLAG_C, regD.x != 0);
			AdjustSign(regA.x);
			AdjustZero(regA.x);
			AdjustParity(regA.x);
			break;
		}
		case 0x28: // IMUL
		{
			TICKRM(132, 140); // Varies
			int32_t result = (int16_t)regA.x * (int16_t)(val);
			LogPrintf(LOG_DEBUG, "IMUL16, %d * %d = %d", (int16_t)regA.x, (int16_t)(val), result);
			regD.x = GetHWord(result);
			regA.x = GetLWord(result);
			DWORD tmp = Widen(regA.x);
			bool ocFlags = (tmp != (DWORD)result);
			SetFlag(FLAG_O, ocFlags);
			SetFlag(FLAG_C, ocFlags);
			AdjustSign(regA.x);
			AdjustZero(regA.x);
			AdjustParity(regA.x);
			break;
		}
		case 0x30:
		{
			TICKRM(150, 158); // Varies
			LogPrintf(LOG_DEBUG, "DIV16");
			if (val == 0)
			{
				INT(0);
				return;
			}
			DWORD dividend = MakeDword(regD.x, regA.x);
			DWORD quotient = dividend / val;
			if (quotient > 0xFFFF)
			{
				INT(0);
				return;
			}
			WORD remainder = dividend % val;
			LogPrintf(LOG_DEBUG, "DIV16 %08X / %04X = %04X r %04X", dividend, val, quotient, remainder);
			regA.x = (WORD)quotient;
			regD.x = remainder;
			break;
		}
		case 0x38:
		{
			TICKRM(170, 178); // Varies
			LogPrintf(LOG_DEBUG, "IDIV16");
			if (val == 0)
			{
				INT(0);
				return;
			}
			int32_t dividend = (int32_t)MakeDword(regD.x, regA.x);
			int32_t quotient = dividend / int16_t(val);
			if (quotient > 32767 || quotient < -32767)
			{
				INT(0);
				return;
			}
			int16_t remainder = dividend % int16_t(val);
			LogPrintf(LOG_DEBUG, "IDIV16 %08X / %04X = %04X r %04X", dividend, val, quotient, remainder);
			regA.x = (WORD)quotient;
			regD.x = (WORD)remainder;
			break;
		}
		default:
			throw(std::exception("not possible"));
		}
	}

	void CPU8086::XCHG8(SourceDest8 sd)
	{
		XCHG8(*(sd.source), *(sd.dest));
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
		WORD source = sd.source.GetValue();
		WORD dest = sd.dest.GetValue();

		XCHG16(source, dest);
		sd.source.SetValue(source);
		sd.dest.SetValue(dest);
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
		PUSH(m.GetValue());
	}

	void CPU8086::PUSH(WORD w)
	{
		LogPrintf(LOG_DEBUG, "PUSH %04X", w);
		m_memory.Write8(S2A(regSS.x, --regSP.x), GetHByte(w));
		m_memory.Write8(S2A(regSS.x, --regSP.x), GetLByte(w));
	}

	void CPU8086::POP(Mem16 dest)
	{
		BYTE lo = m_memory.Read8(S2A(regSS.x, regSP.x++));
		BYTE hi = m_memory.Read8(S2A(regSS.x, regSP.x++));
		WORD w = MakeWord(hi, lo);
		dest.SetValue(w);
		LogPrintf(LOG_DEBUG, "POP %04X", w);
	}

	void CPU8086::LODS8()
	{
		LogPrintf(LOG_DEBUG, "LODS8, SI=%04X", regSI.x);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", segOverride);
		}

		if (PreREP())
		{
			regA.hl.l = m_memory.Read8(S2A(inSegOverride ? segOverride : regDS.x, regSI.x));
			IndexIncDec(regSI.x);
		}
		PostREP(false);
	}

	void CPU8086::LODS16()
	{
		LogPrintf(LOG_DEBUG, "LODS16, SI=%04X", regSI.x);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", segOverride);
		}

		if (PreREP())
		{
			SourceDest16 sd;
			sd.dest = regA;
			sd.source.SetAddress(m_memory, S2A(inSegOverride ? segOverride : regDS.x, regSI.x));
			sd.dest.SetValue(sd.source.GetValue());

			IndexIncDec(regSI.x);
			IndexIncDec(regSI.x);
		}
		PostREP(false);
	}

	void CPU8086::STOS8()
	{
		LogPrintf(LOG_DEBUG, "STOS8, DI=%04X", regDI.x);

		if (PreREP())
		{
			m_memory.Write8(S2A(regES.x, regDI.x), regA.hl.l);
			IndexIncDec(regDI.x);
		}
		PostREP(false);
	}
	void CPU8086::STOS16()
	{
		LogPrintf(LOG_DEBUG, "STOS16, DI=%04X", regDI.x);

		if (PreREP())
		{
			SourceDest16 sd;
			sd.dest.SetAddress(m_memory, S2A(regES.x, regDI.x));
			sd.source = regA;
			sd.dest.SetValue(sd.source.GetValue());

			IndexIncDec(regDI.x);
			IndexIncDec(regDI.x);
		}
		PostREP(false);
	}

	void CPU8086::SCAS8()
	{
		LogPrintf(LOG_DEBUG, "SCAS8, DI=%04X", regDI.x);

		if (PreREP())
		{
			SourceDest8 sd;

			sd.source = m_memory.GetPtr8(S2A(regES.x, regDI.x));
			sd.dest = &regA.hl.l;
			Arithmetic8(sd, rawCmp8);

			IndexIncDec(regDI.x);
		}
		PostREP(true);
	}
	void CPU8086::SCAS16()
	{
		LogPrintf(LOG_DEBUG, "SCAS16, DI=%04X", regDI.x);

		if (PreREP())
		{
			SourceDest16 sd;

			sd.source.SetAddress(m_memory, S2A(regES.x, regDI.x));
			sd.dest = regA;
			Arithmetic16(sd, rawCmp16);

			IndexIncDec(regDI.x);
			IndexIncDec(regDI.x);
		}
		PostREP(true);
	}

	void CPU8086::MOVS8()
	{
		LogPrintf(LOG_DEBUG, "MOVS8, SI=%04X, DI=%04X", regSI.x, regDI.x);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", segOverride);
		}

		if (PreREP())
		{
			BYTE val = m_memory.Read8(S2A(inSegOverride ? segOverride : regDS.x, regSI.x));
			m_memory.Write8(S2A(regES.x, regDI.x), val);
			IndexIncDec(regSI.x);
			IndexIncDec(regDI.x);
		}
		PostREP(false);
	}
	void CPU8086::MOVS16()
	{
		LogPrintf(LOG_DEBUG, "MOVS16, SI=%04X, DI=%04X", regSI.x, regDI.x);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", segOverride);
		}

		if (PreREP())
		{
			SourceDest16 sd;
			sd.source.SetAddress(m_memory, S2A(inSegOverride ? segOverride : regDS.x, regSI.x));
			sd.dest.SetAddress(m_memory, S2A(regES.x, regDI.x));

			sd.dest.SetValue(sd.source.GetValue());

			IndexIncDec(regSI.x);
			IndexIncDec(regSI.x);

			IndexIncDec(regDI.x);
			IndexIncDec(regDI.x);
		}
		PostREP(false);
	}

	void CPU8086::CMPS8()
	{
		LogPrintf(LOG_DEBUG, "CMPS8, SI=%04X, DI=%04X", regSI.x, regDI.x);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", segOverride);
		}

		if (PreREP())
		{
			SourceDest8 sd;
			sd.dest = m_memory.GetPtr8(S2A(inSegOverride ? segOverride : regDS.x, regSI.x));
			sd.source = m_memory.GetPtr8(S2A(regES.x, regDI.x));

			Arithmetic8(sd, rawCmp8);

			IndexIncDec(regSI.x);

			IndexIncDec(regDI.x);
		}
		PostREP(true);
	}
	void CPU8086::CMPS16()
	{
		LogPrintf(LOG_DEBUG, "CMPS16, SI=%04X, DI=%04X", regSI.x, regDI.x);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", segOverride);
		}

		if (PreREP())
		{
			SourceDest16 sd;
			
			sd.dest.SetAddress(m_memory, S2A(inSegOverride ? segOverride : regDS.x, regSI.x));
			sd.source.SetAddress(m_memory, S2A(regES.x, regDI.x));

			Arithmetic16(sd, rawCmp16);

			IndexIncDec(regSI.x);
			IndexIncDec(regSI.x);

			IndexIncDec(regDI.x);
			IndexIncDec(regDI.x);
		}
		PostREP(true);
	}

	void CPU8086::REP(bool z)
	{
		LogPrintf(LOG_DEBUG, "REP, Z=%d, cx=%04X", z, regC.x);

		inRep = true;
		repZ = z;
		repIP = regIP.x-1;
	}

	bool CPU8086::PreREP()
	{
		if (!inRep)
			return true;

		if (regC.x == 0)
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

		--regC.x;
		LogPrintf(LOG_DEBUG, "PostRep, cx=%04X", regC.x);
		if (!checkZ || (GetFlag(FLAG_Z) == repZ))
		{ 
			regIP.x = repIP;
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
		segOverride = val;
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
			char ch = (regA.hl.l < 32) ? '.' : (char)regA.hl.l;
			
			switch (regA.hl.h)
			{
			case 0x00: LogPrintf(LOG_ERROR, "INT10: 0x00 - Set video mode [%02X]", regA.hl.l); break;
			case 0x01: LogPrintf(LOG_DEBUG, "INT10: 0x01 - Set text-mode cursor shape [%02X]-[%02X]", regC.hl.h, regC.hl.l); break;
			case 0x02: LogPrintf(LOG_DEBUG, "INT10: 0x02 - Set cursor position p[%d]:[%02d ,%02d]", regB.hl.h, regD.hl.l, regD.hl.h); break;
			case 0x03: LogPrintf(LOG_DEBUG, "INT10: 0x03 - Get cursor position p[%d]", regB.hl.h); break;
			case 0x04: LogPrintf(LOG_DEBUG, "INT10: 0x04 - Read light pen"); break;
			case 0x05: LogPrintf(LOG_ERROR, "INT10: 0x05 - Select page p[%d]", regA.hl.l); break;
			case 0x06: LogPrintf(LOG_DEBUG, "INT10: 0x06 - Scroll up"); break;
			case 0x07: LogPrintf(LOG_DEBUG, "INT10: 0x07 - Scroll down"); break;
			case 0x08: LogPrintf(LOG_DEBUG, "INT10: 0x08 - Read char & attr at cursor"); break;
			case 0x09: LogPrintf(LOG_ERROR, "INT10: 0x09 - Write char & attr at cursor ch=[%02d]['%c'], p=[%d], color=[%02d], times=[%d]", regA.hl.l, ch, regB.hl.h, regB.hl.l, regC.x); break;
			case 0x0A: LogPrintf(LOG_ERROR, "INT10: 0x0A - Write char at cursor ch=[%02d]['%c'], p=[%d], times=[%d]", regA.hl.l, ch, regB.hl.h, regC.x); break;
			case 0x0B: LogPrintf(LOG_DEBUG, "INT10: 0x0B - Set background/border color / Set palette"); break;
			case 0x0C: LogPrintf(LOG_DEBUG, "INT10: 0x0C - Write pixel"); break;
			case 0x0D: LogPrintf(LOG_DEBUG, "INT10: 0x0D - Read pixel"); break;
			case 0x0E: LogPrintf(LOG_ERROR, "INT10: 0x0E - Teletype output ch=[%02d]['%c'] p=[%d]", regA.hl.l, ch, regB.hl.h); break;
			case 0x0F: LogPrintf(LOG_DEBUG, "INT10: 0x0F - Get video mode"); break;
			case 0x11: LogPrintf(LOG_DEBUG, "INT10: 0x11 - Change charset"); break;
			default: LogPrintf(LOG_DEBUG, "INT10: Other function ah=%02X", regA.hl.h); break;
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
			WORD cyl = regC.hl.h;
			cyl |= ((regC.hl.l & 0b11000000) << 2);
			switch (regA.hl.h)
			{
			case 0x00: LogPrintf(LOG_INFO, "DISK[%02X]: Reset drive", regD.hl.l); break;
			case 0x01: LogPrintf(LOG_INFO, "DISK[%02X]: Get Status drive", regD.hl.l); break;
			case 0x02: LogPrintf(LOG_INFO, "DISK[%02X]: Read Sectors count=[%d], cyl=[%d] head=[%d] sect=[%d]", regD.hl.l, regA.hl.l, cyl, regD.hl.h, regC.hl.l & 63); break;
			case 0x03: LogPrintf(LOG_INFO, "DISK[%02X]: Write Sectors count=[%d], cyl=[%d] head=[%d] sect=[%d]", regD.hl.l, regA.hl.l, cyl, regD.hl.h, regC.hl.l & 63); break;
			case 0x04: LogPrintf(LOG_INFO, "DISK[%02X]: Verify Sectors count=[%d], cyl=[%d] head=[%d] sect=[%d]", regD.hl.l, regA.hl.l, cyl, regD.hl.h, regC.hl.l & 63); break;

			default: LogPrintf(LOG_INFO, "DISK[%02X], Other function ah=%02X", regD.hl.l, regA.hl.h); break;
			}
		}
		else if (interrupt == 0x21)
		{
			LogPrintf(LOG_DEBUG, "DOS");

#if 0
			switch (regA.hl.h)
			{				
			case 0x09: LogPrintf(LOG_ERROR, "INT21: 0x09 - Print String @[%04X:%04X]", regDS.x, regD.x); break;
			case 0x0A: LogPrintf(LOG_ERROR, "INT21: 0x0A - Buffered input"); break;
			case 0x0E: LogPrintf(LOG_ERROR, "INT21: 0x0E - Select Disk dl=%02X", regD.hl.l); break;
			case 0x2B: LogPrintf(LOG_ERROR, "INT21: 0x2B - Set system date[%04d-%02d-%02d]", regC.x, regD.hl.h, regD.hl.l); break;
			default: LogPrintf(LOG_ERROR, "INT21: Other function ah=%02X", regA.hl.h); break;
			}
#endif
		}
#if 0
		else if (interrupt >= 0x09 && interrupt < 0x10)
		{
			LogPrintf(LOG_WARNING, "IRQ(%d)", interrupt - 8);
		}
#endif
		PUSH(flags);
		PUSH(regCS.x);
		PUSH(inRep ? repIP : regIP.x);
		if (inRep)
		{
			inRep = false;
		}

		SetFlag(FLAG_T, false);
		CLI();
		
		ADDRESS interruptAddress = interrupt * 4;
		regCS.x = m_memory.Read16(interruptAddress + 2);
		regIP.x = m_memory.Read16(interruptAddress);
	}

	void CPU8086::IRET()
	{
		LogPrintf(LOG_DEBUG, "IRET");
		POP(regIP);
		POP(regCS);
		POP(flags);
	}

	void CPU8086::MultiFunc(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "Multifunc, op=%02X", op2);

		Mem16 dest = GetModRM16(op2);
		WORD val = dest.GetValue();

		switch (op2 & 0x38)
		{
		// INC/DEC MEM16
		case 0x00: TICK(15); INC16(val); dest.SetValue(val);  break;
		case 0x08: TICK(15); DEC16(val); dest.SetValue(val); break;
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
		regMem.dest.SetValue(regMem.source.GetValue());
		
		// Read segment
		regMem.source.Increment(m_memory);
		destSegment = regMem.source.GetValue();

		LogPrintf(LOG_DEBUG, "LoadPtr Loaded [%04X:%04X]", destSegment, regMem.dest.GetValue());

	}

	void CPU8086::XLAT()
	{
		LogPrintf(LOG_DEBUG, "XLAT");

		WORD offset = regB.x + regA.hl.l; // TODO: Wrap around?
		regA.hl.l = m_memory.Read8(S2A(inSegOverride ? segOverride : regDS.x, offset));
	}

	void CPU8086::AAA()
	{
		LogPrintf(LOG_DEBUG, "AAA");
		
		if (GetFlag(FLAG_A) || ((regA.hl.l & 15) > 9))
		{
			++regA.hl.h;
			regA.hl.l += 6;
			SetFlag(FLAG_A, true);
			SetFlag(FLAG_C, true);
		}
		else
		{
			SetFlag(FLAG_A, false);
			SetFlag(FLAG_C, false);
		}
		regA.hl.l &= 0x0F;

		AdjustSign(regA.hl.l);
		AdjustZero(regA.hl.l);
		AdjustParity(regA.hl.l);
	}

	void CPU8086::AAS()
	{
		LogPrintf(LOG_DEBUG, "AAS");

		if (GetFlag(FLAG_A) || ((regA.hl.l & 15) > 9))
		{
			--regA.hl.h;
			regA.hl.l -= 6;
			SetFlag(FLAG_A, true);
			SetFlag(FLAG_C, true);
		}
		else
		{
			SetFlag(FLAG_A, false);
			SetFlag(FLAG_C, false);
		}
		regA.hl.l &= 0x0F;

		AdjustSign(regA.hl.l);
		AdjustZero(regA.hl.l);
		AdjustParity(regA.hl.l);
	}

	void CPU8086::AAM(BYTE base)
	{
		LogPrintf(LOG_DEBUG, "AAM base %d", base);
		if (base == 0)
		{
			INT(0);
			return;
		}

		regA.hl.h = regA.hl.l / base;
		regA.hl.l %= base;

		AdjustSign(regA.hl.l);
		AdjustZero(regA.hl.l);
		AdjustParity(regA.hl.l);
	}

	void CPU8086::AAD(BYTE base)
	{
		LogPrintf(LOG_DEBUG, "AAD base %d", base);

		ArithmeticImm8(regA.hl.l, base * regA.hl.h, rawAdd8);
		regA.hl.h = 0;
	}

	void CPU8086::DAA()
	{
		LogPrintf(LOG_DEBUG, "DAA");

		if (GetFlag(FLAG_A) || ((regA.hl.l & 15) > 9))
		{
			regA.hl.l += 6;
			SetFlag(FLAG_A, true);
		}

		if (GetFlag(FLAG_C) || regA.hl.l > 0x9F)
		{
			regA.hl.l += 0x60;
			SetFlag(FLAG_C, true);
		}

		AdjustSign(regA.hl.l);
		AdjustZero(regA.hl.l);
		AdjustParity(regA.hl.l);
	}

	void CPU8086::DAS()
	{
		LogPrintf(LOG_DEBUG, "DAS");

		BYTE oldAL = regA.hl.l;
		bool oldCF = GetFlag(FLAG_C);
		SetFlag(FLAG_C, false);

		if (GetFlag(FLAG_A) || ((regA.hl.l & 15) > 9))
		{
			regA.hl.l -= 6;
			SetFlag(FLAG_C, oldCF || (oldAL < 6));
			SetFlag(FLAG_A, true);
		}
		else
		{
			SetFlag(FLAG_A, false);
		}

		if (oldCF || oldAL > 0x99)
		{
			regA.hl.l -= 0x60;
			SetFlag(FLAG_C, true);
		}

		AdjustSign(regA.hl.l);
		AdjustZero(regA.hl.l);
		AdjustParity(regA.hl.l);
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
		WORD& segment = std::get<0>(segoff);
		WORD& offset = std::get<1>(segoff);

		offset = (direct ? 0 : offset) + displacement;

		dest.SetValue(offset);
	}

	void CPU8086::Serialize(json& to)
	{
		to["ax"] = regA.x;
		to["bx"] = regB.x;
		to["cx"] = regC.x;
		to["dx"] = regD.x;

		to["sp"] = regSP.x;
		to["bp"] = regBP.x;
		to["si"] = regSI.x;
		to["di"] = regDI.x;

		to["cs"] = regCS.x;
		to["ds"] = regDS.x;
		to["ss"] = regSS.x;
		to["es"] = regES.x;

		to["ip"] = regIP.x;
		to["flags"] = flags.x;

		to["lastOp"] = m_lastOp;
		to["irqPending"] = m_irqPending;

		to["inRep"] = inRep;
		to["repIP"] = repIP;
		to["repZ"] = repZ;

		to["inSegOverride"] = inSegOverride;
		to["segOverride"] = segOverride;
	}

	void CPU8086::Deserialize(json& from)
	{
		regA.x = from["ax"];
		regB.x = from["bx"];
		regC.x = from["cx"];
		regD.x = from["dx"];

		regSP.x = from["sp"];
		regBP.x = from["bp"];
		regSI.x = from["si"];
		regDI.x = from["di"];

		regCS.x = from["cs"];
		regDS.x = from["ds"];
		regSS.x = from["ss"];
		regES.x = from["es"];

		regIP.x = from["ip"];
		flags.x = from["flags"];

		m_lastOp = from["lastOp"];
		m_irqPending = from["irqPending"];

		inRep = from["inRep"];
		repIP = from["repIP"];
		repZ = from["repZ"];

		inSegOverride = from["inSegOverride"];
		segOverride = from["segOverride"];
	}
}