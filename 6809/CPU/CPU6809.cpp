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

		m_opcodes[0x10] = [=]() { ExecPage2(FetchByte()); }; // Page 2 sub intructions
		m_opcodes[0x11] = [=]() { ExecPage3(FetchByte()); }; // Page 3 sub intructions

		m_opcodes[0x1F] = [=]() { TFR(FetchByte()); }; // TFR r,r

		m_opcodes[0x20] = [=]() { BRA(true); }; // BRA
		m_opcodes[0x21] = [=]() { BRA(false); }; // BRN

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

		m_opcodes[0x80] = [=]() { SUB8(m_reg.ab.A, FetchByte()); }; // SUBA imm
		m_opcodes[0x81] = [=]() { CMP8(m_reg.ab.A, FetchByte()); }; // CMPA imm
		m_opcodes[0x83] = [=]() { SUB16(m_reg.D, FetchWord()); }; // SUBD imm
		m_opcodes[0x86] = [=]() { LD8(m_reg.ab.A, FetchByte()); }; // LDA imm
		m_opcodes[0x8C] = [=]() { CMP16(m_reg.X, FetchWord()); }; // CMPX imm
		m_opcodes[0x8E] = [=]() { LD16(m_reg.X, FetchWord()); }; // LDX imm

		m_opcodes[0x90] = [=]() { SUB8(m_reg.ab.A, FetchDirectByte()); }; // SUBA direct
		m_opcodes[0x91] = [=]() { CMP8(m_reg.ab.A, FetchDirectByte()); }; // CMPA direct
		m_opcodes[0x93] = [=]() { SUB16(m_reg.D, FetchDirectWord()); }; // SUBD direct
		m_opcodes[0x96] = [=]() { LD8(m_reg.ab.A, FetchDirectByte()); }; // LDA direct
		m_opcodes[0x97] = [=]() { ST8Direct(m_reg.ab.A); }; // STA direct
		m_opcodes[0x9C] = [=]() { CMP16(m_reg.X, FetchDirectWord()); }; // CMPX direct
		m_opcodes[0x9E] = [=]() { LD16(m_reg.X, FetchDirectWord()); }; // LDX direct
		m_opcodes[0x9F] = [=]() { ST16Direct(m_reg.X); }; // STU direct

		m_opcodes[0xC0] = [=]() { SUB8(m_reg.ab.B, FetchByte()); }; // SUBB imm
		m_opcodes[0xC1] = [=]() { CMP8(m_reg.ab.B, FetchByte()); }; // CMPB imm
		m_opcodes[0xC6] = [=]() { LD8(m_reg.ab.B, FetchByte()); }; // LDB imm
		m_opcodes[0xCC] = [=]() { LD16(m_reg.D, FetchWord()); }; // LDD imm
		m_opcodes[0xCE] = [=]() { LD16(m_reg.U, FetchWord()); }; // LDU imm

		m_opcodes[0xD0] = [=]() { SUB8(m_reg.ab.B, FetchDirectByte()); }; // SUBB direct
		m_opcodes[0xD1] = [=]() { CMP8(m_reg.ab.B, FetchDirectByte()); }; // CMPB direct
		m_opcodes[0xD6] = [=]() { LD8(m_reg.ab.B, FetchDirectByte()); }; // LDB direct
		m_opcodes[0xD7] = [=]() { ST8Direct(m_reg.ab.B); }; // STB direct
		m_opcodes[0xDC] = [=]() { LD16(m_reg.D, FetchDirectWord()); }; // LDD direct
		m_opcodes[0xDD] = [=]() { ST16Direct(m_reg.D); }; // STD direct
		m_opcodes[0xDE] = [=]() { LD16(m_reg.U, FetchDirectWord()); }; // LDU direct
		m_opcodes[0xDF] = [=]() { ST16Direct(m_reg.U); }; // STU direct


		InitPage2(); // Prefix 0x10
		InitPage3(); // Prefix 0x11
	}

	// Prefix 0x10
	void CPU6809::InitPage2()
	{
		// Start with main opcode table for fallthrough
		m_opcodesPage2 = m_opcodes;

		// Override with actual page 2 opcodes

		m_opcodesPage2[0x21] = [=]() { LBRA(false); }; // LBRN

		m_opcodesPage2[0x22] = [=]() { LBRA((GetFlag(FLAG_Z) == false) && (GetFlag(FLAG_C) == false)); }; // LBHI
		m_opcodesPage2[0x23] = [=]() { LBRA((GetFlag(FLAG_Z) == true) || (GetFlag(FLAG_C) == true)); }; // LBLS

		m_opcodesPage2[0x24] = [=]() { LBRA(GetFlag(FLAG_C) == false); }; // LBCC / LBHS
		m_opcodesPage2[0x25] = [=]() { LBRA(GetFlag(FLAG_C) == true); }; // LBCS / LBLO

		m_opcodesPage2[0x26] = [=]() { LBRA(GetFlag(FLAG_Z) == false); }; // LBNE
		m_opcodesPage2[0x27] = [=]() { LBRA(GetFlag(FLAG_Z) == true); }; // LBEQ

		m_opcodesPage2[0x28] = [=]() { LBRA(GetFlag(FLAG_V) == false); }; // LBVC
		m_opcodesPage2[0x29] = [=]() { LBRA(GetFlag(FLAG_V) == true); }; // LBVS

		m_opcodesPage2[0x2A] = [=]() { LBRA(GetFlag(FLAG_N) == false); }; // LBPL
		m_opcodesPage2[0x2B] = [=]() { LBRA(GetFlag(FLAG_N) == true); }; // LBMI

		m_opcodesPage2[0x2C] = [=]() { LBRA(GetFlag(FLAG_N) == GetFlag(FLAG_V)); }; // LBGE
		m_opcodesPage2[0x2D] = [=]() { LBRA(GetFlag(FLAG_N) != GetFlag(FLAG_V)); }; // LBLT

		m_opcodesPage2[0x2E] = [=]() { LBRA((GetFlag(FLAG_N) == GetFlag(FLAG_V)) && (GetFlag(FLAG_Z) == false)); }; // LBGT
		m_opcodesPage2[0x2F] = [=]() { LBRA((GetFlag(FLAG_N) != GetFlag(FLAG_V)) || (GetFlag(FLAG_Z) == true)); }; // LBLE

		m_opcodesPage2[0x83] = [=]() { CMP16(m_reg.D, FetchWord()); }; // CMPD imm
		m_opcodesPage2[0x8C] = [=]() { CMP16(m_reg.Y, FetchWord()); }; // CMPY imm
		m_opcodesPage2[0x8E] = [=]() { LD16(m_reg.Y, FetchWord()); }; // LDY imm

		m_opcodesPage2[0x93] = [=]() { CMP16(m_reg.D, FetchDirectWord()); }; // CMPD direct
		m_opcodesPage2[0x9C] = [=]() { CMP16(m_reg.Y, FetchDirectWord()); }; // CMPY direct
		m_opcodesPage2[0x9E] = [=]() { LD16(m_reg.Y, FetchDirectWord()); }; // LDY direct
		m_opcodesPage2[0x9F] = [=]() { ST16Direct(m_reg.Y); }; // STY direct

		m_opcodesPage2[0xCE] = [=]() { LD16(m_reg.S, FetchWord()); }; // LDS imm
		m_opcodesPage2[0xDE] = [=]() { LD16(m_reg.S, FetchDirectWord()); }; // LDS direct
		m_opcodesPage2[0xDF] = [=]() { ST16Direct(m_reg.S); }; // STS direct
	}

	// Prefix 0x11
	void CPU6809::InitPage3()
	{
		// Start with main opcode table for fallthrough
		m_opcodesPage3 = m_opcodes;

		// Override actual page 3 opcodes

		m_opcodesPage3[0x83] = [=]() { CMP16(m_reg.U, FetchWord()); }; // CMPU imm
		m_opcodesPage3[0x8C] = [=]() { CMP16(m_reg.S, FetchWord()); }; // CMPS imm

		m_opcodesPage3[0x93] = [=]() { CMP16(m_reg.U, FetchDirectWord()); }; // CMPU direct
		m_opcodesPage3[0x9C] = [=]() { CMP16(m_reg.S, FetchDirectWord()); }; // CMPS direct
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

	WORD CPU6809::FetchWord()
	{
		// Big endian
		BYTE h = FetchByte();
		BYTE l = FetchByte();

		return MakeWord(h, l);
	}

	BYTE CPU6809::FetchDirectByte()
	{
		ADDRESS address = MakeWord(m_reg.DP, FetchByte());
		return m_memory.Read8(address);
	}

	WORD CPU6809::FetchDirectWord()
	{
		ADDRESS address = MakeWord(m_reg.DP, FetchByte());
		return m_memory.Read16be(address);
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

	// TODO: handle: - multiples prefixes
	//               - adjust timing when fallthrough to main table instructions
	void CPU6809::ExecPage2(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP1, op2);
		exec(m_opcodesPage2, op2);
	}

	void CPU6809::ExecPage3(BYTE op2)
	{
		// TODO: handle multiples prefixes
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP2, op2);
		exec(m_opcodesPage3, op2);
	}

	void CPU6809::exec(OpcodeTable& table, BYTE opcode)
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
			LogPrintf(LOG_ERROR, "CPU: Exception at address 0x%04X, subopcode 0x%02X! Stopping CPU", m_programCounter, opcode);
			m_state = CPUState::STOP;
		}
	}

	void CPU6809::Interrupt()
	{
	}

	// Branching
	void CPU6809::BRA(bool condition)
	{
		int8_t rel = (int8_t)FetchByte();
		if (condition == true)
		{
			m_programCounter += rel;
			// No time penalty for short branches
		}
	}

	void CPU6809::LBRA(bool condition)
	{
		int16_t rel = (int16_t)FetchWord();
		if (condition == true)
		{
			m_programCounter += rel;
			TICKT3();
		}
	}

	void CPU6809::AdjustNZ(BYTE val)
	{
		SetFlag(FLAG_N, GetBit(val, 7));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU6809::AdjustNZ(WORD val)
	{
		SetFlag(FLAG_N, GetBit(val, 15));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU6809::LD8(BYTE& dest, BYTE src)
	{
		dest = src;
		AdjustNZ(dest);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6809::LD16(WORD& dest, WORD src)
	{
		dest = src;
		AdjustNZ(dest);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6809::ST8Direct(BYTE src)
	{

	}
	void CPU6809::ST16Direct(WORD src)
	{

	}

	WORD CPU6809::GetReg(RegCode code) const
	{
		switch (code)
		{
		case RegCode::D: return m_reg.D;
		case RegCode::X: return m_reg.X;
		case RegCode::Y: return m_reg.Y;
		case RegCode::U: return m_reg.U;
		case RegCode::S: return m_reg.S;
		case RegCode::PC: return m_programCounter;

		case RegCode::A: return MakeWord(0xFF, m_reg.ab.A);
		case RegCode::B: return MakeWord(0xFF, m_reg.ab.B);
		case RegCode::CC: return MakeWord(m_reg.flags, m_reg.flags);
		case RegCode::DP: return MakeWord(m_reg.DP, m_reg.DP);

		default: return 0xFFFF;
		}
	}

	BYTE& CPU6809::GetReg8(RegCode code)
	{
		assert(GetBit((BYTE)code, 3) == true);

		switch (code)
		{
		case RegCode::A:  return m_reg.ab.A;
		case RegCode::B:  return m_reg.ab.B;
		case RegCode::CC: return m_reg.flags;
		case RegCode::DP: return m_reg.DP;

		default: return m_reg.void8;
		}
	}

	WORD& CPU6809::GetReg16(RegCode code)
	{
		assert(GetBit((BYTE)code, 3) == false);

		switch (code)
		{
		case RegCode::D:  return m_reg.D;
		case RegCode::X:  return m_reg.X;
		case RegCode::Y:  return m_reg.Y;
		case RegCode::U:  return m_reg.U;
		case RegCode::S:  return m_reg.S;
		case RegCode::PC: return m_PC;

		default: return m_reg.void16;
		}
	}

	void CPU6809::TFR(BYTE sd)
	{
		const WORD source = GetReg(GetSourceRegCode(sd));

		const RegCode destCode = GetDestRegCode(sd);

		if (isDestRegWide(sd))
		{
			GetReg16(destCode) = source;
		}
		else
		{
			GetReg8(destCode) = GetLByte(source);
		}
	}

	// TODO: Validate flags
	void CPU6809::SUB8(BYTE& dest, BYTE src, bool borrow)
	{
		BYTE oldDest = dest;

		// AC flag
		BYTE loNibble = (dest & 0x0F) - (src & 0x0F);

		WORD temp = dest - src;
		if (borrow)
		{
			temp--;
			loNibble--;
		}

		dest = (BYTE)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_H, (loNibble > 0x0F));
		SetFlag(FLAG_C, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	// TODO: Validate flags
	void CPU6809::SUB16(WORD& dest, WORD src, bool borrow)
	{
		WORD oldDest = dest;

		DWORD temp = dest - src;
		if (borrow)
		{
			temp--;
		}

		dest = (WORD)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_C, (temp > 0xFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
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
