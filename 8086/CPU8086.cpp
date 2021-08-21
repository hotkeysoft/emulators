#include "stdafx.h"
#include "CPU8086.h"

namespace emul
{
	BYTE rawAdd8(SourceDest8 sd) { *(sd.dest) += *(sd.source); return *(sd.dest); }
	BYTE rawOr8(SourceDest8 sd) { *(sd.dest) |= *(sd.source); return *(sd.dest); }
	BYTE rawAdc8(SourceDest8 sd) { throw(std::exception("rawAdc8 not implemented")); }
	BYTE rawSbb8(SourceDest8 sd) { throw(std::exception("rawSbb8 not implemented")); }
	BYTE rawAnd8(SourceDest8 sd) { *(sd.dest) &= *(sd.source); return *(sd.dest); }
	BYTE rawSub8(SourceDest8 sd) { *(sd.dest) -= *(sd.source); return *(sd.dest); }
	BYTE rawXor8(SourceDest8 sd) { *(sd.dest) ^= *(sd.source); return *(sd.dest); }
	BYTE rawCmp8(SourceDest8 sd) { return *(sd.dest)-*(sd.source); }

	WORD rawAdd16(SourceDest16 sd) { *(sd.dest) += *(sd.source); return *(sd.dest); }
	WORD rawOr16(SourceDest16 sd) { *(sd.dest) |= *(sd.source); return *(sd.dest); }
	WORD rawAdc16(SourceDest16 sd) { throw(std::exception("rawAdc16 not implemented")); }
	WORD rawSbb16(SourceDest16 sd) { throw(std::exception("rawSbb16 not implemented")); }
	WORD rawAnd16(SourceDest16 sd) { *(sd.dest) &= *(sd.source); return *(sd.dest); }
	WORD rawSub16(SourceDest16 sd) { *(sd.dest) -= *(sd.source); return *(sd.dest); }
	WORD rawXor16(SourceDest16 sd) { *(sd.dest) ^= *(sd.source); return *(sd.dest); }
	WORD rawCmp16(SourceDest16 sd) { return *(sd.dest) - *(sd.source); }

	CPU8086::CPU8086(Memory& memory, MemoryMap& mmap)
		: CPU(CPU8086_ADDRESS_BITS, memory, mmap), Logger("CPU8086")
	{
	}

	CPU8086::~CPU8086()
	{

	}

	void CPU8086::Exec(BYTE opcode)
	{
		++regIP;

		switch(opcode)
		{
		// ADD rm+r=>rm (4)
		// --------
		// REG8/MEM8, REG8
		case 0x00: Arithmetic8(GetModRegRM8(FetchByte(), false), rawAdd8); break;
		// REG16/MEM16, REG16
		case 0x01: Arithmetic16(GetModRegRM16(FetchByte(), false), rawAdd16); break;
		// REG8, REG8/MEM8
		case 0x02: Arithmetic8(GetModRegRM8(FetchByte(), true), rawAdd8); break;
		// REG16, REG16/MEM16
		case 0x03: Arithmetic16(GetModRegRM16(FetchByte(), true), rawAdd16); break;

		// ADD i=>a (2-3)
		// --------
		// AL, IMMED8
		case 0x04: NotImplemented(opcode); break;
		// AL, IMMED16
		case 0x05: NotImplemented(opcode); break;

		// PUSH ES (1)
		case 0x06: NotImplemented(opcode); break;
		// POP ES (1)
		case 0x07: NotImplemented(opcode); break;

		// OR rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x08: Arithmetic8(GetModRegRM8(FetchByte(), false), rawOr8); break;
		// REG16/MEM16, REG16
		case 0x09: Arithmetic16(GetModRegRM16(FetchByte(), false), rawOr16); break;
		// REG8, REG8/MEM8
		case 0x0A: Arithmetic8(GetModRegRM8(FetchByte(), true), rawOr8); break;
		// REG16, REG16/MEM16
		case 0x0B: Arithmetic16(GetModRegRM16(FetchByte(), true), rawOr16); break;

		// OR i=>a (2-3)
		// ----------
		// AL, IMMED8
		case 0x0C: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x0D: NotImplemented(opcode); break;

		// PUSH CS (1)
		case 0x0E: NotImplemented(opcode); break;

		// ADC rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x10: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x11: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x12: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x13: NotImplemented(opcode); break;

		// ADC i=>a (2-3)
		// ----------
		// AL, IMMED8
		case 0x14: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x15: NotImplemented(opcode); break;

		// PUSH SS (1)
		case 0x16: NotImplemented(opcode); break;
		// POP SS (1)
		case 0x17: NotImplemented(opcode); break;

		// SBB rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x18: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x19: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x1A: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x1B: NotImplemented(opcode); break;

		// SBB i=>a (2-3)
		// ----------
		// AL, IMMED8
		case 0x1C: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x1D: NotImplemented(opcode); break;

		// PUSH DS (1)
		case 0x1E: NotImplemented(opcode); break;
		// POP DS (1)
		case 0x1F: NotImplemented(opcode); break;

		// AND rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x20: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x21: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x22: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x23: NotImplemented(opcode); break;

		// AND i=>a (2-3)
		// ----------
		// AL, IMMED8
		case 0x24: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x25: NotImplemented(opcode); break;

		// ES Segment Override
		case 0x26: NotImplemented(opcode); break;

		// DAA (1)
		case 0x27: NotImplemented(opcode); break;

		// SUB rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x28: Arithmetic8(GetModRegRM8(FetchByte(), false), rawSub8); break;
			// REG16/MEM16, REG16
		case 0x29: Arithmetic16(GetModRegRM16(FetchByte(), false), rawSub16); break;
			// REG8, REG8/MEM8
		case 0x2A: Arithmetic8(GetModRegRM8(FetchByte(), true), rawSub8); break;
			// REG16, REG16/MEM16
		case 0x2B: Arithmetic16(GetModRegRM16(FetchByte(), true), rawSub16); break;

		// SUB i=>a (2-3)
		// ----------
		// AL, IMMED8
		case 0x2C: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x2D: NotImplemented(opcode); break;

		// CS Segment Override
		case 0x2E: NotImplemented(opcode); break;

		// DAS (1)
		case 0x2F: NotImplemented(opcode); break;

		// XOR rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x30: Arithmetic8(GetModRegRM8(FetchByte(), false), rawXor8); break;
		// REG16/MEM16, REG16
		case 0x31: Arithmetic16(GetModRegRM16(FetchByte(), false), rawXor16); break;
		// REG8, REG8/MEM8
		case 0x32: Arithmetic8(GetModRegRM8(FetchByte(), true), rawXor8); break;
		// REG16, REG16/MEM16
		case 0x33: Arithmetic16(GetModRegRM16(FetchByte(), true), rawXor16); break;

		// XOR i=>a (2-3)
		// ----------
		// AL, IMMED8
		case 0x34: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x35: NotImplemented(opcode); break;

		// SS Segment Override
		case 0x36: NotImplemented(opcode); break;

		// AAA (1)
		case 0x37: NotImplemented(opcode); break;

		// CMP rm+r=>r (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x38: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x39: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x3A: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x3B: NotImplemented(opcode); break;

		// CMP i=>a (2)
		// ----------
		// AL, IMMED8
		case 0x3C: NotImplemented(opcode); break;
		// AX, IMMED16
		case 0x3D: NotImplemented(opcode); break;

		// DS Segment Override
		case 0x3E: NotImplemented(opcode); break;

		// AAS (1)
		case 0x3F: NotImplemented(opcode); break;

		// INC r (1)
		// ----------
		// INC AX
		case 0x40: INC16(regA.x); break;
		// INC CX
		case 0x41: INC16(regC.x); break;
		// INC DX
		case 0x42: INC16(regD.x); break;
		// INC BX
		case 0x43: INC16(regB.x); break;
		// INC SP
		case 0x44: INC16(regSP); break;
		// INC BP
		case 0x45: INC16(regBP); break;
		// INC SI
		case 0x46: INC16(regSI); break;
		// INC DI
		case 0x47: INC16(regDI); break;

		// DEC r (1)
		// ----------
		// DEC AX
		case 0x48: NotImplemented(opcode); break;
		// DEC CX
		case 0x49: NotImplemented(opcode); break;
		// DEC DX
		case 0x4A: NotImplemented(opcode); break;
		// DEC BX
		case 0x4B: NotImplemented(opcode); break;
		// DEC SP
		case 0x4C: NotImplemented(opcode); break;
		// DEC BP
		case 0x4D: NotImplemented(opcode); break;
		// DEC SI
		case 0x4E: NotImplemented(opcode); break;
		// DEC DI
		case 0x4F: NotImplemented(opcode); break;

		// PUSH r (1)
		// ----------
		// PUSH AX
		case 0x50: NotImplemented(opcode); break;
		// PUSH CX
		case 0x51: NotImplemented(opcode); break;
		// PUSH DX
		case 0x52: NotImplemented(opcode); break;
		// PUSH BX
		case 0x53: NotImplemented(opcode); break;
		// PUSH SP
		case 0x54: NotImplemented(opcode); break;
		// PUSH BP
		case 0x55: NotImplemented(opcode); break;
		// PUSH SI
		case 0x56: NotImplemented(opcode); break;
		// PUSH DI
		case 0x57: NotImplemented(opcode); break;

		// POP r (1)
		// ----------
		// POP AX
		case 0x58: NotImplemented(opcode); break;
		// POP CX
		case 0x59: NotImplemented(opcode); break;
		// POP DX
		case 0x5A: NotImplemented(opcode); break;
		// POP BX
		case 0x5B: NotImplemented(opcode); break;
		// POP SP
		case 0x5C: NotImplemented(opcode); break;
		// POP BP
		case 0x5D: NotImplemented(opcode); break;
		// POP SI
		case 0x5E: NotImplemented(opcode); break;
		// POP DI
		case 0x5F: NotImplemented(opcode); break;

		// JO (2)
		case 0x70: JMPif(GetFlag(FLAG_O)); break;
		// JNO (2)
		case 0x71: JMPif(!GetFlag(FLAG_O)); break;
		// JB/JNAE/JC (2)
		case 0x72: JMPif(GetFlag(FLAG_C)); break;
		// JNB/JAE/JNC (2)
		case 0x73: JMPif(!GetFlag(FLAG_C)); break;
		// JE/JZ (2)
		case 0x74: JMPif(GetFlag(FLAG_Z)); break;
		// JNE/JNZ (2)
		case 0x75: JMPif(!GetFlag(FLAG_Z)); break;
		// JBE/JNA (2)
		case 0x76: JMPif(GetFlagNotAbove()); break;
		// JNBE/JA (2)
		case 0x77: JMPif(!GetFlagNotAbove()); break;
		// JS (2)
		case 0x78: JMPif(GetFlag(FLAG_S)); break;
		// JNS (2)
		case 0x79: JMPif(!GetFlag(FLAG_S)); break;
		// JP/JPE (2)
		case 0x7A: JMPif(GetFlag(FLAG_P)); break;
		// JNP/JPO (2)
		case 0x7B: JMPif(!GetFlag(FLAG_P)); break;
		// JL/JNGE (2)
		case 0x7C: JMPif(!GetFlagNotLess()); break;
		// JNL/JGE (2)
		case 0x7D: JMPif(GetFlagNotLess()); break;
		// JLE/JNG (2)
		case 0x7E: JMPif(GetFlagGreater()); break;
		// JNLE/JG (2)
		case 0x7F: JMPif(GetFlagGreater()); break;

		//----------
		// ADD/OR/ADC/SBB/AND/SUB/XOR/CMP i=>rm (5-6)
		// ----------
		// REG8/MEM8, IMM8
		case 0x80: ArithmeticImm8(FetchByte(), FetchByte()); break;
		// REG16/MEM16, IMM16
		case 0x81: ArithmeticImm16(FetchByte(), FetchWord()); break;

		// ADD/--/ADC/SBB/---/SUB/---/CMP i=>rm (5-6)??
		// ----------
		// REG8/MEM8, IMM8 ?
		case 0x82: NotImplemented(opcode); break;
		// REG16/MEM16, IMM16 ?
		case 0x83: NotImplemented(opcode); break;

		// TEST rm+r (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x84: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x85: NotImplemented(opcode); break;

		// XCHG rm<=>r (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x86: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x87: NotImplemented(opcode); break;

		// MOV rm<=>r (4)
		// ----------
		// REG8/MEM8, REG8
		case 0x88: MOV8(GetModRegRM8(FetchByte(), false)); break;
		// REG16/MEM16, REG16
		case 0x89: MOV16(GetModRegRM16(FetchByte(), false)); break;
		// REG8, REG8/MEM8
		case 0x8A: MOV8(GetModRegRM8(FetchByte(), true)); break;
		// REG16, REG16/MEM16
		case 0x8B: MOV16(GetModRegRM16(FetchByte(), true)); break;

		// MOV sr=>rm (4)
		// ----------
		// MOV REG16/MEM16, SEGREG
		case 0x8C: MOV16(GetModRegRM16(FetchByte(), false, true)); break;

		// LEA (4)
		// ----------
		// REG16, MEM16
		case 0x8D: NotImplemented(opcode); break;

		// MOV rm=>sr (4)
		// ----------
		// MOV SEGREG, REG16/MEM16		
		case 0x8E: MOV16(GetModRegRM16(FetchByte(), true, true)); break;

		// POP rm (4)
		// ----------
		// POP REG16/MEM16
		case 0x8F: NotImplemented(opcode); break;

		// XCHG rm<=>a (1)
		// ----------
		// XCHG AX, AX (NOP)
		case 0x90: NotImplemented(opcode); break;
		// XCHG AX, CX
		case 0x91: NotImplemented(opcode); break;
		// XCHG AX, DX
		case 0x92: NotImplemented(opcode); break;
		// XCHG AX, BX
		case 0x93: NotImplemented(opcode); break;
		// XCHG AX, SP
		case 0x94: NotImplemented(opcode); break;
		// XCHG AX, BP
		case 0x95: NotImplemented(opcode); break;
		// XCHG AX, SI
		case 0x96: NotImplemented(opcode); break;
		// XCHG AX, DI
		case 0x97: NotImplemented(opcode); break;

		// CBW
		case 0x98: NotImplemented(opcode); break;
		// CWD
		case 0x99: NotImplemented(opcode); break;

		// CALL Far (5)
		case 0x9A: NotImplemented(opcode); break;

		// WAIT (1)
		case 0x9B: NotImplemented(opcode); break;

		// PUSHF (1)
		case 0x9C: NotImplemented(opcode); break;
		// POPF (1)
		case 0x9D: NotImplemented(opcode); break;
		// SAHF (1)
		case 0x9E: SAHF(); break;
		// LAHF (1)
		case 0x9F: LAHF(); break;

		// MOV m=>a (3)
		// ----------
		// MOV AL, MEM8
		case 0xA0: NotImplemented(opcode); break;
		// MOV AX, MEM16
		case 0xA1: NotImplemented(opcode); break;

		// MOV a=>m (3)
		// ----------
		// MOV MEM8, AL
		case 0xA2: NotImplemented(opcode); break;
		// MOV MEM16, AX
		case 0xA3: NotImplemented(opcode); break;

		// MOVS (1)
		// ----------
		// MOVS DEST-STR8, SRC-STR8
		case 0xA4: NotImplemented(opcode); break;
		// MOVS DEST-STR16, SRC-STR16
		case 0xA5: NotImplemented(opcode); break;

		// CMPS (1)
		// ----------
		// CMPS DEST-STR8, SRC-STR8
		case 0xA6: NotImplemented(opcode); break;
		// CMPS DEST-STR16, SRC-STR16
		case 0xA7: NotImplemented(opcode); break;

		// TEST i+a (2)
		// ----------
		// TEST AL, IMM8
		case 0xA8: NotImplemented(opcode); break;
		// TEST AX, IMM16
		case 0xA9: NotImplemented(opcode); break;

		// STOS (1)
		// ----------
		// STOS DEST-STR8
		case 0xAA: NotImplemented(opcode); break;
		// STOS DEST-STR16
		case 0xAB: NotImplemented(opcode); break;

		// LODS (1)
		// ----------
		// LDOS SRC-STR8
		case 0xAC: NotImplemented(opcode); break;
		// LDOS SRC-STR16
		case 0xAD: NotImplemented(opcode); break;

		// SCAS (1)
		// ----------
		// SCAS DEST-STR8
		case 0xAE: NotImplemented(opcode); break;
		// SCAS DEST-STR16
		case 0xAF: NotImplemented(opcode); break;

		// MOV i=>r (2-3)
		// ----------
		// MOV AL, IMM8
		case 0xB0: MOV8(regA.hl.l, FetchByte()); break;
		// MOV CL, IMM8
		case 0xB1: MOV8(regC.hl.l, FetchByte()); break;
		// MOV DL, IMM8
		case 0xB2: MOV8(regD.hl.l, FetchByte()); break;
		// MOV BL, IMM8
		case 0xB3: MOV8(regB.hl.l, FetchByte()); break;
		// MOV AH, IMM8
		case 0xB4: MOV8(regA.hl.h, FetchByte()); break;
		// MOV CH, IMM8
		case 0xB5: MOV8(regC.hl.h, FetchByte()); break;
		// MOV DH, IMM8
		case 0xB6: MOV8(regD.hl.h, FetchByte()); break;
		// MOV BH, IMM8
		case 0xB7: MOV8(regB.hl.h, FetchByte()); break;

		// MOV AX, IMM16
		case 0xB8: MOV16(regA.x, FetchWord()); break;
		// MOV CX, IMM16
		case 0xB9: MOV16(regC.x, FetchWord()); break;
		// MOV DX, IMM16
		case 0xBA: MOV16(regD.x, FetchWord()); break;
		// MOV BX, IMM16
		case 0xBB: MOV16(regB.x, FetchWord()); break;
		// MOV SP, IMM16
		case 0xBC: MOV16(regSP, FetchWord()); break;
		// MOV BP, IMM16
		case 0xBD: MOV16(regBP, FetchWord()); break;
		// MOV SI, IMM16
		case 0xBE: MOV16(regSI, FetchWord()); break;
		// MOV DI, IMM16
		case 0xBF: MOV16(regDI, FetchWord()); break;

		// RET SP+IMM16 (3)
		case 0xC2: RETNear(true, FetchWord()); break;
		// RET Near (1)
		case 0xC3: RETNear(); break;

		// LES REG16, MEM16 (4)
		case 0xC4: NotImplemented(opcode); break;
		// LDS REG16, MEM16 (4)
		case 0xC5: NotImplemented(opcode); break;

		// MOV i=>rm (5-6)
		// ----------
		// MOV MEM8, IMM8
		case 0xC6: NotImplemented(opcode); break;
		// MOV MEM16, IMM16
		case 0xC7: NotImplemented(opcode); break;

		// RET Far SP+IMM16 (3)
		case 0xCA: NotImplemented(opcode); break;
		// RET Far (1)
		case 0xCB: NotImplemented(opcode); break;

		// INT3 (1)
		case 0xCC: NotImplemented(opcode); break;
		// INT INN8 (2)
		case 0xCD: NotImplemented(opcode); break;
		// INTO (1)
		case 0xCE: NotImplemented(opcode); break;
		// IRET (1)
		case 0xCF: NotImplemented(opcode); break;

		// ROL/ROR/RCL/RCR/SAL|SHL/SHR/---/SAR
		// ----------
		// REG8/MEM8, 1
		case 0xD0: SHIFTROT8(FetchByte(), 1); break;
		// REG16/MEM16, 1
		case 0xD1: SHIFTROT16(FetchByte(), 1); break;
		// REG8/MEM8, CL
		case 0xD2: SHIFTROT8(FetchByte(), regC.hl.l); break;
		// REG16/MEM16, CL
		case 0xD3: SHIFTROT16(FetchByte(), regC.hl.l); break;

		// AAM
		case 0xD4: NotImplemented(opcode); break;
		// AAD
		case 0xD5: NotImplemented(opcode); break;

		// XLAT (1)
		case 0xD7: NotImplemented(opcode); break;

		// ESC (2) ??
		case 0xD8: NotImplemented(opcode); break;
		case 0xD9: NotImplemented(opcode); break;
		case 0xDA: NotImplemented(opcode); break;
		case 0xDB: NotImplemented(opcode); break;
		case 0xDC: NotImplemented(opcode); break;
		case 0xDD: NotImplemented(opcode); break;
		case 0xDE: NotImplemented(opcode); break;
		case 0xDF: NotImplemented(opcode); break;

		// LOOPNZ/LOOPNE (2)
		case 0xE0: NotImplemented(opcode); break;
		// LOOPZ/LOOPE (2)
		case 0xE1: NotImplemented(opcode); break;
		// LOOP (2)
		case 0xE2: LOOP(FetchByte()); break;
		// JCXZ (2)
		case 0xE3: NotImplemented(opcode); break;

		// IN fixed (2)
		// --------
		// IN AL, IMM8
		case 0xE4: IN8(FetchByte()); break;
		// IN AX, IMM8
		case 0xE5: NotImplemented(opcode); break;

		// OUT fixed (2)
		// --------
		// OUT PORT8, AL
		case 0xE6: OUT8(FetchByte()); break;
		// OUT PORT8, AX
		case 0xE7: NotImplemented(opcode); break;

		// CALL Near (3)
		case 0xE8: NotImplemented(opcode); break;
		// JUMP Near (3)
		case 0xE9: JMPNear(FetchWord()); break;
		// JUMP Far (5)
		case 0xEA: JMPfar(); break;
		// JUMP Near Short (2)
		case 0xEB: JMPNear(FetchByte()); break;

		// IN variable (1)
		// --------
		// IN AL, DX
		case 0xEC: IN8(regD.x); break;
		// IN AX, DX
		case 0xED: NotImplemented(opcode); break;

		// OUT variable (1)
		// --------
		// OUT AL, DX
		case 0xEE: OUT8(regD.x); break;
		// OUT AX, DX
		case 0xEF: NotImplemented(opcode); break;

		// LOCK (1)
		case 0xF0: NotImplemented(opcode); break;

		// REP (1)
		case 0xF2: NotImplemented(opcode); break;
		case 0xF3: NotImplemented(opcode); break;

		// HLT (1)
		case 0xF4: HLT(); break;
		// CMC (1)
		case 0xF5: CMC(); break;
		// CLC (1)

		// TEST/---/NOT/NEG/MUL/IMUL/DIV/IDIV
		// --------
		// REG8/MEM8 (, IMM8 {TEST})
		case 0xF6: NotImplemented(opcode); break;
		// REG16/MEM16 (, IMM16 {TEST})
		case 0xF7: NotImplemented(opcode); break;

		case 0xF8: CLC(); break;
		// STC (1)
		case 0xF9: STC(); break;
		// CLI (1)
		case 0xFA: CLI(); break;
		// STI (1)
		case 0xFB: STI(); break;
		// CLD (1)
		case 0xFC: CLD(); break;
		// STD (1)
		case 0xFD: STD(); break;

		// INC/DEC/---/---/---/---/---/---
		// --------
		// REG8/MEM8
		case 0xFE: INCDEC8(FetchByte()); break;

		// INC/DEC/CALL/CALL/JMP/JMP/PUSH/---
		case 0xFF: NotImplemented(opcode); break;
		default:
			UnknownOpcode(opcode);
		}
	}

	void CPU8086::AddDevice(PortConnector& ports)
	{
		return m_ports.Connect(ports);
	}

	void CPU8086::Reset()
	{
		CPU::Reset();

		ClearFlags();
		regIP = 0x0000;
		regCS = 0xFFFF;
		regDS = 0x0000;
		regSS = 0x0000;
		regES = 0x0000;
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
			regCS, regIP,
			regDS, regSI,
			regES, regDI,
			regSS, regSP,
			regBP,
			PRINTF_BYTE_TO_BIN_INT16(flags));
	}

	void CPU8086::ClearFlags()
	{
		flags = FLAG_R1 | FLAG_R3 | FLAG_R5 | FLAG_R12 | FLAG_R13 | FLAG_R14 | FLAG_R15;
	}

	BYTE CPU8086::FetchByte()
	{
		BYTE b;
		m_memory.Read(GetCurrentAddress(), b);
		++regIP;
		return b;
	}
	WORD CPU8086::FetchWord()
	{
		Register r;
		m_memory.Read(GetCurrentAddress(), r.hl.l);
		++regIP;
		m_memory.Read(GetCurrentAddress(), r.hl.h);
		++regIP;

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
		throw std::exception("GetReg8: invalid reg value");
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
		throw std::exception("GetReg8: invalid reg value");
	}

	const char* CPU8086::GetReg16Str(BYTE reg, bool segReg)
	{
		switch (reg & 7)
		{
		case 0: return segReg ? "ES" : "AX";
		case 1: return segReg ? "CS" : "CX";
		case 2: return segReg ? "SS" : "DX";
		case 3: return segReg ? "DS" : "BX";

		case 4: return "SP";
		case 5: return "BP";
		case 6: return "SI";
		case 7: return "DI";
		}
		throw std::exception("GetReg16: invalid reg value");
	}

	WORD* CPU8086::GetReg16(BYTE reg, bool segReg)
	{
		switch (reg & 7)
		{
		case 0: return segReg ? &regES : &regA.x;
		case 1: return segReg ? &regCS : &regC.x;
		case 2: return segReg ? &regSS : &regD.x;
		case 3: return segReg ? &regDS : &regB.x;

		case 4: return &regSP;
		case 5: return &regBP;
		case 6: return &regSI;
		case 7: return &regDI;
		}
		throw std::exception("GetReg16: invalid reg value");
	}

	SegmentOffset CPU8086::GetEA(BYTE modregrm)
	{
		switch (modregrm & 7)
		{
		case 0: return std::make_tuple(regDS, regB.x + regSI);
		case 1: return std::make_tuple(regDS, regB.x + regDI);
		case 2: return std::make_tuple(regSS, regBP + regSI);
		case 3: return std::make_tuple(regSS, regBP + regDI);
		case 4: return std::make_tuple(regDS, regSI);
		case 5: return std::make_tuple(regDS, regDI);
		case 6: return std::make_tuple(regSS, regBP);
		case 7: return std::make_tuple(regDS, regB.x);
		}
		throw std::exception("GetReg16: impossible modregrm value");
	}

	const char* CPU8086::GetEAStr(BYTE modregrm)
	{
		switch (modregrm & 7)
		{
		case 0: return "(BX)+(SI)";
		case 1: return "(BX)+(DI)";
		case 2: return "(BP)+(SI)";
		case 3: return "(BP)+(DI)";
		case 4: return "(SI)";
		case 5: return "(DI)";
		case 6: return "(BP)";
		case 7: return "(BX)";
		}
		throw std::exception("GetReg16: impossible modregrm value");
	}

	BYTE* CPU8086::GetModRM8(BYTE modrm)
	{
		WORD displacement = 0;
		bool direct = false;
		switch (modrm & 0xC0)
		{
		case 0xC0: // REG
			LogPrintf(LOG_DEBUG, "GetModRM8: RM=>REG %s", GetReg8Str(modrm));
			return GetReg8(modrm);
		case 0x00: // NO DISP (or DIRECT)
			if ((modrm & 7) == 6) // Direct 
			{
				direct = true;
				displacement = FetchWord();
			}
			else
			{
				LogPrintf(LOG_DEBUG, "GetModRM8: MEM disp=0");
			}
			break;
		case 0x40:
			displacement = widen(FetchByte());
			LogPrintf(LOG_DEBUG, "GetModRM8: MEM disp8=%04X", displacement);
			break;
		case 0x80:
			displacement = FetchWord();
			throw std::exception("GetModRM8: MEM disp16=%04X", displacement);
			break;
		default:
			throw std::exception("GetModRM8: not implemented");
		}

		SegmentOffset segoff = GetEA(modrm);
		WORD& segment = std::get<0>(segoff);
		WORD& offset = std::get<1>(segoff);

		offset = (direct ? 0 : offset) + displacement;
		if (direct)
		{
			LogPrintf(LOG_DEBUG, "GetModRegRM8: MEM ea= %04X", offset);
		}
		else
		{
			LogPrintf(LOG_DEBUG, "GetModRegRM8: MEM ea=%s+%04X=%04X", GetEAStr(modrm), displacement, offset);
		}
		return m_memory.GetPtr8(S2A(segment, offset));
	}

	SourceDest8 CPU8086::GetModRegRM8(BYTE modregrm, bool toReg)
	{
		LogPrintf(LOG_DEBUG, "GetModRegRM8: modregrm=%d, toReg=%d", modregrm, toReg);

		SourceDest8 sd;

		// reg part
		LogPrintf(LOG_DEBUG, "GetModRegRM8: REG %s", GetReg8Str(modregrm >> 3));
		BYTE* reg = GetReg8(modregrm >> 3);
		BYTE* modrm = GetModRM8(modregrm);

		sd.source = toReg ? modrm : reg;
		sd.dest = toReg ? reg : modrm;

		return sd;
	}

	SourceDest16 CPU8086::GetModRegRM16(BYTE modregrm, bool toReg, bool segReg)
	{
		LogPrintf(LOG_DEBUG, "GetModRegRM16: modregrm=%d, toReg=%d", modregrm, toReg);

		SourceDest16 sd;

		// reg part
		LogPrintf(LOG_DEBUG, "GetModRegRM16: REG %s", GetReg16Str(modregrm >> 3, segReg));
		WORD* reg = GetReg16(modregrm >> 3, segReg);

		// modrm
		WORD* modrm;
		switch (modregrm & 0xC0)
		{
		case 0xC0: // REG
			LogPrintf(LOG_DEBUG, "GetModRM16: RM=>REG %s", GetReg16Str(modregrm));
			modrm = GetReg16(modregrm);
			break;
		default:
			throw std::exception("GetModRegRM16: not implemented");
		}

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
		AdjustParity(getLByte(data));
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
		//TODO
	}

	void CPU8086::STI()
	{
		LogPrintf(LOG_DEBUG, "STI");
		SetFlag(FLAG_I, true);
		//TODO
	}

	void CPU8086::HLT()
	{
		LogPrintf(LOG_ERROR, "HALT");
		EnableLog(true);
		Dump();
		m_state = CPUState::STOP;
	}
	
	void CPU8086::JMPfar()
	{
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		LogPrintf(LOG_DEBUG, "JMPfar %02X|%02X", segment, offset);
		regCS = segment;
		regIP = offset;
	}

	void CPU8086::JMPNear(BYTE offset)
	{
		LogPrintf(LOG_DEBUG, "JMPNear Byte offset %02X", offset);
		regIP += widen(offset);
		Dump();
	}
	void CPU8086::JMPNear(WORD offset)
	{
		LogPrintf(LOG_DEBUG, "JMPNear Word offset %04X", offset);
		regIP += offset;
		Dump();
	}

	void CPU8086::NotImplemented(BYTE op)
	{
		LogPrintf(LOG_ERROR, "Not implemented op=%x", op);
		m_state = CPUState::STOP;
	}

	void CPU8086::INCDEC8(BYTE op2)
	{
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

		Dump();
	}

	void CPU8086::INC8(BYTE& b)
	{
		LogPrintf(LOG_DEBUG, "INC8");
		BYTE before = b;

		++b;

		// Flags: ODITSZAPC
		//        XnnnXXXXn
		SetFlag(FLAG_O, getMSB(before) != getMSB(b));
		AdjustSign(b);
		AdjustZero(b);
		SetFlag(FLAG_A, ((before ^ b) & 0x18) == 0x18);
		AdjustParity(b);
	}
	void CPU8086::DEC8(BYTE& b)
	{
		LogPrintf(LOG_DEBUG, "DEC8");
		BYTE before = b;

		--b;

		// Flags: ODITSZAPC
		//        XnnnXXXXn
		SetFlag(FLAG_O, getMSB(before) != getMSB(b));
		AdjustSign(b);
		AdjustZero(b);
		SetFlag(FLAG_A, ((before ^ b) & 0x18) == 0x18);
		AdjustParity(b);
	}

	void CPU8086::INC16(WORD& w)
	{
		LogPrintf(LOG_DEBUG, "INC16");
		WORD before = w;

		++w;

		// Flags: ODITSZAPC
		//        XnnnXXXXn
		SetFlag(FLAG_O, getMSB(before) != getMSB(w));
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, ((before ^ w) & 0x18) == 0x18 );
		AdjustParity(w);
	}
	void CPU8086::DEC16(WORD& w)
	{
		LogPrintf(LOG_DEBUG, "DEC16");
		WORD before = w;

		--w;

		// Flags: ODITSZAPC
		//        XnnnXXXXn
		SetFlag(FLAG_O, getMSB(before) != getMSB(w));
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, ((before ^ w) & 0x18) == 0x18);
		AdjustParity(w);
	}

	void CPU8086::MOV8(BYTE& d, BYTE s)
	{
		LogPrintf(LOG_DEBUG, "MOV8");

		d = s;
		// Flags: ODITSZAPC
		//        nnnnnnnnn
	}
	void CPU8086::MOV8(SourceDest8 sd)
	{
		LogPrintf(LOG_DEBUG, "MOV8");

		*(sd.dest) = *(sd.source);
		// Flags: ODITSZAPC
		//        nnnnnnnnn
	}

	void CPU8086::MOV16(WORD& d, WORD s)
	{
		LogPrintf(LOG_DEBUG, "MOV16");

		d = s;
		// Flags: ODITSZAPC
		//        nnnnnnnnn
	}
	void CPU8086::MOV16(SourceDest16 sd)
	{
		LogPrintf(LOG_DEBUG, "MOV16");

		*(sd.dest) = *(sd.source);
		// Flags: ODITSZAPC
		//        nnnnnnnnn
	}

	void CPU8086::SAHF()
	{
		LogPrintf(LOG_DEBUG, "SAHF");

		const WORD mask = FLAG_C | FLAG_P | FLAG_A | FLAG_Z | FLAG_S;

		flags &= (~mask);
		flags |= (regA.hl.h & mask);
		Dump();
	}
	void CPU8086::LAHF()
	{
		LogPrintf(LOG_DEBUG, "LAHF");

		regA.hl.h = (flags & 0x00FF);
		Dump();
	}

	void CPU8086::JMPif(bool cond)
	{
		LogPrintf(LOG_DEBUG, "JMPif %d", cond);
		BYTE offset = FetchByte();
		if (cond)
		{
			regIP += offset;
		}
	}

	void CPU8086::SHIFTROT8(BYTE op2, BYTE count)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT8 op2=" PRINTF_BIN_PATTERN_INT8 ", count=%d", PRINTF_BYTE_TO_BIN_INT8(op2), count);
		if (count == 0)
			return; // TODO

		BYTE* b = GetModRM8(op2);
		BYTE& dest = *b;
		BYTE before = dest;
		switch (op2 & 0x38)
		{
		case 0x00: // ROL
			LogPrintf(LOG_DEBUG, "SHIFTROT8 ROL");
			throw(std::exception("SHIFTROT8 ROL not implemented"));
			break;
		case 0x08: // ROR
			LogPrintf(LOG_DEBUG, "SHIFTROT8 ROR");
			throw(std::exception("SHIFTROT8 ROR not implemented"));
			break;
		case 0x10: // RCL
			LogPrintf(LOG_DEBUG, "SHIFTROT8 RCL");
			throw(std::exception("SHIFTROT8 RCL not implemented"));
			break;
		case 0x18: // RCR
			LogPrintf(LOG_DEBUG, "SHIFTROT8 RCR");
			throw(std::exception("SHIFTROT8 RCR not implemented"));
			break;
		case 0x20: // SHL/SAL
			// Flags: ODITSZAPC
			//        XnnnnnnnX
			LogPrintf(LOG_DEBUG, "SHIFTROT8 SHL");
			if (count > 1) {
				dest <<= (count - 1);
			}
			before = dest;
			SetFlag(FLAG_C, getMSB(dest));
			dest <<= 1;
			break;
		case 0x28: // SHR
			// Flags: ODITSZAPC
			//        XnnnnnnnX
			LogPrintf(LOG_DEBUG, "SHIFTROT8 SHR");
			if (count > 1) {
				dest >>= (count - 1);
			}
			before = dest;
			SetFlag(FLAG_C, getLSB(dest));
			dest >>= 1;
			break;
		case 0x38: // SAR
			LogPrintf(LOG_DEBUG, "SHIFTROT8 SAR");
			throw(std::exception("SHIFTROT8 SAR not implemented"));
			break;
		default: 
			break;
		}

		SetFlag(FLAG_O, getMSB(before) != getMSB(dest));
		AdjustSign(dest);
		AdjustZero(dest);
		SetFlag(FLAG_A, ((before ^ dest) & 0x18) == 0x18);
		AdjustParity(dest);

		Dump();
	}

	void CPU8086::SHIFTROT16(BYTE op2, BYTE count)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT16 op2=" PRINTF_BIN_PATTERN_INT8 ", count=%d", PRINTF_BYTE_TO_BIN_INT8(op2), count);
		throw(std::exception("SHIFTROT16 not implemented"));
	}

	void CPU8086::Arithmetic8(SourceDest8 sd, RawOpFunc8 func)
	{
		BYTE before = *(sd.dest);
		BYTE after = func(sd);

		SetFlag(FLAG_O, getMSB(before) != getMSB(after));
		AdjustSign(after);
		AdjustZero(after);
		SetFlag(FLAG_A, ((before ^ after) & 0x18) == 0x18);
		AdjustParity(after);
	}
	void CPU8086::Arithmetic16(SourceDest16 sd, RawOpFunc16 func)
	{
		WORD before = *(sd.dest);
		WORD after = func(sd);

		SetFlag(FLAG_O, getMSB(before) != getMSB(after));
		AdjustSign(after);
		AdjustZero(after);
		SetFlag(FLAG_A, ((before ^ after) & 0x18) == 0x18);
		AdjustParity(after);
	}

	void CPU8086::IN8(WORD port)
	{
		m_ports.In(port, regA.hl.l);
	}

	void CPU8086::OUT8(WORD port)
	{
		m_ports.Out(port, regA.hl.l);
	}

	void CPU8086::LOOP(BYTE offset)
	{
		--regC.x;
		if (regC.x)
		{
			regIP += widen(offset);
		}
	}

	void CPU8086::RETNear(bool pop, WORD value)
	{
		LogPrintf(LOG_DEBUG, "RETNear [%s][%d]", pop?"Pop":"NoPop", value);

		Dump();

		BYTE ipLo, ipHi;
		m_memory.Read(S2A(regSS, regSP++), ipLo);
		m_memory.Read(S2A(regSS, regSP++), ipHi);
		regIP = getWord(ipHi, ipLo);
		regSP += value;

		Dump();
	}

	void CPU8086::ArithmeticImm8(BYTE op2, BYTE imm)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticImm8");

		RawOpFunc8 func;

		switch (op2 & 0x38)
		{
		case 0x00: func = rawAdd8; break;
		case 0x08: func = rawOr8; break;
		case 0x10: func = rawAdc8; break;
		case 0x18: func = rawSbb8; break;
		case 0x20: func = rawAnd8; break;
		case 0x28: func = rawSub8; break;
		case 0x30: func = rawXor8; break;
		case 0x38: func = rawCmp8; break;
		default:
			throw(std::exception("not possible"));
		}

		SourceDest8 sd;
		sd.source = &imm;
		sd.dest = GetModRM8(op2);

		BYTE before = *(sd.dest);
		BYTE after = func(sd);

		SetFlag(FLAG_O, getMSB(before) != getMSB(after));
		AdjustSign(after);
		AdjustZero(after);
		SetFlag(FLAG_A, ((before ^ after) & 0x18) == 0x18);
		AdjustParity(after);

	}
	void CPU8086::ArithmeticImm16(BYTE op2, WORD imm)
	{
		LogPrintf(LOG_DEBUG, "ArithmeticImm16");
		throw(std::exception("ArithmeticImm16 not implemented"));
	}

}
