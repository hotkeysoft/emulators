#include "stdafx.h"
#include "CPU.h"

namespace emul
{
	CPU::CPU(Memory& memory) : Logger("CPU"), m_memory(memory)
	{
		for (int i = 0; i < 256; i++)
		{
			m_opcodesTable[i] = &CPU::UnknownOpcode;
		}
	}

	CPU::~CPU()
	{
	}

	void CPU::Reset()
	{
		m_programCounter = 0;
		m_state = STOP;
		m_timeTicks = 0;
	}

	void CPU::Run()
	{
		m_state = RUN;
		while (Step() && m_state == RUN);
	}

	bool CPU::Step()
	{
		try
		{
			// Fetch opcode
			BYTE opcode = m_memory.Read8(m_programCounter);
			m_state = RUN;

			// Execute instruction
			(this->*m_opcodesTable[opcode])(opcode);
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "Error processing instruction at 0x%04X! Stopping CPU.\n", m_programCounter);
			m_state = STOP;
		}

		return (m_state == RUN);
	}

	void CPU::DumpUnassignedOpcodes()
	{
		for (int i = 0; i < 256; ++i)
		{
			if (m_opcodesTable[i] == &CPU::UnknownOpcode)
			{
				LogPrintf(LOG_INFO, "Unassigned: 0x%02X\t0%03o\n", i, i);
			}
		}
	}

	void CPU::AddOpcode(BYTE opcode, OPCodeFunction f)
	{
		if (m_opcodesTable[opcode] != &CPU::UnknownOpcode)
		{
			LogPrintf(LOG_ERROR, "CPU: Opcode (0x%02X) already defined!\n", opcode);
		}

		m_opcodesTable[opcode] = f;
	}

	void CPU::UnknownOpcode(BYTE opcode)
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X! Stopping CPU.\n", opcode, m_programCounter);
		m_state = STOP;
	}

	bool CPU::isParityOdd(BYTE b)
	{
		BYTE parity = 0;
		for (int i = 0; i < 8; i++)
		{
			parity ^= (b & 1);
			b = b >> 1;
		}

		return (parity != 0);
	}
}