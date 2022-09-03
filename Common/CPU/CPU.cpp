#include "stdafx.h"

#include "CPU.h"

namespace emul
{
	CPU::CPU(size_t addressBits, Memory& memory) :
		Logger("CPU"),
		m_memory(memory)
	{
	}

	CPU::~CPU()
	{
	}

	void CPU::Reset()
	{
	}

	void CPU::Run()
	{
		m_state = CPUState::RUN;
		while (Step() && m_state == CPUState::RUN);
	}

	bool CPU::Step()
	{
		try
		{
			m_opTicks = 0;

			// Fetch opcode
			m_state = CPUState::RUN;

			// Execute instruction
			Exec(FetchByte());
		}
		catch (std::exception e)
		{
			EnableLog(LOG_ERROR);
			LogPrintf(LOG_ERROR, "Error processing instruction at 0x%04X! [%s] Stopping CPU.\n", GetCurrentAddress(), e.what());
			m_state = CPUState::STOP;
		}

		return (m_state == CPUState::RUN);
	}

	WORD CPU::FetchWord()
	{
		BYTE l = FetchByte();
		BYTE h = FetchByte();

		return MakeWord(h, l);
	}

}
