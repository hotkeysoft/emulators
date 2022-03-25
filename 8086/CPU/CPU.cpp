#include "stdafx.h"

#include "CPU.h"

namespace emul
{
	CPU::CPU(size_t addressBits, Memory& memory) : 
		Logger("CPU"), 
		m_state(CPUState::STOP),
		m_memory(memory)
	{
	}

	CPU::~CPU()
	{
	}

	void CPU::Reset()
	{
		m_state = CPUState::STOP;
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
			unsigned char opcode = m_memory.Read8(GetCurrentAddress());
			
			// Execute instruction
			Exec(opcode);
		}
		catch (std::exception e)
		{	
			EnableLog(LOG_ERROR);
			LogPrintf(LOG_ERROR, "Error processing instruction at 0x%04X! [%s] Stopping CPU.\n", GetCurrentAddress(), e.what());
			m_state = CPUState::STOP;
		}

		return (m_state == CPUState::RUN);
	}
}
