#include "stdafx.h"
#include "CPUZ80.h"

using cpuInfo::Opcode;
using cpuInfo::MiscTiming;

namespace emul
{
	CPUZ80::CPUZ80(Memory& memory) : CPUZ80("z80", memory)
	{
	}

	CPUZ80::CPUZ80(const char* cpuid, Memory& memory) :
		Logger(cpuid),
		CPU8080(cpuid, memory)
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

		m_dataBusEnable = true;

		ClearFlags(m_regAlt.flags);
	}

	void CPUZ80::Interrupt()
	{
		// Process NMI
		if (m_nmiLatch.IsLatched())
		{
			LogPrintf(LOG_DEBUG, "NMI");
			m_interruptAcknowledge = true;
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
			LogPrintf(LOG_DEBUG, "INT");
			m_interruptAcknowledge = true;
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

		// ADD HL, RR (16 bit)
		m_opcodes[0011] = [=]() { addHL(GetBC()); }; // BC
		m_opcodes[0031] = [=]() { addHL(GetDE()); }; // DE
		m_opcodes[0051] = [=]() { addHL(GetHL()); }; // HL
		m_opcodes[0071] = [=]() { addHL(m_regSP); }; // SP

		// RLCA, RRCA, RLA, RRA don't adjust 'base' flags P/V, S and Z
		m_opcodes[0007] = [=]() { RLCA(); };
		m_opcodes[0017] = [=]() { RRCA(); };
		m_opcodes[0027] = [=]() { RLA(); };
		m_opcodes[0037] = [=]() { RRA(); };

		// DJNZ rel8 - Decrement B, jump relative if non zero
		m_opcodes[0020] = [=]() { jumpRelIF(--m_reg.B, FetchByte()); }; // 0x10

		// CPL (Complement accumulator)
		m_opcodes[0057] = [=]() { m_reg.A = ~m_reg.A; SetFlag(FLAG_H, true); SetFlag(FLAG_N, true); };

		// SCF (Set Carry)
		m_opcodes[0067] = [=]() { SetFlag(FLAG_CY, true); SetFlag(FLAG_H, false); SetFlag(FLAG_N, false); };

		// CCF (Complement carry)
		m_opcodes[0077] = [=]() { SetFlag(FLAG_H, GetFlag(FLAG_CY)); ComplementFlag(FLAG_CY); SetFlag(FLAG_N, false); };

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
		m_opcodes[0333] = [=]() { m_ioRequest = true; In(MakeWord(m_reg.A, FetchByte()), m_reg.A); };
		m_opcodes[0323] = [=]() { m_ioRequest = true; Out(MakeWord(m_reg.A, FetchByte()), m_reg.A); };

		InitBITS(); // Bit instructions (Prefix 0xCB)
		InitBITSxy(); // Bit instructions, indexed (Prefix 0xDDCF, 0xFDCB)
		InitEXTD();	// Extended instructions (Prefix 0xED)
		InitIXY();   // IX/IY instructions (Prefix 0xDD, 0xFD)
	}

	void CPUZ80::InitBITS()
	{
		m_opcodesBITS.resize(256);
		int added = 0;

		BYTE* const regs[] = { &m_reg.B, &m_reg.C, &m_reg.D, &m_reg.E, &m_reg.H, &m_reg.L, nullptr, &m_reg.A };

		auto addOpRegs = [&](BYTE base, std::function<void(CPUZ80*, BYTE& dest)> func)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				if (regs[i])
				{
					m_opcodesBITS[base + i] = [=]() { func(this, *regs[i]); };
				}
				else
				{
					m_opcodesBITS[base + i] = [=]() { MEMop(func); };
				}
				++added;
			}
		};

		auto addBitGet = [&](BYTE base, BYTE bit)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				if (regs[i])
				{
					m_opcodesBITS[base + i] = [=]() { BITget(bit, *regs[i]); };
				}
				else
				{
					m_opcodesBITS[base + i] = [=]() { BITget(bit, ReadMem()); };
				}
				++added;
			}
		};

		auto addBitSet = [&](BYTE base, BYTE bit, bool set)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				if (regs[i])
				{
					m_opcodesBITS[base + i] = [=]() { BITset(bit, set, *regs[i]); };
				}
				else
				{
					m_opcodesBITS[base + i] = [=]() { BITsetM(bit, set); };
				}
				++added;
			}
		};

		addOpRegs(0x00, &CPUZ80::RLC); // 8-bit rotation to the left
		addOpRegs(0x08, &CPUZ80::RRC); // 8-bit rotation to the right
		addOpRegs(0x10, &CPUZ80::RL);  // 9-bit rotation to the left
		addOpRegs(0x18, &CPUZ80::RR);  // 9-bit rotation to the right
		addOpRegs(0x20, &CPUZ80::SLA); // Arithmetic Shift Right 1 bit
		addOpRegs(0x28, &CPUZ80::SRA); // Arithmetic Shift Right 1 bit
		addOpRegs(0x30, &CPUZ80::SLL); // SLL/SL1: Undocumented, SLA but set low bit to 1
		addOpRegs(0x38, &CPUZ80::SRL); // Arithmetic Shift Right 1 bit, clear bit 7

		// BIT: Test bit n of (IX+s8) and set Z flag
		addBitGet(0x40, 0);
		addBitGet(0x48, 1);
		addBitGet(0x50, 2);
		addBitGet(0x58, 3);
		addBitGet(0x60, 4);
		addBitGet(0x68, 5);
		addBitGet(0x70, 6);
		addBitGet(0x78, 7);

		// RES: Clear bit n of (IX+s8), optional working register
		addBitSet(0x80, 0, false);
		addBitSet(0x88, 1, false);
		addBitSet(0x90, 2, false);
		addBitSet(0x98, 3, false);
		addBitSet(0xA0, 4, false);
		addBitSet(0xA8, 5, false);
		addBitSet(0xB0, 6, false);
		addBitSet(0xB8, 7, false);

		// SET: Set bit n of (IX+s8), optional working register
		addBitSet(0xC0, 0, true);
		addBitSet(0xC8, 1, true);
		addBitSet(0xD0, 2, true);
		addBitSet(0xD8, 3, true);
		addBitSet(0xE0, 4, true);
		addBitSet(0xE8, 5, true);
		addBitSet(0xF0, 6, true);
		addBitSet(0xF8, 7, true);

		assert(added == 256);
	}

	void CPUZ80::InitBITSxy()
	{
		m_opcodesBITSxy.resize(256);
		int added = 0;

		BYTE* const regs[] = { &m_reg.B, &m_reg.C, &m_reg.D, &m_reg.E, &m_reg.H, &m_reg.L, &m_regDummy, &m_reg.A };

		auto addOpRegs = [&](BYTE base, std::function<void(CPUZ80*, BYTE& dest)> func)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				m_opcodesBITSxy[base + i] = [=]() { IDXop(func, *regs[i]); };
				++added;
			}
		};

		auto addBitGet = [&](BYTE base, BYTE bit)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				m_opcodesBITSxy[base + i] = [=]() { BITgetIXY(bit); };
				++added;
			}
		};

		auto addBitSet = [&](BYTE base, BYTE bit, bool set)
		{
			for (size_t i = 0; i < 8; ++i)
			{
				m_opcodesBITSxy[base + i] = [=]() { BITsetIXY(bit, set, *regs[i]); };
				++added;
			}
		};

		addOpRegs(0x00, &CPUZ80::RLC); // 8-bit rotation to the left
		addOpRegs(0x08, &CPUZ80::RRC); // 8-bit rotation to the right
		addOpRegs(0x10, &CPUZ80::RL);  // 9-bit rotation to the left
		addOpRegs(0x18, &CPUZ80::RR);  // 9-bit rotation to the right
		addOpRegs(0x20, &CPUZ80::SLA); // Arithmetic Shift Right 1 bit
		addOpRegs(0x28, &CPUZ80::SRA); // Arithmetic Shift Right 1 bit
		addOpRegs(0x30, &CPUZ80::SLL); // SLL/SL1: Undocumented, SLA but set low bit to 1
		addOpRegs(0x38, &CPUZ80::SRL); // Arithmetic Shift Right 1 bit, clear bit 7

		// BIT: Test bit n of (IX+s8) and set Z flag
		addBitGet(0x40, 0);
		addBitGet(0x48, 1);
		addBitGet(0x50, 2);
		addBitGet(0x58, 3);
		addBitGet(0x60, 4);
		addBitGet(0x68, 5);
		addBitGet(0x70, 6);
		addBitGet(0x78, 7);

		// RES: Clear bit n of (IX+s8), optional working register
		addBitSet(0x80, 0, false);
		addBitSet(0x88, 1, false);
		addBitSet(0x90, 2, false);
		addBitSet(0x98, 3, false);
		addBitSet(0xA0, 4, false);
		addBitSet(0xA8, 5, false);
		addBitSet(0xB0, 6, false);
		addBitSet(0xB8, 7, false);

		// SET: Set bit n of (IX+s8), optional working register
		addBitSet(0xC0, 0, true);
		addBitSet(0xC8, 1, true);
		addBitSet(0xD0, 2, true);
		addBitSet(0xD8, 3, true);
		addBitSet(0xE0, 4, true);
		addBitSet(0xE8, 5, true);
		addBitSet(0xF0, 6, true);
		addBitSet(0xF8, 7, true);

		assert(added == 256);
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

		// OUT (C), r
		m_opcodesEXTD[0x41] = [=]() { OUTc(m_reg.B); };
		m_opcodesEXTD[0x49] = [=]() { OUTc(m_reg.C); };
		m_opcodesEXTD[0x51] = [=]() { OUTc(m_reg.D); };
		m_opcodesEXTD[0x59] = [=]() { OUTc(m_reg.E); };
		m_opcodesEXTD[0x61] = [=]() { OUTc(m_reg.H); };
		m_opcodesEXTD[0x69] = [=]() { OUTc(m_reg.L); };
		m_opcodesEXTD[0x71] = [=]() { OUTc(m_regDummy); };
		m_opcodesEXTD[0x79] = [=]() { OUTc(m_reg.A); };

		// NEG
		m_opcodesEXTD[0x44] = [=]() { NEG(); };
		m_opcodesEXTD[0x4C] = [=]() { NEG(); }; // Undocumented
		m_opcodesEXTD[0x54] = [=]() { NEG(); }; // Undocumented
		m_opcodesEXTD[0x5C] = [=]() { NEG(); }; // Undocumented
		m_opcodesEXTD[0x64] = [=]() { NEG(); }; // Undocumented
		m_opcodesEXTD[0x6C] = [=]() { NEG(); }; // Undocumented
		m_opcodesEXTD[0x74] = [=]() { NEG(); }; // Undocumented
		m_opcodesEXTD[0x7C] = [=]() { NEG(); }; // Undocumented

		// RETN Return from NMI interrupt
		m_opcodesEXTD[0x45] = [=]() { RETN(); };
		m_opcodesEXTD[0x55] = [=]() { RETN(); }; // Undocumented
		m_opcodesEXTD[0x65] = [=]() { RETN(); }; // Undocumented
		m_opcodesEXTD[0x75] = [=]() { RETN(); }; // Undocumented
		m_opcodesEXTD[0x5D] = [=]() { RETN(); }; // Undocumented
		m_opcodesEXTD[0x6D] = [=]() { RETN(); }; // Undocumented

		// RETI Return from interrupt
		m_opcodesEXTD[0x4D] = [=]() { retIF(true); };
		m_opcodesEXTD[0x7D] = [=]() { retIF(true); };

		m_opcodesEXTD[0x67] = [=]() { RRD(); };
		m_opcodesEXTD[0x6F] = [=]() { RLD(); };

		m_opcodesEXTD[0xA0] = [=]() { LDI(); };
		m_opcodesEXTD[0xA1] = [=]() { CPI(); };
		m_opcodesEXTD[0xA2] = [=]() { INI(); };
		m_opcodesEXTD[0xA3] = [=]() { OUTI(); };

		m_opcodesEXTD[0xA8] = [=]() { LDD(); };
		m_opcodesEXTD[0xA9] = [=]() { CPD(); };
		m_opcodesEXTD[0xAA] = [=]() { IND(); };
		m_opcodesEXTD[0xAB] = [=]() { OUTD(); };

		m_opcodesEXTD[0xB0] = [=]() { rep(&CPUZ80::LDI); }; // LDIR
		m_opcodesEXTD[0xB1] = [=]() { rep(&CPUZ80::CPI, true); }; // CPIR
		m_opcodesEXTD[0xB2] = [=]() { repz(&CPUZ80::INI); }; // INIR
		m_opcodesEXTD[0xB3] = [=]() { repz(&CPUZ80::OUTI); }; // OTIR

		m_opcodesEXTD[0xB8] = [=]() { rep(&CPUZ80::LDD); }; // LDDR
		m_opcodesEXTD[0xB9] = [=]() { rep(&CPUZ80::CPD, true); }; // CPDR
		m_opcodesEXTD[0xBA] = [=]() { repz(&CPUZ80::IND); }; // INDR
		m_opcodesEXTD[0xBB] = [=]() { repz(&CPUZ80::OUTD); }; // OTR
	}

	void CPUZ80::InitIXY()
	{
		m_opcodesIXY.resize(256);
		std::fill(m_opcodesIXY.begin(), m_opcodesIXY.end(), [=]() { NOP(); });

		// Same as equivalent instructions in main op table
		const std::vector<BYTE> opCopy = {
			0004, 0005, 0006,
			0014, 0015, 0016,
			0024, 0025, 0026,
			0034, 0035, 0036,
			0074, 0075, 0076,
			0200, 0201, 0202, 0203, 0207,
			0210, 0211, 0212, 0213, 0217,
			0220, 0221, 0222, 0223, 0227,
			0230, 0231, 0232, 0233, 0237,
			0240, 0241, 0242, 0243, 0247,
			0250, 0251, 0252, 0253, 0257,
			0260, 0261, 0262, 0263, 0267,
			0270, 0271, 0272, 0273, 0277
		};

		for (BYTE op : opCopy)
		{
			m_opcodesIXY[op] = m_opcodes[op];
		}

		// New IX/IY Instructions

		// LOAD
		// ----------

		// ADD IXY, BC
		m_opcodesIXY[0x09] = [=]() { addIXY(GetBC()); };

		// ADD IXY, DE
		m_opcodesIXY[0x19] = [=]() { addIXY(GetDE()); };

		// LD IXY, i16
		m_opcodesIXY[0x21] = [=]() { *m_currIdx = FetchWord(); };

		// LD (i16), IXY
		m_opcodesIXY[0x22] = [=]() { m_memory.Write16(FetchWord(), *m_currIdx); };

		// INC IXY
		m_opcodesIXY[0x23] = [=]() { ++(*m_currIdx); };

		// INC IXY(h) (Undocumented)
		m_opcodesIXY[0x24] = [=]() { inc(GetIdxH()); };

		// DEC IXY(h) (Undocumented)
		m_opcodesIXY[0x25] = [=]() { dec(GetIdxH()); };

		// LD IXY(h), i8 (Undocumented)
		m_opcodesIXY[0x26] = [=]() { SetHByte(*m_currIdx, FetchByte()); };

		// ADD IXY, IXY
		m_opcodesIXY[0x29] = [=]() { addIXY(*m_currIdx); };

		// LD IXY, (i16)
		m_opcodesIXY[0x2A] = [=]() { *m_currIdx = m_memory.Read16(FetchWord()); };

		// DEC IXY
		m_opcodesIXY[0x2B] = [=]() { --(*m_currIdx); };

		// INC IXY(l) (Undocumented)
		m_opcodesIXY[0x2C] = [=]() { inc(GetIdxL()); };

		// DEC IXY(l) (Undocumented)
		m_opcodesIXY[0x2D] = [=]() { dec(GetIdxL()); };

		// LD IXY(l), i8 (Undocumented)
		m_opcodesIXY[0x2E] = [=]() { SetLByte(*m_currIdx, FetchByte()); };

		// INC (IXY+s8)
		m_opcodesIXY[0x34] = [=]() { INCXY(FetchByte()); };

		// DEC (IXY+s8)
		m_opcodesIXY[0x35] = [=]() { DECXY(FetchByte()); };

		// LD (IXY+s8), i8
		m_opcodesIXY[0x36] = [=]() { loadImm8toIdx(*m_currIdx); };

		// ADD IXY, SP
		m_opcodesIXY[0x39] = [=]() { addIXY(m_regSP); };

		// LD r, (IXY+s8)
		m_opcodesIXY[0x46] = [=]() { m_reg.B = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x4E] = [=]() { m_reg.C = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x56] = [=]() { m_reg.D = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x5E] = [=]() { m_reg.E = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x66] = [=]() { m_reg.H = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x6E] = [=]() { m_reg.L = ReadMemIdx(FetchByte()); };
		m_opcodesIXY[0x7E] = [=]() { m_reg.A = ReadMemIdx(FetchByte()); };

		// LD (IXY+s8), r
		m_opcodesIXY[0x70] = [=]() { WriteMemIdx(FetchByte(), m_reg.B); };
		m_opcodesIXY[0x71] = [=]() { WriteMemIdx(FetchByte(), m_reg.C); };
		m_opcodesIXY[0x72] = [=]() { WriteMemIdx(FetchByte(), m_reg.D); };
		m_opcodesIXY[0x73] = [=]() { WriteMemIdx(FetchByte(), m_reg.E); };
		m_opcodesIXY[0x74] = [=]() { WriteMemIdx(FetchByte(), m_reg.H); };
		m_opcodesIXY[0x75] = [=]() { WriteMemIdx(FetchByte(), m_reg.L); };
		m_opcodesIXY[0x77] = [=]() { WriteMemIdx(FetchByte(), m_reg.A); };

		// LD B, r (Undocumented)
		m_opcodesIXY[0x40] = [=]() { m_reg.B = m_reg.B; };   // LD B,B (NOP)
		m_opcodesIXY[0x41] = [=]() { m_reg.B = m_reg.C; };   // LD B,C
		m_opcodesIXY[0x42] = [=]() { m_reg.B = m_reg.D; };   // LD B,D
		m_opcodesIXY[0x43] = [=]() { m_reg.B = m_reg.E; };   // LD B,E
		m_opcodesIXY[0x44] = [=]() { m_reg.B = GetIdxH(); }; // LD B,IXYh
		m_opcodesIXY[0x45] = [=]() { m_reg.B = GetIdxL(); }; // LD B,IXYl
		m_opcodesIXY[0x47] = [=]() { m_reg.B = m_reg.A; };   // LD B,A

		// LD C, r (Undocumented)
		m_opcodesIXY[0x48] = [=]() { m_reg.C = m_reg.B; };   // LD C,B
		m_opcodesIXY[0x49] = [=]() { m_reg.C = m_reg.C; };   // LD C,C (NOP)
		m_opcodesIXY[0x4A] = [=]() { m_reg.C = m_reg.D; };   // LD C,D
		m_opcodesIXY[0x4B] = [=]() { m_reg.C = m_reg.E; };   // LD C,E
		m_opcodesIXY[0x4C] = [=]() { m_reg.C = GetIdxH(); }; // LD C,IXYh
		m_opcodesIXY[0x4D] = [=]() { m_reg.C = GetIdxL(); }; // LD C,IXYl
		m_opcodesIXY[0x4F] = [=]() { m_reg.C = m_reg.A; };   // LD C,A

		// LD D, r (Undocumented)
		m_opcodesIXY[0x50] = [=]() { m_reg.D = m_reg.B; };   // LD D,B
		m_opcodesIXY[0x51] = [=]() { m_reg.D = m_reg.C; };   // LD D,C
		m_opcodesIXY[0x52] = [=]() { m_reg.D = m_reg.D; };   // LD D,D (NOP)
		m_opcodesIXY[0x53] = [=]() { m_reg.D = m_reg.E; };   // LD D,E
		m_opcodesIXY[0x54] = [=]() { m_reg.D = GetIdxH(); }; // LD C,IXYh
		m_opcodesIXY[0x55] = [=]() { m_reg.D = GetIdxL(); }; // LD C,IXYl
		m_opcodesIXY[0x57] = [=]() { m_reg.D = m_reg.A; };   // LD D,A

		// LD E, r (Undocumented)
		m_opcodesIXY[0x58] = [=]() { m_reg.E = m_reg.B; };   // LD E,B
		m_opcodesIXY[0x59] = [=]() { m_reg.E = m_reg.C; };   // LD E,C
		m_opcodesIXY[0x5A] = [=]() { m_reg.E = m_reg.D; };   // LD E,D
		m_opcodesIXY[0x5B] = [=]() { m_reg.E = m_reg.E; };   // LD E,E (NOP)
		m_opcodesIXY[0x5C] = [=]() { m_reg.E = GetIdxH(); }; // LD E,IXYh
		m_opcodesIXY[0x5D] = [=]() { m_reg.E = GetIdxL(); }; // LD E,IXYl
		m_opcodesIXY[0x5F] = [=]() { m_reg.E = m_reg.A; };   // LD E,A

		// LD IXY(h), r (Undocumented)
		m_opcodesIXY[0x60] = [=]() { GetIdxH() = m_reg.B; };   // LD IXYh,B
		m_opcodesIXY[0x61] = [=]() { GetIdxH() = m_reg.C; };   // LD IXYh,C
		m_opcodesIXY[0x62] = [=]() { GetIdxH() = m_reg.D; };   // LD IXYh,D
		m_opcodesIXY[0x63] = [=]() { GetIdxH() = m_reg.E; };   // LD IXYh,E
		m_opcodesIXY[0x64] = [=]() { GetIdxH() = GetIdxH(); }; // LD IXYh,IXYh (NOP)
		m_opcodesIXY[0x65] = [=]() { GetIdxH() = GetIdxL(); }; // LD IXYh,IXYl
		m_opcodesIXY[0x67] = [=]() { GetIdxH() = m_reg.A; };   // LD IXYh,A

		// LD IXY(l), r (Undocumented)
		m_opcodesIXY[0x68] = [=]() { GetIdxL() = m_reg.B; };   // LD IXYl,B
		m_opcodesIXY[0x69] = [=]() { GetIdxL() = m_reg.C; };   // LD IXYl,C
		m_opcodesIXY[0x6A] = [=]() { GetIdxL() = m_reg.D; };   // LD IXYl,D
		m_opcodesIXY[0x6B] = [=]() { GetIdxL() = m_reg.E; };   // LD IXYl,E
		m_opcodesIXY[0x6C] = [=]() { GetIdxL() = GetIdxH(); }; // LD IXYl,IXYh
		m_opcodesIXY[0x6D] = [=]() { GetIdxL() = GetIdxL(); }; // LD IXYl,IXYl (NOP)
		m_opcodesIXY[0x6F] = [=]() { GetIdxL() = m_reg.A; };   // LD IXYl,A

		// LD A, r (Undocumented)
		m_opcodesIXY[0x78] = [=]() { m_reg.A = m_reg.B; };   // LD A,B
		m_opcodesIXY[0x79] = [=]() { m_reg.A = m_reg.C; };   // LD A,C
		m_opcodesIXY[0x7A] = [=]() { m_reg.A = m_reg.D; };   // LD A,D
		m_opcodesIXY[0x7B] = [=]() { m_reg.A = m_reg.E; };   // LD A,E
		m_opcodesIXY[0x7C] = [=]() { m_reg.A = GetIdxH(); }; // LD A,IXYh
		m_opcodesIXY[0x7D] = [=]() { m_reg.A = GetIdxL(); }; // LD A,IXYl
		m_opcodesIXY[0x7F] = [=]() { m_reg.A = m_reg.A; };   // LD A,A (NOP)

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

		// POP IXY
		m_opcodesIXY[0xE1] = [=]() { pop(*m_currIdx); };

		//  EX (SP), IXY
		m_opcodesIXY[0xE3] = [=]() { EXSPIXY(); };

		// PUSH IXY
		m_opcodesIXY[0xE5] = [=]() { push(*m_currIdx); };

		// JP (IXY)
		m_opcodesIXY[0xE9] = [=]() { m_programCounter = *m_currIdx; };

		// LD SP, IXY
		m_opcodesIXY[0xF9] = [=]() { m_regSP = *m_currIdx; };
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

	void CPUZ80::IDXop(std::function<void(CPUZ80*, BYTE& dest)> func, BYTE& reg)
	{
		reg = ReadMemIdx(m_currOffset);
		func(this, reg);
		WriteMemIdx(m_currOffset, reg);
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
			LogPrintf(LOG_ERROR, "CPU: Exception at address 0x%04X, subopcode 0x%02X! Stopping CPU", m_programCounter, opcode);
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
		LogPrintf(LOG_ERROR, "[%s][0x%02X] not implemented @ 0x%04X", opStr, m_opcode, m_programCounter);
		throw std::exception("Not implemented");
	}

	void CPUZ80::push(WORD src)
	{
		CPU8080::push(GetHByte(src), GetLByte(src));
	}

	void CPUZ80::pop(WORD& dest)
	{
		BYTE h, l;
		CPU8080::pop(h, l);
		SetHByte(dest, h);
		SetLByte(dest, l);
	}


	void CPUZ80::addHL(WORD src)
	{
		// Sign, zero and overflow are unaffected, so save them
		bool savedFlagS = GetFlag(FLAG_S);
		bool savedFlagZ = GetFlag(FLAG_Z);
		bool savedFlagPV = GetFlag(FLAG_PV);

		WORD dest = GetHL();
		add16(dest, src);
		SetHL(dest);

		// Restore flags
		SetFlag(FLAG_S, savedFlagS);
		SetFlag(FLAG_Z, savedFlagZ);
		SetFlag(FLAG_PV, savedFlagPV);
	}

	void CPUZ80::adcHL(WORD src)
	{
		WORD dest = GetHL();
		add16(dest, src, GetFlag(FLAG_CY));
		SetHL(dest);
		SetFlag(FLAG_Z, dest == 0);
	}

	void CPUZ80::addIXY(WORD src)
	{
		// Sign, zero and overflow are unaffected, so save them
		bool savedFlagS = GetFlag(FLAG_S);
		bool savedFlagZ = GetFlag(FLAG_Z);
		bool savedFlagPV = GetFlag(FLAG_PV);

		add16(*m_currIdx, src);

		// Restore flags
		SetFlag(FLAG_S, savedFlagS);
		SetFlag(FLAG_Z, savedFlagZ);
		SetFlag(FLAG_PV, savedFlagPV);
	}

	void CPUZ80::add16(WORD& dest, WORD src, bool carry)
	{
		// add operations are done on A register, save it and restore at the end
		BYTE savedA = m_reg.A;

		// Low byte
		m_reg.A = GetLByte(dest);
		add(GetLByte(src), carry);
		SetLByte(dest, m_reg.A);

		// High byte
		m_reg.A = GetHByte(dest);
		add(GetHByte(src), GetFlag(FLAG_CY));
		SetHByte(dest, m_reg.A);

		// Restore A register
		m_reg.A = savedA;
	}

	void CPUZ80::sbcHL(WORD src)
	{
		WORD dest = GetHL();
		sub16(dest, src, GetFlag(FLAG_CY));
		SetHL(dest);
		SetFlag(FLAG_Z, dest == 0);
	}

	void CPUZ80::sub16(WORD& dest, WORD src, bool borrow)
	{
		// add operations are done on A register, save it and restore at the end
		BYTE savedA = m_reg.A;

		// Low byte
		m_reg.A = GetLByte(dest);
		sub(GetLByte(src), borrow);
		SetLByte(dest, m_reg.A);

		// High byte
		m_reg.A = GetHByte(dest);
		sub(GetHByte(src), GetFlag(FLAG_CY));
		SetHByte(dest, m_reg.A);

		// Restore A register
		m_reg.A = savedA;
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
		SetFlag(FLAG_N, true);
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
		SetFlag(FLAG_N, true);
		return res;
	}

	void CPUZ80::inc(BYTE& reg)
	{
		BYTE before = reg;
		bool beforeH = GetBit(reg, 3);
		CPU8080::inc(reg);
		// Set Overflow flag
		SetFlag(FLAG_PV, !GetMSB(before) && GetMSB(reg));
	}

	void CPUZ80::dec(BYTE& reg)
	{
		BYTE before = reg;
		CPU8080::dec(reg);
		// Set Overflow flag
		SetFlag(FLAG_PV, GetMSB(before) && !GetMSB(reg));
		SetFlag(FLAG_N, true);
	}

	void CPUZ80::EXAF()
	{
		Swap(m_reg.A, m_regAlt.A);
		Swap(m_reg.flags, m_regAlt.flags);
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
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_PV, m_iff2);
	}

	// From "The Undocumented Z80 Documented":
	// The LDI/LDIR/LDD/LDDR instructions affect the flags in a strange way. At every
	// iteration, a byte is copied. Take that byte and add the value of register A to it.
	// Call that value n. Now, the flags are :
	//	- YF flag A copy of bit 1 of n.
	//	- HF flag Always reset.
	//	- XF flag A copy of bit 3 of n.
	//	- PV flag Set if BC not 0.
	//	- SF, ZF, CF flags These flags are unchanged

	// Does a sort of "LD (DE),(HL)", then increments DE, HL, and decrements BC.
	void CPUZ80::LDI()
	{
		BYTE src = ReadMem();
		m_memory.Write8(GetDE(), src);

		INX(m_reg.D, m_reg.E);
		INX(m_reg.H, m_reg.L);
		DCX(m_reg.B, m_reg.C);

		// Now for the flags, see comments above
		BYTE n = m_reg.A + src;

		SetFlag(FLAG_F5, GetBit(n, 1));
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_F3, GetBit(n, 3));
		SetFlag(FLAG_N, false);
		SetFlag(FLAG_PV, GetBC() != 0);
	}

	// Does a sort of "LD (DE),(HL)", then decrements DE, HL, and BC.
	void CPUZ80::LDD()
	{
		BYTE src = ReadMem();
		m_memory.Write8(GetDE(), src);

		DCX(m_reg.D, m_reg.E);
		DCX(m_reg.H, m_reg.L);
		DCX(m_reg.B, m_reg.C);

		// Now for the flags, see comments above
		BYTE n = m_reg.A + src;

		SetFlag(FLAG_F5, GetBit(n, 1));
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_F3, GetBit(n, 3));
		SetFlag(FLAG_N, false);
		SetFlag(FLAG_PV, GetBC() != 0);
	}

	void CPUZ80::CPI()
	{
		// Carry is preserved
		bool carry = GetFlag(FLAG_CY);
		cmp(ReadMem());

		INX(m_reg.H, m_reg.L);
		DCX(m_reg.B, m_reg.C);

		SetFlag(FLAG_PV, GetBC() != 0);
		// TODO: FLAG_F5, FLAG_F3
		SetFlag(FLAG_CY, carry);
	}

	void CPUZ80::CPD()
	{
		// Carry is preserved
		bool carry = GetFlag(FLAG_CY);
		cmp(ReadMem());

		DCX(m_reg.H, m_reg.L);
		DCX(m_reg.B, m_reg.C);

		SetFlag(FLAG_PV, GetBC() != 0);
		// TODO: FLAG_F5, FLAG_F3
		SetFlag(FLAG_CY, carry);
	}

	void CPUZ80::INI()
	{
		BYTE value;
		INc(value);
		WriteMem(value);

		INX(m_reg.H, m_reg.L);
		dec(m_reg.B);

		SetFlag(FLAG_N, false);
	}

	void CPUZ80::IND()
	{
		BYTE value;
		INc(value);
		WriteMem(value);

		DCX(m_reg.H, m_reg.L);
		dec(m_reg.B);

		SetFlag(FLAG_N, true);
	}

	void CPUZ80::OUTI()
	{
		BYTE value = ReadMem();
		OUTc(value);

		INX(m_reg.H, m_reg.L);
		dec(m_reg.B);

		SetFlag(FLAG_N, false);
	}

	void CPUZ80::OUTD()
	{
		BYTE value = ReadMem();
		OUTc(value);

		DCX(m_reg.H, m_reg.L);
		dec(m_reg.B);

		SetFlag(FLAG_N, true);
	}

	// Repeats an instruction until BC=0 or zero (if checkZ)
	void CPUZ80::rep(std::function<void(CPUZ80*)> func, bool checkZ)
	{
		func(this);
		jumpRelIF(GetFlag(FLAG_PV) && (!checkZ || !GetFlag(FLAG_Z)), -2);
	}

	void CPUZ80::repz(std::function<void(CPUZ80*)> func)
	{
		func(this);
		jumpRelIF(!GetFlag(FLAG_Z), -2);
	}

	// The contents of A are negated (two's complement). Operation is the same as subtracting A from zero.
	void CPUZ80::NEG()
	{
		BYTE oldA = m_reg.A;
		m_reg.A = 0;
		sub(oldA);
		SetFlag(FLAG_N, true);
	}

	// RLC/RL/RRC/RR: The 'adjustBaseFlags' flag is for the RLCA/RLA/RRCA/RRA versions
	// which don't modify P/V, S and Z

	// 8-bit rotation to the left. The bit leaving on the left is copied into the carry, and to bit 0.
	void CPUZ80::rlc(BYTE& dest, bool adjustBaseFlags)
	{
		bool msb = GetMSB(dest);

		dest <<= 1;
		SetBit(dest, 0, msb);

		if (adjustBaseFlags)
		{
			AdjustBaseFlags(dest);
		}
		else
		{
			SetFlag(FLAG_N, false);
		}
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, msb);
	}

	// 9-bit rotation to the left.
	// The carry value is put into 0th bit of the register, and the leaving 7th bit is put into the carry.
	void CPUZ80::rl(BYTE& dest, bool adjustBaseFlags)
	{
		bool msb = GetMSB(dest);

		dest <<= 1;
		SetBit(dest, 0, GetFlag(FLAG_CY));

		if (adjustBaseFlags)
		{
			AdjustBaseFlags(dest);
		}
		else
		{
			SetFlag(FLAG_N, false);
		}
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, msb);
	}

	// 8-bit rotation to the right. the bit leaving on the right is copied into the carry, and into bit 7.
	void CPUZ80::rrc(BYTE& dest, bool adjustBaseFlags)
	{
		bool lsb = GetLSB(dest);

		dest >>= 1;
		SetBit(dest, 7, lsb);

		if (adjustBaseFlags)
		{
			AdjustBaseFlags(dest);
		}
		else
		{
			SetFlag(FLAG_N, false);
		}
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, lsb);
	}

	// 9-bit rotation to the right.
	// The carry is copied into bit 7, and the bit leaving on the right is copied into the carry.
	void CPUZ80::rr(BYTE& dest, bool adjustBaseFlags)
	{
		bool lsb = GetLSB(dest);

		dest >>= 1;
		SetBit(dest, 7, GetFlag(FLAG_CY));

		if (adjustBaseFlags)
		{
			AdjustBaseFlags(dest);
		}
		else
		{
			SetFlag(FLAG_N, false);
		}
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, lsb);
	}

	// Arithmetic shift left 1 bit, bit 7 goes to carry flag, bit 0 is set to bit0.
	void CPUZ80::sla(BYTE& dest, bool bit0)
	{
		bool msb = GetMSB(dest);
		dest <<= 1;

		SetBit(dest, 0, bit0);

		AdjustBaseFlags(dest);
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, msb);
	}

	// Arithmetic shift right 1 bit, bit 0 goes to carry flag, bit 7 remains unchanged.
	void CPUZ80::sra(BYTE& dest, bool clearBit7)
	{
		bool lsb = GetLSB(dest);
		bool msb = GetMSB(dest);
		dest >>= 1;

		SetBit(dest, 7, clearBit7 ? 0 : msb);

		AdjustBaseFlags(dest);
		SetFlag(FLAG_H, false);
		SetFlag(FLAG_CY, lsb);
	}

	void CPUZ80::BITget(BYTE bit, BYTE src)
	{
		src &= (1 << bit);
		AdjustBaseFlags(src);
		SetFlag(FLAG_H, true);
	}

	void CPUZ80::BITgetIXY(BYTE bit)
	{
		BYTE src = ReadMemIdx(m_currOffset);
		src &= (1 << bit);
		AdjustBaseFlags(src);
		SetFlag(FLAG_H, true);
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

	void CPUZ80::INCXY(BYTE offset)
	{
		BYTE temp = ReadMemIdx(offset);
		inc(temp);
		WriteMemIdx(offset, temp);
	}

	void CPUZ80::DECXY(BYTE offset)
	{
		BYTE temp = ReadMemIdx(offset);
		dec(temp);
		WriteMemIdx(offset, temp);
	}

	void CPUZ80::INc(BYTE& dest)
	{
		m_ioRequest = true;
		In(MakeWord(m_reg.B, m_reg.C), dest);
		AdjustBaseFlags(dest);
		SetFlag(FLAG_H, false);
	}

	void CPUZ80::OUTc(BYTE& dest)
	{
		Out(MakeWord(m_reg.B, m_reg.C), dest);
	}

	void CPUZ80::DAA()
	{
		BYTE adjust = 0;
		bool carry = GetFlag(FLAG_CY);
		if (GetFlag(FLAG_H) || ((m_reg.A & 0x0F) > 9))
		{
			adjust |= 0x06;
			SetFlag(FLAG_H, true);
		}

		if (GetFlag(FLAG_CY) || (m_reg.A > 0x99))
		{
			adjust |= 0x60;
			carry = true;
		}

		bool neg = GetFlag(FLAG_N);
		if (neg)
		{
			sub(adjust);

		}
		else
		{
			add(adjust);
		}
		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_N, neg);
		SetFlag(FLAG_CY, carry);
	}

	// Rotates left a 12 bit value A[3:0]|(HL)[7:0]
	// High nibble of A is preserved
	//
	// (h/l are high/low nibbles of 8 bit value)
	// |  Ah  |  Al  | MEMh | MEMl |
	// | xxxx | xxxx | xxxx | xxxx |
	// | [4]  | [4]  |     [8]     |
	// | [4]  | [       12       ] |
	//
	// Before RLD:
	// | mmmm | nnnn | oooo | pppp |
	// After RLD:
	// | mmmm | oooo | pppp | nnnn |
	//
	// Before RLD
	// A = 0x1234
	// (HL) = 0x5678
	// After RLD:
	// A = 0x1256
	// (HL) = 0x7834
	void CPUZ80::RLD()
	{
		WORD temp = (WORD)ReadMem();

		// Move (HL) left 4 bits
		temp <<= 4;

		// Rotate A low nibble to end
		temp |= (m_reg.A & 0x0F);

		// Put A high nibble at the top
		temp |= ((m_reg.A & 0xF0) << 8);

		// Put back in A and (HL)
		m_reg.A = GetHByte(temp);
		WriteMem(GetLByte(temp));

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_H, false);
	}

	// Same as RLD, but rotation is to the right
	// A = 0x1234
	// (HL) = 0x5678
	// After RRD:
	// A = 0x1278
	// (HL) = 0x3456
	void CPUZ80::RRD()
	{
		// [0][0][Mh][Ml]
		WORD temp = (WORD)ReadMem();
		// [Ah][Al][Mh][Ml]
		SetHByte(temp, m_reg.A);

		// Keep [Ml]
		BYTE lowNibble = temp & 0x0F;

		// [0][Ah][Al][Mh]
		temp >>= 4;

		// (HL)=[Al][Mh]
		WriteMem(GetLByte(temp));

		// Clear A low nibble
		m_reg.A &= 0xF0;

		// Put Ml in A low nibble
		m_reg.A |= lowNibble;

		AdjustBaseFlags(m_reg.A);
		SetFlag(FLAG_H, false);
	}

	void CPUZ80::RETN()
	{
		CPU8080::retIF(true);
		m_iff1 = m_iff1;
	}

	void CPUZ80::EXSPIXY()
	{
		BYTE oldL = GetIdxL();
		BYTE oldH = GetIdxH();

		GetIdxL() = m_memory.Read8(m_regSP);
		GetIdxH() = m_memory.Read8(m_regSP + 1);

		m_memory.Write8(m_regSP, oldL);
		m_memory.Write8(m_regSP + 1, oldH);
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

	void CPUZ80::Serialize(json& to)
	{
		CPU8080::Serialize(to);

		to["regAlt.A"] = m_regAlt.A;
		to["regAlt.flags"] = m_regAlt.flags;
		to["regAlt.B"] = m_regAlt.B;
		to["regAlt.C"] = m_regAlt.C;
		to["regAlt.D"] = m_regAlt.D;
		to["regAlt.E"] = m_regAlt.E;
		to["regAlt.H"] = m_regAlt.H;
		to["regAlt.L"] = m_regAlt.L;

		to["reg.IX"] = m_regIX;
		to["reg.IY"] = m_regIY;
		to["reg.I"] = m_regI;
		to["reg.R"] = m_regR;

		to["iff1"] = m_iff1;
		to["iff2"] = m_iff2;

		to["dataBusEnable"] = m_dataBusEnable;

		m_nmiLatch.Serialize(to["nmi"]);
		m_intLatch.Serialize(to["int"]);
	}

	void CPUZ80::Deserialize(const json& from)
	{
		CPU8080::Deserialize(from);

		m_regAlt.A = from["regAlt.A"];
		m_regAlt.flags = from["regAlt.flags"];
		m_regAlt.B = from["regAlt.B"];
		m_regAlt.C = from["regAlt.C"];
		m_regAlt.D = from["regAlt.D"];
		m_regAlt.E = from["regAlt.E"];
		m_regAlt.H = from["regAlt.H"];
		m_regAlt.L = from["regAlt.L"];

		m_regIX = from["reg.IX"];
		m_regIY = from["reg.IY"];
		m_regI = from["reg.I"];
		m_regR = from["reg.R"];

		m_iff1 = from["iff1"];
		m_iff2 = from["iff2"];

		m_dataBusEnable = from["dataBusEnable"];

		m_nmiLatch.Deserialize(from["nmi"]);
		m_intLatch.Deserialize(from["int"]);

	}
}
