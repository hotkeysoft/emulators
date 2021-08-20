#include "stdafx.h"
#include "CPU8086.h"

namespace emul
{
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
		case 0x00: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x01: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x02: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x03: NotImplemented(opcode); break;

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
		case 0x08: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x09: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x0A: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x0B: NotImplemented(opcode); break;

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
		case 0x28: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x29: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x2A: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x2B: NotImplemented(opcode); break;

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
		case 0x30: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x31: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x32: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x33: NotImplemented(opcode); break;

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
		case 0x76: NotImplemented(opcode); break;
		// JNBE/JA (2)
		case 0x77: NotImplemented(opcode); break;
		// JS (2)
		case 0x78: JMPif(GetFlag(FLAG_S)); break;
		// JNS (2)
		case 0x79: JMPif(!GetFlag(FLAG_S)); break;
		// JP/JPE (2)
		case 0x7A: JMPif(GetFlag(FLAG_P)); break;
		// JNP/JPO (2)
		case 0x7B: JMPif(!GetFlag(FLAG_P)); break;
		// JL/JNGE (2)
		case 0x7C: NotImplemented(opcode); break;
		// JNL/JGE (2)
		case 0x7D: NotImplemented(opcode); break;
		// JLE/JNG (2)
		case 0x7E: NotImplemented(opcode); break;
		// JNLE/JG (2)
		case 0x7F: NotImplemented(opcode); break;

		//----------
		// ADD/OR/ADC/SBB/AND/SUB/XOR/CMP i=>rm (5-6)
		// ----------
		// REG8/MEM8, IMM8
		case 0x80: NotImplemented(opcode); break;
		// REG16/MEM16, IMM16
		case 0x81: NotImplemented(opcode); break;

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
		case 0x88: NotImplemented(opcode); break;
		// REG16/MEM16, REG16
		case 0x89: NotImplemented(opcode); break;
		// REG8, REG8/MEM8
		case 0x8A: NotImplemented(opcode); break;
		// REG16, REG16/MEM16
		case 0x8B: NotImplemented(opcode); break;

		// MOV sr=>rm (4)
		// ----------
		// MOV REG16/MEM16, SEGREG
		case 0x8C: NotImplemented(opcode); break;

		// LEA (4)
		// ----------
		// REG16, MEM16
		case 0x8D: NotImplemented(opcode); break;

		// MOV rm=>sr (4)
		// ----------
		// MOV SEGREG, REG16/MEM16
		case 0x8E: NotImplemented(opcode); break;

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
		case 0xC2: NotImplemented(opcode); break;
		// RET Near (1)
		case 0xC3: NotImplemented(opcode); break;

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
		case 0xD0: NotImplemented(opcode); break;
		// REG16/MEM16, 1
		case 0xD1: NotImplemented(opcode); break;
		// REG8/MEM8, CL
		case 0xD2: NotImplemented(opcode); break;
		// REG16/MEM16, CL
		case 0xD3: NotImplemented(opcode); break;

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
		case 0xE2: NotImplemented(opcode); break;
		// JCXZ (2)
		case 0xE3: NotImplemented(opcode); break;

		// IN fixed (2)
		// --------
		// IN AL, IMM8
		case 0xE4: NotImplemented(opcode); break;
		// IN AX, IMM8
		case 0xE5: NotImplemented(opcode); break;

		// OUT fixed (2)
		// --------
		// OUT AL, IMM8
		case 0xE6: NotImplemented(opcode); break;
		// OUT AX, IMM8
		case 0xE7: NotImplemented(opcode); break;

		// CALL Near (3)
		case 0xE8: NotImplemented(opcode); break;
		// JUMP Near (3)
		case 0xE9: NotImplemented(opcode); break;
		// JUMP Far (5)
		case 0xEA: JMPfar(); break;
		// JUMP Near Short (2)
		case 0xEB: NotImplemented(opcode); break;

		// IN variable (1)
		// --------
		// IN AL, DX
		case 0xEC: NotImplemented(opcode); break;
		// IN AX, DX
		case 0xED: NotImplemented(opcode); break;

		// OUT variable (1)
		// --------
		// OUT AL, DX
		case 0xEE: NotImplemented(opcode); break;
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
		case 0xFE: NotImplemented(opcode); break;

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
		LogPrintf(LOG_DEBUG,
			"AH|AL %02X|%02X\n"
			"BH|BL %02X|%02X\n"
			"CH|CL %02X|%02X\n"
			"DH|DL %02X|%02X\n"
			"\n"
			"CS|IP %04X|%04X\n"
			"FLAGS xxxxODITSZxAxPxC\n"
			"      " PRINTF_BIN_PATTERN_INT16 
			"\n"
			"\n",
			regA.hl.h, regA.hl.l,
			regB.hl.h, regB.hl.l,
			regC.hl.h, regC.hl.l,
			regD.hl.h, regD.hl.l,
			regCS, regIP,
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
		SetFlag(FLAG_P, IsParityEven(data));
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
		m_state = CPUState::STOP;
	}
	
	void CPU8086::JMPfar()
	{
		LogPrintf(LOG_DEBUG, "JMPfar");
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		regCS = segment;
		regIP = offset;
	}

	void CPU8086::NotImplemented(BYTE op)
	{
		LogPrintf(LOG_ERROR, "Not implemented op=%x", op);
		m_state = CPUState::STOP;
	}

	void CPU8086::INC16(WORD& w)
	{
		LogPrintf(LOG_DEBUG, "INC16");

		++w;

		// Flags: ODITSZAPC
		//        XnnnXXXXn
		SetFlag(FLAG_O, w==0); // TODO Check
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, (w & 0x0F) == 0);
		AdjustParity(w);
	}
	void CPU8086::DEC16(WORD& w)
	{
		LogPrintf(LOG_DEBUG, "DEC16");

		--w;

		// Flags: ODITSZAPC
		//        XnnnXXXXn
		SetFlag(FLAG_O, w=0xFFFF); // TODO CHECK
		AdjustSign(w);
		AdjustZero(w);
		SetFlag(FLAG_A, (w & 0x0F) != 0x0F);
		AdjustParity(w);
	}

	void CPU8086::MOV8(BYTE& d, BYTE s)
	{
		LogPrintf(LOG_DEBUG, "MOV8");

		d = s;
		// Flags: ODITSZAPC
		//        nnnnnnnnn
		Dump();
	}

	void CPU8086::MOV16(WORD& d, WORD s)
	{
		LogPrintf(LOG_DEBUG, "MOV16");

		d = s;
		// Flags: ODITSZAPC
		//        nnnnnnnnn
		Dump();
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

}
