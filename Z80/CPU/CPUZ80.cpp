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
		m_opcodes[0313] = [=]() { BITS(FetchByte()); };
		m_opcodes[0335] = [=]() { IX(FetchByte()); };
		m_opcodes[0355] = [=]() { EXTD(FetchByte()); };
		m_opcodes[0375] = [=]() { IY(FetchByte()); };
	}

	void CPUZ80::Reset()
	{
		CPU8080::Reset();

		m_regAlt.A = 0;
		m_regAlt.B = 0;
		m_regAlt.C = 0;
		m_regAlt.D = 0;
		m_regAlt.E = 0;
		m_regAlt.H = 0;
		m_regAlt.L = 0;

		m_regIX = 0;
		m_regIY = 0;

		ClearFlags(m_regAlt.flags);
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

	void CPUZ80::EXAF()
	{
		LogPrintf(LOG_ERROR, "EXAF, not implemented");
		throw std::exception("EXAF, not implemented");
	}

	void CPUZ80::DJNZ()
	{
		LogPrintf(LOG_ERROR, "DJNZ, not implemented");
		throw std::exception("DJNZ, not implemented");
	}
	void CPUZ80::EXX()
	{
		LogPrintf(LOG_ERROR, "EXX, not implemented");
		throw std::exception("EXX, not implemented");
	}

	void CPUZ80::BITS(BYTE op2)
	{
		LogPrintf(LOG_ERROR, "BITS, not implemented");
		throw std::exception("BITS, not implemented");
	}

	void CPUZ80::IX(BYTE op2)
	{
		LogPrintf(LOG_ERROR, "IX, not implemented");
		throw std::exception("IX, not implemented");
	}

	void CPUZ80::EXTD(BYTE op2)
	{
		LogPrintf(LOG_ERROR, "EXTD, not implemented");
		throw std::exception("EXTD, not implemented");
	}

	void CPUZ80::IY(BYTE op2)
	{
		LogPrintf(LOG_ERROR, "IY, not implemented");
		throw std::exception("IY, not implemented");
	}
}
