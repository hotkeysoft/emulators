#include "stdafx.h"
#include "CPU6502.h"

using cpuInfo::Opcode;

namespace emul
{
	CPU6502::CPU6502(Memory& memory) : CPU6502("6502", memory)
	{
	}


	CPU6502::CPU6502(const char* cpuid, Memory& memory) :
		CPU(CPU6502_ADDRESS_BITS, memory),
		m_info(cpuid),
		Logger(cpuid)
	{
		try
		{
			m_info.LoadConfig();
		}
		catch (nlohmann::detail::exception e)
		{
			LogPrintf(LOG_ERROR, "Fatal json error loading config: %s", e.what());
			throw;
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "Fatal error loading config: %s", e.what());
			throw;
		}
	}

	void CPU6502::Init()
	{
		m_opcodes.resize(256);
		std::fill(m_opcodes.begin(), m_opcodes.end(), [=]() { UnknownOpcode(); });


		//m_opcodes[0x00] = [=]() { };
	}

	CPU6502::~CPU6502()
	{
	}

	void CPU6502::Reset()
	{
		CPU::Reset();

		m_reg.A = 0;
		m_reg.B = 0;
		m_reg.C = 0;
		m_reg.D = 0;
		m_reg.E = 0;
		m_reg.H = 0;
		m_reg.L = 0;

		m_regSP = 0;
		m_programCounter = 0;

		m_ioRequest = false;
		m_interruptAcknowledge = false;
		m_interruptsEnabled = false;

		ClearFlags(m_reg.flags);
	}

	void CPU6502::ClearFlags(BYTE& flags)
	{
		flags = FLAG_RESERVED_ON;
	}

	void CPU6502::SetFlags(BYTE f)
	{
		SetBitMask(f, FLAG_RESERVED_OFF, false);
		SetBitMask(f, FLAG_RESERVED_ON, true);
		m_reg.flags = f;
	}

	BYTE CPU6502::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		return b;
	}

	bool CPU6502::Step()
	{
		bool ret = true;
		if (m_state != CPUState::HALT)
		{
			ret = CPU::Step();
		}

		if (m_state == CPUState::HALT)
		{
			Halt();
			m_opTicks = 1;
			ret = true;
		}

		if (ret)
		{
			Interrupt();
		}

		return ret;
	}

	void CPU6502::Dump()
	{
		LogPrintf(LOG_DEBUG, "AF = %02X %02X\tCY = %c", m_reg.A, m_reg.flags, GetFlag(FLAG_CY)?'1':'0');
		LogPrintf(LOG_DEBUG, "BC = %02X %02X\tP  = %c", m_reg.B, m_reg.C, GetFlag(FLAG_P)?'1':'0');
		LogPrintf(LOG_DEBUG, "DE = %02X %02X\tAC = %c", m_reg.D, m_reg.E, GetFlag(FLAG_AC)?'1':'0');
		LogPrintf(LOG_DEBUG, "HL = %02X %02X\tZ  = %c", m_reg.H, m_reg.L, GetFlag(FLAG_Z)?'1':'0');
		LogPrintf(LOG_DEBUG, "SP = %04X   \tS  = %c", m_regSP, GetFlag(FLAG_S)?'1':'0');
		LogPrintf(LOG_DEBUG, "PC = %04X", m_programCounter);
		LogPrintf(LOG_DEBUG, "");
	}

	void CPU6502::UnknownOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X", m_opcode, m_programCounter);
	}

	void CPU6502::Exec(BYTE opcode)
	{
		// Reset control lines
		m_interruptAcknowledge = false;
		m_ioRequest = false;

		m_opcode = opcode;
		m_currTiming = &m_info.GetOpcodeTiming(opcode);

		try
		{
			// Fetch the function corresponding to the opcode and run it
			{
				auto& opFunc = m_opcodes[opcode];
				opFunc();
			}

			TICK();
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "CPU: Exception at address 0x%04X! Stopping CPU", m_programCounter);
			m_state = CPUState::STOP;
		}
	}

	void CPU6502::Interrupt()
	{
		if (m_interruptsEnabled)
		{
			m_interruptAcknowledge = true;
			return;
		}

		// TODO: Not implemented
	}

	void CPU6502::AdjustBaseFlags(BYTE val)
	{
		SetFlag(FLAG_Z, (val == 0));
		SetFlag(FLAG_S, GetBit(val, 7));
		SetFlag(FLAG_P, IsParityEven(val));
	}

}
