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
		m_opcodes[0b0001] = [=]() { MOVE_b(); }; // MOVE.b
		m_opcodes[0b0010] = [=]() { MOVE_l(); }; // MOVE.l, MOVEA.l
		m_opcodes[0b0011] = [=]() { MOVE_w(); }; // MOVE.w, MOVEA.w
		m_opcodes[0b0100] = [=]() { Exec(4, GetSubopcode6()); }; // Misc
		m_opcodes[0b0101] = [=]() { Exec(5, GetSubopcode6()); }; // ADDQ,SUBQ,Scc,DBcc
		m_opcodes[0b0110] = [=]() { Exec(6, GetSubopcode4()); }; // BRA,BSR,Bcc
		m_opcodes[0b0111] = [=]() { MOVEQ(); };
		m_opcodes[0b1000] = [=]() { Exec(8, GetSubopcode6()); }; // DIVU,DIVS,SBCD,OR
		m_opcodes[0b1001] = [=]() { Exec(9, GetSubopcode6()); }; // SUB,SUBX,SUBA
		m_opcodes[0b1010] = [=]() { IllegalInstruction(); };
		m_opcodes[0b1011] = [=]() { Exec(11, GetSubopcode6()); }; // EOR,CMPM,CMP,CMPA
		m_opcodes[0b1100] = [=]() { Exec(12, GetSubopcode6()); }; // MULU,MULS,ABCD,EXG,AND
		m_opcodes[0b1101] = [=]() { Exec(13, GetSubopcode6()); }; // ADD,ADDX,ADDA
		m_opcodes[0b1110] = [=]() { SHIFT(); }; // Shift, Rotate
		m_opcodes[0b1111] = [=]() { IllegalInstruction(); };

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
		std::fill(table.begin(), table.end(), [=]() { IllegalInstruction(); });
	}

	// b0000: Immediate,Bit,MOVEP
	void CPU68000::InitGroup0(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// CMPI #<data>, <ea>
		table[060] = [=]() { BYTE src = FetchByte(); CMP_b(GetEAByte(EAMode::GroupDataAlt), src); };
		table[061] = [=]() { WORD src = FetchWord(); CMP_w(GetEAWord(EAMode::GroupDataAlt), src); };
		table[062] = [=]() { DWORD src = FetchLong(); CMP_l(GetEALong(EAMode::GroupDataAlt), src); };

	}

	// b0100: Misc
	void CPU68000::InitGroup4(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[007] = [=]() { LEA(m_reg.ADDR[0]); }; // LEA <ea>,A0
		table[017] = [=]() { LEA(m_reg.ADDR[1]); }; // LEA <ea>,A1
		table[027] = [=]() { LEA(m_reg.ADDR[2]); }; // LEA <ea>,A2

		// MOVE <ea>, SR
		table[033] = [=]() { Privileged(); MOVE_w_toSR(GetEAWord(EAMode::GroupData)); };

		table[037] = [=]() { LEA(m_reg.ADDR[3]); }; // LEA <ea>,A3

		// MOVEM <register list>, <ea>
		table[042] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXT_w() : MOVEM_w_toEA(FetchWord()); };
		table[043] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXT_w() : MOVEM_l_toEA(FetchWord()); };

		table[047] = [=]() { LEA(m_reg.ADDR[4]); }; // LEA <ea>,A4
		table[057] = [=]() { LEA(m_reg.ADDR[5]); }; // LEA <ea>,A5

		// MOVEM <ea>, <register list>
		table[062] = [=]() { MOVEM_w_fromEA(FetchWord()); };
		table[063] = [=]() { MOVEM_l_fromEA(FetchWord()); };

		table[067] = [=]() { LEA(m_reg.ADDR[6]); }; // LEA <ea>,A6
		table[077] = [=]() { LEA(m_reg.ADDR[7]); }; // LEA <ea>,A7

//		table[072] = [=]() { JSR(); }; // JSR <ea>
		table[073] = [=]() { JMP(); }; // JMP <ea>

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

		table[000] = [=]() { BRA(); }; // BRA
		//table[001] = [=]() { BSR(); }; // BSR
		table[002] = [=]() { BRA(FlagHI()); }; // BHI
		table[003] = [=]() { BRA(FlagLS()); }; // BLS
		table[004] = [=]() { BRA(FlagCC()); }; // BCC
		table[005] = [=]() { BRA(FlagCS()); }; // BCS
		table[006] = [=]() { BRA(FlagNE()); }; // BNE
		table[007] = [=]() { BRA(FlagEQ()); }; // BEQ
		table[010] = [=]() { BRA(FlagVC()); }; // BVC
		table[011] = [=]() { BRA(FlagVS()); }; // BVS
		table[012] = [=]() { BRA(FlagPL()); }; // BPL
		table[013] = [=]() { BRA(FlagMI()); }; // BMI
		table[014] = [=]() { BRA(FlagGE()); }; // BGE
		table[015] = [=]() { BRA(FlagLT()); }; // BLT
		table[016] = [=]() { BRA(FlagGT()); }; // BGT
		table[017] = [=]() { BRA(FlagLE()); }; // BLE
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

		table[000] = [=]() { ANDb_fromEA(m_reg.D0b); }; // AND.b <ea>,D0
		table[001] = [=]() { ANDw_fromEA(m_reg.D0w); }; // AND.w <ea>,D0
		table[002] = [=]() { ANDl_fromEA(m_reg.D0); }; // AND.l <ea>,D0

		table[004] = [=]() { ANDb_toEA(m_reg.D0b); }; // AND.b D0,<ea>
		table[005] = [=]() { ANDw_toEA(m_reg.D0w); }; // AND.b D0,<ea>
		table[006] = [=]() { ANDl_toEA(m_reg.D0); }; // AND.b D0,<ea>
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

		ADDRESS reset = ReadL(GetVectorAddress(VECTOR::ResetPC));
		ADDRESS ssp = ReadL(GetVectorAddress(VECTOR::ResetSSP));

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
			m_reg.SSP = m_reg.A7;
			// Put USP in A7
			m_reg.A7 = m_reg.USP;
		}
		else // User -> Supervisor
		{
			// Save A7 in USP
			m_reg.USP = m_reg.A7;
			// Put SSP in A7
			m_reg.A7 = m_reg.SSP;
		}

		SetFlag(FLAG_S, newMode);
	}

	WORD CPU68000::FetchWord()
	{
		WORD w = ReadW(GetCurrentAddress());
		m_programCounter += 2;
		m_programCounter &= ADDRESS_MASK;
		return w;
	}

	DWORD CPU68000::FetchLong()
	{
		DWORD dw = ReadL(GetCurrentAddress());
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

	void CPU68000::IllegalInstruction()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%04X) at address 0x%08X", m_opcode, m_programCounter);
		Exception(VECTOR::IllegalInstruction);
	}

	// TODO: Temp
	void CPU68000::Exception(VECTOR v)
	{
		LogPrintf(LOG_ERROR, "CPU: Exception (%d) at address 0x%08X", v, m_programCounter);

		throw std::exception("Exception");
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

	void CPU68000::AdjustNZ(BYTE val)
	{
		SetFlag(FLAG_N, GetMSB(val));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU68000::AdjustNZ(WORD val)
	{
		SetFlag(FLAG_N, GetMSB(val));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU68000::AdjustNZ(DWORD val)
	{
		SetFlag(FLAG_N, GetMSB(val));
		SetFlag(FLAG_Z, val == 0);
	}

	BYTE CPU68000::GetEAByte(EAMode groupCheck)
	{
		m_eaMode = GetEAMode(m_opcode);
		EACheck(groupCheck);

		const int reg = GetOpcodeRegisterIndex();

		switch (GetEAMode(m_opcode))
		{
		case EAMode::DRegDirect:
			return GetLByte(m_reg.DATA[reg]);
		case EAMode::ARegDirect:
			return GetLByte(m_reg.ADDR[reg]);
		case EAMode::Immediate:
			return FetchByte();
		default:
			return ReadB(rawGetEA(OP_BYTE));
		}
	}

	WORD CPU68000::GetEAWord(EAMode groupCheck)
	{
		m_eaMode = GetEAMode(m_opcode);
		EACheck(groupCheck);

		const int reg = GetOpcodeRegisterIndex();

		switch (m_eaMode)
		{
		case EAMode::DRegDirect:
			return GetLWord(m_reg.DATA[reg]);
		case EAMode::ARegDirect:
			return GetLWord(m_reg.ADDR[reg]);
		case EAMode::Immediate:
			return FetchWord();
		default:
			return ReadW(rawGetEA(OP_WORD));
		}
	}

	DWORD CPU68000::GetEALong(EAMode groupCheck)
	{
		m_eaMode = GetEAMode(m_opcode);
		EACheck(groupCheck);

		const int reg = GetOpcodeRegisterIndex();

		switch (m_eaMode)
		{
		case EAMode::DRegDirect:
			return m_reg.DATA[reg];
		case EAMode::ARegDirect:
			return m_reg.ADDR[reg];
		case EAMode::Immediate:
			return FetchLong();
		default:
			return ReadL(rawGetEA(OP_LONG));
		}
	}

	EAMode CPU68000::GetEAMode(WORD opcode)
	{
		constexpr WORD ModeMask = 0b111000;
		constexpr WORD ExtModeMask = 0b000111;

		const WORD mode = opcode & ModeMask;

		if (mode != ModeMask)
		{
			return EAMode(1 << (mode >> 3));
		}
		else if (WORD extMode = (opcode & ExtModeMask); extMode <= 4)
		{
			return EAMode(1 << (extMode + 7));
		}
		else
		{
			return EAMode::Invalid;
		}
	}

	ADDRESS CPU68000::rawGetEA(int size)
	{
		assert(size == 1 || size == 2 || size == 4);

		DWORD& An = m_reg.ADDR[GetOpcodeRegisterIndex()];

		ADDRESS ea = 0;
		switch (m_eaMode)
		{
		// Register Direct modes
		case EAMode::DRegDirect: // Data Register Direct, EA = Dn
		case EAMode::ARegDirect: // Address Register Direct, EA = An
			IllegalInstruction();
			break;

		// Memory Address modes

		// Address Register Indirect, EA = (An)
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::ARegIndirect:
			ea = An;
			break;

		// Address Register Indirect with Postincrement, EA=(An), An += N
		// TODO: Data reference
		case EAMode::ARegIndirectPostinc:
			ea = An;
			An += size;
			break;

		// Address Register Indirect with Predecrement, AN -= N, EA=(An)
		// TODO: Data reference
		case EAMode::ARegIndirectPredec:
			An -= size;
			ea = An;
			break;

		// Address Register Indirect with Displacement, EA = (An) + d
		case EAMode::ARegIndirectDisp:
			NotImplementedOpcode("GetMemAddress: Address Register Indirect with Displacement");
			break;

		// Address Register Indirect with Index, EA = (An) + (Ri) + d
		case EAMode::ARegIndirectIndex:
			NotImplementedOpcode("GetMemAddress: Address Register Indirect with Index");
			break;

		// Absolute Short Address, EA given
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AbsShort:
			ea = Widen(FetchWord());
			break;

		// Absolute Long Address, EA given
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AbsLong:
			ea = FetchLong();
			break;

		// Program Counter with Displacement, EA = (PC) + d
		// TODO: Program reference
		case EAMode::PCDisp:
			ea = m_programCounter;
			ea += Widen(FetchWord());
			break;

		// Program Counter with Index, EA = (PC) + (Ri) + d
		// TODO: Program reference
		case EAMode::PCIndex:
			NotImplementedOpcode("GetMemAddress: Program Counter with Index");
			break;

		// Immediate Data
		case EAMode::Immediate:
		default:
			IllegalInstruction();
			break;
		}

		return ea & ADDRESS_MASK;
	}

	void CPU68000::LEA(DWORD& dest)
	{
		ADDRESS src = GetEA(OP_LONG, EAMode::GroupControl);

		switch (m_eaMode)
		{
		case EAMode::DRegDirect:
		case EAMode::ARegDirect:
		case EAMode::ARegIndirectPostinc:
		case EAMode::ARegIndirectPredec:
		case EAMode::Immediate:
			IllegalInstruction();
			return;
		}

		dest = src;
	}

	void CPU68000::MOVEQ()
	{
		if (GetBit(m_opcode, 8))
		{
			IllegalInstruction();
		}

		int reg = (m_opcode >> 9) & 7;
		DWORD value = DoubleWiden(GetLByte(m_opcode));
		AdjustNZ(value);
		SetFlag(FLAG_V, false);
		SetFlag(FLAG_C, false);

		m_reg.DATA[reg] = value;
	}

	void CPU68000::MOVE_b()
	{
		BYTE source = GetEAByte(EAMode::GroupData);
		AdjustNZ(source);
		SetFlag(FLAG_V, false);
		SetFlag(FLAG_C, false);

		// For destination, need to shuffle the bits
		// (opcode order is: 0|0| size | DestReg|DestMode | SrcMode|SrcReg)
		const WORD destMode = (m_opcode >> 3) & 0b111000;
		const WORD destReg = (m_opcode >> 9) & 0b000111;

		// dest == Data reg
		if (destMode == 0)
		{
			SetLByte(m_reg.DATA[destReg], source);
		}
		else // Dest is an address
		{
			m_opcode = destMode | destReg;
			ADDRESS dest = GetEA(OP_BYTE, EAMode::GroupDataAlt);
			WriteB(dest, source);
		}
	}


	void CPU68000::MOVE_w_toSR(WORD src)
	{
		if (m_eaMode == EAMode::ARegDirect)
		{
			IllegalInstruction();
		}

		SetFlags(src);
	}

	void CPU68000::MOVEM_w_fromEA(WORD regs)
	{
		ADDRESS src = GetEA(OP_WORD, EAMode::GroupControlAltPostinc);

		// Set only for postincrement mode, where
		// we need to update its value at the end
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPostinc) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		DWORD* dest = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++dest)
		{
			if (GetLSB(regs))
			{
				SetLWord(*dest, ReadW(src));
				src += 2;
			}

			regs >>= 1;
		}

		// Adjust the An register to point to the new address
		if (addrReg)
		{
			*addrReg = src;
		}
	}

	void CPU68000::MOVEM_l_fromEA(WORD regs)
	{
		ADDRESS src = GetEA(OP_LONG, EAMode::GroupControlAltPredec);

		// Set only for postincrement mode, where
		// we need to update its value at the end
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPostinc) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		DWORD* dest = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++dest)
		{
			if (GetLSB(regs))
			{
				*dest = ReadL(src);
				src += 4;
			}

			regs >>= 1;
		}

		// Adjust the An register to point to the new address
		if (addrReg)
		{
			*addrReg = src;
		}
	}

	void CPU68000::BRA(bool cond)
	{
		DWORD rel = DoubleWiden(GetLByte(m_opcode));
		if (rel == 0)
		{
			rel = Widen(FetchWord());
		}

		if (cond)
		{
			m_programCounter += rel;
			m_programCounter &= ADDRESS_MASK;
		}
	}

	void CPU68000::JMP()
	{
		DWORD addr = GetEA(OP_LONG, EAMode::GroupControl);
		m_programCounter = addr;
	}


	void CPU68000::ANDb_fromEA(BYTE& dest)
	{

	}
	void CPU68000::ANDw_fromEA(WORD& dest)
	{

	}
	void CPU68000::ANDl_fromEA(DWORD& dest)
	{

	}

	void CPU68000::ANDb_toEA(BYTE src)
	{

	}
	void CPU68000::ANDw_toEA(WORD src)
	{

	}
	void CPU68000::ANDl_toEA(DWORD src)
	{

	}

	void CPU68000::ADD_b(BYTE& dest, BYTE src)
	{
		BYTE oldDest = dest;

		WORD temp = dest + src;

		dest = (BYTE)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU68000::ADD_w(WORD& dest, WORD src)
	{
		WORD oldDest = dest;

		DWORD temp = dest + src;

		dest = (WORD)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > 0xFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU68000::ADD_l(DWORD& dest, DWORD src)
	{
		DWORD oldDest = dest;

		QWORD temp = (uint64_t)dest + src;

		dest = (DWORD)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > 0xFFFFFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU68000::SUB_b(BYTE& dest, BYTE src, FLAG carryFlag)
	{
		BYTE oldDest = dest;

		WORD temp = dest - src;

		dest = (BYTE)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	void CPU68000::SUB_w(WORD& dest, WORD src, FLAG carryFlag)
	{
		WORD oldDest = dest;

		DWORD temp = dest - src;

		dest = (WORD)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > 0xFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	void CPU68000::SUB_l(DWORD& dest, DWORD src, FLAG carryFlag)
	{
		DWORD oldDest = dest;

		QWORD temp = (QWORD)dest - src;

		dest = (DWORD)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > 0xFFFFFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	void CPU68000::Serialize(json& to)
	{
		to["opcode"] = m_opcode;
		to["registers"] = m_reg.DataAddress;
		to["usp"] = m_reg.USP;
		to["ssp"] = m_reg.SSP;
		to["flags"] = m_reg.flags;
		to["pc"] = m_programCounter;
	}
	void CPU68000::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];
		m_reg.DataAddress = from["registers"];
		m_reg.USP = from["usp"];
		m_reg.SSP = from["ssp"];
		m_reg.flags = from["flags"];
		m_programCounter = from["pc"];
	}
}
