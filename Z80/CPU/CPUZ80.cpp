#include "stdafx.h"
#include "CPUZ80.h"

using cpuInfo::Opcode;
using cpuInfo::CPUType;
using cpuInfo::MiscTiming;

namespace emul
{
	CPUZ80::CPUZ80(Memory& memory) : CPUZ80(CPUType::z80, memory)
	{
	}

	CPUZ80::CPUZ80(cpuInfo::CPUType type, Memory& memory) :
		Logger("Z80"),
		CPU8080(type, memory)
	{
		FLAG_RESERVED_ON = (FLAG)0;
		FLAG_RESERVED_OFF = (FLAG)0;
	}

	void CPUZ80::Exec(BYTE opcode)
	{
		CPU8080::Exec(opcode);
		REFRESH();
	}

	void CPUZ80::Halt()
	{
		CPU8080::Halt();
		REFRESH();
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

		m_regI = 0;
		m_regR = 0;

		m_interruptMode = InterruptMode::IM0;

		ClearFlags(m_regAlt.flags);
	}

	void CPUZ80::Interrupt()
	{
		// Process NMI
		if (m_nmiLatch.IsLatched())
		{
			if (m_state == CPUState::HALT)
			{
				++m_programCounter;
			}
			m_nmiLatch.ResetLatch();
			pushPC();
			m_iff1 = false;
			m_programCounter = 0x0066;
			REFRESH();
			TICKMISC(MiscTiming::TRAP);
			m_state = CPUState::RUN;
		}
		else if (m_iff1 && m_intLatch.IsLatched()) // Process INT
		{
			LogPrintf(LOG_WARNING, "INT");
			if (m_state == CPUState::HALT)
			{
				++m_programCounter;
			}

			// TODO: INT is level triggered (not edge)
			m_intLatch.ResetLatch();

			DI();
			pushPC();

			switch (m_interruptMode)
			{
			case InterruptMode::IM0:
				LogPrintf(LOG_ERROR, "InterruptMode::IM0 not implemented");
				throw std::exception("InterruptMode::IM0 not implemented");
				break;
			case InterruptMode::IM1:
				m_programCounter = 0x0038;
				break;
			case InterruptMode::IM2:
				LogPrintf(LOG_ERROR, "InterruptMode::IM2 not implemented");
				throw std::exception("InterruptMode::IM2 not implemented");
				break;
			}

			REFRESH();
			TICKMISC(MiscTiming::IRQ);
			m_state = CPUState::RUN;
		}	
	}

	void CPUZ80::Init()
	{
		CPU8080::Init();

		m_opcodes[0010] = [=]() { EXAF(); }; // 0x08

		// DJNZ rel8 - Decrement B, jump relative if non zero
		m_opcodes[0020] = [=]() { jumpRelIF(--m_reg.B, FetchByte()); }; // 0x10

		// Jump Relative
		m_opcodes[0030] = [=]() { jumpRelIF(true, FetchByte()); };
		m_opcodes[0040] = [=]() { jumpRelIF(GetFlag(FLAG_Z) == false, FetchByte()); };  // JNZ
		m_opcodes[0050] = [=]() { jumpRelIF(GetFlag(FLAG_Z) == true, FetchByte()); };   // JZ
		m_opcodes[0060] = [=]() { jumpRelIF(GetFlag(FLAG_CY) == false, FetchByte()); }; // JNC
		m_opcodes[0070] = [=]() { jumpRelIF(GetFlag(FLAG_CY) == true, FetchByte()); };  // JC

		m_opcodes[0331] = [=]() { EXX(); };  // 0xD9

		// Extended instructions
		m_opcodes[0313] = [=]() { BITS(FetchByte()); }; // 0xCB
		m_opcodes[0335] = [=]() { m_currIdx = &m_regIX; IXY(FetchByte()); }; // 0xDD
		m_opcodes[0355] = [=]() { EXTD(FetchByte()); }; // 0xED
		m_opcodes[0375] = [=]() { m_currIdx = &m_regIY; IXY(FetchByte()); }; // 0xFD

		// Replace IN/OUT Opcodes with Z80 version
		// On Z80, IN/OUT put reg A value in A8..A15
		m_opcodes[0333] = [=]() { In(MakeWord(m_reg.A, FetchByte()), m_reg.A); };
		m_opcodes[0323] = [=]() { Out(MakeWord(m_reg.A, FetchByte()), m_reg.A); };

		InitBITS(); // Bit instructions (Prefix 0xCB)
		InitBITSxy(); // Bit instructions, indexed (Prefix 0xDDCF, 0xFDCB)
		InitEXTD();	// Extended instructions (Prefix 0xED)
		InitIXY();   // IX/IY instructions (Prefix 0xDD, 0xFD)
	}

	void CPUZ80::InitBITS()
	{
		m_opcodesBITS.resize(256);
		std::fill(m_opcodesBITS.begin(), m_opcodesBITS.end(), [=]() { NotImplemented("BITS"); });

		// Rotate Left through carry
		m_opcodesBITS[0x00] = [=]() { RLC(m_reg.B); };
		m_opcodesBITS[0x01] = [=]() { RLC(m_reg.C); };
		m_opcodesBITS[0x02] = [=]() { RLC(m_reg.D); };
		m_opcodesBITS[0x03] = [=]() { RLC(m_reg.E); };
		m_opcodesBITS[0x04] = [=]() { RLC(m_reg.H); };
		m_opcodesBITS[0x05] = [=]() { RLC(m_reg.L); };
		//m_opcodesBITS[0x06] = [=]() { MEMop(&CPUZ80::RLC); };
		m_opcodesBITS[0x07] = [=]() { RLC(m_reg.A); };

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

	void CPUZ80::InitBITSxy()
	{
		m_opcodesBITSxy.resize(256);
		std::fill(m_opcodesBITSxy.begin(), m_opcodesBITSxy.end(), [=]() { NotImplemented("BITSxy"); });

		// Rotate Left
		//m_opcodesBITSxy[0x10] = [=]() { RL(m_reg.B); };
		//m_opcodesBITSxy[0x11] = [=]() { RL(m_reg.C); };
		//m_opcodesBITSxy[0x12] = [=]() { RL(m_reg.D); };
		//m_opcodesBITSxy[0x13] = [=]() { RL(m_reg.E); };
		//m_opcodesBITSxy[0x14] = [=]() { RL(m_reg.H); };
		//m_opcodesBITSxy[0x15] = [=]() { RL(m_reg.L); };
		////m_opcodesBITSxy[0x16] = [=]() { MEMop(&CPUZ80::RL); };
		//m_opcodesBITSxy[0x17] = [=]() { RL(m_reg.A); };

		// BIT: Test bit n of (IX+s8) and set Z flag
		m_opcodesBITSxy[0x40] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x41] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x42] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x43] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x44] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x45] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x46] = [=]() { BITgetIXY(0); };
		m_opcodesBITSxy[0x47] = [=]() { BITgetIXY(0); };

		m_opcodesBITSxy[0x48] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x49] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x4A] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x4B] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x4C] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x4D] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x4E] = [=]() { BITgetIXY(1); };
		m_opcodesBITSxy[0x4F] = [=]() { BITgetIXY(1); };

		m_opcodesBITSxy[0x50] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x51] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x52] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x53] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x54] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x55] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x56] = [=]() { BITgetIXY(2); };
		m_opcodesBITSxy[0x57] = [=]() { BITgetIXY(2); };

		m_opcodesBITSxy[0x58] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x59] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x5A] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x5B] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x5C] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x5D] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x5E] = [=]() { BITgetIXY(3); };
		m_opcodesBITSxy[0x5F] = [=]() { BITgetIXY(3); };

		m_opcodesBITSxy[0x60] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x61] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x62] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x63] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x64] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x65] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x66] = [=]() { BITgetIXY(4); };
		m_opcodesBITSxy[0x67] = [=]() { BITgetIXY(4); };

		m_opcodesBITSxy[0x68] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x69] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x6A] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x6B] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x6C] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x6D] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x6E] = [=]() { BITgetIXY(5); };
		m_opcodesBITSxy[0x6F] = [=]() { BITgetIXY(5); };

		m_opcodesBITSxy[0x70] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x71] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x72] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x73] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x74] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x75] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x76] = [=]() { BITgetIXY(6); };
		m_opcodesBITSxy[0x77] = [=]() { BITgetIXY(6); };

		m_opcodesBITSxy[0x78] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x79] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x7A] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x7B] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x7C] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x7D] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x7E] = [=]() { BITgetIXY(7); };
		m_opcodesBITSxy[0x7F] = [=]() { BITgetIXY(7); };

		// RES: Clear bit n of (IX+s8), optional working register
		m_opcodesBITSxy[0x80] = [=]() { BITsetIXY(0, false, m_reg.B); };
		m_opcodesBITSxy[0x81] = [=]() { BITsetIXY(0, false, m_reg.C); };
		m_opcodesBITSxy[0x82] = [=]() { BITsetIXY(0, false, m_reg.D); };
		m_opcodesBITSxy[0x83] = [=]() { BITsetIXY(0, false, m_reg.E); };
		m_opcodesBITSxy[0x84] = [=]() { BITsetIXY(0, false, m_reg.H); };
		m_opcodesBITSxy[0x85] = [=]() { BITsetIXY(0, false, m_reg.L); };
		m_opcodesBITSxy[0x86] = [=]() { BITsetIXY(0, false, m_regDummy); };
		m_opcodesBITSxy[0x87] = [=]() { BITsetIXY(0, false, m_reg.A); };

		m_opcodesBITSxy[0x88] = [=]() { BITsetIXY(1, false, m_reg.B); };
		m_opcodesBITSxy[0x89] = [=]() { BITsetIXY(1, false, m_reg.C); };
		m_opcodesBITSxy[0x8A] = [=]() { BITsetIXY(1, false, m_reg.D); };
		m_opcodesBITSxy[0x8B] = [=]() { BITsetIXY(1, false, m_reg.E); };
		m_opcodesBITSxy[0x8C] = [=]() { BITsetIXY(1, false, m_reg.H); };
		m_opcodesBITSxy[0x8D] = [=]() { BITsetIXY(1, false, m_reg.L); };
		m_opcodesBITSxy[0x8E] = [=]() { BITsetIXY(1, false, m_regDummy); };
		m_opcodesBITSxy[0x8F] = [=]() { BITsetIXY(1, false, m_reg.A); };

		m_opcodesBITSxy[0x90] = [=]() { BITsetIXY(2, false, m_reg.B); };
		m_opcodesBITSxy[0x91] = [=]() { BITsetIXY(2, false, m_reg.C); };
		m_opcodesBITSxy[0x92] = [=]() { BITsetIXY(2, false, m_reg.D); };
		m_opcodesBITSxy[0x93] = [=]() { BITsetIXY(2, false, m_reg.E); };
		m_opcodesBITSxy[0x94] = [=]() { BITsetIXY(2, false, m_reg.H); };
		m_opcodesBITSxy[0x95] = [=]() { BITsetIXY(2, false, m_reg.L); };
		m_opcodesBITSxy[0x96] = [=]() { BITsetIXY(2, false, m_regDummy); };
		m_opcodesBITSxy[0x97] = [=]() { BITsetIXY(2, false, m_reg.A); };

		m_opcodesBITSxy[0x98] = [=]() { BITsetIXY(3, false, m_reg.B); };
		m_opcodesBITSxy[0x99] = [=]() { BITsetIXY(3, false, m_reg.C); };
		m_opcodesBITSxy[0x9A] = [=]() { BITsetIXY(3, false, m_reg.D); };
		m_opcodesBITSxy[0x9B] = [=]() { BITsetIXY(3, false, m_reg.E); };
		m_opcodesBITSxy[0x9C] = [=]() { BITsetIXY(3, false, m_reg.H); };
		m_opcodesBITSxy[0x9D] = [=]() { BITsetIXY(3, false, m_reg.L); };
		m_opcodesBITSxy[0x9E] = [=]() { BITsetIXY(3, false, m_regDummy); };
		m_opcodesBITSxy[0x9F] = [=]() { BITsetIXY(3, false, m_reg.A); };

		m_opcodesBITSxy[0xA0] = [=]() { BITsetIXY(4, false, m_reg.B); };
		m_opcodesBITSxy[0xA1] = [=]() { BITsetIXY(4, false, m_reg.C); };
		m_opcodesBITSxy[0xA2] = [=]() { BITsetIXY(4, false, m_reg.D); };
		m_opcodesBITSxy[0xA3] = [=]() { BITsetIXY(4, false, m_reg.E); };
		m_opcodesBITSxy[0xA4] = [=]() { BITsetIXY(4, false, m_reg.H); };
		m_opcodesBITSxy[0xA5] = [=]() { BITsetIXY(4, false, m_reg.L); };
		m_opcodesBITSxy[0xA6] = [=]() { BITsetIXY(4, false, m_regDummy); };
		m_opcodesBITSxy[0xA7] = [=]() { BITsetIXY(4, false, m_reg.A); };

		m_opcodesBITSxy[0xA8] = [=]() { BITsetIXY(5, false, m_reg.B); };
		m_opcodesBITSxy[0xA9] = [=]() { BITsetIXY(5, false, m_reg.C); };
		m_opcodesBITSxy[0xAA] = [=]() { BITsetIXY(5, false, m_reg.D); };
		m_opcodesBITSxy[0xAB] = [=]() { BITsetIXY(5, false, m_reg.E); };
		m_opcodesBITSxy[0xAC] = [=]() { BITsetIXY(5, false, m_reg.H); };
		m_opcodesBITSxy[0xAD] = [=]() { BITsetIXY(5, false, m_reg.L); };
		m_opcodesBITSxy[0xAE] = [=]() { BITsetIXY(5, false, m_regDummy); };
		m_opcodesBITSxy[0xAF] = [=]() { BITsetIXY(5, false, m_reg.A); };

		m_opcodesBITSxy[0xB0] = [=]() { BITsetIXY(6, false, m_reg.B); };
		m_opcodesBITSxy[0xB1] = [=]() { BITsetIXY(6, false, m_reg.C); };
		m_opcodesBITSxy[0xB2] = [=]() { BITsetIXY(6, false, m_reg.D); };
		m_opcodesBITSxy[0xB3] = [=]() { BITsetIXY(6, false, m_reg.E); };
		m_opcodesBITSxy[0xB4] = [=]() { BITsetIXY(6, false, m_reg.H); };
		m_opcodesBITSxy[0xB5] = [=]() { BITsetIXY(6, false, m_reg.L); };
		m_opcodesBITSxy[0xB6] = [=]() { BITsetIXY(6, false, m_regDummy); };
		m_opcodesBITSxy[0xB7] = [=]() { BITsetIXY(6, false, m_reg.A); };

		m_opcodesBITSxy[0xB8] = [=]() { BITsetIXY(7, false, m_reg.B); };
		m_opcodesBITSxy[0xB9] = [=]() { BITsetIXY(7, false, m_reg.C); };
		m_opcodesBITSxy[0xBA] = [=]() { BITsetIXY(7, false, m_reg.D); };
		m_opcodesBITSxy[0xBB] = [=]() { BITsetIXY(7, false, m_reg.E); };
		m_opcodesBITSxy[0xBC] = [=]() { BITsetIXY(7, false, m_reg.H); };
		m_opcodesBITSxy[0xBD] = [=]() { BITsetIXY(7, false, m_reg.L); };
		m_opcodesBITSxy[0xBE] = [=]() { BITsetIXY(7, false, m_regDummy); };
		m_opcodesBITSxy[0xBF] = [=]() { BITsetIXY(7, false, m_reg.A); };

		// SET: Set bit n of (IX+s8), optional working register
		m_opcodesBITSxy[0xC0] = [=]() { BITsetIXY(0, true, m_reg.B); };
		m_opcodesBITSxy[0xC1] = [=]() { BITsetIXY(0, true, m_reg.C); };
		m_opcodesBITSxy[0xC2] = [=]() { BITsetIXY(0, true, m_reg.D); };
		m_opcodesBITSxy[0xC3] = [=]() { BITsetIXY(0, true, m_reg.E); };
		m_opcodesBITSxy[0xC4] = [=]() { BITsetIXY(0, true, m_reg.H); };
		m_opcodesBITSxy[0xC5] = [=]() { BITsetIXY(0, true, m_reg.L); };
		m_opcodesBITSxy[0xC6] = [=]() { BITsetIXY(0, true, m_regDummy); };
		m_opcodesBITSxy[0xC7] = [=]() { BITsetIXY(0, true, m_reg.A); };

		m_opcodesBITSxy[0xC8] = [=]() { BITsetIXY(1, true, m_reg.B); };
		m_opcodesBITSxy[0xC9] = [=]() { BITsetIXY(1, true, m_reg.C); };
		m_opcodesBITSxy[0xCA] = [=]() { BITsetIXY(1, true, m_reg.D); };
		m_opcodesBITSxy[0xCB] = [=]() { BITsetIXY(1, true, m_reg.E); };
		m_opcodesBITSxy[0xCC] = [=]() { BITsetIXY(1, true, m_reg.H); };
		m_opcodesBITSxy[0xCD] = [=]() { BITsetIXY(1, true, m_reg.L); };
		m_opcodesBITSxy[0xCE] = [=]() { BITsetIXY(1, true, m_regDummy); };
		m_opcodesBITSxy[0xCF] = [=]() { BITsetIXY(1, true, m_reg.A); };

		m_opcodesBITSxy[0xD0] = [=]() { BITsetIXY(2, true, m_reg.B); };
		m_opcodesBITSxy[0xD1] = [=]() { BITsetIXY(2, true, m_reg.C); };
		m_opcodesBITSxy[0xD2] = [=]() { BITsetIXY(2, true, m_reg.D); };
		m_opcodesBITSxy[0xD3] = [=]() { BITsetIXY(2, true, m_reg.E); };
		m_opcodesBITSxy[0xD4] = [=]() { BITsetIXY(2, true, m_reg.H); };
		m_opcodesBITSxy[0xD5] = [=]() { BITsetIXY(2, true, m_reg.L); };
		m_opcodesBITSxy[0xD6] = [=]() { BITsetIXY(2, true, m_regDummy); };
		m_opcodesBITSxy[0xD7] = [=]() { BITsetIXY(2, true, m_reg.A); };

		m_opcodesBITSxy[0xD8] = [=]() { BITsetIXY(3, true, m_reg.B); };
		m_opcodesBITSxy[0xD9] = [=]() { BITsetIXY(3, true, m_reg.C); };
		m_opcodesBITSxy[0xDA] = [=]() { BITsetIXY(3, true, m_reg.D); };
		m_opcodesBITSxy[0xDB] = [=]() { BITsetIXY(3, true, m_reg.E); };
		m_opcodesBITSxy[0xDC] = [=]() { BITsetIXY(3, true, m_reg.H); };
		m_opcodesBITSxy[0xDD] = [=]() { BITsetIXY(3, true, m_reg.L); };
		m_opcodesBITSxy[0xDE] = [=]() { BITsetIXY(3, true, m_regDummy); };
		m_opcodesBITSxy[0xDF] = [=]() { BITsetIXY(3, true, m_reg.A); };

		m_opcodesBITSxy[0xE0] = [=]() { BITsetIXY(4, true, m_reg.B); };
		m_opcodesBITSxy[0xE1] = [=]() { BITsetIXY(4, true, m_reg.C); };
		m_opcodesBITSxy[0xE2] = [=]() { BITsetIXY(4, true, m_reg.D); };
		m_opcodesBITSxy[0xE3] = [=]() { BITsetIXY(4, true, m_reg.E); };
		m_opcodesBITSxy[0xE4] = [=]() { BITsetIXY(4, true, m_reg.H); };
		m_opcodesBITSxy[0xE5] = [=]() { BITsetIXY(4, true, m_reg.L); };
		m_opcodesBITSxy[0xE6] = [=]() { BITsetIXY(4, true, m_regDummy); };
		m_opcodesBITSxy[0xE7] = [=]() { BITsetIXY(4, true, m_reg.A); };

		m_opcodesBITSxy[0xE8] = [=]() { BITsetIXY(5, true, m_reg.B); };
		m_opcodesBITSxy[0xE9] = [=]() { BITsetIXY(5, true, m_reg.C); };
		m_opcodesBITSxy[0xEA] = [=]() { BITsetIXY(5, true, m_reg.D); };
		m_opcodesBITSxy[0xEB] = [=]() { BITsetIXY(5, true, m_reg.E); };
		m_opcodesBITSxy[0xEC] = [=]() { BITsetIXY(5, true, m_reg.H); };
		m_opcodesBITSxy[0xED] = [=]() { BITsetIXY(5, true, m_reg.L); };
		m_opcodesBITSxy[0xEE] = [=]() { BITsetIXY(5, true, m_regDummy); };
		m_opcodesBITSxy[0xEF] = [=]() { BITsetIXY(5, true, m_reg.A); };

		m_opcodesBITSxy[0xF0] = [=]() { BITsetIXY(6, true, m_reg.B); };
		m_opcodesBITSxy[0xF1] = [=]() { BITsetIXY(6, true, m_reg.C); };
		m_opcodesBITSxy[0xF2] = [=]() { BITsetIXY(6, true, m_reg.D); };
		m_opcodesBITSxy[0xF3] = [=]() { BITsetIXY(6, true, m_reg.E); };
		m_opcodesBITSxy[0xF4] = [=]() { BITsetIXY(6, true, m_reg.H); };
		m_opcodesBITSxy[0xF5] = [=]() { BITsetIXY(6, true, m_reg.L); };
		m_opcodesBITSxy[0xF6] = [=]() { BITsetIXY(6, true, m_regDummy); };
		m_opcodesBITSxy[0xF7] = [=]() { BITsetIXY(6, true, m_reg.A); };

		m_opcodesBITSxy[0xF8] = [=]() { BITsetIXY(7, true, m_reg.B); };
		m_opcodesBITSxy[0xF9] = [=]() { BITsetIXY(7, true, m_reg.C); };
		m_opcodesBITSxy[0xFA] = [=]() { BITsetIXY(7, true, m_reg.D); };
		m_opcodesBITSxy[0xFB] = [=]() { BITsetIXY(7, true, m_reg.E); };
		m_opcodesBITSxy[0xFC] = [=]() { BITsetIXY(7, true, m_reg.H); };
		m_opcodesBITSxy[0xFD] = [=]() { BITsetIXY(7, true, m_reg.L); };
		m_opcodesBITSxy[0xFE] = [=]() { BITsetIXY(7, true, m_regDummy); };
		m_opcodesBITSxy[0xFF] = [=]() { BITsetIXY(7, true, m_reg.A); };
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
		m_opcodesEXTD[0x57] = [=]() { LDAri(m_regI); };
		m_opcodesEXTD[0x5F] = [=]() { LDAri(m_regR); };

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

		// IN r, (C)
		m_opcodesEXTD[0x40] = [=]() { INc(m_reg.B); };
		m_opcodesEXTD[0x48] = [=]() { INc(m_reg.C); };
		m_opcodesEXTD[0x50] = [=]() { INc(m_reg.D); };
		m_opcodesEXTD[0x58] = [=]() { INc(m_reg.E); };
		m_opcodesEXTD[0x60] = [=]() { INc(m_reg.H); };
		m_opcodesEXTD[0x68] = [=]() { INc(m_reg.L); };
		m_opcodesEXTD[0x70] = [=]() { INc(m_regDummy); };
		m_opcodesEXTD[0x78] = [=]() { INc(m_reg.A); };

		// TODO
		m_opcodesEXTD[0x41] = [=]() { NotImplemented("EXTD[0x41]"); };
		m_opcodesEXTD[0x44] = [=]() { NotImplemented("EXTD[0x44]"); };
		m_opcodesEXTD[0x45] = [=]() { NotImplemented("EXTD[0x45]"); };

		m_opcodesEXTD[0x49] = [=]() { NotImplemented("EXTD[0x49]"); };
		m_opcodesEXTD[0x4C] = [=]() { NotImplemented("EXTD[0x4C]"); };
		m_opcodesEXTD[0x4D] = [=]() { NotImplemented("EXTD[0x4D]"); };

		m_opcodesEXTD[0x51] = [=]() { NotImplemented("EXTD[0x51]"); };
		m_opcodesEXTD[0x54] = [=]() { NotImplemented("EXTD[0x54]"); };
		m_opcodesEXTD[0x55] = [=]() { NotImplemented("EXTD[0x55]"); };

		m_opcodesEXTD[0x59] = [=]() { NotImplemented("EXTD[0x59]"); };
		m_opcodesEXTD[0x5C] = [=]() { NotImplemented("EXTD[0x5C]"); };
		m_opcodesEXTD[0x5D] = [=]() { NotImplemented("EXTD[0x5D]"); };

		m_opcodesEXTD[0x61] = [=]() { NotImplemented("EXTD[0x61]"); };
		m_opcodesEXTD[0x64] = [=]() { NotImplemented("EXTD[0x64]"); };
		m_opcodesEXTD[0x65] = [=]() { NotImplemented("EXTD[0x65]"); };
		m_opcodesEXTD[0x67] = [=]() { NotImplemented("EXTD[0x67]"); };

		m_opcodesEXTD[0x69] = [=]() { NotImplemented("EXTD[0x69]"); };
		m_opcodesEXTD[0x6C] = [=]() { NotImplemented("EXTD[0x6C]"); };
		m_opcodesEXTD[0x6D] = [=]() { NotImplemented("EXTD[0x6D]"); };
		m_opcodesEXTD[0x6F] = [=]() { NotImplemented("EXTD[0x6F]"); };

		m_opcodesEXTD[0x71] = [=]() { NotImplemented("EXTD[0x71]"); };
		m_opcodesEXTD[0x74] = [=]() { NotImplemented("EXTD[0x74]"); };
		m_opcodesEXTD[0x75] = [=]() { NotImplemented("EXTD[0x75]"); };

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

	void CPUZ80::InitIXY()
	{
		m_opcodesIXY.resize(256);
		std::fill(m_opcodesIXY.begin(), m_opcodesIXY.end(), [=]() { NotImplemented("IXY"); });

		// LOAD
		// ----------

		// LD IXY, i16
		m_opcodesIXY[0x21] = [=]() { *m_currIdx = FetchWord(); };

		// LD (i16), IXY
		m_opcodesIXY[0x22] = [=]() { m_memory.Write16(FetchWord(), *m_currIdx); };

		// LD IXY(h), i8 (Undocumented)
		m_opcodesIXY[0x26] = [=]() { SetHByte(*m_currIdx, FetchByte()); };

		// LD IXY, (i16)
		m_opcodesIXY[0x2A] = [=]() { *m_currIdx = m_memory.Read16(FetchWord()); };

		// LD IXY(l), i8 (Undocumented)
		m_opcodesIXY[0x2E] = [=]() { SetLByte(*m_currIdx, FetchByte()); };

		// LD (IXY+s8), i8 
		m_opcodesIXY[0x36] = [=]() { loadImm8toIdx(*m_currIdx); };


		// LD r, IXY(h) (Undocumented)
		m_opcodesIXY[0x44] = [=]() { m_reg.B = GetHByte(*m_currIdx); };
		m_opcodesIXY[0x4C] = [=]() { m_reg.C = GetHByte(*m_currIdx); };
		m_opcodesIXY[0x54] = [=]() { m_reg.D = GetHByte(*m_currIdx); };
		m_opcodesIXY[0x5C] = [=]() { m_reg.E = GetHByte(*m_currIdx); };
		m_opcodesIXY[0x64] = [=]() { SetHByte(*m_currIdx, GetHByte(*m_currIdx)); }; // NOP+NOP equivalent
		m_opcodesIXY[0x6C] = [=]() { SetHByte(*m_currIdx, GetLByte(*m_currIdx)); };
		m_opcodesIXY[0x7C] = [=]() { m_reg.A = GetHByte(*m_currIdx); };

		// LD r, IXY(l) (Undocumented)
		m_opcodesIXY[0x45] = [=]() { m_reg.B = GetLByte(*m_currIdx); };
		m_opcodesIXY[0x4D] = [=]() { m_reg.C = GetLByte(*m_currIdx); };
		m_opcodesIXY[0x55] = [=]() { m_reg.D = GetLByte(*m_currIdx); };
		m_opcodesIXY[0x5D] = [=]() { m_reg.E = GetLByte(*m_currIdx); };
		m_opcodesIXY[0x65] = [=]() { SetLByte(*m_currIdx, GetHByte(*m_currIdx)); };
		m_opcodesIXY[0x6D] = [=]() { SetLByte(*m_currIdx, GetLByte(*m_currIdx)); }; // NOP+NOP equivalent
		m_opcodesIXY[0x7D] = [=]() { m_reg.A = GetLByte(*m_currIdx); };

		// LD r, (IXY+s8)
		m_opcodesIXY[0x46] = [=]() { m_reg.B = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x4E] = [=]() { m_reg.C = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x56] = [=]() { m_reg.D = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x5E] = [=]() { m_reg.E = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x66] = [=]() { SetHByte(*m_currIdx, ReadMemIdx(FetchByte())); }; // NOP+NOP equivalent
		m_opcodesIXY[0x6E] = [=]() { SetHByte(*m_currIdx, ReadMemIdx(FetchByte())); };
		m_opcodesIXY[0x7E] = [=]() { m_reg.A = ReadMemIdx(FetchByte()); };

		// LD IXY(h), r (Undocumented)
		m_opcodesIXY[0x60] = [=]() { SetHByte(*m_currIdx, m_reg.B); };
		m_opcodesIXY[0x61] = [=]() { SetHByte(*m_currIdx, m_reg.D); };
		m_opcodesIXY[0x62] = [=]() { SetHByte(*m_currIdx, m_reg.D); };
		m_opcodesIXY[0x63] = [=]() { SetHByte(*m_currIdx, m_reg.E); };
		m_opcodesIXY[0x67] = [=]() { SetHByte(*m_currIdx, m_reg.A); };

		// LD IXY(l), r (Undocumented)
		m_opcodesIXY[0x68] = [=]() { SetLByte(*m_currIdx, m_reg.B); };
		m_opcodesIXY[0x69] = [=]() { SetLByte(*m_currIdx, m_reg.D); };
		m_opcodesIXY[0x6A] = [=]() { SetLByte(*m_currIdx, m_reg.D); };
		m_opcodesIXY[0x6B] = [=]() { SetLByte(*m_currIdx, m_reg.E); };
		m_opcodesIXY[0x6F] = [=]() { SetLByte(*m_currIdx, m_reg.A); };

		// ADD|ADC|SUB|SBC|AND|XOR|OR|CP A, IXY(h|l) (Undocumented)
		m_opcodesIXY[0x84] = [=]() { add(GetHByte(*m_currIdx)); }; // ADD IXY(h)
		m_opcodesIXY[0x85] = [=]() { add(GetLByte(*m_currIdx)); }; // ADD IXY(l)

		m_opcodesIXY[0x8C] = [=]() { add(GetHByte(*m_currIdx), GetFlag(FLAG_CY)); }; // ADC IXY(h)
		m_opcodesIXY[0x8D] = [=]() { add(GetLByte(*m_currIdx), GetFlag(FLAG_CY)); }; // ADC IXY(l)

		m_opcodesIXY[0x94] = [=]() { sub(GetHByte(*m_currIdx)); }; // SUB IXY(h)
		m_opcodesIXY[0x95] = [=]() { sub(GetLByte(*m_currIdx)); }; // SUB IXY(l)

		m_opcodesIXY[0x9C] = [=]() { sub(GetHByte(*m_currIdx), GetFlag(FLAG_CY)); }; // SBC IXY(h)
		m_opcodesIXY[0x9D] = [=]() { sub(GetLByte(*m_currIdx), GetFlag(FLAG_CY)); }; // SBC IXY(l)

		m_opcodesIXY[0xA4] = [=]() { ana(GetHByte(*m_currIdx)); }; // AND IXY(h)
		m_opcodesIXY[0xA5] = [=]() { ana(GetLByte(*m_currIdx)); }; // AND IXY(l)

		m_opcodesIXY[0xAC] = [=]() { xra(GetHByte(*m_currIdx)); }; // XOR IXY(h)
		m_opcodesIXY[0xAD] = [=]() { xra(GetLByte(*m_currIdx)); }; // XOR IXY(l)

		m_opcodesIXY[0xB4] = [=]() { ora(GetHByte(*m_currIdx)); }; // OR IXY(h)
		m_opcodesIXY[0xB5] = [=]() { ora(GetLByte(*m_currIdx)); }; // OR IXY(l)

		m_opcodesIXY[0xBC] = [=]() { cmp(GetHByte(*m_currIdx)); }; // CMP IXY(h)
		m_opcodesIXY[0xBD] = [=]() { cmp(GetLByte(*m_currIdx)); }; // CMP IXY(l)
 
		// ADD|ADC|SUB|SBC|AND|XOR|OR|CP A, (IXY+s8)
		m_opcodesIXY[0x86] = [=]() { add(ReadMemIdx(FetchByte())); }; // ADD
		m_opcodesIXY[0x8E] = [=]() { add(ReadMemIdx(FetchByte()), GetFlag(FLAG_CY)); }; // ADC

		m_opcodesIXY[0x96] = [=]() { sub(ReadMemIdx(FetchByte())); }; // SUB
		m_opcodesIXY[0x9E] = [=]() { sub(ReadMemIdx(FetchByte()), GetFlag(FLAG_CY)); }; // SBC

		m_opcodesIXY[0xA6] = [=]() { ana(ReadMemIdx(FetchByte())); }; // AND
		m_opcodesIXY[0xAE] = [=]() { xra(ReadMemIdx(FetchByte())); }; // XOR

		m_opcodesIXY[0xB6] = [=]() { ora(ReadMemIdx(FetchByte())); }; // OR
		m_opcodesIXY[0xBE] = [=]() { cmp(ReadMemIdx(FetchByte())); }; // CMP

		// IXY Bit instructions 
		m_opcodesIXY[0xCB] = [=]() { BITSxy(); };
	}

	BYTE CPUZ80::ReadMemIdx(BYTE offset)
	{
		WORD base = *m_currIdx;
		base += Widen(offset);
		return m_memory.Read8(base);
	}

	void CPUZ80::WriteMemIdx(BYTE offset, BYTE value)
	{
		WORD base = *m_currIdx;
		base += Widen(offset);
		m_memory.Write8(base, value);
	}

	void CPUZ80::MEMop(std::function<void(CPUZ80*, BYTE& dest)> func)
	{
		BYTE temp = ReadMem();
		func(this, temp);
		WriteMem(temp);
	}

	void CPUZ80::loadImm8toIdx(WORD base)
	{
		base += Widen(FetchByte());
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

	void CPUZ80::add(BYTE src, bool carry)
	{
		BYTE oldA = m_reg.A;
		CPU8080::add(src, carry);
		
		// TODO: Improve this
		// Set Overflow flag
		// If 2 Two's Complement numbers are added, and they both have the same sign (both positive or both negative), 
		// then overflow occurs if and only if the result has the opposite sign. 
		// Overflow never occurs when adding operands with different signs. 
		SetFlag(FLAG_PV, (GetMSB(oldA) == GetMSB(src)) && (GetMSB(m_reg.A) != GetMSB(src)));
	}

	void CPUZ80::sub(BYTE src, bool borrow)
	{
		BYTE oldA = m_reg.A;
		CPU8080::sub(src, borrow);

		// TODO: Improve this
		// Set Overflow flag
		// If 2 Two's Complement numbers are subtracted, and their signs are different, 
		// then overflow occurs if and only if the result has the same sign as what is being subtracted.
		SetFlag(FLAG_PV, (GetMSB(oldA) != GetMSB(src)) && (GetMSB(m_reg.A) == GetMSB(src)));
	}

	BYTE CPUZ80::cmp(BYTE src)
	{
		BYTE oldA = m_reg.A;
		BYTE res = CPU8080::cmp(src);

		// TODO: Improve this
		// Set Overflow flag
		// If 2 Two's Complement numbers are subtracted, and their signs are different, 
		// then overflow occurs if and only if the result has the same sign as what is being subtracted.
		SetFlag(FLAG_PV, (GetMSB(oldA) != GetMSB(src)) && (GetMSB(res) == GetMSB(src)));
		return res;
	}

	void CPUZ80::inc(BYTE& reg)
	{
		BYTE before = reg;
		CPU8080::inc(reg);
		// Set Overflow flag
		SetFlag(FLAG_PV, GetMSB(before) != GetMSB(reg));
	}

	void CPUZ80::dec(BYTE& reg)
	{
		BYTE before = reg;
		CPU8080::dec(reg);
		// Set Overflow flag
		SetFlag(FLAG_PV, GetMSB(before) != GetMSB(reg));
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
		REFRESH();
	}

	void CPUZ80::BITSxy()
	{
		m_currOffset = FetchByte();
		BYTE op2 = FetchByte();
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP4, op2);
		exec(m_opcodesBITSxy, op2);
	}

	void CPUZ80::EXTD(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP2, op2);
		exec(m_opcodesEXTD, op2);
		REFRESH();
	}

	void CPUZ80::IXY(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP3, op2);
		exec(m_opcodesIXY, op2);
		REFRESH();
	}

	void CPUZ80::LDAri(BYTE src)
	{
		m_reg.A = src;
		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_PV, m_iff2);
	}

	void CPUZ80::RLC(BYTE& dest)
	{
		bool msb = GetBit(dest, 7);

		dest = (dest << 1);
		dest |= (msb ? 1 : 0);

		AdjustBaseFlags(dest);
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, msb);
	}

	void CPUZ80::RL(BYTE& dest)
	{
		bool msb = GetBit(dest, 7);

		dest = (dest << 1);
		dest |= (GetFlag(FLAG_CY) ? 1 : 0);

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

	void CPUZ80::BITgetIXY(BYTE bit)
	{
		BYTE src = ReadMemIdx(m_currOffset);
		AdjustBaseFlags(src);
		SetFlag(FLAG_H, true);
		SetFlag(FLAG_Z, !GetBit(src, bit));
	}

	void CPUZ80::BITset(BYTE bit, bool set, BYTE& dest)
	{
		SetBit(dest, bit, set);
	}

	void CPUZ80::BITsetIXY(BYTE bit, bool set, BYTE& dest)
	{
		dest = ReadMemIdx(m_currOffset);
		SetBit(dest, bit, set);
		WriteMemIdx(m_currOffset, dest);
	}

	void CPUZ80::BITsetM(BYTE bit, bool set)
	{
		BYTE temp = ReadMem();
		BITset(bit, set, temp);
		WriteMem(temp);
	}

	void CPUZ80::INc(BYTE& dest)
	{
		In(GetBC(), dest);
		AdjustBaseFlags(dest);
	}

	void CPUZ80::DI()
	{
		m_iff1 = false;
		m_iff2 = false;
	}
	void CPUZ80::EI()
	{
		m_iff1 = true;
		m_iff2 = true;
	}
}
