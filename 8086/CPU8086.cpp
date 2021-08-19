#include "stdafx.h"
#include "CPU8086.h"

namespace emul
{
	CPU8086::CPU8086(Memory& memory, MemoryMap& mmap)
		: CPU(CPU8086_ADDRESS_BITS, memory, mmap), Logger("CPU8086")

		//-------------------------
		// Data Transfer
	{
		//----------
		// MOV

		// MOV rm<=>r (4)
		AddOpcode(0x88, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x89, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x8A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x8B, (OPCodeFunction)(&CPU8086::HLT));

		// MOV i=>rm (5-6)
		AddOpcode(0xC6, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xC7, (OPCodeFunction)(&CPU8086::HLT));

		// MOV i=>r (2-3)
		AddOpcode(0xB0, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB1, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB2, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB3, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB4, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB5, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB6, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB7, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB8, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xB9, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xBA, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xBB, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xBC, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xBD, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xBE, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xBF, (OPCodeFunction)(&CPU8086::HLT));

		// MOV m=>a (3)
		AddOpcode(0xA0, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xA1, (OPCodeFunction)(&CPU8086::HLT));

		// MOV a=>m (3)
		AddOpcode(0xA2, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xA3, (OPCodeFunction)(&CPU8086::HLT));

		// MOV rm=>sr (4)
		AddOpcode(0x8E, (OPCodeFunction)(&CPU8086::HLT));

		// MOV sr=>rm (4)
		AddOpcode(0x8C, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// PUSH

		// PUSH r (1)
		AddOpcode(0x50, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x51, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x52, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x53, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x54, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x55, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x56, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x57, (OPCodeFunction)(&CPU8086::HLT));

		// PUSH sr (1)
		AddOpcode(0x06, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x0E, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x16, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x1E, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// POP

		// POP rm (4)
		AddOpcode(0x8F, (OPCodeFunction)(&CPU8086::HLT));

		// POP r (1)
		AddOpcode(0x58, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x59, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x5A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x5B, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x5C, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x5D, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x5E, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x5F, (OPCodeFunction)(&CPU8086::HLT));

		// POP sr (1)
		AddOpcode(0x07, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x0F, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x17, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x1F, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// XCHG

		// XCHG rm<=>r (4)
		AddOpcode(0x86, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x87, (OPCodeFunction)(&CPU8086::HLT));

		// XCHG rm<=>a (1)
		AddOpcode(0x90, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x91, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x92, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x93, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x94, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x95, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x96, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x97, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// IN

		// IN fixed (2)
		AddOpcode(0xE4, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xE5, (OPCodeFunction)(&CPU8086::HLT));

		// IN variable (1)
		AddOpcode(0xEC, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xED, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// OUT

		// OUT fixed (2)
		AddOpcode(0xE6, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xE7, (OPCodeFunction)(&CPU8086::HLT));

		// OUT variable (1)
		AddOpcode(0xEE, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xEF, (OPCodeFunction)(&CPU8086::HLT));

		// XLAT (1)
		AddOpcode(0xD7, (OPCodeFunction)(&CPU8086::HLT));
		// LEA (4)
		AddOpcode(0x8D, (OPCodeFunction)(&CPU8086::HLT));
		// LDS (4)
		AddOpcode(0xC5, (OPCodeFunction)(&CPU8086::HLT));
		// LES (4)
		AddOpcode(0xC4, (OPCodeFunction)(&CPU8086::HLT));
		// LAHF (1)
		AddOpcode(0x9F, (OPCodeFunction)(&CPU8086::HLT));
		// SAHF (1)
		AddOpcode(0x9E, (OPCodeFunction)(&CPU8086::HLT));
		// PUSHF (1)
		AddOpcode(0x9C, (OPCodeFunction)(&CPU8086::HLT));
		// POPF (1)
		AddOpcode(0x9D, (OPCodeFunction)(&CPU8086::HLT));

		//-------------------------
		// Arithmetic

		//----------
		// ADD

		// ADD rm+r=>rm (4)
		AddOpcode(0x00, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x01, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x02, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x03, (OPCodeFunction)(&CPU8086::HLT));

		// ADD i=>a (2-3)
		AddOpcode(0x04, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x05, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// ADC

		// ADC rm+r=>rm (4)
		AddOpcode(0x10, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x11, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x12, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x13, (OPCodeFunction)(&CPU8086::HLT));

		// ADC i=>a (2-3)
		AddOpcode(0x14, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x15, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// INC

		// INC r (1)
		AddOpcode(0x40, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x41, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x42, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x43, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x44, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x45, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x46, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x47, (OPCodeFunction)(&CPU8086::HLT));

		// AAA (1)
		AddOpcode(0x37, (OPCodeFunction)(&CPU8086::HLT));
		// DAA (1)
		AddOpcode(0x27, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// SUB

		// SUB rm+r=>rm (4)
		AddOpcode(0x28, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x29, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x2A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x2B, (OPCodeFunction)(&CPU8086::HLT));

		// SUB i=>a (2-3)
		AddOpcode(0x2C, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x2D, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// SBB

		// SBB rm+r=>rm (4)
		AddOpcode(0x18, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x19, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x1A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x1B, (OPCodeFunction)(&CPU8086::HLT));

		// SBB i=>a (2-3)
		AddOpcode(0x1C, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x1D, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// DEC

		// DEC r (1)
		AddOpcode(0x48, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x49, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x4A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x4B, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x4C, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x4D, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x4E, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x4F, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// CMP

		// CMP rm+r=>r (4)
		AddOpcode(0x38, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x39, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x3A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x3B, (OPCodeFunction)(&CPU8086::HLT));

		// CMP i=>a (2)
		AddOpcode(0x3C, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x3D, (OPCodeFunction)(&CPU8086::HLT));

		// AAS (1)
		AddOpcode(0x3F, (OPCodeFunction)(&CPU8086::HLT));
		// DAS (1)
		AddOpcode(0x2F, (OPCodeFunction)(&CPU8086::HLT));
		// AAM
		AddOpcode(0xD4, (OPCodeFunction)(&CPU8086::HLT));
		// AAD
		AddOpcode(0xD5, (OPCodeFunction)(&CPU8086::HLT));
		// CBW
		AddOpcode(0x98, (OPCodeFunction)(&CPU8086::HLT));
		// CWD
		AddOpcode(0x99, (OPCodeFunction)(&CPU8086::HLT));

		//-------------------------
		// Logic

		//----------
		// AND

		// AND rm+r=>rm (4)
		AddOpcode(0x20, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x21, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x22, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x23, (OPCodeFunction)(&CPU8086::HLT));

		// AND i=>a (2-3)
		AddOpcode(0x24, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x25, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// TEST

		// TEST rm+r (4)
		AddOpcode(0x84, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x85, (OPCodeFunction)(&CPU8086::HLT));

		// TEST i+a (2)
		AddOpcode(0xA8, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xA9, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// OR

		// OR rm+r=>rm (4)
		AddOpcode(0x08, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x09, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x0A, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x0B, (OPCodeFunction)(&CPU8086::HLT));

		// OR i=>a (2-3)
		AddOpcode(0x0C, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x0D, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// XOR

		// XOR rm+r=>rm (4)
		AddOpcode(0x30, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x31, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x32, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x33, (OPCodeFunction)(&CPU8086::HLT));

		// XOR i=>a (2-3)
		AddOpcode(0x34, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x35, (OPCodeFunction)(&CPU8086::HLT));

		//-------------------------
		// String Manipulation

		// REP (1)
		AddOpcode(0xF2, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xF3, (OPCodeFunction)(&CPU8086::HLT));

		// MOVS (1)
		AddOpcode(0xA4, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xA5, (OPCodeFunction)(&CPU8086::HLT));

		// CMPS (1)
		AddOpcode(0xA6, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xA7, (OPCodeFunction)(&CPU8086::HLT));

		// SCAS (1)
		AddOpcode(0xAE, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xAF, (OPCodeFunction)(&CPU8086::HLT));

		// LODS (1)
		AddOpcode(0xAC, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xAD, (OPCodeFunction)(&CPU8086::HLT));

		// STOS (1)
		AddOpcode(0xAA, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xAB, (OPCodeFunction)(&CPU8086::HLT));

		//-------------------------
		// Control Transfer

		//----------
		// CALL

		// CALL Near (3)
		AddOpcode(0xE8, (OPCodeFunction)(&CPU8086::HLT));
		// CALL Far (5)
		AddOpcode(0x9A, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// JUMP

		// JUMP Near (3)
		AddOpcode(0xE9, (OPCodeFunction)(&CPU8086::HLT));

		// JUMP Near Short (2)
		AddOpcode(0xEB, (OPCodeFunction)(&CPU8086::HLT));

		// JUMP Far (5)
		AddOpcode(0xEA, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// RET

		// RET Near (1)
		AddOpcode(0xC3, (OPCodeFunction)(&CPU8086::HLT));

		// RET sp+i (3)
		AddOpcode(0xC2, (OPCodeFunction)(&CPU8086::HLT));

		// RET Far (1)
		AddOpcode(0xCB, (OPCodeFunction)(&CPU8086::HLT));

		// RET Far sp+i (3)
		AddOpcode(0xCA, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// Conditional jump/loop

		// JE/JZ (2)
		AddOpcode(0x74, (OPCodeFunction)(&CPU8086::HLT));
		// JL/JNGE (2)
		AddOpcode(0x7C, (OPCodeFunction)(&CPU8086::HLT));
		// JLE/JNG (2)
		AddOpcode(0x7E, (OPCodeFunction)(&CPU8086::HLT));
		// JB/JNAE (2)
		AddOpcode(0x72, (OPCodeFunction)(&CPU8086::HLT));
		// JBE/JNA (2)
		AddOpcode(0x76, (OPCodeFunction)(&CPU8086::HLT));
		// JP/JPE (2)
		AddOpcode(0x7A, (OPCodeFunction)(&CPU8086::HLT));
		// JO (2)
		AddOpcode(0x70, (OPCodeFunction)(&CPU8086::HLT));
		// JS (2)
		AddOpcode(0x78, (OPCodeFunction)(&CPU8086::HLT));
		// JNE/JNZ (2)
		AddOpcode(0x75, (OPCodeFunction)(&CPU8086::HLT));
		// JNL/JGE (2)
		AddOpcode(0x7D, (OPCodeFunction)(&CPU8086::HLT));
		// JNLE/JG (2)
		AddOpcode(0x7F, (OPCodeFunction)(&CPU8086::HLT));
		// JNB/JAE (2)
		AddOpcode(0x73, (OPCodeFunction)(&CPU8086::HLT));
		// JNBE/JA (2)
		AddOpcode(0x77, (OPCodeFunction)(&CPU8086::HLT));
		// JNP/JPO (2)
		AddOpcode(0x7B, (OPCodeFunction)(&CPU8086::HLT));
		// JNO (2)
		AddOpcode(0x71, (OPCodeFunction)(&CPU8086::HLT));
		// JNS (2)
		AddOpcode(0x79, (OPCodeFunction)(&CPU8086::HLT));

		// LOOP (2)
		AddOpcode(0xE2, (OPCodeFunction)(&CPU8086::HLT));
		// LOOPZ/LOOPE (2)
		AddOpcode(0xE1, (OPCodeFunction)(&CPU8086::HLT));
		// LOOPNZ/LOOPNE (2)
		AddOpcode(0xE0, (OPCodeFunction)(&CPU8086::HLT));
		// JCXZ (2)
		AddOpcode(0xE3, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// Interrupt

		// INT (2)
		AddOpcode(0xCD, (OPCodeFunction)(&CPU8086::HLT));

		// INT3 (1)
		AddOpcode(0xCC, (OPCodeFunction)(&CPU8086::HLT));

		// INTO (1)
		AddOpcode(0xCE, (OPCodeFunction)(&CPU8086::HLT));

		// IRET (1)
		AddOpcode(0xCF, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// Processor Control

		// CLC (1)
		AddOpcode(0xF8, (OPCodeFunction)(&CPU8086::HLT));
		// CMC (1)
		AddOpcode(0xF5, (OPCodeFunction)(&CPU8086::HLT));
		// STC (1)
		AddOpcode(0xF9, (OPCodeFunction)(&CPU8086::HLT));
		// CLD (1)
		AddOpcode(0xFC, (OPCodeFunction)(&CPU8086::HLT));
		// STD (1)
		AddOpcode(0xFD, (OPCodeFunction)(&CPU8086::HLT));
		// CLI (1)
		AddOpcode(0xFA, (OPCodeFunction)(&CPU8086::HLT));
		// STI (1)
		AddOpcode(0xFB, (OPCodeFunction)(&CPU8086::HLT));

		// HLT (1)
		AddOpcode(0xF4, (OPCodeFunction)(&CPU8086::HLT));

		// WAIT (1)
		AddOpcode(0x9B, (OPCodeFunction)(&CPU8086::NotImplemented));
		// ESC (2)
		AddOpcode(0xD8, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xD9, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDA, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDB, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDC, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDD, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDE, (OPCodeFunction)(&CPU8086::NotImplemented));
		AddOpcode(0xDF, (OPCodeFunction)(&CPU8086::NotImplemented));

		// LOCK (1)
		AddOpcode(0xF0, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// MULTI ops

		//----------
		// ADD/ADC/AND/SUB/SBB/CMP/OR/XOR i=>rm (5-6)
		AddOpcode(0x80, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x81, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x82, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x83, (OPCodeFunction)(&CPU8086::HLT));

		// INC/DEC rm (4)
		AddOpcode(0xFE, (OPCodeFunction)(&CPU8086::HLT));
		// INC/DEC/PUSH/CALL/JMP
		AddOpcode(0xFF, (OPCodeFunction)(&CPU8086::HLT));

		// NEG/MUL/IMUL/DIV/IDIV/NOT/TEST
		AddOpcode(0xF6, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xF7, (OPCodeFunction)(&CPU8086::HLT));

		// SHL/SAL/SHR/SAR/ROL/ROR/RCL/RCR (4)
		AddOpcode(0xD0, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xD1, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xD2, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0xD3, (OPCodeFunction)(&CPU8086::HLT));

		//----------
		// Segment Override
		AddOpcode(0x26, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x2E, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x36, (OPCodeFunction)(&CPU8086::HLT));
		AddOpcode(0x3E, (OPCodeFunction)(&CPU8086::HLT));
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
		LogPrintf(LOG_DEBUG, "\n");
	}

	void CPU8086::ClearFlags()
	{
		flags = FLAG_R1 | FLAG_R3 | FLAG_R5 | FLAG_R12 | FLAG_R13 | FLAG_R14 | FLAG_R15;
	}

	void CPU8086::HLT(BYTE op)
	{
		LogPrintf(LOG_ERROR, "HALT op=%x", op);
		m_state = CPUState::STOP;
	}
	
	void CPU8086::NotImplemented(BYTE op)
	{
		LogPrintf(LOG_ERROR, "Not implemented op=%x", op);
	}

}