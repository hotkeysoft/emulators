#include "stdafx.h"
#include "CPU6809.h"

using cpuInfo::Opcode;

namespace emul
{


	CPU6809::CPU6809(Memory& memory) : CPU6809("6809", memory)
	{
		static_assert(sizeof(m_reg.D) == 2);
		static_assert(sizeof(m_reg.ab.A) == 1);
		static_assert(sizeof(m_reg.ab.B) == 1);
	}

	CPU6809::CPU6809(const char* cpuid, Memory& memory) :
		CPU(CPU6809_ADDRESS_BITS, memory),
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

	void CPU6809::Init()
	{
		m_opcodes.resize(256);
		std::fill(m_opcodes.begin(), m_opcodes.end(), [=]() { UnknownOpcode(); });

		//m_opcodes[0x00] = [=]() {  };
	}

	CPU6809::~CPU6809()
	{
	}

	void CPU6809::Reset()
	{
		CPU::Reset();

		// TODO: Check if values are reset
		m_reg.D = 0;
		m_reg.S = 0;
		m_reg.U = 0;
		m_reg.X = 0;
		m_reg.Y = 0;

		m_nmiEnabled = false;
		m_nmi.ResetLatch();
		m_irq = false;
		m_firq = false;

		ADDRESS resetAddr = m_memory.Read16be(ADDR_RESET);

		m_programCounter = resetAddr;

		ClearFlags(m_reg.flags);
	}

	void CPU6809::Reset(ADDRESS overrideAddress)
	{
		Reset();
		m_programCounter = overrideAddress;
	}


	BYTE CPU6809::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		m_programCounter &= 0xFFFF;
		return b;
	}

	bool CPU6809::Step()
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

	void CPU6809::Dump()
	{
		LogPrintf(LOG_ERROR, "AB = %04X", m_reg.D);
		LogPrintf(LOG_ERROR, "X  = %04X", m_reg.X);
		LogPrintf(LOG_ERROR, "Y  = %04X", m_reg.Y);
		LogPrintf(LOG_ERROR, "Flags EFHINZVC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_reg.flags));
		LogPrintf(LOG_ERROR, "S  = %04X", m_reg.S);
		LogPrintf(LOG_ERROR, "U  = %04X", m_reg.U);
		LogPrintf(LOG_ERROR, "DP = %02X", m_reg.DP);
		LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
	}

	void CPU6809::UnknownOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X", m_opcode, m_programCounter);
		Dump();
		throw std::exception("Unknown opcode");
	}

	void CPU6809::Exec(BYTE opcode)
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

	void CPU6809::Interrupt()
	{
	}

	void CPU6809::Serialize(json& to)
	{
		to["opcode"] = m_opcode;

		to["nmiEnabled"] = m_nmiEnabled;
		m_nmi.Serialize(to["nmi"]);
		to["irq"] = m_irq;
		to["firq"] = m_firq;

		to["flags"] = m_reg.flags;
		to["pc"] = m_programCounter;

		to["d"] = m_reg.D;
		to["s"] = m_reg.S;
		to["u"] = m_reg.U;
		to["x"] = m_reg.X;
		to["y"] = m_reg.Y;
	}
	void CPU6809::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];

		m_reg.flags = from["flags"];
		m_programCounter = from["pc"];

		m_nmiEnabled = from["nmiEnabled"];
		m_nmi.Deserialize(from["nmi"]);
		m_firq = from["firq"];
		m_irq = from["irq"];

		m_reg.D = from["d"];
		m_reg.S = from["s"];
		m_reg.U = from["u"];
		m_reg.X = from["x"];
		m_reg.Y = from["y"];
	}
}
