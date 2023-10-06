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

		m_opcodes[0x01] = [=]() { }; // NOP

		m_opcodes[0x06] = [=]() { SetFlags(m_reg.A); }; // TAP
		m_opcodes[0x07] = [=]() { m_reg.A = m_reg.flags; }; // TPA

		m_opcodes[0x08] = [=]() { ++m_reg.IX; SetFlag(FLAG_Z, m_reg.IX == 0); }; // INX
		m_opcodes[0x09] = [=]() { --m_reg.IX; SetFlag(FLAG_Z, m_reg.IX == 0); }; // DEX

		// Flags
		m_opcodes[0x0A] = [=]() { SetFlag(FLAG_V, false); }; // CLV
		m_opcodes[0x0B] = [=]() { SetFlag(FLAG_V, true); }; // SEV
		m_opcodes[0x0C] = [=]() { SetFlag(FLAG_C, false); }; // CLC
		m_opcodes[0x0D] = [=]() { SetFlag(FLAG_C, true); }; // SEC
		m_opcodes[0x0E] = [=]() { SetFlag(FLAG_I, false); }; // CLI // TODO: Apply at next instruction
		m_opcodes[0x0F] = [=]() { SetFlag(FLAG_I, true); }; // SEI

		//TODO m_opcodes[0x10] = [=]() {}; // SBA
		//TODO m_opcodes[0x11] = [=]() {}; // CBA

		m_opcodes[0x16] = [=]() { m_reg.B = m_reg.A; AdjustNZ(m_reg.B); SetFlag(FLAG_V, false); }; // TAB
		m_opcodes[0x17] = [=]() { m_reg.A = m_reg.B; AdjustNZ(m_reg.A); SetFlag(FLAG_V, false); }; // TBA

		//TODO m_opcodes[0x19] = [=]() {}; // DAA

		//TODO m_opcodes[0x1B] = [=]() {}; // ABA

		m_opcodes[0x20] = [=]() { BRA(true); }; // BRA

		m_opcodes[0x22] = [=]() { BRA((GetFlag(FLAG_Z) == false) && (GetFlag(FLAG_C) == false)); }; // BHI
		m_opcodes[0x23] = [=]() { BRA((GetFlag(FLAG_Z) == true) || (GetFlag(FLAG_C) == true)); }; // BLS
		m_opcodes[0x24] = [=]() { BRA(GetFlag(FLAG_C) == false); }; // BCC / BHS
		m_opcodes[0x25] = [=]() { BRA(GetFlag(FLAG_C) == true); }; // BCS / BLO
		m_opcodes[0x26] = [=]() { BRA(GetFlag(FLAG_Z) == false); }; // BNE
		m_opcodes[0x27] = [=]() { BRA(GetFlag(FLAG_Z) == true); }; // BEQ
		m_opcodes[0x28] = [=]() { BRA(GetFlag(FLAG_V) == false); }; // BVC
		m_opcodes[0x29] = [=]() { BRA(GetFlag(FLAG_V) == true); }; // BVS
		m_opcodes[0x2A] = [=]() { BRA(GetFlag(FLAG_N) == false); }; // BPL
		m_opcodes[0x2B] = [=]() { BRA(GetFlag(FLAG_N) == true); }; // BMI
		m_opcodes[0x2C] = [=]() { BRA(GetFlag(FLAG_N) == GetFlag(FLAG_V)); }; // BGE
		m_opcodes[0x2D] = [=]() { BRA(GetFlag(FLAG_N) != GetFlag(FLAG_V)); }; // BLT
		m_opcodes[0x2E] = [=]() { BRA((GetFlag(FLAG_N) == GetFlag(FLAG_V)) && (GetFlag(FLAG_Z) == false)); }; // BGT
		m_opcodes[0x2F] = [=]() { BRA((GetFlag(FLAG_N) != GetFlag(FLAG_V)) || (GetFlag(FLAG_Z) == true)); }; // BLE

		// LDA A
		m_opcodes[0x86] = [=]() { LD8(m_reg.A, FetchByte()); }; // LDA A imm
		m_opcodes[0x96] = [=]() { LD8(m_reg.A, GetMemDirectByte()); }; // LDA A dir
		m_opcodes[0xA6] = [=]() { LD8(m_reg.A, GetMemIndexedByte()); }; // LDA A idx
		m_opcodes[0xB6] = [=]() { LD8(m_reg.A, GetMemExtendedByte()); }; // LDA A ext

		// LDA B
		m_opcodes[0xC6] = [=]() { LD8(m_reg.B, FetchByte()); }; // LDA B imm
		m_opcodes[0xD6] = [=]() { LD8(m_reg.B, GetMemDirectByte()); }; // LDA B dir
		m_opcodes[0xE6] = [=]() { LD8(m_reg.B, GetMemIndexedByte()); }; // LDA B idx
		m_opcodes[0xF6] = [=]() { LD8(m_reg.B, GetMemExtendedByte()); }; // LDA B ext

		// STA A
		m_opcodes[0x97] = [=]() { ST8(GetDirect(), m_reg.A); }; // STA A dir
		m_opcodes[0xA7] = [=]() { ST8(GetIndexed(), m_reg.A); }; // STA A idx
		m_opcodes[0xB7] = [=]() { ST8(GetExtended(), m_reg.A); }; // STA A ext

		// STA B
		m_opcodes[0xD7] = [=]() { ST8(GetDirect(), m_reg.B); }; // STA B dir
		m_opcodes[0xE7] = [=]() { ST8(GetIndexed(), m_reg.B); }; // STA B idx
		m_opcodes[0xF7] = [=]() { ST8(GetExtended(), m_reg.B); }; // STA B ext

		// RTS
		m_opcodes[0x39] = [=]() { RTS(); }; // RTS

		// JSR
		m_opcodes[0xAD] = [=]() { JSR(GetIndexed()); }; // JSR idx
		m_opcodes[0xBD] = [=]() { JSR(GetExtended()); }; // JSR ext

		// JMP
		m_opcodes[0x6E] = [=]() { JMP(GetIndexed()); }; // JMP idx
		m_opcodes[0x7E] = [=]() { JMP(GetExtended()); }; // JMP ext

		// LDS
		m_opcodes[0x8E] = [=]() { LD16(m_reg.SP, FetchWord()); }; // LDS imm
		m_opcodes[0x9E] = [=]() { LD16(m_reg.SP, GetMemDirectWord()); }; // LDS dir
		m_opcodes[0xAE] = [=]() { LD16(m_reg.SP, GetMemIndexedWord()); }; // LDS idx
		m_opcodes[0xBE] = [=]() { LD16(m_reg.SP, GetMemExtendedWord()); }; // LDS ext

		// LDX
		m_opcodes[0xCE] = [=]() { LD16(m_reg.IX, FetchWord()); }; // LDX imm
		m_opcodes[0xDE] = [=]() { LD16(m_reg.IX, GetMemDirectWord()); }; // LDX dir
		m_opcodes[0xEE] = [=]() { LD16(m_reg.IX, GetMemIndexedWord()); }; // LDX idx
		m_opcodes[0xFE] = [=]() { LD16(m_reg.IX, GetMemExtendedWord()); }; // LDX ext

		// STS
		m_opcodes[0x9F] = [=]() { ST16(GetDirect(), m_reg.SP); }; // STS dir
		m_opcodes[0xAF] = [=]() { ST16(GetIndexed(), m_reg.SP); }; // STS idx
		m_opcodes[0xBF] = [=]() { ST16(GetExtended(), m_reg.SP); }; // STS ext

		// STX
		m_opcodes[0xDF] = [=]() { ST16(GetDirect(), m_reg.IX); }; // STX dir
		m_opcodes[0xEF] = [=]() { ST16(GetIndexed(), m_reg.IX); }; // STX idx
		m_opcodes[0xFF] = [=]() { ST16(GetExtended(), m_reg.IX); }; // STX ext

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
		SetFlag(FLAG_I, true);
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

	WORD CPU6800::FetchWord()
	{
		// Big endian
		BYTE h = FetchByte();
		BYTE l = FetchByte();

		return MakeWord(h, l);
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
		LogPrintf(LOG_ERROR, "A = %02X", m_reg.A);
		LogPrintf(LOG_ERROR, "B = %02X", m_reg.B);
		LogPrintf(LOG_ERROR, "X = %02X", m_reg.IX);
		LogPrintf(LOG_ERROR, "Flags --HINZVC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_reg.flags));
		LogPrintf(LOG_ERROR, "SP = %04X", m_reg.SP);
		LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
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
		ADDRESS vector = 0xFFFF;

		if (m_nmi.IsLatched())
		{
			LogPrintf(LOG_INFO, "[%zu] NMI", emul::g_ticks);
			m_nmi.ResetLatch();
			vector = ADDR_NMI;
		}
		else if (m_irq && !GetFlag(FLAG_I))
		{
			LogPrintf(LOG_INFO, "[%zu] IRQ", emul::g_ticks);
			vector = ADDR_IRQ;
		}

		if (vector != 0xFFFF)
		{
			SetFlag(FLAG_I, true);   // Disable IRQ
			m_programCounter = m_memory.Read16be(vector);
		}
	}

	void CPU6800::AdjustNZ(BYTE val)
	{
		SetFlag(FLAG_N, GetMSB(val));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU6800::AdjustNZ(WORD val)
	{
		SetFlag(FLAG_N, GetMSB(val));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU6800::pushAll()
	{
		pushW(m_PC);
		pushW(m_reg.IX);
		pushB(m_reg.A);
		pushB(m_reg.B);
		pushB(m_reg.flags);
	}

	void CPU6800::popAll()
	{
		SetFlags(popB());
		m_reg.B = popB();
		m_reg.A = popB();
		m_reg.IX = popW();
		m_programCounter = popW();
	}

	void CPU6800::pushB(BYTE value)
	{
		m_memory.Write8(--m_reg.SP, value);
	}

	void CPU6800::pushW(WORD value)
	{
		pushB(GetLByte(value));
		pushB(GetHByte(value));
	}

	BYTE CPU6800::popB()
	{
		return m_memory.Read8(m_reg.SP++);
	}

	WORD CPU6800::popW()
	{
		BYTE h = popB();
		BYTE l = popB();
		return MakeWord(h, l);
	}

	// Branching
	void CPU6800::BRA(bool condition)
	{
		SBYTE rel = FetchSignedByte();
		if (condition == true)
		{
			m_PC += rel;
		}
	}


	void CPU6800::JSR(ADDRESS dest)
	{
		pushW(m_PC);
		m_programCounter = dest;
	}

	void CPU6800::RTS()
	{
		m_programCounter = popW();
	}
	
	void CPU6800::LD8(BYTE& dest, BYTE src)
	{
		dest = src;
		AdjustNZ(dest);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6800::LD16(WORD& dest, WORD src)
	{
		dest = src;
		AdjustNZ(dest);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6800::ST8(ADDRESS dest, BYTE src)
	{
		m_memory.Write8(dest, src);

		AdjustNZ(src);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6800::ST16(ADDRESS dest, WORD src)
	{
		m_memory.Write16be(dest, src);

		AdjustNZ(src);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6800::Serialize(json& to)
	{
		to["opcode"] = m_opcode;

		m_nmi.Serialize(to["nmi"]);
		to["irq"] = m_irq;

		to["a"] = m_reg.A;
		to["b"] = m_reg.B;
		to["ix"] = m_reg.IX;
		to["sp"] = m_reg.SP;
		to["pc"] = m_programCounter;
		to["flags"] = m_reg.flags;
	}
	void CPU6800::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];

		m_nmi.Deserialize(from["nmi"]);
		m_irq = from["irq"];

		m_reg.A = from["a"];
		m_reg.B = from["b"];
		m_reg.IX = from["ix"];
		m_reg.SP = from["sp"];
		m_programCounter = from["pc"];
		m_reg.flags = from["flags"];
	}
}
