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


		// Logical and Arithmetic Commands

		// ORA
		m_opcodes[0x09] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0x05] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x15] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x01] = [=]() { UnknownOpcode(); }; // izx
		m_opcodes[0x11] = [=]() { UnknownOpcode(); }; // izy
		m_opcodes[0x0D] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x1D] = [=]() { UnknownOpcode(); }; // abx
		m_opcodes[0x19] = [=]() { UnknownOpcode(); }; // aby

		// AND
		m_opcodes[0x29] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0x25] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x35] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x21] = [=]() { UnknownOpcode(); }; // izx
		m_opcodes[0x31] = [=]() { UnknownOpcode(); }; // izy
		m_opcodes[0x2D] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x3D] = [=]() { UnknownOpcode(); }; // abx
		m_opcodes[0x39] = [=]() { UnknownOpcode(); }; // aby

		// EOR
		m_opcodes[0x49] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0x45] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x55] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x41] = [=]() { UnknownOpcode(); }; // izx
		m_opcodes[0x51] = [=]() { UnknownOpcode(); }; // izy
		m_opcodes[0x4D] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x5D] = [=]() { UnknownOpcode(); }; // abx
		m_opcodes[0x59] = [=]() { UnknownOpcode(); }; // aby

		// ADC
		m_opcodes[0x69] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0x65] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x75] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x61] = [=]() { UnknownOpcode(); }; // izx
		m_opcodes[0x71] = [=]() { UnknownOpcode(); }; // izy
		m_opcodes[0x6D] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x7D] = [=]() { UnknownOpcode(); }; // abx
		m_opcodes[0x79] = [=]() { UnknownOpcode(); }; // aby

		// SBC
		m_opcodes[0xE9] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0xE5] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0xF5] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0xE1] = [=]() { UnknownOpcode(); }; // izx
		m_opcodes[0xF1] = [=]() { UnknownOpcode(); }; // izy
		m_opcodes[0xED] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0xFD] = [=]() { UnknownOpcode(); }; // abx
		m_opcodes[0xF9] = [=]() { UnknownOpcode(); }; // aby

		// CMP
		m_opcodes[0xC9] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0xC5] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0xD5] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0xC1] = [=]() { UnknownOpcode(); }; // izx
		m_opcodes[0xD1] = [=]() { UnknownOpcode(); }; // izy
		m_opcodes[0xCD] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0xDD] = [=]() { UnknownOpcode(); }; // abx
		m_opcodes[0xD9] = [=]() { UnknownOpcode(); }; // aby

		// CPX
		m_opcodes[0xE0] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0xE4] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0xEC] = [=]() { UnknownOpcode(); }; // abs

		// CPY
		m_opcodes[0xC0] = [=]() { UnknownOpcode(); }; // imm
		m_opcodes[0xC4] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0xCC] = [=]() { UnknownOpcode(); }; // abs

		// DEC
		m_opcodes[0xC6] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0xD6] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0xCE] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0xDE] = [=]() { UnknownOpcode(); }; // abx

		// DEX
		m_opcodes[0xCA] = [=]() { UnknownOpcode(); }; // imp

		// DEY
		m_opcodes[0x88] = [=]() { UnknownOpcode(); }; // imp

		// INC
		m_opcodes[0xE6] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0xF6] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0xEE] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0xFE] = [=]() { UnknownOpcode(); }; // abx

		// INX
		m_opcodes[0xE8] = [=]() { UnknownOpcode(); }; // imp

		// INY
		m_opcodes[0xC8] = [=]() { UnknownOpcode(); }; // imp

		// ASL
		m_opcodes[0x0A] = [=]() { UnknownOpcode(); }; // imp
		m_opcodes[0x06] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x16] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x0E] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x1E] = [=]() { UnknownOpcode(); }; // abx

		// ROL
		m_opcodes[0x2A] = [=]() { UnknownOpcode(); }; // imp
		m_opcodes[0x26] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x36] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x2E] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x3E] = [=]() { UnknownOpcode(); }; // abx

		// LSR
		m_opcodes[0x4A] = [=]() { UnknownOpcode(); }; // imp
		m_opcodes[0x46] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x56] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x4E] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x5E] = [=]() { UnknownOpcode(); }; // abx

		// ROR
		m_opcodes[0x6A] = [=]() { UnknownOpcode(); }; // imp
		m_opcodes[0x66] = [=]() { UnknownOpcode(); }; // zp
		m_opcodes[0x76] = [=]() { UnknownOpcode(); }; // zpx
		m_opcodes[0x6E] = [=]() { UnknownOpcode(); }; // abs
		m_opcodes[0x7E] = [=]() { UnknownOpcode(); }; // abx

		// Move Commands


		// Jump Commands


		// Flag Commands

		// CARRY
		m_opcodes[0x18] = [=]() { SetFlag(FLAG_C, false); }; // CLC
		m_opcodes[0x38] = [=]() { SetFlag(FLAG_C, true); };  // SEC

		// DECIMAL
		m_opcodes[0xD8] = [=]() { SetFlag(FLAG_D, false); }; // CLD
		m_opcodes[0xF8] = [=]() { SetFlag(FLAG_D, true); };  // SED

		// INTERRUPT
		m_opcodes[0x58] = [=]() { SetFlag(FLAG_I, false); }; // CLI
		m_opcodes[0x78] = [=]() { SetFlag(FLAG_I, true); };  // SEI

		// OVERFLOW
		m_opcodes[0xB8] = [=]() { SetFlag(FLAG_V, false); }; // CLV

		// NOP
		m_opcodes[0xEA] = [=]() { }; // NOP
	}

	CPU6502::~CPU6502()
	{
	}

	void CPU6502::Reset()
	{
		CPU::Reset();

		// TODO: Check if values are reset
		m_reg.A = 0;
		m_reg.X = 0;
		m_reg.Y = 0;
		m_reg.SP = 0;

		// Read reset vector
		ADDRESS resetVector = m_memory.Read16(ADDR_RESET);
		LogPrintf(LOG_INFO, "Reset vector: %04X", resetVector);
		m_programCounter = resetVector;

		ClearFlags(m_reg.flags);
	}

	void CPU6502::ClearFlags(BYTE& flags)
	{
		flags = FLAG_RESERVED_ON;
	}

	void CPU6502::SetFlags(BYTE f)
	{
		SetBitMask(f, FLAG_RESERVED_ON, true);
		m_reg.flags = f;
	}

	BYTE CPU6502::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		m_programCounter &= 0xFFFF;
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
		LogPrintf(LOG_ERROR, "A = %02X", m_reg.A);
		LogPrintf(LOG_ERROR, "X = %02X", m_reg.X);
		LogPrintf(LOG_ERROR, "Y = %02X", m_reg.Y);
		LogPrintf(LOG_ERROR, "Flags NV-BDIZC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_reg.flags));
		LogPrintf(LOG_ERROR, "SP = %04X", m_reg.SP);
		LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
	}

	void CPU6502::UnknownOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X", m_opcode, m_programCounter);
		Dump();
		throw std::exception("Unknown opcode");
	}

	void CPU6502::Exec(BYTE opcode)
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

	void CPU6502::Interrupt()
	{
		if (m_interruptsEnabled)
		{
		}

		// TODO: Not implemented
	}

	void CPU6502::AdjustBaseFlags(BYTE val)
	{
	}

}
