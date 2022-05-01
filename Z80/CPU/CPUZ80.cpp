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

		m_opcodes[0010] = [=]() { EXAF(); };
		m_opcodes[0020] = [=]() { DJNZ(); };
		m_opcodes[0331] = [=]() { EXX(); };

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


		// TODO
		m_opcodesEXTD[0x40] = [=]() { NotImplemented("EXTD[0x40]"); };
		m_opcodesEXTD[0x41] = [=]() { NotImplemented("EXTD[0x41]"); };
		m_opcodesEXTD[0x42] = [=]() { NotImplemented("EXTD[0x42]"); };
		m_opcodesEXTD[0x44] = [=]() { NotImplemented("EXTD[0x44]"); };
		m_opcodesEXTD[0x45] = [=]() { NotImplemented("EXTD[0x45]"); };

		m_opcodesEXTD[0x48] = [=]() { NotImplemented("EXTD[0x48]"); };
		m_opcodesEXTD[0x49] = [=]() { NotImplemented("EXTD[0x49]"); };
		m_opcodesEXTD[0x4A] = [=]() { NotImplemented("EXTD[0x4A]"); };
		m_opcodesEXTD[0x4C] = [=]() { NotImplemented("EXTD[0x4C]"); };
		m_opcodesEXTD[0x4D] = [=]() { NotImplemented("EXTD[0x4D]"); };

		m_opcodesEXTD[0x50] = [=]() { NotImplemented("EXTD[0x50]"); };
		m_opcodesEXTD[0x51] = [=]() { NotImplemented("EXTD[0x51]"); };
		m_opcodesEXTD[0x52] = [=]() { NotImplemented("EXTD[0x52]"); };
		m_opcodesEXTD[0x54] = [=]() { NotImplemented("EXTD[0x54]"); };
		m_opcodesEXTD[0x55] = [=]() { NotImplemented("EXTD[0x55]"); };

		m_opcodesEXTD[0x58] = [=]() { NotImplemented("EXTD[0x58]"); };
		m_opcodesEXTD[0x59] = [=]() { NotImplemented("EXTD[0x59]"); };
		m_opcodesEXTD[0x5A] = [=]() { NotImplemented("EXTD[0x5A]"); };
		m_opcodesEXTD[0x5C] = [=]() { NotImplemented("EXTD[0x5C]"); };
		m_opcodesEXTD[0x5D] = [=]() { NotImplemented("EXTD[0x5D]"); };

		m_opcodesEXTD[0x60] = [=]() { NotImplemented("EXTD[0x60]"); };
		m_opcodesEXTD[0x61] = [=]() { NotImplemented("EXTD[0x61]"); };
		m_opcodesEXTD[0x62] = [=]() { NotImplemented("EXTD[0x62]"); };
		m_opcodesEXTD[0x64] = [=]() { NotImplemented("EXTD[0x64]"); };
		m_opcodesEXTD[0x65] = [=]() { NotImplemented("EXTD[0x65]"); };
		m_opcodesEXTD[0x67] = [=]() { NotImplemented("EXTD[0x67]"); };

		m_opcodesEXTD[0x68] = [=]() { NotImplemented("EXTD[0x68]"); };
		m_opcodesEXTD[0x69] = [=]() { NotImplemented("EXTD[0x69]"); };
		m_opcodesEXTD[0x6A] = [=]() { NotImplemented("EXTD[0x6A]"); };
		m_opcodesEXTD[0x6C] = [=]() { NotImplemented("EXTD[0x6C]"); };
		m_opcodesEXTD[0x6D] = [=]() { NotImplemented("EXTD[0x6D]"); };
		m_opcodesEXTD[0x6F] = [=]() { NotImplemented("EXTD[0x6F]"); };

		m_opcodesEXTD[0x70] = [=]() { NotImplemented("EXTD[0x70]"); };
		m_opcodesEXTD[0x71] = [=]() { NotImplemented("EXTD[0x71]"); };
		m_opcodesEXTD[0x72] = [=]() { NotImplemented("EXTD[0x72]"); };
		m_opcodesEXTD[0x74] = [=]() { NotImplemented("EXTD[0x74]"); };
		m_opcodesEXTD[0x75] = [=]() { NotImplemented("EXTD[0x75]"); };

		m_opcodesEXTD[0x78] = [=]() { NotImplemented("EXTD[0x78]"); };
		m_opcodesEXTD[0x79] = [=]() { NotImplemented("EXTD[0x79]"); };
		m_opcodesEXTD[0x7A] = [=]() { NotImplemented("EXTD[0x7A]"); };
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

	void CPUZ80::NotImplemented(const char* opStr)
	{
		LogPrintf(LOG_ERROR, "[%s] not implemented", opStr);
		throw std::exception("Not implemented");
	}

	void CPUZ80::EXAF()
	{
		NotImplemented("EXAF");
	}

	void CPUZ80::DJNZ()
	{
		NotImplemented("DJNZ");
	}

	void CPUZ80::EXX()
	{
		NotImplemented("EXX");
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
}
