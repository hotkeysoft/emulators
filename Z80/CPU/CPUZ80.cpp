#include "stdafx.h"
#include "CPUZ80.h"

using cpuInfo::Opcode;
using cpuInfo::CPUType;

namespace emul
{
	CPUZ80::CPUZ80(Memory& memory, Interrupts& interrupts) : CPUZ80(CPUType::z80, memory, interrupts)
	{
	}

	CPUZ80::CPUZ80(cpuInfo::CPUType type, Memory& memory, Interrupts& interrupts) :
		Logger("Z80"),
		CPU8080(type, memory, interrupts)
	{
		FLAG_RESERVED_ON = (FLAG)0;
		FLAG_RESERVED_OFF = (FLAG)0;
	}

	void CPUZ80::Reset()
	{
		CPU8080::Reset();

		memset(&m_reg, 0xFF, sizeof(Registers));
		memset(&m_regAlt, 0xFF, sizeof(Registers));

		m_regIX = 0xFFFF;
		m_regIY = 0xFFFF;

		m_iff1 = false;
		m_iff2 = false;

		m_interruptMode = InterruptMode::IM0;

		ClearFlags(m_regAlt.flags);
	}

	void CPUZ80::Init()
	{
		CPU8080::Init();

		// Jump Relative
		m_opcodes[0030] = [=]() { jumpRelIF(true, FetchByte()); };
		m_opcodes[0040] = [=]() { jumpRelIF(GetFlag(FLAG_Z) == false, FetchByte()); };  // JNZ
		m_opcodes[0050] = [=]() { jumpRelIF(GetFlag(FLAG_Z) == true, FetchByte()); };   // JZ
		m_opcodes[0060] = [=]() { jumpRelIF(GetFlag(FLAG_CY) == false, FetchByte()); }; // JNC
		m_opcodes[0070] = [=]() { jumpRelIF(GetFlag(FLAG_CY) == true, FetchByte()); };  // JC

		m_opcodes[0010] = [=]() { EXAF(); }; // 0x08

		// DJNZ rel8 - Decrement B, jump relative if non zero
		m_opcodes[0020] = [=]() { jumpRelIF(--m_reg.B, FetchByte()); }; // 0x10

		m_opcodes[0331] = [=]() { EXX(); };  // 0xD9

		// Extended instructions
		m_opcodes[0313] = [=]() { BITS(FetchByte()); }; // 0xCB
		m_opcodes[0335] = [=]() { IX(FetchByte()); }; // 0xDD
		m_opcodes[0355] = [=]() { EXTD(FetchByte()); }; // 0xED
		m_opcodes[0375] = [=]() { IY(FetchByte()); }; // 0xFD
	
		InitBITS(); // Bit instructions (Prefix 0xCB)
		InitEXTD();	// Extended instructions (Prefix 0xED)
		InitIX();   // IX instructions (Prefix 0xDD)
		InitIY();   // IY instructions (Prefix 0xFD)
	}

	void CPUZ80::InitBITS()
	{
		m_opcodesBITS.resize(256);
		std::fill(m_opcodesBITS.begin(), m_opcodesBITS.end(), [=]() { NotImplemented("BITS"); });

		// Rotate Left
		m_opcodesBITS[0x10] = [=]() { RL(m_reg.B); };
		m_opcodesBITS[0x11] = [=]() { RL(m_reg.C); };
		m_opcodesBITS[0x12] = [=]() { RL(m_reg.D); };
		m_opcodesBITS[0x13] = [=]() { RL(m_reg.E); };
		m_opcodesBITS[0x14] = [=]() { RL(m_reg.H); };
		m_opcodesBITS[0x15] = [=]() { RL(m_reg.L); };
		//m_opcodesBITS[0x16] = [=]() { MEMop(&CPUZ80::RL); };
		m_opcodesBITS[0x17] = [=]() { RL(m_reg.A); };

		// BIT: Test bit n and set Z flag
		m_opcodesBITS[0x40] = [=]() { BITget(0, m_reg.B); };
		m_opcodesBITS[0x41] = [=]() { BITget(0, m_reg.C); };
		m_opcodesBITS[0x42] = [=]() { BITget(0, m_reg.D); };
		m_opcodesBITS[0x43] = [=]() { BITget(0, m_reg.E); };
		m_opcodesBITS[0x44] = [=]() { BITget(0, m_reg.H); };
		m_opcodesBITS[0x45] = [=]() { BITget(0, m_reg.L); };
		m_opcodesBITS[0x46] = [=]() { BITget(0, ReadMem()); };
		m_opcodesBITS[0x47] = [=]() { BITget(0, m_reg.A); };

		m_opcodesBITS[0x48] = [=]() { BITget(1, m_reg.B); };
		m_opcodesBITS[0x49] = [=]() { BITget(1, m_reg.C); };
		m_opcodesBITS[0x4A] = [=]() { BITget(1, m_reg.D); };
		m_opcodesBITS[0x4B] = [=]() { BITget(1, m_reg.E); };
		m_opcodesBITS[0x4C] = [=]() { BITget(1, m_reg.H); };
		m_opcodesBITS[0x4D] = [=]() { BITget(1, m_reg.L); };
		m_opcodesBITS[0x4E] = [=]() { BITget(1, ReadMem()); };
		m_opcodesBITS[0x4F] = [=]() { BITget(1, m_reg.A); };

		m_opcodesBITS[0x50] = [=]() { BITget(2, m_reg.B); };
		m_opcodesBITS[0x51] = [=]() { BITget(2, m_reg.C); };
		m_opcodesBITS[0x52] = [=]() { BITget(2, m_reg.D); };
		m_opcodesBITS[0x53] = [=]() { BITget(2, m_reg.E); };
		m_opcodesBITS[0x54] = [=]() { BITget(2, m_reg.H); };
		m_opcodesBITS[0x55] = [=]() { BITget(2, m_reg.L); };
		m_opcodesBITS[0x56] = [=]() { BITget(2, ReadMem()); };
		m_opcodesBITS[0x57] = [=]() { BITget(2, m_reg.A); };

		m_opcodesBITS[0x58] = [=]() { BITget(3, m_reg.B); };
		m_opcodesBITS[0x59] = [=]() { BITget(3, m_reg.C); };
		m_opcodesBITS[0x5A] = [=]() { BITget(3, m_reg.D); };
		m_opcodesBITS[0x5B] = [=]() { BITget(3, m_reg.E); };
		m_opcodesBITS[0x5C] = [=]() { BITget(3, m_reg.H); };
		m_opcodesBITS[0x5D] = [=]() { BITget(3, m_reg.L); };
		m_opcodesBITS[0x5E] = [=]() { BITget(3, ReadMem()); };
		m_opcodesBITS[0x5F] = [=]() { BITget(3, m_reg.A); };

		m_opcodesBITS[0x60] = [=]() { BITget(4, m_reg.B); };
		m_opcodesBITS[0x61] = [=]() { BITget(4, m_reg.C); };
		m_opcodesBITS[0x62] = [=]() { BITget(4, m_reg.D); };
		m_opcodesBITS[0x63] = [=]() { BITget(4, m_reg.E); };
		m_opcodesBITS[0x64] = [=]() { BITget(4, m_reg.H); };
		m_opcodesBITS[0x65] = [=]() { BITget(4, m_reg.L); };
		m_opcodesBITS[0x66] = [=]() { BITget(4, ReadMem()); };
		m_opcodesBITS[0x67] = [=]() { BITget(4, m_reg.A); };

		m_opcodesBITS[0x68] = [=]() { BITget(5, m_reg.B); };
		m_opcodesBITS[0x69] = [=]() { BITget(5, m_reg.C); };
		m_opcodesBITS[0x6A] = [=]() { BITget(5, m_reg.D); };
		m_opcodesBITS[0x6B] = [=]() { BITget(5, m_reg.E); };
		m_opcodesBITS[0x6C] = [=]() { BITget(5, m_reg.H); };
		m_opcodesBITS[0x6D] = [=]() { BITget(5, m_reg.L); };
		m_opcodesBITS[0x6E] = [=]() { BITget(5, ReadMem()); };
		m_opcodesBITS[0x6F] = [=]() { BITget(5, m_reg.A); };

		m_opcodesBITS[0x70] = [=]() { BITget(6, m_reg.B); };
		m_opcodesBITS[0x71] = [=]() { BITget(6, m_reg.C); };
		m_opcodesBITS[0x72] = [=]() { BITget(6, m_reg.D); };
		m_opcodesBITS[0x73] = [=]() { BITget(6, m_reg.E); };
		m_opcodesBITS[0x74] = [=]() { BITget(6, m_reg.H); };
		m_opcodesBITS[0x75] = [=]() { BITget(6, m_reg.L); };
		m_opcodesBITS[0x76] = [=]() { BITget(6, ReadMem()); };
		m_opcodesBITS[0x77] = [=]() { BITget(6, m_reg.A); };

		m_opcodesBITS[0x78] = [=]() { BITget(7, m_reg.B); };
		m_opcodesBITS[0x79] = [=]() { BITget(7, m_reg.C); };
		m_opcodesBITS[0x7A] = [=]() { BITget(7, m_reg.D); };
		m_opcodesBITS[0x7B] = [=]() { BITget(7, m_reg.E); };
		m_opcodesBITS[0x7C] = [=]() { BITget(7, m_reg.H); };
		m_opcodesBITS[0x7D] = [=]() { BITget(7, m_reg.L); };
		m_opcodesBITS[0x7E] = [=]() { BITget(7, ReadMem()); };
		m_opcodesBITS[0x7F] = [=]() { BITget(7, m_reg.A); };

		// SET: Set bit n
		m_opcodesBITS[0xC0] = [=]() { BITset(0, true, m_reg.B); };
		m_opcodesBITS[0xC1] = [=]() { BITset(0, true, m_reg.C); };
		m_opcodesBITS[0xC2] = [=]() { BITset(0, true, m_reg.D); };
		m_opcodesBITS[0xC3] = [=]() { BITset(0, true, m_reg.E); };
		m_opcodesBITS[0xC4] = [=]() { BITset(0, true, m_reg.H); };
		m_opcodesBITS[0xC5] = [=]() { BITset(0, true, m_reg.L); };
		m_opcodesBITS[0xC6] = [=]() { BITsetM(0, true); };
		m_opcodesBITS[0xC7] = [=]() { BITset(0, true, m_reg.A); };

		m_opcodesBITS[0xC8] = [=]() { BITset(1, true, m_reg.B); };
		m_opcodesBITS[0xC9] = [=]() { BITset(1, true, m_reg.C); };
		m_opcodesBITS[0xCA] = [=]() { BITset(1, true, m_reg.D); };
		m_opcodesBITS[0xCB] = [=]() { BITset(1, true, m_reg.E); };
		m_opcodesBITS[0xCC] = [=]() { BITset(1, true, m_reg.H); };
		m_opcodesBITS[0xCD] = [=]() { BITset(1, true, m_reg.L); };
		m_opcodesBITS[0xCE] = [=]() { BITsetM(1, true); };
		m_opcodesBITS[0xCF] = [=]() { BITset(1, true, m_reg.A); };

		m_opcodesBITS[0xD0] = [=]() { BITset(2, true, m_reg.B); };
		m_opcodesBITS[0xD1] = [=]() { BITset(2, true, m_reg.C); };
		m_opcodesBITS[0xD2] = [=]() { BITset(2, true, m_reg.D); };
		m_opcodesBITS[0xD3] = [=]() { BITset(2, true, m_reg.E); };
		m_opcodesBITS[0xD4] = [=]() { BITset(2, true, m_reg.H); };
		m_opcodesBITS[0xD5] = [=]() { BITset(2, true, m_reg.L); };
		m_opcodesBITS[0xD6] = [=]() { BITsetM(2, true); };
		m_opcodesBITS[0xD7] = [=]() { BITset(2, true, m_reg.A); };

		m_opcodesBITS[0xD8] = [=]() { BITset(3, true, m_reg.B); };
		m_opcodesBITS[0xD9] = [=]() { BITset(3, true, m_reg.C); };
		m_opcodesBITS[0xDA] = [=]() { BITset(3, true, m_reg.D); };
		m_opcodesBITS[0xDB] = [=]() { BITset(3, true, m_reg.E); };
		m_opcodesBITS[0xDC] = [=]() { BITset(3, true, m_reg.H); };
		m_opcodesBITS[0xDD] = [=]() { BITset(3, true, m_reg.L); };
		m_opcodesBITS[0xDE] = [=]() { BITsetM(3, true); };
		m_opcodesBITS[0xDF] = [=]() { BITset(3, true, m_reg.A); };

		m_opcodesBITS[0xE0] = [=]() { BITset(4, true, m_reg.B); };
		m_opcodesBITS[0xE1] = [=]() { BITset(4, true, m_reg.C); };
		m_opcodesBITS[0xE2] = [=]() { BITset(4, true, m_reg.D); };
		m_opcodesBITS[0xE3] = [=]() { BITset(4, true, m_reg.E); };
		m_opcodesBITS[0xE4] = [=]() { BITset(4, true, m_reg.H); };
		m_opcodesBITS[0xE5] = [=]() { BITset(4, true, m_reg.L); };
		m_opcodesBITS[0xE6] = [=]() { BITsetM(4, true); };
		m_opcodesBITS[0xE7] = [=]() { BITset(4, true, m_reg.A); };

		m_opcodesBITS[0xE8] = [=]() { BITset(5, true, m_reg.B); };
		m_opcodesBITS[0xE9] = [=]() { BITset(5, true, m_reg.C); };
		m_opcodesBITS[0xEA] = [=]() { BITset(5, true, m_reg.D); };
		m_opcodesBITS[0xEB] = [=]() { BITset(5, true, m_reg.E); };
		m_opcodesBITS[0xEC] = [=]() { BITset(5, true, m_reg.H); };
		m_opcodesBITS[0xED] = [=]() { BITset(5, true, m_reg.L); };
		m_opcodesBITS[0xEE] = [=]() { BITsetM(5, true); };
		m_opcodesBITS[0xEF] = [=]() { BITset(5, true, m_reg.A); };

		m_opcodesBITS[0xF0] = [=]() { BITset(6, true, m_reg.B); };
		m_opcodesBITS[0xF1] = [=]() { BITset(6, true, m_reg.C); };
		m_opcodesBITS[0xF2] = [=]() { BITset(6, true, m_reg.D); };
		m_opcodesBITS[0xF3] = [=]() { BITset(6, true, m_reg.E); };
		m_opcodesBITS[0xF4] = [=]() { BITset(6, true, m_reg.H); };
		m_opcodesBITS[0xF5] = [=]() { BITset(6, true, m_reg.L); };
		m_opcodesBITS[0xF6] = [=]() { BITsetM(6, true); };
		m_opcodesBITS[0xF7] = [=]() { BITset(6, true, m_reg.A); };

		m_opcodesBITS[0xF8] = [=]() { BITset(7, true, m_reg.B); };
		m_opcodesBITS[0xF9] = [=]() { BITset(7, true, m_reg.C); };
		m_opcodesBITS[0xFA] = [=]() { BITset(7, true, m_reg.D); };
		m_opcodesBITS[0xFB] = [=]() { BITset(7, true, m_reg.E); };
		m_opcodesBITS[0xFC] = [=]() { BITset(7, true, m_reg.H); };
		m_opcodesBITS[0xFD] = [=]() { BITset(7, true, m_reg.L); };
		m_opcodesBITS[0xFE] = [=]() { BITsetM(7, true); };
		m_opcodesBITS[0xFF] = [=]() { BITset(7, true, m_reg.A); };
	}

	void CPUZ80::InitEXTD()
	{
		// All empty opcodes are NOPs
		m_opcodesEXTD.resize(256);
		std::fill(m_opcodesEXTD.begin(), m_opcodesEXTD.end(), [=]() { NOP(); });

		// LD (i16), r16
		m_opcodesEXTD[0x43] = [=]() { m_memory.Write16(FetchWord(), GetBC()); };
		m_opcodesEXTD[0x53] = [=]() { m_memory.Write16(FetchWord(), GetDE()); };
		m_opcodesEXTD[0x63] = [=]() { m_memory.Write16(FetchWord(), GetHL()); };
		m_opcodesEXTD[0x73] = [=]() { m_memory.Write16(FetchWord(), m_regSP); };

		// LD r16, (i16)
		m_opcodesEXTD[0x4B] = [=]() { SetBC(m_memory.Read16(FetchWord())); };
		m_opcodesEXTD[0x5B] = [=]() { SetDE(m_memory.Read16(FetchWord())); };
		m_opcodesEXTD[0x6B] = [=]() { SetHL(m_memory.Read16(FetchWord())); };
		m_opcodesEXTD[0x7B] = [=]() { m_regSP = m_memory.Read16(FetchWord()); };

		// LD A, R|I
		m_opcodesEXTD[0x57] = [=]() { m_reg.A = m_regI; };
		m_opcodesEXTD[0x5F] = [=]() { m_reg.A = m_regR; };

		// LD R|I, A
		m_opcodesEXTD[0x47] = [=]() { m_regI = m_reg.A; };
		m_opcodesEXTD[0x4F] = [=]() { m_regR = m_reg.A; };


		// Interrupt Mode
		m_opcodesEXTD[0x46] = [=]() { m_interruptMode = InterruptMode::IM0; };
		m_opcodesEXTD[0x4E] = [=]() { m_interruptMode = InterruptMode::IM0; }; // Undocumented
		m_opcodesEXTD[0x56] = [=]() { m_interruptMode = InterruptMode::IM1; };
		m_opcodesEXTD[0x5E] = [=]() { m_interruptMode = InterruptMode::IM2; };
		m_opcodesEXTD[0x66] = [=]() { m_interruptMode = InterruptMode::IM0; };
		m_opcodesEXTD[0x6E] = [=]() { m_interruptMode = InterruptMode::IM0; }; // Undocumented
		m_opcodesEXTD[0x76] = [=]() { m_interruptMode = InterruptMode::IM1; };
		m_opcodesEXTD[0x7E] = [=]() { m_interruptMode = InterruptMode::IM2; };

		// SBC HL, (BC|DE|HL|SP)
		m_opcodesEXTD[0x42] = [=]() { sbcHL(GetBC()); }; // BC
		m_opcodesEXTD[0x52] = [=]() { sbcHL(GetDE()); }; // DE
		m_opcodesEXTD[0x62] = [=]() { sbcHL(GetHL()); }; // HL
		m_opcodesEXTD[0x72] = [=]() { sbcHL(m_regSP); }; // SP

		// ADC HL, (BC|DE|HL|SP)
		m_opcodesEXTD[0x4A] = [=]() { adcHL(GetBC()); }; // BC
		m_opcodesEXTD[0x5A] = [=]() { adcHL(GetDE()); }; // DE
		m_opcodesEXTD[0x6A] = [=]() { adcHL(GetHL()); }; // HL
		m_opcodesEXTD[0x7A] = [=]() { adcHL(m_regSP); }; // SP

		// TODO
		m_opcodesEXTD[0x40] = [=]() { NotImplemented("EXTD[0x40]"); };
		m_opcodesEXTD[0x41] = [=]() { NotImplemented("EXTD[0x41]"); };
		m_opcodesEXTD[0x44] = [=]() { NotImplemented("EXTD[0x44]"); };
		m_opcodesEXTD[0x45] = [=]() { NotImplemented("EXTD[0x45]"); };

		m_opcodesEXTD[0x48] = [=]() { NotImplemented("EXTD[0x48]"); };
		m_opcodesEXTD[0x49] = [=]() { NotImplemented("EXTD[0x49]"); };
		m_opcodesEXTD[0x4C] = [=]() { NotImplemented("EXTD[0x4C]"); };
		m_opcodesEXTD[0x4D] = [=]() { NotImplemented("EXTD[0x4D]"); };

		m_opcodesEXTD[0x50] = [=]() { NotImplemented("EXTD[0x50]"); };
		m_opcodesEXTD[0x51] = [=]() { NotImplemented("EXTD[0x51]"); };
		m_opcodesEXTD[0x54] = [=]() { NotImplemented("EXTD[0x54]"); };
		m_opcodesEXTD[0x55] = [=]() { NotImplemented("EXTD[0x55]"); };

		m_opcodesEXTD[0x58] = [=]() { NotImplemented("EXTD[0x58]"); };
		m_opcodesEXTD[0x59] = [=]() { NotImplemented("EXTD[0x59]"); };
		m_opcodesEXTD[0x5C] = [=]() { NotImplemented("EXTD[0x5C]"); };
		m_opcodesEXTD[0x5D] = [=]() { NotImplemented("EXTD[0x5D]"); };

		m_opcodesEXTD[0x60] = [=]() { NotImplemented("EXTD[0x60]"); };
		m_opcodesEXTD[0x61] = [=]() { NotImplemented("EXTD[0x61]"); };
		m_opcodesEXTD[0x64] = [=]() { NotImplemented("EXTD[0x64]"); };
		m_opcodesEXTD[0x65] = [=]() { NotImplemented("EXTD[0x65]"); };
		m_opcodesEXTD[0x67] = [=]() { NotImplemented("EXTD[0x67]"); };

		m_opcodesEXTD[0x68] = [=]() { NotImplemented("EXTD[0x68]"); };
		m_opcodesEXTD[0x69] = [=]() { NotImplemented("EXTD[0x69]"); };
		m_opcodesEXTD[0x6C] = [=]() { NotImplemented("EXTD[0x6C]"); };
		m_opcodesEXTD[0x6D] = [=]() { NotImplemented("EXTD[0x6D]"); };
		m_opcodesEXTD[0x6F] = [=]() { NotImplemented("EXTD[0x6F]"); };

		m_opcodesEXTD[0x70] = [=]() { NotImplemented("EXTD[0x70]"); };
		m_opcodesEXTD[0x71] = [=]() { NotImplemented("EXTD[0x71]"); };
		m_opcodesEXTD[0x74] = [=]() { NotImplemented("EXTD[0x74]"); };
		m_opcodesEXTD[0x75] = [=]() { NotImplemented("EXTD[0x75]"); };

		m_opcodesEXTD[0x78] = [=]() { NotImplemented("EXTD[0x78]"); };
		m_opcodesEXTD[0x79] = [=]() { NotImplemented("EXTD[0x79]"); };
		m_opcodesEXTD[0x7C] = [=]() { NotImplemented("EXTD[0x7C]"); };
		m_opcodesEXTD[0x7D] = [=]() { NotImplemented("EXTD[0x7D]"); };

		m_opcodesEXTD[0xA0] = [=]() { NotImplemented("EXTD[0xA0]"); };
		m_opcodesEXTD[0xA1] = [=]() { NotImplemented("EXTD[0xA1]"); };
		m_opcodesEXTD[0xA2] = [=]() { NotImplemented("EXTD[0xA2]"); };
		m_opcodesEXTD[0xA3] = [=]() { NotImplemented("EXTD[0xA3]"); };

		m_opcodesEXTD[0xA8] = [=]() { NotImplemented("EXTD[0xA8]"); };
		m_opcodesEXTD[0xA9] = [=]() { NotImplemented("EXTD[0xA9]"); };
		m_opcodesEXTD[0xAA] = [=]() { NotImplemented("EXTD[0xAA]"); };
		m_opcodesEXTD[0xAB] = [=]() { NotImplemented("EXTD[0xAB]"); };

		m_opcodesEXTD[0xB0] = [=]() { NotImplemented("EXTD[0xB0]"); };
		m_opcodesEXTD[0xB1] = [=]() { NotImplemented("EXTD[0xB1]"); };
		m_opcodesEXTD[0xB2] = [=]() { NotImplemented("EXTD[0xB2]"); };
		m_opcodesEXTD[0xB3] = [=]() { NotImplemented("EXTD[0xB3]"); };

		m_opcodesEXTD[0xB8] = [=]() { NotImplemented("EXTD[0xB8]"); };
		m_opcodesEXTD[0xB9] = [=]() { NotImplemented("EXTD[0xB9]"); };
		m_opcodesEXTD[0xBA] = [=]() { NotImplemented("EXTD[0xBA]"); };
		m_opcodesEXTD[0xBB] = [=]() { NotImplemented("EXTD[0xBB]"); };
	}

	void CPUZ80::InitIX()
	{
		m_opcodesIX.resize(256);
		std::fill(m_opcodesIX.begin(), m_opcodesIX.end(), [=]() { NotImplemented("IX"); });
	}

	void CPUZ80::InitIY()
	{
		m_opcodesIY.resize(256);
		std::fill(m_opcodesIY.begin(), m_opcodesIY.end(), [=]() { NotImplemented("IY"); });

		// LOAD
		// ----------

		// LD IY, i16
		m_opcodesIY[0x21] = [=]() { m_regIY = FetchWord(); }; 

		// LD (i16), IY
		m_opcodesIY[0x22] = [=]() { m_memory.Write16(FetchWord(), m_regIY); };

		// LD IY(h), i8 (Undocumented)
		m_opcodesIY[0x26] = [=]() { SetHByte(m_regIY, FetchByte()); };

		// LD IY, (i16)
		m_opcodesIY[0x2A] = [=]() { m_regIY = m_memory.Read16(FetchWord()); };

		// LD IY(l), i8 (Undocumented)
		m_opcodesIY[0x2E] = [=]() { SetLByte(m_regIY, FetchByte()); };

		// LD (IY+i8), i8 
		m_opcodesIY[0x36] = [=]() { loadImm8toIdx(m_regIY); };

		// LD r, IY(h) (Undocumented)
		m_opcodesIY[0x44] = [=]() { m_reg.B = GetHByte(m_regIY); }; 
		m_opcodesIY[0x4C] = [=]() { m_reg.C = GetHByte(m_regIY); };
		m_opcodesIY[0x54] = [=]() { m_reg.D = GetHByte(m_regIY); };
		m_opcodesIY[0x5C] = [=]() { m_reg.E = GetHByte(m_regIY); };
		m_opcodesIY[0x64] = [=]() { SetHByte(m_regIY, GetHByte(m_regIY)); }; // NOP+NOP equivalent
		m_opcodesIY[0x6C] = [=]() { SetHByte(m_regIY, GetLByte(m_regIY)); };
		m_opcodesIY[0x7C] = [=]() { m_reg.A = GetHByte(m_regIY); };

		// LD r, IY(l) (Undocumented)
		m_opcodesIY[0x45] = [=]() { m_reg.B = GetLByte(m_regIY); };
		m_opcodesIY[0x4D] = [=]() { m_reg.C = GetLByte(m_regIY); };
		m_opcodesIY[0x55] = [=]() { m_reg.D = GetLByte(m_regIY); };
		m_opcodesIY[0x5D] = [=]() { m_reg.E = GetLByte(m_regIY); };
		m_opcodesIY[0x65] = [=]() { SetLByte(m_regIY, GetHByte(m_regIY)); }; 
		m_opcodesIY[0x6D] = [=]() { SetLByte(m_regIY, GetLByte(m_regIY)); }; // NOP+NOP equivalent
		m_opcodesIY[0x7D] = [=]() { m_reg.A = GetLByte(m_regIY); };

		// ADD|ADC|SUB|SBC|AND|XOR|OR|CP A, IY(h|l) (Undocumented)
		m_opcodesIY[0x84] = [=]() { add(GetHByte(m_regIY)); }; // ADD IY(h)
		m_opcodesIY[0x85] = [=]() { add(GetLByte(m_regIY)); }; // ADD IY(l)

		m_opcodesIY[0x8C] = [=]() { add(GetHByte(m_regIY), GetFlag(FLAG_CY)); }; // ADC IY(h)
		m_opcodesIY[0x8D] = [=]() { add(GetLByte(m_regIY), GetFlag(FLAG_CY)); }; // ADC IY(l)

		m_opcodesIY[0x94] = [=]() { sub(GetHByte(m_regIY)); }; // SUB IY(h)
		m_opcodesIY[0x95] = [=]() { sub(GetLByte(m_regIY)); }; // SUB IY(l)

		m_opcodesIY[0x9C] = [=]() { sub(GetHByte(m_regIY), GetFlag(FLAG_CY)); }; // SBC IY(h)
		m_opcodesIY[0x9D] = [=]() { sub(GetLByte(m_regIY), GetFlag(FLAG_CY)); }; // SBC IY(l)

		m_opcodesIY[0xA4] = [=]() { ana(GetHByte(m_regIY)); }; // AND IY(h)
		m_opcodesIY[0xA5] = [=]() { ana(GetLByte(m_regIY)); }; // AND IY(l)

		m_opcodesIY[0xAC] = [=]() { xra(GetHByte(m_regIY)); }; // XOR IY(h)
		m_opcodesIY[0xAD] = [=]() { xra(GetLByte(m_regIY)); }; // XOR IY(l)

		m_opcodesIY[0xB4] = [=]() { ora(GetHByte(m_regIY)); }; // OR IY(h)
		m_opcodesIY[0xB5] = [=]() { ora(GetLByte(m_regIY)); }; // OR IY(l)

		m_opcodesIY[0xBC] = [=]() { cmp(GetHByte(m_regIY)); }; // CMP IY(h)
		m_opcodesIY[0xBD] = [=]() { cmp(GetLByte(m_regIY)); }; // CMP IY(l)
 
		// ADD|ADC|SUB|SBC|AND|XOR|OR|CP A, (IY+i8)
		m_opcodesIY[0x86] = [=]() { add(ReadMemIdx(m_regIY)); }; // ADD
		m_opcodesIY[0x8E] = [=]() { add(ReadMemIdx(m_regIY), GetFlag(FLAG_CY)); }; // ADC

		m_opcodesIY[0x96] = [=]() { sub(ReadMemIdx(m_regIY)); }; // SUB
		m_opcodesIY[0x9E] = [=]() { sub(ReadMemIdx(m_regIY), GetFlag(FLAG_CY)); }; // SBC

		m_opcodesIY[0xA6] = [=]() { ana(ReadMemIdx(m_regIY)); }; // AND
		m_opcodesIY[0xAE] = [=]() { xra(ReadMemIdx(m_regIY)); }; // XOR

		m_opcodesIY[0xB6] = [=]() { ora(ReadMemIdx(m_regIY)); }; // OR
		m_opcodesIY[0xBE] = [=]() { cmp(ReadMemIdx(m_regIY)); }; // CMP
	}

	BYTE CPUZ80::ReadMemIdx(WORD base)
	{
		base += FetchByte(); // TODO: sign extension?
		return m_memory.Read8(base);
	}

	void CPUZ80::MEMop(std::function<void(CPUZ80*, BYTE& dest)> func)
	{
		BYTE temp = ReadMem();
		func(this, temp);
		WriteMem(temp);
	}

	void CPUZ80::loadImm8toIdx(WORD base)
	{
		base += FetchByte(); // TODO: sign extension?
		BYTE imm = FetchByte();
		m_memory.Write8(base, imm);
	}

	void CPUZ80::jumpRelIF(bool condition, BYTE offset)
	{
		if (condition == true)
		{
			WORD address = (WORD)m_programCounter + Widen(offset);
			m_programCounter = address;
			TICKT3();
		}
		else
		{
			int a = 1;
		}
	}

	void CPUZ80::exec(OpcodeTable& table, BYTE opcode)
	{
		try
		{
			// Fetch the function corresponding to the opcode and run it
			{
				auto& opFunc = table[opcode];
				opFunc();
			}
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "CPU: Exception at address 0x%04X! Stopping CPU", m_programCounter);
			m_state = CPUState::STOP;
		}
	}

	void CPUZ80::AdjustBaseFlags(BYTE val)
	{
		SetFlag(FLAG_Z, (val == 0));
		SetFlag(FLAG_F3, GetBit(val, 3));
		SetFlag(FLAG_F5, GetBit(val, 5));	
		SetFlag(FLAG_S, GetBit(val, 7));
		SetFlag(FLAG_PV, IsParityEven(val));
		SetFlag(FLAG_N, false);
	}

	void CPUZ80::AdjustBaseFlags(WORD val)
	{
		SetFlag(FLAG_Z, (val == 0));
		SetFlag(FLAG_F3, GetBit(val, 3));
		SetFlag(FLAG_F5, GetBit(val, 5));
		SetFlag(FLAG_S, GetBit(val, 15));
		SetFlag(FLAG_N, false);
	}

	void CPUZ80::NotImplemented(const char* opStr)
	{
		LogPrintf(LOG_ERROR, "[%s] not implemented @ 0x%04X", opStr, m_programCounter);
		throw std::exception("Not implemented");
	}

	void CPUZ80::adcHL(WORD src)
	{
		DWORD temp = GetHL() + src;
		if (GetFlag(FLAG_CY))
		{
			temp++;
		}

		SetHL(temp);

		AdjustBaseFlags((WORD)temp);
		SetFlag(FLAG_CY, (temp > 0xFFFF));
		SetFlag(FLAG_PV, false); // TODO: Overflow 
	}

	void CPUZ80::sbcHL(WORD src)
	{
		int temp = GetHL() - src;
		if (GetFlag(FLAG_CY))
		{
			temp--;
		}

		SetHL(temp);

		AdjustBaseFlags((WORD)temp);
		SetFlag(FLAG_CY, (temp < 0));	
		SetFlag(FLAG_PV, false); // TODO: Overflow 
		SetFlag(FLAG_N, true);
	}

	void CPUZ80::EXAF()
	{
		NotImplemented("EXAF");
	}

	void CPUZ80::EXX()
	{
		Swap(m_reg.B, m_regAlt.B);
		Swap(m_reg.C, m_regAlt.C);
		Swap(m_reg.D, m_regAlt.D);
		Swap(m_reg.E, m_regAlt.E);
		Swap(m_reg.H, m_regAlt.H);
		Swap(m_reg.L, m_regAlt.L);
	}

	void CPUZ80::BITS(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP1, op2);
		exec(m_opcodesBITS, op2);
	}

	void CPUZ80::EXTD(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP2, op2);
		exec(m_opcodesEXTD, op2);
	}

	void CPUZ80::IX(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP3, op2);
		exec(m_opcodesIX, op2);
	}

	void CPUZ80::IY(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP4, op2);
		exec(m_opcodesIY, op2);
	}

	void CPUZ80::RL(BYTE& dest)
	{
		bool msb = GetBit(dest, 7);

		dest = (dest << 1);
		dest |= (msb ? 1 : 0);

		AdjustBaseFlags(dest);
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, msb);
	}

	void CPUZ80::BITget(BYTE bit, BYTE src)
	{
		AdjustBaseFlags(src);
		SetFlag(FLAG_H, true);
		SetFlag(FLAG_Z, !GetBit(src, bit));
	}

	void CPUZ80::BITset(BYTE bit, bool set, BYTE& dest)
	{
		SetBit(dest, bit, set);
	}

	void CPUZ80::BITsetM(BYTE bit, bool set)
	{
		BYTE temp = ReadMem();
		BITset(bit, set, temp);
		WriteMem(temp);
	}
}
