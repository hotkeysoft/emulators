#include "stdafx.h"
#include "CPU68000.h"

using cpuInfo::Opcode;

namespace emul::cpu68k
{
	static const BitMaskW Mask_EXT("000xxx");

	CPU68000::CPU68000(Memory& memory) : CPU68000("68000", memory)
	{
	}

	CPU68000::CPU68000(const char* cpuid, Memory& memory) :
		CPU(CPU68000_ADDRESS_BITS, memory),
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

	CPU68000::~CPU68000()
	{
	}

	void CPU68000::Init()
	{
		InitTable(m_opcodes, 16);

		m_opcodes[0b0000] = [=]() { Exec(0, GetSubopcode6()); }; // Immediate,Bit,MOVEP
		m_opcodes[0b0001] = [=]() { MOVEb(); }; // MOVE.b
		m_opcodes[0b0010] = [=]() { MOVEl(); }; // MOVE.l, MOVEA.l
		m_opcodes[0b0011] = [=]() { MOVEw(); }; // MOVE.w,MOVEA.w
		m_opcodes[0b0100] = [=]() { Exec(4, GetSubopcode6()); }; // Misc
		m_opcodes[0b0101] = [=]() { Exec(5, GetSubopcode6()); }; // ADDQ,SUBQ,Scc,DBcc
		m_opcodes[0b0110] = [=]() { Exec(6, GetSubopcode4()); }; // BRA,BSR,Bcc
		m_opcodes[0b0111] = [=]() { MOVEQ(); };
		m_opcodes[0b1000] = [=]() { Exec(8, GetSubopcode6()); }; // DIVU,DIVS,SBCD,OR
		m_opcodes[0b1001] = [=]() { Exec(9, GetSubopcode6()); }; // SUB,SUBX,SUBA
		m_opcodes[0b1010] = [=]() { InvalidOpcode(); };
		m_opcodes[0b1011] = [=]() { Exec(11, GetSubopcode6()); }; // EOR,CMPM,CMP,CMPA
		m_opcodes[0b1100] = [=]() { Exec(12, GetSubopcode6()); }; // MULU,MULS,ABCD,EXG,AND
		m_opcodes[0b1101] = [=]() { Exec(13, GetSubopcode6()); }; // ADD,ADDX,ADDA
		m_opcodes[0b1110] = [=]() { SHIFT(); }; // Shift, Rotate
		m_opcodes[0b1111] = [=]() { InvalidOpcode(); };

		InitGroup0(m_subOpcodes[0], 64);
		InitGroup4(m_subOpcodes[4], 64);
		InitGroup5(m_subOpcodes[5], 64);
		InitGroup6(m_subOpcodes[6], 16);
		InitGroup8(m_subOpcodes[8], 64);
		InitGroup9(m_subOpcodes[9], 64);
		InitGroup11(m_subOpcodes[11], 64);
		InitGroup12(m_subOpcodes[12], 64);
		InitGroup13(m_subOpcodes[13], 64);
	}

	void CPU68000::InitTable(OpcodeTable& table, size_t size)
	{
		table.resize(size);
		std::fill(table.begin(), table.end(), [=]() { InvalidOpcode(); });
	}

	// b0000: Immediate,Bit,MOVEP
	void CPU68000::InitGroup0(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b0100: Misc
	void CPU68000::InitGroup4(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// MOVEM <register list>, <ea>
		table[042] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEM_w_toMem(FetchWord()); };
		table[043] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEM_l_toMem(FetchWord()); };

		// MOVEM <ea>, <register list>
		table[062] = [=]() { MOVEM_w_fromMem(FetchWord()); };
		table[063] = [=]() { MOVEM_l_fromMem(FetchWord()); };
	}

	// b0101: ADDQ,SUBQ,Scc,DBcc
	void CPU68000::InitGroup5(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b0110: BRA,BSR,Bcc
	void CPU68000::InitGroup6(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b1000: DIVU,DIVS,SBCD,OR
	void CPU68000::InitGroup8(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b1001: SUB,SUBX,SUBA
	void CPU68000::InitGroup9(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b1011: EOR,CMPM,CMP,CMPA
	void CPU68000::InitGroup11(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b1100: MULU,MULS,ABCD,EXG,AND
	void CPU68000::InitGroup12(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	// b1101: ADD,ADDX,ADDA
	void CPU68000::InitGroup13(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);
	}

	void CPU68000::Reset()
	{
		CPU::Reset();

		// TODO: Check if values are reset
		m_reg.SSP = 0;
		m_reg.USP = 0;

		std::fill(std::begin(m_reg.DataAddress), std::end(m_reg.DataAddress), 0);

		ADDRESS reset = m_memory.Read32be(GetVectorAddress(VECTOR::ResetPC));
		ADDRESS ssp = m_memory.Read32be(GetVectorAddress(VECTOR::ResetSSP));

		LogPrintf(LOG_INFO, "Initial PC:  %08X", reset);
		LogPrintf(LOG_INFO, "Initial SSP: %08X", ssp);
		m_programCounter = reset;
		m_reg.SSP = ssp;

		ClearFlags(m_reg.flags);
		SetSupervisorMode(true);
		SetInterruptMask(7);
		SetTrace(false);
	}

	void CPU68000::Reset(ADDRESS overrideAddress)
	{
		Reset();
		m_programCounter = overrideAddress;
	}

	void CPU68000::ClearFlags(WORD& flags)
	{
		flags = 0;
	}

	void CPU68000::SetFlags(WORD f)
	{
		bool supervisorMode = (f & FLAG_S);
		SetSupervisorMode(supervisorMode);

		SetBitMask(f, FLAG_RESERVED_OFF, false);
		m_reg.flags = f;
	}

	void CPU68000::SetSupervisorMode(bool newMode)
	{
		const bool currMode = IsSupervisorMode();
		if (newMode == currMode)
			return; // Nothing to do

		if (currMode) // Supervisor -> User
		{
			// Save A7 in SSP
			m_reg.SSP = m_reg.ADDR[7];
			// Put USP in A7
			m_reg.ADDR[7] = m_reg.USP;
		}
		else // User -> Supervisor
		{
			// Save A7 in USP
			m_reg.USP = m_reg.ADDR[7];
			// Put SSP in A7
			m_reg.ADDR[7] = m_reg.SSP;
		}

		SetFlag(FLAG_S, newMode);
	}

	// 68000 only fetches Words
	BYTE CPU68000::FetchByte()
	{
		throw std::exception("No Byte Access allowed");
		return 0;
	}

	WORD CPU68000::FetchWord()
	{
		WORD w = m_memory.Read16be(GetCurrentAddress());
		m_programCounter += 2;
		m_programCounter &= ADDRESS_MASK;
		return w;
	}

	DWORD CPU68000::FetchLong()
	{
		DWORD dw = m_memory.Read32be(GetCurrentAddress());
		m_programCounter += 4;
		m_programCounter &= ADDRESS_MASK;
		return dw;
	}

	// 68000 only fetches Words
	void CPU68000::Exec(BYTE opcode)
	{
		throw std::exception("Not used");
	}

	bool CPU68000::InternalStep()
	{
		try
		{
			m_opTicks = 0;

			// Fetch opcode
			m_state = CPUState::RUN;

			// Execute instruction
			WORD op = FetchWord();
			Exec(op);
		}
		catch (std::exception e)
		{
			EnableLog(LOG_ERROR);
			LogPrintf(LOG_ERROR, "Error processing instruction at 0x%04X! [%s] Stopping CPU.\n", GetCurrentAddress(), e.what());
			m_state = CPUState::STOP;
		}

		return (m_state == CPUState::RUN);
	}

	bool CPU68000::Step()
	{
		bool ret = true;
		if (m_state != CPUState::HALT)
		{
			ret = InternalStep();
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

	void CPU68000::Dump()
	{
		for (int i = 0; i < 8; ++i)
		{
			LogPrintf(LOG_ERROR, "DATA[%d] = %04X", i, m_reg.DATA[i]);
		}
		for (int i = 0; i < 8; ++i)
		{
			LogPrintf(LOG_ERROR, "ADDR[%d] = %04X", i, m_reg.ADDR[i]);
		}

		LogPrintf(LOG_ERROR, "Flags TTSM0III000XNZVC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT16, PRINTF_BYTE_TO_BIN_INT16(m_reg.flags));
		LogPrintf(LOG_ERROR, "USP = %04X", m_reg.USP);
		LogPrintf(LOG_ERROR, "SSP = %04X", m_reg.SSP);
		LogPrintf(LOG_ERROR, "PC  = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
	}

	void CPU68000::NotImplementedOpcode(const char* name)
	{
		LogPrintf(LOG_ERROR, "CPU: NotImplemented Opcode [%s](0x%04X) at address 0x%08X", name, m_opcode, m_programCounter);
		throw std::exception("Not implemented");
	}

	void CPU68000::InvalidOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%04X) at address 0x%08X", m_opcode, m_programCounter);
		Dump();
		throw std::exception("Unknown opcode");
	}

	void CPU68000::Exec(WORD opcode)
	{
		m_opcode = opcode;

		// Get group from upper 4 bits
		WORD opGroup = (opcode >> 12) & 15;

		m_currTiming = &m_info.GetOpcodeTiming(opGroup);

		try
		{
			// Fetch the function corresponding to the opcode and run it
			{
				auto& opFunc = m_opcodes[opGroup];
				opFunc();
			}

			TICK();
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "CPU: Exception at address 0x%08X! Stopping CPU", m_programCounter);
			m_state = CPUState::STOP;
		}
	}

	void CPU68000::Exec(WORD group, WORD subOpcode)
	{
		auto& opFunc = m_subOpcodes[group][subOpcode];
		opFunc();
	}

	void CPU68000::Interrupt()
	{

	}

	BYTE CPU68000::GetEAByte()
	{
		constexpr WORD RegisterMask = 0b000111;
		const WORD reg = m_opcode & RegisterMask;

		switch (GetEAMode(m_opcode))
		{
		case EAMode::DataRegDirect:
			return GetLByte(m_reg.DATA[reg]);
		case EAMode::AddrRegDirect:
			return GetLByte(m_reg.ADDR[reg]);
		case EAMode::Immediate:
			return FetchLong();

		default: return GetLByte(m_memory.Read16be(GetEA(OP_BYTE)));
		}
	}

	WORD CPU68000::GetEAWord()
	{
		constexpr WORD RegisterMask = 0b000111;
		const WORD reg = m_opcode & RegisterMask;

		switch (m_eaMode = GetEAMode(m_opcode))
		{
		case EAMode::DataRegDirect:
			return GetLWord(m_reg.DATA[reg]);
		case EAMode::AddrRegDirect:
			return GetLWord(m_reg.ADDR[reg]);
		case EAMode::Immediate:
			return FetchLong();

		default: return m_memory.Read16be(GetEA(OP_WORD));
		}
	}

	DWORD CPU68000::GetEALong()
	{
		constexpr WORD RegisterMask = 0b000111;
		const WORD reg = m_opcode & RegisterMask;

		switch (m_eaMode = GetEAMode(m_opcode))
		{
		case EAMode::DataRegDirect:
			return m_reg.DATA[reg];
		case EAMode::AddrRegDirect:
			return m_reg.ADDR[reg];
		case EAMode::Immediate:
			return FetchLong();

		default: return m_memory.Read32be(GetEA(OP_LONG));
		}
	}

	EAMode CPU68000::GetEAMode(WORD opcode)
	{
		constexpr WORD ModeMask = 0b111000;
		constexpr WORD ExtModeMask = 0b000111;

		const WORD mode = opcode & ModeMask;

		if (mode != ModeMask)
		{
			return EAMode(mode);
		}
		else switch (opcode & ExtModeMask)
		{
		case 0b000: return EAMode::AbsoluteShort;
		case 0b001: return EAMode::AbsoluteLong;
		case 0b010: return EAMode::ProgramCounterDisplacement;
		case 0b011: return EAMode::ProgramCounterIndex;
		case 0b100: return EAMode::Immediate;
		default: return EAMode::Invalid;
		}
	}

	ADDRESS CPU68000::GetEA(int size)
	{
		assert(size == 1 || size == 2 || size == 4);

		constexpr WORD RegisterMask = 0b000111;
		const WORD reg = m_opcode & RegisterMask;

		DWORD& An = m_reg.ADDR[reg];

		ADDRESS ea = 0;
		switch (m_eaMode = GetEAMode(m_opcode))
		{
		// Register Direct modes
		case EAMode::DataRegDirect: // Data Register Direct, EA = Dn
		case EAMode::AddrRegDirect: // Address Register Direct, EA = An
			InvalidOpcode();
			break;

		// Memory Address modes

		// Address Register Indirect, EA = (An)
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AddrRegIndirect:
			ea = An;
			break;

		// Address Register Indirect with Postincrement, EA=(An), An += N
		// TODO: Data reference
		case EAMode::AddrRegIndirectPostIncrement:
			ea = An;
			An += size;
			break;

		// Address Register Indirect with Predecrement, AN -= N, EA=(An)
		// TODO: Data reference
		case EAMode::AddrRegIndirectPreDecrement:
			An -= size;
			ea = An;
			break;

		// Address Register Indirect with Displacement, EA = (An) + d
		case EAMode::AddrRegIndirectDisplacement:
			NotImplementedOpcode("GetMemAddress: Address Register Indirect with Displacement");
			break;

		// Address Register Indirect with Index, EA = (An) + (Ri) + d
		case EAMode::AddrRegIndirectIndex:
			NotImplementedOpcode("GetMemAddress: Address Register Indirect with Index");
			break;

		// Absolute Short Address, EA given
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AbsoluteShort:
			ea = Widen(FetchWord());
			break;

		// Absolute Long Address, EA given
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AbsoluteLong:
			ea = FetchLong();
			break;

		// Program Counter with Displacement, EA = (PC) + d
		// TODO: Program reference
		case EAMode::ProgramCounterDisplacement:
			NotImplementedOpcode("GetMemAddress: Program Counter with Displacement");
			break;

		// Program Counter with Index, EA = (PC) + (Ri) + d
		// TODO: Program reference
		case EAMode::ProgramCounterIndex:
			NotImplementedOpcode("GetMemAddress: Program Counter with Index");
			break;

		// Immediate Data
		case EAMode::Immediate:
		default:
			InvalidOpcode();
			break;
		}

		return ea & ADDRESS_MASK;
	}

	void CPU68000::MOVEM_l_fromMem(WORD regs)
	{
		ADDRESS src = GetEA(OP_LONG);

		bool predecrement = false;

		switch (m_eaMode)
		{
		case EAMode::AddrRegIndirectPreDecrement:
			ReverseBits(regs);
			break;
		case EAMode::DataRegDirect:
		case EAMode::AddrRegDirect:
		case EAMode::AddrRegIndirectPostIncrement:
		case EAMode::ProgramCounterDisplacement:
		case EAMode::ProgramCounterIndex:
		case EAMode::Immediate:
			InvalidOpcode();
			return;
		}

		DWORD* dest = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++dest)
		{
			if (GetLSB(regs))
			{
				*dest = m_memory.Read32be(src);
				src += 4;
			}

			regs >>= 1;
		}
	}

	void CPU68000::Serialize(json& to)
	{
		to["opcode"] = m_opcode;
		to["dataAddress"] = m_reg.DataAddress;
		to["usp"] = m_reg.USP;
		to["ssp"] = m_reg.SSP;
		to["flags"] = m_reg.flags;
		to["pc"] = m_programCounter;
	}
	void CPU68000::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];
		m_reg.DataAddress = from["dataAddress"];
		m_reg.USP = from["usp"];
		m_reg.SSP = from["ssp"];
		m_reg.flags = from["flags"];
		m_programCounter = from["pc"];
	}
}
