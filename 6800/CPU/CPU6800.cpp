#include "stdafx.h"
#include "CPU6800.h"

using cpuInfo::Opcode;

namespace emul
{
	CPU6800::CPU6800(Memory& memory) : CPU6800("6800", memory)
	{
	}


	CPU6800::CPU6800(const char* cpuid, Memory& memory) :
		CPU(CPU6800_ADDRESS_BITS, memory),
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

	void CPU6800::Init()
	{
		m_opcodes.resize(256);
		std::fill(m_opcodes.begin(), m_opcodes.end(), [=]() { UnknownOpcode(); });

		m_opcodes[0x00] = [=]() { };
	}

	CPU6800::~CPU6800()
	{
	}

	void CPU6800::Reset()
	{
		CPU::Reset();

		// Read reset vector
		ADDRESS resetVector = m_memory.Read16(ADDR_RESET);
		LogPrintf(LOG_INFO, "Reset vector: %04X", resetVector);
		m_programCounter = resetVector;

		ClearFlags(m_reg.flags);
	}

	void CPU6800::Reset(ADDRESS overrideAddress)
	{
		Reset();
		m_programCounter = overrideAddress;
	}

	void CPU6800::ClearFlags(BYTE& flags)
	{
		flags = FLAG_RESERVED_ON;
	}

	void CPU6800::SetFlags(BYTE f)
	{
		SetBitMask(f, FLAG_RESERVED_ON, true);
		m_reg.flags = f;
	}

	BYTE CPU6800::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		m_programCounter &= 0xFFFF;
		return b;
	}

	bool CPU6800::Step()
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

	void CPU6800::Dump()
	{
		//LogPrintf(LOG_ERROR, "A = %02X", m_reg.A);
		//LogPrintf(LOG_ERROR, "X = %02X", m_reg.X);
		//LogPrintf(LOG_ERROR, "Y = %02X", m_reg.Y);
		//LogPrintf(LOG_ERROR, "Flags NV-BDIZC");
		//LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_reg.flags));
		//LogPrintf(LOG_ERROR, "SP = %04X", m_reg.SP);
		//LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		//LogPrintf(LOG_ERROR, "");
	}

	void CPU6800::UnknownOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X", m_opcode, m_programCounter);
		Dump();
		throw std::exception("Unknown opcode");
	}

	void CPU6800::Exec(BYTE opcode)
	{
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

	void CPU6800::Interrupt()
	{
	}

	void CPU6800::AdjustNZ(BYTE val)
	{
		//SetFlag(FLAG_N, GetBit(val, 7));
		//SetFlag(FLAG_Z, val == 0);
	}

	void CPU6800::Serialize(json& to)
	{
		to["opcode"] = m_opcode;
		to["pc"] = m_programCounter;
	}
	void CPU6800::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];
		m_programCounter = from["pc"];
	}
}
