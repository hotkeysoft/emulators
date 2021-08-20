#include "stdafx.h"
#include "CPU8086.h"

namespace emul
{
	CPU8086::CPU8086(Memory& memory, MemoryMap& mmap)
		: CPU(CPU8086_ADDRESS_BITS, memory, mmap), Logger("CPU8086")
	{
		// ADD rm+r=>rm (4)
		// --------
		// REG8/MEM8, REG8
		AddOpcode(0x00, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x01, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x02, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x03, (OPCodeFunction)(&CPU8086::HLT));

		// ADD i=>a (2-3)
		// --------
		// AL, IMMED8
		AddOpcode(0x04, (OPCodeFunction)(&CPU8086::HLT));
		// AL, IMMED16
		AddOpcode(0x05, (OPCodeFunction)(&CPU8086::HLT));

		// PUSH ES (1)
		AddOpcode(0x06, (OPCodeFunction)(&CPU8086::HLT));
		// POP ES (1)
		AddOpcode(0x07, (OPCodeFunction)(&CPU8086::HLT));

		// OR rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x08, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x09, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x0A, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x0B, (OPCodeFunction)(&CPU8086::HLT));

		// OR i=>a (2-3)
		// ----------
		// AL, IMMED8
		AddOpcode(0x0C, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x0D, (OPCodeFunction)(&CPU8086::HLT));

		// PUSH CS (1)
		AddOpcode(0x0E, (OPCodeFunction)(&CPU8086::HLT));

		// ADC rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x10, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x11, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x12, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x13, (OPCodeFunction)(&CPU8086::HLT));

		// ADC i=>a (2-3)
		// ----------
		// AL, IMMED8
		AddOpcode(0x14, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x15, (OPCodeFunction)(&CPU8086::HLT));

		// PUSH SS (1)
		AddOpcode(0x16, (OPCodeFunction)(&CPU8086::HLT));
		// POP SS (1)
		AddOpcode(0x17, (OPCodeFunction)(&CPU8086::HLT));

		// SBB rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x18, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x19, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x1A, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x1B, (OPCodeFunction)(&CPU8086::HLT));

		// SBB i=>a (2-3)
		// ----------
		// AL, IMMED8
		AddOpcode(0x1C, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x1D, (OPCodeFunction)(&CPU8086::HLT));

		// PUSH DS (1)
		AddOpcode(0x1E, (OPCodeFunction)(&CPU8086::HLT));
		// POP DS (1)
		AddOpcode(0x1F, (OPCodeFunction)(&CPU8086::HLT));

		// AND rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x20, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x21, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x22, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x23, (OPCodeFunction)(&CPU8086::HLT));

		// AND i=>a (2-3)
		// ----------
		// AL, IMMED8
		AddOpcode(0x24, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x25, (OPCodeFunction)(&CPU8086::HLT));

		// ES Segment Override
		AddOpcode(0x26, (OPCodeFunction)(&CPU8086::HLT));

		// DAA (1)
		AddOpcode(0x27, (OPCodeFunction)(&CPU8086::HLT));

		// SUB rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x28, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x29, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x2A, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x2B, (OPCodeFunction)(&CPU8086::HLT));

		// SUB i=>a (2-3)
		// ----------
		// AL, IMMED8
		AddOpcode(0x2C, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x2D, (OPCodeFunction)(&CPU8086::HLT));

		// CS Segment Override
		AddOpcode(0x2E, (OPCodeFunction)(&CPU8086::HLT));

		// DAS (1)
		AddOpcode(0x2F, (OPCodeFunction)(&CPU8086::HLT));

		// XOR rm+r=>rm (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x30, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x31, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x32, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x33, (OPCodeFunction)(&CPU8086::HLT));

		// XOR i=>a (2-3)
		// ----------
		// AL, IMMED8
		AddOpcode(0x34, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x35, (OPCodeFunction)(&CPU8086::HLT));

		// SS Segment Override
		AddOpcode(0x36, (OPCodeFunction)(&CPU8086::HLT));

		// AAA (1)
		AddOpcode(0x37, (OPCodeFunction)(&CPU8086::HLT));

		// CMP rm+r=>r (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x38, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x39, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x3A, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x3B, (OPCodeFunction)(&CPU8086::HLT));

		// CMP i=>a (2)
		// ----------
		// AL, IMMED8
		AddOpcode(0x3C, (OPCodeFunction)(&CPU8086::HLT));
		// AX, IMMED16
		AddOpcode(0x3D, (OPCodeFunction)(&CPU8086::HLT));

		// DS Segment Override
		AddOpcode(0x3E, (OPCodeFunction)(&CPU8086::HLT));

		// AAS (1)
		AddOpcode(0x3F, (OPCodeFunction)(&CPU8086::HLT));

		// INC r (1)
		// ----------
		// INC AX
		AddOpcode(0x40, (OPCodeFunction)(&CPU8086::HLT));
		// INC CX
		AddOpcode(0x41, (OPCodeFunction)(&CPU8086::HLT));
		// INC DX
		AddOpcode(0x42, (OPCodeFunction)(&CPU8086::HLT));
		// INC BX
		AddOpcode(0x43, (OPCodeFunction)(&CPU8086::HLT));
		// INC SP
		AddOpcode(0x44, (OPCodeFunction)(&CPU8086::HLT));
		// INC BP
		AddOpcode(0x45, (OPCodeFunction)(&CPU8086::HLT));
		// INC SI
		AddOpcode(0x46, (OPCodeFunction)(&CPU8086::HLT));
		// INC DI
		AddOpcode(0x47, (OPCodeFunction)(&CPU8086::HLT));

		// DEC r (1)
		// ----------
		// DEC AX
		AddOpcode(0x48, (OPCodeFunction)(&CPU8086::HLT));
		// DEC CX
		AddOpcode(0x49, (OPCodeFunction)(&CPU8086::HLT));
		// DEC DX
		AddOpcode(0x4A, (OPCodeFunction)(&CPU8086::HLT));
		// DEC BX
		AddOpcode(0x4B, (OPCodeFunction)(&CPU8086::HLT));
		// DEC SP
		AddOpcode(0x4C, (OPCodeFunction)(&CPU8086::HLT));
		// DEC BP
		AddOpcode(0x4D, (OPCodeFunction)(&CPU8086::HLT));
		// DEC SI
		AddOpcode(0x4E, (OPCodeFunction)(&CPU8086::HLT));
		// DEC DI
		AddOpcode(0x4F, (OPCodeFunction)(&CPU8086::HLT));

		// PUSH r (1)
		// ----------
		// PUSH AX
		AddOpcode(0x50, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH CX
		AddOpcode(0x51, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH DX
		AddOpcode(0x52, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH BX
		AddOpcode(0x53, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH SP
		AddOpcode(0x54, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH BP
		AddOpcode(0x55, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH SI
		AddOpcode(0x56, (OPCodeFunction)(&CPU8086::HLT));
		// PUSH DI
		AddOpcode(0x57, (OPCodeFunction)(&CPU8086::HLT));

		// POP r (1)
		// ----------
		// POP AX
		AddOpcode(0x58, (OPCodeFunction)(&CPU8086::HLT));
		// POP CX
		AddOpcode(0x59, (OPCodeFunction)(&CPU8086::HLT));
		// POP DX
		AddOpcode(0x5A, (OPCodeFunction)(&CPU8086::HLT));
		// POP BX
		AddOpcode(0x5B, (OPCodeFunction)(&CPU8086::HLT));
		// POP SP
		AddOpcode(0x5C, (OPCodeFunction)(&CPU8086::HLT));
		// POP BP
		AddOpcode(0x5D, (OPCodeFunction)(&CPU8086::HLT));
		// POP SI
		AddOpcode(0x5E, (OPCodeFunction)(&CPU8086::HLT));
		// POP DI
		AddOpcode(0x5F, (OPCodeFunction)(&CPU8086::HLT));

		// JO (2)
		AddOpcode(0x70, (OPCodeFunction)(&CPU8086::HLT));
		// JNO (2)
		AddOpcode(0x71, (OPCodeFunction)(&CPU8086::HLT));
		// JB/JNAE (2)
		AddOpcode(0x72, (OPCodeFunction)(&CPU8086::HLT));
		// JNB/JAE (2)
		AddOpcode(0x73, (OPCodeFunction)(&CPU8086::HLT));
		// JE/JZ (2)
		AddOpcode(0x74, (OPCodeFunction)(&CPU8086::HLT));
		// JNE/JNZ (2)
		AddOpcode(0x75, (OPCodeFunction)(&CPU8086::HLT));
		// JBE/JNA (2)
		AddOpcode(0x76, (OPCodeFunction)(&CPU8086::HLT));
		// JNBE/JA (2)
		AddOpcode(0x77, (OPCodeFunction)(&CPU8086::HLT));
		// JS (2)
		AddOpcode(0x78, (OPCodeFunction)(&CPU8086::HLT));
		// JNS (2)
		AddOpcode(0x79, (OPCodeFunction)(&CPU8086::HLT));
		// JP/JPE (2)
		AddOpcode(0x7A, (OPCodeFunction)(&CPU8086::HLT));
		// JNP/JPO (2)
		AddOpcode(0x7B, (OPCodeFunction)(&CPU8086::HLT));
		// JL/JNGE (2)
		AddOpcode(0x7C, (OPCodeFunction)(&CPU8086::HLT));
		// JNL/JGE (2)
		AddOpcode(0x7D, (OPCodeFunction)(&CPU8086::HLT));
		// JLE/JNG (2)
		AddOpcode(0x7E, (OPCodeFunction)(&CPU8086::HLT));
		// JNLE/JG (2)
		AddOpcode(0x7F, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// ADD/OR/ADC/SBB/AND/SUB/XOR/CMP i=>rm (5-6)
		// ----------
		// REG8/MEM8, IMM8
		AddOpcode(0x80, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, IMM16
		AddOpcode(0x81, (OPCodeFunction)(&CPU8086::HLT));

		// ADD/--/ADC/SBB/---/SUB/---/CMP i=>rm (5-6)??
		// ----------
		// REG8/MEM8, IMM8 ?
		AddOpcode(0x82, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, IMM16 ?
		AddOpcode(0x83, (OPCodeFunction)(&CPU8086::HLT));

		// TEST rm+r (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x84, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x85, (OPCodeFunction)(&CPU8086::HLT));

		// XCHG rm<=>r (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x86, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x87, (OPCodeFunction)(&CPU8086::HLT));

		// MOV rm<=>r (4)
		// ----------
		// REG8/MEM8, REG8
		AddOpcode(0x88, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, REG16
		AddOpcode(0x89, (OPCodeFunction)(&CPU8086::HLT));
		// REG8, REG8/MEM8
		AddOpcode(0x8A, (OPCodeFunction)(&CPU8086::HLT));
		// REG16, REG16/MEM16
		AddOpcode(0x8B, (OPCodeFunction)(&CPU8086::HLT));

		// MOV sr=>rm (4)
		// ----------
		// MOV REG16/MEM16, SEGREG
		AddOpcode(0x8C, (OPCodeFunction)(&CPU8086::HLT));

		// LEA (4)
		// ----------
		// REG16, MEM16
		AddOpcode(0x8D, (OPCodeFunction)(&CPU8086::HLT));

		// MOV rm=>sr (4)
		// ----------
		// MOV SEGREG, REG16/MEM16
		AddOpcode(0x8E, (OPCodeFunction)(&CPU8086::HLT));

		// POP rm (4)
		// ----------
		// POP REG16/MEM16
		AddOpcode(0x8F, (OPCodeFunction)(&CPU8086::HLT));

		// XCHG rm<=>a (1)
		// ----------
		// XCHG AX, AX (NOP)
		AddOpcode(0x90, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, CX
		AddOpcode(0x91, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, DX
		AddOpcode(0x92, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, BX
		AddOpcode(0x93, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, SP
		AddOpcode(0x94, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, BP
		AddOpcode(0x95, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, SI
		AddOpcode(0x96, (OPCodeFunction)(&CPU8086::HLT));
		// XCHG AX, DI
		AddOpcode(0x97, (OPCodeFunction)(&CPU8086::HLT));

		// CBW
		AddOpcode(0x98, (OPCodeFunction)(&CPU8086::HLT));
		// CWD
		AddOpcode(0x99, (OPCodeFunction)(&CPU8086::HLT));

		// CALL Far (5)
		AddOpcode(0x9A, (OPCodeFunction)(&CPU8086::HLT));

		// WAIT (1)
		AddOpcode(0x9B, (OPCodeFunction)(&CPU8086::NotImplemented));

		// PUSHF (1)
		AddOpcode(0x9C, (OPCodeFunction)(&CPU8086::HLT));
		// POPF (1)
		AddOpcode(0x9D, (OPCodeFunction)(&CPU8086::HLT));
		// SAHF (1)
		AddOpcode(0x9E, (OPCodeFunction)(&CPU8086::HLT));
		// LAHF (1)
		AddOpcode(0x9F, (OPCodeFunction)(&CPU8086::HLT));

		// MOV m=>a (3)
		// ----------
		// MOV AL, MEM8
		AddOpcode(0xA0, (OPCodeFunction)(&CPU8086::HLT));
		// MOV AX, MEM16
		AddOpcode(0xA1, (OPCodeFunction)(&CPU8086::HLT));

		// MOV a=>m (3)
		// ----------
		// MOV MEM8, AL
		AddOpcode(0xA2, (OPCodeFunction)(&CPU8086::HLT));
		// MOV MEM16, AX
		AddOpcode(0xA3, (OPCodeFunction)(&CPU8086::HLT));

		// MOVS (1)
		// ----------
		// MOVS DEST-STR8, SRC-STR8
		AddOpcode(0xA4, (OPCodeFunction)(&CPU8086::HLT));
		// MOVS DEST-STR16, SRC-STR16
		AddOpcode(0xA5, (OPCodeFunction)(&CPU8086::HLT));

		// CMPS (1)
		// ----------
		// CMPS DEST-STR8, SRC-STR8
		AddOpcode(0xA6, (OPCodeFunction)(&CPU8086::HLT));
		// CMPS DEST-STR16, SRC-STR16
		AddOpcode(0xA7, (OPCodeFunction)(&CPU8086::HLT));

		// TEST i+a (2)
		// ----------
		// TEST AL, IMM8
		AddOpcode(0xA8, (OPCodeFunction)(&CPU8086::HLT));
		// TEST AX, IMM16
		AddOpcode(0xA9, (OPCodeFunction)(&CPU8086::HLT));

		// STOS (1)
		// ----------
		// STOS DEST-STR8
		AddOpcode(0xAA, (OPCodeFunction)(&CPU8086::HLT));
		// STOS DEST-STR16
		AddOpcode(0xAB, (OPCodeFunction)(&CPU8086::HLT));

		// LODS (1)
		// ----------
		// LDOS SRC-STR8
		AddOpcode(0xAC, (OPCodeFunction)(&CPU8086::HLT));
		// LDOS SRC-STR16
		AddOpcode(0xAD, (OPCodeFunction)(&CPU8086::HLT));

		// SCAS (1)
		// ----------
		// SCAS DEST-STR8
		AddOpcode(0xAE, (OPCodeFunction)(&CPU8086::HLT));
		// SCAS DEST-STR16
		AddOpcode(0xAF, (OPCodeFunction)(&CPU8086::HLT));


		// MOV i=>r (2-3)
		// ----------
		// MOV AL, IMM8
		AddOpcode(0xB0, (OPCodeFunction)(&CPU8086::HLT));
		// MOV CL, IMM8
		AddOpcode(0xB1, (OPCodeFunction)(&CPU8086::HLT));
		// MOV DL, IMM8
		AddOpcode(0xB2, (OPCodeFunction)(&CPU8086::HLT));
		// MOV BL, IMM8
		AddOpcode(0xB3, (OPCodeFunction)(&CPU8086::HLT));
		// MOV AH, IMM8
		AddOpcode(0xB4, (OPCodeFunction)(&CPU8086::HLT));
		// MOV CH, IMM8
		AddOpcode(0xB5, (OPCodeFunction)(&CPU8086::HLT));
		// MOV DH, IMM8
		AddOpcode(0xB6, (OPCodeFunction)(&CPU8086::HLT));
		// MOV BH, IMM8
		AddOpcode(0xB7, (OPCodeFunction)(&CPU8086::HLT));

		// MOV AX, IMM16
		AddOpcode(0xB8, (OPCodeFunction)(&CPU8086::HLT));
		// MOV CX, IMM16
		AddOpcode(0xB9, (OPCodeFunction)(&CPU8086::HLT));
		// MOV DX, IMM16
		AddOpcode(0xBA, (OPCodeFunction)(&CPU8086::HLT));
		// MOV BX, IMM16
		AddOpcode(0xBB, (OPCodeFunction)(&CPU8086::HLT));
		// MOV SP, IMM16
		AddOpcode(0xBC, (OPCodeFunction)(&CPU8086::HLT));
		// MOV BP, IMM16
		AddOpcode(0xBD, (OPCodeFunction)(&CPU8086::HLT));
		// MOV SI, IMM16
		AddOpcode(0xBE, (OPCodeFunction)(&CPU8086::HLT));
		// MOV DI, IMM16
		AddOpcode(0xBF, (OPCodeFunction)(&CPU8086::HLT));

		// RET SP+IMM16 (3)
		AddOpcode(0xC2, (OPCodeFunction)(&CPU8086::HLT));
		// RET Near (1)
		AddOpcode(0xC3, (OPCodeFunction)(&CPU8086::HLT));

		// LES REG16, MEM16 (4)
		AddOpcode(0xC4, (OPCodeFunction)(&CPU8086::HLT));
		// LDS REG16, MEM16 (4)
		AddOpcode(0xC5, (OPCodeFunction)(&CPU8086::HLT));

		// MOV i=>rm (5-6)
		// ----------
		// MOV MEM8, IMM8
		AddOpcode(0xC6, (OPCodeFunction)(&CPU8086::HLT));
		// MOV MEM16, IMM16
		AddOpcode(0xC7, (OPCodeFunction)(&CPU8086::HLT));

		// RET Far SP+IMM16 (3)
		AddOpcode(0xCA, (OPCodeFunction)(&CPU8086::HLT));
		// RET Far (1)
		AddOpcode(0xCB, (OPCodeFunction)(&CPU8086::HLT));

		// INT3 (1)
		AddOpcode(0xCC, (OPCodeFunction)(&CPU8086::HLT));
		// INT INN8 (2)
		AddOpcode(0xCD, (OPCodeFunction)(&CPU8086::HLT));
		// INTO (1)
		AddOpcode(0xCE, (OPCodeFunction)(&CPU8086::HLT));
		// IRET (1)
		AddOpcode(0xCF, (OPCodeFunction)(&CPU8086::HLT));

		// ROL/ROR/RCL/RCR/SAL|SHL/SHR/---/SAR
		// ----------
		// REG8/MEM8, 1
		AddOpcode(0xD0, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, 1
		AddOpcode(0xD1, (OPCodeFunction)(&CPU8086::HLT));
		// REG8/MEM8, CL
		AddOpcode(0xD2, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16, CL
		AddOpcode(0xD3, (OPCodeFunction)(&CPU8086::HLT));

		// AAM
		AddOpcode(0xD4, (OPCodeFunction)(&CPU8086::HLT));
		// AAD
		AddOpcode(0xD5, (OPCodeFunction)(&CPU8086::HLT));

		// XLAT (1)
		AddOpcode(0xD7, (OPCodeFunction)(&CPU8086::HLT));

		// ESC (2) ??
		AddOpcode(0xD8, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xD9, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDA, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDB, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDC, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDD, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDE, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDF, (OPCodeFunction)(&CPU8086::NotImplemented));

		// LOOPNZ/LOOPNE (2)
		AddOpcode(0xE0, (OPCodeFunction)(&CPU8086::HLT));
		// LOOPZ/LOOPE (2)
		AddOpcode(0xE1, (OPCodeFunction)(&CPU8086::HLT));
		// LOOP (2)
		AddOpcode(0xE2, (OPCodeFunction)(&CPU8086::HLT));
		// JCXZ (2)
		AddOpcode(0xE3, (OPCodeFunction)(&CPU8086::HLT));

		// IN fixed (2)
		// --------
		// IN AL, IMM8
		AddOpcode(0xE4, (OPCodeFunction)(&CPU8086::HLT));
		// IN AX, IMM8
		AddOpcode(0xE5, (OPCodeFunction)(&CPU8086::HLT));

		// OUT fixed (2)
		// --------
		// OUT AL, IMM8
		AddOpcode(0xE6, (OPCodeFunction)(&CPU8086::HLT));
		// OUT AX, IMM8
		AddOpcode(0xE7, (OPCodeFunction)(&CPU8086::HLT));

		// CALL Near (3)
		AddOpcode(0xE8, (OPCodeFunction)(&CPU8086::HLT));
		// JUMP Near (3)
		AddOpcode(0xE9, (OPCodeFunction)(&CPU8086::HLT));
		// JUMP Far (5)
		AddOpcode(0xEA, (OPCodeFunction)(&CPU8086::JMPfar));
		// JUMP Near Short (2)
		AddOpcode(0xEB, (OPCodeFunction)(&CPU8086::HLT));

		// IN variable (1)
		// --------
		// IN AL, DX
		AddOpcode(0xEC, (OPCodeFunction)(&CPU8086::HLT));
		// IN AX, DX
		AddOpcode(0xED, (OPCodeFunction)(&CPU8086::HLT));

		// OUT variable (1)
		// --------
		// OUT AL, DX
		AddOpcode(0xEE, (OPCodeFunction)(&CPU8086::HLT));
		// OUT AX, DX
		AddOpcode(0xEF, (OPCodeFunction)(&CPU8086::HLT));

		// LOCK (1)
		AddOpcode(0xF0, (OPCodeFunction)(&CPU8086::HLT));

		// REP (1)
		AddOpcode(0xF2, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xF3, (OPCodeFunction)(&CPU8086::HLT));

		// HLT (1)
		AddOpcode(0xF4, (OPCodeFunction)(&CPU8086::HLT));
		// CMC (1)
		AddOpcode(0xF5, (OPCodeFunction)(&CPU8086::CMC));
		// CLC (1)

		// TEST/---/NOT/NEG/MUL/IMUL/DIV/IDIV
		// --------
		// REG8/MEM8 (, IMM8 {TEST})
		AddOpcode(0xF6, (OPCodeFunction)(&CPU8086::HLT));
		// REG16/MEM16 (, IMM16 {TEST})
		AddOpcode(0xF7, (OPCodeFunction)(&CPU8086::HLT));

		AddOpcode(0xF8, (OPCodeFunction)(&CPU8086::CLC));
		// STC (1)
		AddOpcode(0xF9, (OPCodeFunction)(&CPU8086::STC));
		// CLI (1)
		AddOpcode(0xFA, (OPCodeFunction)(&CPU8086::CLI));
		// STI (1)
		AddOpcode(0xFB, (OPCodeFunction)(&CPU8086::STI));
		// CLD (1)
		AddOpcode(0xFC, (OPCodeFunction)(&CPU8086::CLD));
		// STD (1)
		AddOpcode(0xFD, (OPCodeFunction)(&CPU8086::STD));

		// INC/DEC/---/---/---/---/---/---
		// --------
		// REG8/MEM8
		AddOpcode(0xFE, (OPCodeFunction)(&CPU8086::HLT));

		// INC/DEC/CALL/CALL/JMP/JMP/PUSH/---
		AddOpcode(0xFF, (OPCodeFunction)(&CPU8086::HLT));
	}

	CPU8086::~CPU8086()
	{

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
		LogPrintf(LOG_DEBUG, "CS|IP %04X|%04X", regCS, regIP);
		LogPrintf(LOG_DEBUG, "\n");
	}

	void CPU8086::ClearFlags()
	{
		flags = FLAG_R1 | FLAG_R3 | FLAG_R5 | FLAG_R12 | FLAG_R13 | FLAG_R14 | FLAG_R15;
	}

	BYTE CPU8086::FetchByte()
	{
		BYTE b = GetCurrentAddress();
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

	void CPU8086::CLC(BYTE)
	{
		LogPrintf(LOG_DEBUG, "CLC");
		++regIP;
		SetFlag(FLAG_CF, false);
	}

	void CPU8086::CMC(BYTE)
	{
		LogPrintf(LOG_DEBUG, "CMC");
		++regIP;
		SetFlag(FLAG_CF, !GetFlag(FLAG_CF));
	}

	void CPU8086::STC(BYTE)
	{
		LogPrintf(LOG_DEBUG, "STC");
		++regIP;
		SetFlag(FLAG_CF, true);
	}

	void CPU8086::CLD(BYTE)
	{
		LogPrintf(LOG_DEBUG, "CLD");
		++regIP;
		SetFlag(FLAG_DF, false);
	}

	void CPU8086::STD(BYTE)
	{
		LogPrintf(LOG_DEBUG, "STD");
		++regIP;
		SetFlag(FLAG_DF, true);
	}

	void CPU8086::CLI(BYTE)
	{
		LogPrintf(LOG_DEBUG, "CLI");
		++regIP;
		SetFlag(FLAG_IF, false);
		//TODO
	}

	void CPU8086::STI(BYTE)
	{
		LogPrintf(LOG_DEBUG, "STI");
		++regIP;
		SetFlag(FLAG_IF, true);
		//TODO
	}

	void CPU8086::HLT(BYTE op)
	{
		LogPrintf(LOG_ERROR, "HALT op=%x", op);
		m_state = CPUState::STOP;
	}
	
	void CPU8086::JMPfar(BYTE op)
	{
		LogPrintf(LOG_DEBUG, "JMPfar op=%x", op);
		++regIP;
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		regCS = segment;
		regIP = offset;
	}

	void CPU8086::NotImplemented(BYTE op)
	{
		LogPrintf(LOG_ERROR, "Not implemented op=%x", op);
	}
}
