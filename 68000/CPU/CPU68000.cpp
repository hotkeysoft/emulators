#include "stdafx.h"
#include "CPU68000.h"

using cpuInfo::Opcode;

namespace emul::cpu68k
{
	static const BitMaskW Mask_EXT("000xxx");
	static const BitMaskW Mask_EXG("00xxxx");
	static const BitMaskW Mask_ABCD("00xxxx");
	static const BitMaskW Mask_ADDX("00xxxx");
	static const BitMaskW Mask_DB("001xxx");
	static const BitMaskW Mask_MOVEP("1xx001xxx");
	static const BitMaskW Mask_SHIFT_MEM("11xxxxxx");
	static const BitMaskW Mask_CMPM("001xxx");
	static const BitMaskW Mask_SWAP("000xxx");

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
		m_opcodes[0b0011] = [=]() { MOVEw(); }; // MOVE.w, MOVEA.w
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

		table[004] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D0w) : BTST(m_reg.D0); };
		table[005] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D0)  : BCHG(m_reg.D0); };
		table[006] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D0w) : BCLR(m_reg.D0); };
		table[007] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D0)  : BSET(m_reg.D0); };

		table[014] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D1w) : BTST(m_reg.D1); };
		table[015] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D1)  : BCHG(m_reg.D1); };
		table[016] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D1w) : BCLR(m_reg.D1); };
		table[017] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D1)  : BSET(m_reg.D1); };

		table[024] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D2w) : BTST(m_reg.D2); };
		table[025] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D2)  : BCHG(m_reg.D2); };
		table[026] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D2w) : BCLR(m_reg.D2); };
		table[027] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D2)  : BSET(m_reg.D2); };

		table[034] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D3w) : BTST(m_reg.D3); };
		table[035] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D3)  : BCHG(m_reg.D3); };
		table[036] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D3w) : BCLR(m_reg.D3); };
		table[037] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D3)  : BSET(m_reg.D3); };

		table[040] = [=]() { BTSTimm(); };
		table[041] = [=]() { BCHGimm(); };
		table[042] = [=]() { BCLRimm(); };
		table[043] = [=]() { BSETimm(); };

		table[044] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D4w) : BTST(m_reg.D4); };
		table[045] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D4)  : BCHG(m_reg.D4); };
		table[046] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D4w) : BCLR(m_reg.D4); };
		table[047] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D4)  : BSET(m_reg.D4); };

		table[054] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D5w) : BTST(m_reg.D5); };
		table[055] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D5)  : BCHG(m_reg.D5); };
		table[056] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D5w) : BCLR(m_reg.D5); };
		table[057] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D5)  : BSET(m_reg.D5); };

		// CMPI #<data>, <ea>
		table[060] = [=]() { BYTE src = FetchByte(); CMPb(GetEAByte(EAMode::GroupDataAlt), src); };
		table[061] = [=]() { WORD src = FetchWord(); CMPw(GetEAWord(EAMode::GroupDataAlt), src); };
		table[062] = [=]() { DWORD src = FetchLong(); CMPl(GetEALong(EAMode::GroupDataAlt), src); };

		table[064] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D6w) : BTST(m_reg.D6); };
		table[065] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D6)  : BCHG(m_reg.D6); };
		table[066] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D6w) : BCLR(m_reg.D6); };
		table[067] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D6)  : BSET(m_reg.D6); };

		table[074] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D7w) : BTST(m_reg.D7); };
		table[075] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D7)  : BCHG(m_reg.D7); };
		table[076] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D7w) : BCLR(m_reg.D7); };
		table[077] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D7)  : BSET(m_reg.D7); };

	}

	// b0100: Misc
	void CPU68000::InitGroup4(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[007] = [=]() { LEA(m_reg.A0); }; // LEA <ea>,A0

		table[010] = [=]() { CLRb(); }; // CLR.b <ea>
		table[011] = [=]() { CLRw(); }; // CLR.w <ea>
		table[012] = [=]() { CLRl(); }; // CLR.l <ea>

		table[017] = [=]() { LEA(m_reg.A1); }; // LEA <ea>,A1
		table[027] = [=]() { LEA(m_reg.A2); }; // LEA <ea>,A2

		// MOVE <ea>, SR
		table[033] = [=]() { Privileged(); MOVE_w_toSR(GetEAWord(EAMode::GroupData)); };

		table[037] = [=]() { LEA(m_reg.A3); }; // LEA <ea>,A3

		table[041] = [=]() { Mask_SWAP.IsMatch(m_opcode) ? SWAPw() : NotImplementedOpcode("PEA.l {idx}"); }; // SWAP
		table[042] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEMwToEA(FetchWord()); }; // MOVEM <register list>, <ea>
		table[043] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEMlToEA(FetchWord()); }; // MOVEM <register list>, <ea>

		table[047] = [=]() { LEA(m_reg.A4); }; // LEA <ea>,A4

		table[050] = [=]() { TSTb(); }; // TST.b <ea>
		table[051] = [=]() { TSTw(); }; // TST.w <ea>
		table[052] = [=]() { TSTl(); }; // TST.l <ea>

		table[057] = [=]() { LEA(m_reg.A5); }; // LEA <ea>,A5

		// MOVEM <ea>, <register list>
		table[062] = [=]() { MOVEMwFromEA(FetchWord()); };
		table[063] = [=]() { MOVEMlFromEA(FetchWord()); };

		table[067] = [=]() { LEA(m_reg.A6); }; // LEA <ea>,A6
		table[077] = [=]() { LEA(m_reg.A7); }; // LEA <ea>,A7

//		table[072] = [=]() { JSR(); }; // JSR <ea>
		table[073] = [=]() { JMP(); }; // JMP <ea>

	}

	// b0101: ADDQ,SUBQ,Scc,DBcc
	void CPU68000::InitGroup5(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// 0 (=8)
		table[000] = [=]() { ADDQb(8); }; // ADDQ.b #8, <ea>
		table[001] = [=]() { ADDQw(8); }; // ADDQ.w #8, <ea>
		table[002] = [=]() { ADDQl(8); }; // ADDQ.l #8, <ea>
		table[003] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(true) : Sccb(true); }; // ST.b <ea> / DBT.w Dn,disp
		table[004] = [=]() { SUBQb(8); }; // SUBQ.b #8, <ea>
		table[005] = [=]() { SUBQw(8); }; // SUBQ.w #8, <ea>
		table[006] = [=]() { SUBQl(8); }; // SUBQ.l #8, <ea>
		table[007] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(false) : Sccb(false); }; // SF.b <ea> / DBF.w Dn,disp

		// 1
		table[010] = [=]() { ADDQb(1); }; // ADDQ.b #1, <ea>
		table[011] = [=]() { ADDQw(1); }; // ADDQ.w #1, <ea>
		table[012] = [=]() { ADDQl(1); }; // ADDQ.l #1, <ea>
		table[013] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagHI()) : Sccb(FlagHI()); }; // SHI.b <ea> / DBHI.w Dn,disp
		table[014] = [=]() { SUBQb(1); }; // SUBQ.b #1, <ea>
		table[015] = [=]() { SUBQw(1); }; // SUBQ.w #1, <ea>
		table[016] = [=]() { SUBQl(1); }; // SUBQ.l #1, <ea>
		table[017] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagLS()) : Sccb(FlagLS()); }; // STLS.b <ea> / DBLS.w Dn,disp

		// 2
		table[020] = [=]() { ADDQb(2); }; // ADDQ.b #2, <ea>
		table[021] = [=]() { ADDQw(2); }; // ADDQ.w #2, <ea>
		table[022] = [=]() { ADDQl(2); }; // ADDQ.l #2, <ea>
		table[023] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagCC()) : Sccb(FlagCC()); }; // SCC.b <ea> / DBCC.w Dn,disp
		table[024] = [=]() { SUBQb(2); }; // SUBQ.b #2, <ea>
		table[025] = [=]() { SUBQw(2); }; // SUBQ.w #2, <ea>
		table[026] = [=]() { SUBQl(2); }; // SUBQ.l #2, <ea>
		table[027] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagCS()) : Sccb(FlagCS()); }; // SCS.b <ea> / DBCS.w Dn,disp

		// 3
		table[030] = [=]() { ADDQb(3); }; // ADDQ.b #3, <ea>
		table[031] = [=]() { ADDQw(3); }; // ADDQ.w #3, <ea>
		table[032] = [=]() { ADDQl(3); }; // ADDQ.l #3, <ea>
		table[033] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagNE()) : Sccb(FlagNE()); }; // SNE.b <ea> / DBNE.w Dn,disp
		table[034] = [=]() { SUBQb(3); }; // SUBQ.b #3, <ea>
		table[035] = [=]() { SUBQw(3); }; // SUBQ.w #3, <ea>
		table[036] = [=]() { SUBQl(3); }; // SUBQ.l #3, <ea>
		table[037] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagEQ()) : Sccb(FlagEQ()); }; // SEQ.b <ea> / DBEQ.w Dn,disp

		// 4
		table[040] = [=]() { ADDQb(4); }; // ADDQ.b #4, <ea>
		table[041] = [=]() { ADDQw(4); }; // ADDQ.w #4, <ea>
		table[042] = [=]() { ADDQl(4); }; // ADDQ.l #4, <ea>
		table[043] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagVC()) : Sccb(FlagVC()); }; // SVC.b <ea> / DBVC.w Dn,disp
		table[044] = [=]() { SUBQb(4); }; // SUBQ.b #4, <ea>
		table[045] = [=]() { SUBQw(4); }; // SUBQ.w #4, <ea>
		table[046] = [=]() { SUBQl(4); }; // SUBQ.l #4, <ea>
		table[047] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagVS()) : Sccb(FlagVS()); }; // SVS.b <ea> / DBVS.w Dn,disp

		// 5
		table[050] = [=]() { ADDQb(5); }; // ADDQ.b #5, <ea>
		table[051] = [=]() { ADDQw(5); }; // ADDQ.w #5, <ea>
		table[052] = [=]() { ADDQl(5); }; // ADDQ.l #5, <ea>
		table[053] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagPL()) : Sccb(FlagPL()); }; // SPL.b <ea> / DBPL.w Dn,disp
		table[054] = [=]() { SUBQb(5); }; // SUBQ.b #5, <ea>
		table[055] = [=]() { SUBQw(5); }; // SUBQ.w #5, <ea>
		table[056] = [=]() { SUBQl(5); }; // SUBQ.l #5, <ea>
		table[057] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagMI()) : Sccb(FlagMI()); }; // SMI.b <ea> / DBMI.w Dn,disp

		// 6
		table[060] = [=]() { ADDQb(6); }; // ADDQ.b #6, <ea>
		table[061] = [=]() { ADDQw(6); }; // ADDQ.w #6, <ea>
		table[062] = [=]() { ADDQl(6); }; // ADDQ.l #6, <ea>
		table[063] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagGE()) : Sccb(FlagGE()); }; // SGE.b <ea> / DBGE.w Dn,disp
		table[064] = [=]() { SUBQb(6); }; // SUBQ.b #6, <ea>
		table[065] = [=]() { SUBQw(6); }; // SUBQ.w #6, <ea>
		table[066] = [=]() { SUBQl(6); }; // SUBQ.l #6, <ea>
		table[067] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagLT()) : Sccb(FlagLT()); }; // SLT.b <ea> / DBLT.w Dn,disp

		// 7
		table[070] = [=]() { ADDQb(7); }; // ADDQ.b #7, <ea>
		table[071] = [=]() { ADDQw(7); }; // ADDQ.w #7, <ea>
		table[072] = [=]() { ADDQl(7); }; // ADDQ.l #7, <ea>
		table[073] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagGT()) : Sccb(FlagGT()); }; // SGT.b <ea> / DBGT.w Dn,disp
		table[074] = [=]() { SUBQb(7); }; // SUBQ.b #7, <ea>
		table[075] = [=]() { SUBQw(7); }; // SUBQ.w #7, <ea>
		table[076] = [=]() { SUBQl(7); }; // SUBQ.l #7, <ea>
		table[073] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagLE()) : Sccb(FlagLE()); }; // SLE.b <ea> / DBLE.w Dn,disp
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

		// A0
		table[003] = [=]() { m_reg.A0 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A0
		table[007] = [=]() { m_reg.A0 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A0

		// A1
		table[013] = [=]() { m_reg.A1 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A1
		table[017] = [=]() { m_reg.A1 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A1

		// A2
		table[023] = [=]() { m_reg.A2 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A2
		table[027] = [=]() { m_reg.A2 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A2

		// A3
		table[033] = [=]() { m_reg.A3 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A3
		table[037] = [=]() { m_reg.A3 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A3

		// A4
		table[043] = [=]() { m_reg.A4 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A4
		table[047] = [=]() { m_reg.A4 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A4

		// A5
		table[053] = [=]() { m_reg.A5 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A5
		table[057] = [=]() { m_reg.A5 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A5

		// A6
		table[063] = [=]() { m_reg.A6 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A6
		table[067] = [=]() { m_reg.A6 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A6

		// A7
		table[073] = [=]() { m_reg.A7 -= Widen(GetEAWord(EAMode::GroupAll)); }; // SUBA.w <ea>,A7
		table[077] = [=]() { m_reg.A7 -= GetEALong(EAMode::GroupAll); }; // SUBA.l <ea>,A7
	}

	// b1011: EOR,CMPM,CMP,CMPA
	void CPU68000::InitGroup11(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { CMPb(m_reg.D0b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D0
		table[001] = [=]() { CMPw(m_reg.D0w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D0
		table[002] = [=]() { CMPl(m_reg.D0, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D0

		table[004] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D0b); }; // EOR.b D0,{idx}
		table[005] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D0w); }; // EOR.b D0,{idx}
		table[006] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D0);  }; // EOR.b D0,{idx}

		// D1
		table[010] = [=]() { CMPb(m_reg.D1b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D1
		table[011] = [=]() { CMPw(m_reg.D1w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D1
		table[012] = [=]() { CMPl(m_reg.D1, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D1

		table[014] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D1b); }; // EOR.b D1,{idx}
		table[015] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D1w); }; // EOR.b D1,{idx}
		table[016] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D1);  }; // EOR.b D1,{idx}

		// D2
		table[020] = [=]() { CMPb(m_reg.D2b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D2
		table[021] = [=]() { CMPw(m_reg.D2w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D2
		table[022] = [=]() { CMPl(m_reg.D2, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D2

		table[024] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D2b); }; // EOR.b D2,{idx}
		table[025] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D2w); }; // EOR.b D2,{idx}
		table[026] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D2);  }; // EOR.b D2,{idx}

		// D3
		table[030] = [=]() { CMPb(m_reg.D3b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D3
		table[031] = [=]() { CMPw(m_reg.D3w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D3
		table[032] = [=]() { CMPl(m_reg.D3, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D3

		table[034] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D3b); }; // EOR.b D3,{idx}
		table[035] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D3w); }; // EOR.b D3,{idx}
		table[036] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D3);  }; // EOR.b D3,{idx}

		// D4
		table[040] = [=]() { CMPb(m_reg.D4b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D4
		table[041] = [=]() { CMPw(m_reg.D4w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D4
		table[042] = [=]() { CMPl(m_reg.D4, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D4

		table[044] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D4b); }; // EOR.b D4,{idx}
		table[045] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D4w); }; // EOR.b D4,{idx}
		table[046] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D4);  }; // EOR.b D4,{idx}

		// D5
		table[050] = [=]() { CMPb(m_reg.D5b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D5
		table[051] = [=]() { CMPw(m_reg.D5w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D5
		table[052] = [=]() { CMPl(m_reg.D5, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D5

		table[054] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D5b); }; // EOR.b D5,{idx}
		table[055] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D5w); }; // EOR.b D5,{idx}
		table[056] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D5);  }; // EOR.b D5,{idx}

		// D6
		table[060] = [=]() { CMPb(m_reg.D6b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D6
		table[061] = [=]() { CMPw(m_reg.D6w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D6
		table[062] = [=]() { CMPl(m_reg.D6, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D6

		table[064] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D6b); }; // EOR.b D6,{idx}
		table[065] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D6w); }; // EOR.b D6,{idx}
		table[066] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D6);  }; // EOR.b D6,{idx}

		// D7
		table[070] = [=]() { CMPb(m_reg.D7b, GetEAByte(EAMode::GroupData)); }; // CMP.b <ea>,D7
		table[071] = [=]() { CMPw(m_reg.D7w, GetEAWord(EAMode::GroupAll)); }; // CMP.w <ea>,D7
		table[072] = [=]() { CMPl(m_reg.D7, GetEALong(EAMode::GroupAll)); }; // CMP.l <ea>,D7

		table[074] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORbToEA(m_reg.D7b); }; // EOR.b D7,{idx}
		table[075] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORwToEA(m_reg.D7w); }; // EOR.b D7,{idx}
		table[076] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORlToEA(m_reg.D7);  }; // EOR.b D7,{idx}
	}

	// b1100: MULU,MULS,ABCD,EXG,AND
	void CPU68000::InitGroup12(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { ANDb(m_reg.D0b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D0
		table[001] = [=]() { ANDw(m_reg.D0w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D0
		table[002] = [=]() { ANDl(m_reg.D0,  GetEALong(EAMode::GroupData)); }; // AND.l <ea>,D0

		table[004] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D0b); }; // AND.b D0,<ea>
		table[005] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D0w); }; // AND.b D0,<ea>
		table[006] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D0); };  // AND.b D0,<ea>

		// D1
		table[010] = [=]() { ANDb(m_reg.D1b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D1
		table[011] = [=]() { ANDw(m_reg.D1w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D1
		table[012] = [=]() { ANDl(m_reg.D1,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D1

		table[014] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D1b); }; // AND.b D1,<ea>
		table[015] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D1w); }; // AND.b D1,<ea>
		table[016] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D1); };  // AND.b D1,<ea>

		// D2
		table[020] = [=]() { ANDb(m_reg.D2b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D2
		table[021] = [=]() { ANDw(m_reg.D2w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D2
		table[022] = [=]() { ANDl(m_reg.D2,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D2

		table[024] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D2b); }; // AND.b D2,<ea>
		table[025] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D2w); }; // AND.b D2,<ea>
		table[026] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D2); };  // AND.b D2,<ea>

		// D3
		table[030] = [=]() { ANDb(m_reg.D3b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D3
		table[031] = [=]() { ANDw(m_reg.D3w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D3
		table[032] = [=]() { ANDl(m_reg.D3,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D3

		table[034] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D3b); }; // AND.b D3,<ea>
		table[035] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D3w); }; // AND.b D3,<ea>
		table[036] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D3); };  // AND.b D3,<ea>

		// D4
		table[040] = [=]() { ANDb(m_reg.D4b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D4
		table[041] = [=]() { ANDw(m_reg.D4w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D4
		table[042] = [=]() { ANDl(m_reg.D4,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D4

		table[044] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D4b); }; // AND.b D4,<ea>
		table[045] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D4w); }; // AND.b D4,<ea>
		table[046] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D4); };  // AND.b D4,<ea>

		// D5
		table[050] = [=]() { ANDb(m_reg.D5b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D5
		table[051] = [=]() { ANDw(m_reg.D5w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D5
		table[052] = [=]() { ANDl(m_reg.D5,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D5

		table[054] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D5b); }; // AND.b D5,<ea>
		table[055] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D5w); }; // AND.b D5,<ea>
		table[056] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D5); };  // AND.b D5,<ea>

		// D6
		table[060] = [=]() { ANDb(m_reg.D6b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D6
		table[061] = [=]() { ANDw(m_reg.D6w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D6
		table[062] = [=]() { ANDl(m_reg.D6,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D6

		table[064] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D6b); }; // AND.b D6,<ea>
		table[065] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D6w); }; // AND.b D6,<ea>
		table[066] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D6); };  // AND.b D6,<ea>

		// D7
		table[070] = [=]() { ANDb(m_reg.D7b, GetEAByte(EAMode::GroupData)); }; // AND.b <ea>,D7
		table[071] = [=]() { ANDw(m_reg.D7w, GetEAWord(EAMode::GroupData)); }; // AND.w <ea>,D7
		table[072] = [=]() { ANDl(m_reg.D7,  GetEALong(EAMode::GroupData)); };  // AND.l <ea>,D7

		table[074] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D7b); }; // AND.b D7,<ea>
		table[075] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D7w); }; // AND.b D7,<ea>
		table[076] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D7); };  // AND.b D7,<ea>

	}

	// b1101: ADD,ADDX,ADDA
	void CPU68000::InitGroup13(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { ADDb(m_reg.D0b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D0
		table[001] = [=]() { ADDw(m_reg.D0w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D0
		table[002] = [=]() { ADDl(m_reg.D0,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D0
		table[003] = [=]() { m_reg.A0 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A0
		table[004] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D0b); }; // ADD.b D0,<ea>
		table[005] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D0w); }; // ADD.b D0,<ea>
		table[006] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D0); };  // ADD.b D0,<ea>
		table[007] = [=]() { m_reg.A0 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A0

		// D1
		table[010] = [=]() { ADDb(m_reg.D1b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D1
		table[011] = [=]() { ADDw(m_reg.D1w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D1
		table[012] = [=]() { ADDl(m_reg.D1,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D1
		table[013] = [=]() { m_reg.A1 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A1
		table[014] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D1b); }; // ADD.b D1,<ea>
		table[015] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D1w); }; // ADD.b D1,<ea>
		table[016] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D1); };  // ADD.b D1,<ea>
		table[017] = [=]() { m_reg.A1 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A1

		// D2
		table[020] = [=]() { ADDb(m_reg.D2b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D2
		table[021] = [=]() { ADDw(m_reg.D2w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D2
		table[022] = [=]() { ADDl(m_reg.D2,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D2
		table[023] = [=]() { m_reg.A2 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A2
		table[024] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D2b); }; // ADD.b D2,<ea>
		table[025] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D2w); }; // ADD.b D2,<ea>
		table[026] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D2); };  // ADD.b D2,<ea>
		table[027] = [=]() { m_reg.A2 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A2

		// D3
		table[030] = [=]() { ADDb(m_reg.D3b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D3
		table[031] = [=]() { ADDw(m_reg.D3w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D3
		table[032] = [=]() { ADDl(m_reg.D3,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D3
		table[033] = [=]() { m_reg.A3 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A3
		table[034] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D3b); }; // ADD.b D3,<ea>
		table[035] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D3w); }; // ADD.b D3,<ea>
		table[036] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D3); };  // ADD.b D3,<ea>
		table[037] = [=]() { m_reg.A3 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A3

		// D4
		table[040] = [=]() { ADDb(m_reg.D4b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D4
		table[041] = [=]() { ADDw(m_reg.D4w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D4
		table[042] = [=]() { ADDl(m_reg.D4,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D4
		table[043] = [=]() { m_reg.A4 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A4
		table[044] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D4b); }; // ADD.b D4,<ea>
		table[045] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D4w); }; // ADD.b D4,<ea>
		table[046] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D4); };  // ADD.b D4,<ea>
		table[047] = [=]() { m_reg.A4 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A4

		// D5
		table[050] = [=]() { ADDb(m_reg.D5b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D5
		table[051] = [=]() { ADDw(m_reg.D5w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D5
		table[052] = [=]() { ADDl(m_reg.D5,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D5
		table[053] = [=]() { m_reg.A5 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A5
		table[054] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D5b); }; // ADD.b D5,<ea>
		table[055] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D5w); }; // ADD.b D5,<ea>
		table[056] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D5); };  // ADD.b D5,<ea>
		table[057] = [=]() { m_reg.A5 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A5

		// D6
		table[060] = [=]() { ADDb(m_reg.D6b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D6
		table[061] = [=]() { ADDw(m_reg.D6w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D6
		table[062] = [=]() { ADDl(m_reg.D6,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D6
		table[063] = [=]() { m_reg.A6 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A6
		table[064] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D6b); }; // ADD.b D6,<ea>
		table[065] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D6w); }; // ADD.b D6,<ea>
		table[066] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D6); };  // ADD.b D6,<ea>
		table[067] = [=]() { m_reg.A6 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A6

		// D7
		table[070] = [=]() { ADDb(m_reg.D7b, GetEAByte(EAMode::GroupData)); }; // ADD.b <ea>,D7
		table[071] = [=]() { ADDw(m_reg.D7w, GetEAWord(EAMode::GroupData)); }; // ADD.w <ea>,D7
		table[072] = [=]() { ADDl(m_reg.D7,  GetEALong(EAMode::GroupData)); }; // ADD.l <ea>,D7
		table[073] = [=]() { m_reg.A7 += Widen(GetEAWord(EAMode::GroupAll)); }; // ADDA.w <ea>,A7
		table[074] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D7b); }; // ADD.b D7,<ea>
		table[075] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D7w); }; // ADD.b D7,<ea>
		table[076] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D7); };  // ADD.b D7,<ea>
		table[077] = [=]() { m_reg.A7 += GetEALong(EAMode::GroupAll); }; // ADDA.l <ea>,A7
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
			return m_reg.GetDATAb(reg);
		case EAMode::ARegDirect:
			return m_reg.GetADDRb(reg);
		case EAMode::Immediate:
			TICKn(4);
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
			return m_reg.GetDATAw(reg);
		case EAMode::ARegDirect:
			return m_reg.GetADDRw(reg);
		case EAMode::Immediate:
			TICKn(4);
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
			TICKn(8);
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

		int timePenalty = 0;

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
			timePenalty = 4;
			break;

		// Address Register Indirect with Postincrement, EA=(An), An += N
		// TODO: Data reference
		case EAMode::ARegIndirectPostinc:
			ea = An;
			An += size;
			timePenalty = 4;
			break;

		// Address Register Indirect with Predecrement, AN -= N, EA=(An)
		// TODO: Data reference
		case EAMode::ARegIndirectPredec:
			An -= size;
			ea = An;
			timePenalty = 6;
			break;

		// Address Register Indirect with Displacement, EA = (An) + d
		case EAMode::ARegIndirectDisp:
			ea = An;
			ea += Widen(FetchWord());
			timePenalty = 8;
			break;

		// Address Register Indirect with Index, EA = (An) + (Ri) + d
		case EAMode::ARegIndirectIndex:
			NotImplementedOpcode("GetMemAddress: Address Register Indirect with Index");
			timePenalty = 10;
			break;

		// Absolute Short Address, EA given
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AbsShort:
			ea = Widen(FetchWord());
			timePenalty = 8;
			break;

		// Absolute Long Address, EA given
		// TODO: Data reference except jmp and jmp to subroutine instructions
		case EAMode::AbsLong:
			ea = FetchLong();
			timePenalty = 12;
			break;

		// Program Counter with Displacement, EA = (PC) + d
		// TODO: Program reference
		case EAMode::PCDisp:
			ea = m_programCounter;
			ea += Widen(FetchWord());
			timePenalty = 8;
			break;

		// Program Counter with Index, EA = (PC) + (Ri) + d
		// TODO: Program reference
		case EAMode::PCIndex:
			NotImplementedOpcode("GetMemAddress: Program Counter with Index");
			timePenalty = 10;
			break;

		// Immediate Data
		case EAMode::Immediate:
		default:
			IllegalInstruction();
			break;
		}

		if (timePenalty)
		{
			TICKn(timePenalty + ((size == 4) ? 4 : 0));
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
		SetFlag(FLAG_VC, false);

		m_reg.DATA[reg] = value;
	}

	void CPU68000::MOVEb()
	{
		BYTE source = GetEAByte(EAMode::GroupData);
		AdjustNZ(source);
		SetFlag(FLAG_VC, false);

		// For destination, need to shuffle the bits
		// (opcode order is: 0|0| size | DestReg|DestMode | SrcMode|SrcReg)
		const WORD destMode = (m_opcode >> 3) & 0b111000;
		const WORD destReg = (m_opcode >> 9) & 0b000111;

		// dest == Data reg
		if (destMode == 0)
		{
			m_reg.GetDATAb(destReg) = source;
		}
		else // Dest is an address
		{
			m_opcode = destMode | destReg;
			ADDRESS dest = GetEA(OP_BYTE, EAMode::GroupDataAlt);
			WriteB(dest, source);
		}
	}

	void CPU68000::MOVEw()
	{
		WORD source = GetEAWord(EAMode::GroupAll);
		AdjustNZ(source);
		SetFlag(FLAG_VC, false);

		// For destination, need to shuffle the bits
		// (opcode order is: 0|0| size | DestReg|DestMode | SrcMode|SrcReg)
		const WORD destMode = (m_opcode >> 3) & 0b111000;
		const WORD destReg = (m_opcode >> 9) & 0b000111;

		// dest == Data reg
		if (destMode == 0)
		{
			m_reg.GetDATAw(destReg) = source;
		}
		else if (destMode == 0b001000) // An -> MOVEA
		{
			m_reg.ADDR[destReg] = Widen(source);
		}
		else // Dest is an address
		{
			m_opcode = destMode | destReg;
			ADDRESS dest = GetEA(OP_WORD, EAMode::GroupDataAlt);
			WriteW(dest, source);
		}
	}

	void CPU68000::MOVEl()
	{
		DWORD source = GetEALong(EAMode::GroupAll);
		AdjustNZ(source);
		SetFlag(FLAG_VC, false);

		// For destination, need to shuffle the bits
		// (opcode order is: 0|0| size | DestReg|DestMode | SrcMode|SrcReg)
		const WORD destMode = (m_opcode >> 3) & 0b111000;
		const WORD destReg = (m_opcode >> 9) & 0b000111;

		// dest == Data reg
		if (destMode == 0)
		{
			m_reg.DATA[destReg] = source;
		}
		else if (destMode == 0b001000) // An -> MOVEA
		{
			m_reg.ADDR[destReg] = source;
		}
		else // Dest is an address
		{
			m_opcode = destMode | destReg;
			ADDRESS dest = GetEA(OP_LONG, EAMode::GroupDataAlt);
			WriteL(dest, source);
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

	void CPU68000::MOVEMwFromEA(WORD regs)
	{
		ADDRESS src = GetEA(OP_WORD, EAMode::GroupControlAltPostinc);

		// Set only for postincrement mode, where
		// we need we iterate directly through the address reg
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPostinc) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		DWORD* dest = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++dest)
		{
			if (GetLSB(regs))
			{
				if (addrReg)
				{
					*dest = Widen(ReadW(*addrReg));
					*addrReg += 2;
				}
				else
				{
					*dest = Widen(ReadW(src));
					src += 2;
				}
			}

			regs >>= 1;
		}
	}

	void CPU68000::MOVEMlFromEA(WORD regs)
	{
		ADDRESS src = GetEA(OP_LONG, EAMode::GroupControlAltPostinc);

		// Set only for postincrement mode, where
		// we need we iterate directly through the address reg
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPostinc) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		DWORD* dest = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++dest)
		{
			if (GetLSB(regs))
			{
				if (addrReg)
				{
					*dest = ReadL(*addrReg);
					*addrReg += 4;
				}
				else
				{
					*dest = ReadL(src);
					src += 4;
				}
			}

			regs >>= 1;
		}
	}

	void CPU68000::MOVEMwToEA(WORD regs)
	{
		ADDRESS dest = GetEA(OP_WORD, EAMode::GroupControlAltPredec);

		// Set only for predecrement mode, where
		// we need to update its value at the end
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPredec) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		DWORD* src = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++src)
		{
			if (GetLSB(regs))
			{
				if (addrReg)
				{
					*addrReg -= 2;
					WriteW(*addrReg, GetLWord(*src));
				}
				else
				{
					WriteW(dest, GetLWord(*src));
					dest += 2;
				}
			}

			regs >>= 1;
		}
	}
	void CPU68000::MOVEMlToEA(WORD regs)
	{
		ADDRESS dest = GetEA(OP_LONG, EAMode::GroupControlAltPredec);

		// Set only for predecrement mode, where
		// we need to update its value at the end
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPredec) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		DWORD* src = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++src)
		{
			if (GetLSB(regs))
			{
				if (addrReg)
				{
					*addrReg -= 4;
					WriteL(*addrReg, *src);
				}
				else
				{
					WriteL(dest, *src);
					dest += 4;
				}
			}

			regs >>= 1;
		}
	}

	void CPU68000::BRA(bool cond)
	{
		DWORD rel = DoubleWiden(GetLByte(m_opcode));

		// Save PC here because it can be incremented by FetchWord()
		ADDRESS pc = m_programCounter;
		if (rel == 0)
		{
			rel = Widen(FetchWord());
		}

		if (cond)
		{
			m_programCounter = pc + rel;
			m_programCounter &= ADDRESS_MASK;
		}
	}

	void CPU68000::JMP()
	{
		DWORD addr = GetEA(OP_LONG, EAMode::GroupControl);
		m_programCounter = addr;
	}

	void CPU68000::DBccw(bool cond)
	{
		TICKn(6); // TODO: base instruction timings currently hardcoded at 4
		//TICKn(10);
		if (!cond)
		{
			// PC will be changed by FetchWord()
			ADDRESS pc = m_programCounter;

			WORD& reg = m_reg.GetDATAw(GetOpcodeRegisterIndex());
			DWORD rel = Widen(FetchWord());
			if (--reg != 0xFFFF)
			{
				m_programCounter = pc + rel;
				m_programCounter &= ADDRESS_MASK;
			}
			else
			{
				TICKn(4);
			}
		}
		else
		{
			TICKn(2);
			// fall through to next instruction
			m_programCounter += 2;
			m_programCounter &= ADDRESS_MASK;
		}
	}

	void CPU68000::BTSTimm()
	{
		m_eaMode = GetEAMode(m_opcode);

		BYTE bitNumber = FetchByte();

		bool testBit;
		if (m_eaMode == EAMode::DRegDirect)
		{
			// Long only
			testBit = GetBit(m_reg.DATA[GetOpcodeRegisterIndex()], bitNumber % 32);
		}
		else
		{
			// TODO: timings
			TICKn(4);

			// Byte only
			BYTE src = GetEAByte(EAMode::GroupDataNoImm);
			testBit = GetBit(src, bitNumber % 8);
		}
		SetFlag(FLAG_Z, !testBit);
	}

	void CPU68000::BitOps(BitOp bitOp)
	{
		m_eaMode = GetEAMode(m_opcode);

		BYTE bitNumber = FetchByte();

		bool testBit;
		if (m_eaMode == EAMode::DRegDirect)
		{
			// Long only
			bitNumber %= 32;
			DWORD& dest = m_reg.DATA[GetOpcodeRegisterIndex()];

			testBit = GetBit(dest, bitNumber);
			bool newBit = (bitOp == BitOp::SET) ? 1 : ((bitOp == BitOp::CLEAR) ? 0 : !testBit);
			SetBit(dest, bitNumber, newBit);
		}
		else
		{
			// Byte only
			bitNumber %= 8;
			ADDRESS addr = GetEA(OP_BYTE, EAMode::GroupDataNoImm);
			BYTE dest = ReadB(addr);

			testBit = GetBit(dest, bitNumber);
			bool newBit = (bitOp == BitOp::SET) ? 1 : ((bitOp == BitOp::CLEAR) ? 0 : !testBit);
			SetBit(dest, bitNumber, newBit);

			WriteB(addr, dest);
		}
		SetFlag(FLAG_Z, testBit);
	}

	// TST <ea>
	void CPU68000::TSTb()
	{
		BYTE src = GetEAByte(EAMode::GroupDataAlt);
		AdjustNZ(src);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::TSTw()
	{
		WORD src = GetEAWord(EAMode::GroupDataAlt);
		AdjustNZ(src);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::TSTl()
	{
		DWORD src = GetEALong(EAMode::GroupDataAlt);
		AdjustNZ(src);
		SetFlag(FLAG_VC, false);
	}

	// AND <ea>, Dn
	void CPU68000::ANDb(BYTE& dest, BYTE src)
	{
		dest &= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::ANDw(WORD& dest, WORD src)
	{
		dest &= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::ANDl(DWORD& dest, DWORD src)
	{
		dest &= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	// AND Dn, <ea>
	void CPU68000::ANDbToEA(BYTE src)
	{
		NotImplementedOpcode("AND.b Dn -> <ea>");
	}
	void CPU68000::ANDwToEA(WORD src)
	{
		NotImplementedOpcode("AND.w Dn -> <ea>");
	}
	void CPU68000::ANDlToEA(DWORD src)
	{
		NotImplementedOpcode("AND.l Dn -> <ea>");
	}

	void CPU68000::EORb(BYTE& dest, BYTE src)
	{
		dest ^= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::EORw(WORD& dest, WORD src)
	{
		dest ^= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::EORl(DWORD& dest, DWORD src)
	{
		dest ^= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	void CPU68000::EORbToEA(BYTE src)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			BYTE& dest = m_reg.GetDATAb(GetOpcodeRegisterIndex());
			EORb(dest, src);
		}
		else
		{
			ADDRESS ea = GetEA(OP_BYTE, EAMode::GroupDataAlt);

			BYTE dest = ReadB(ea);
			EORb(dest, src);
			WriteB(ea, dest);
		}
	}
	void CPU68000::EORwToEA(WORD src)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			WORD& dest = m_reg.GetDATAw(GetOpcodeRegisterIndex());
			EORw(dest, src);
		}
		else
		{
			ADDRESS ea = GetEA(OP_WORD, EAMode::GroupDataAlt);

			WORD dest = ReadW(ea);
			EORw(dest, src);
			WriteW(ea, dest);
		}
	}
	void CPU68000::EORlToEA(DWORD src)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			DWORD& dest = m_reg.DATA[GetOpcodeRegisterIndex()];
			EORl(dest, src);
		}
		else
		{
			ADDRESS ea = GetEA(OP_LONG, EAMode::GroupDataAlt);

			DWORD dest = ReadL(ea);
			EORl(dest, src);
			WriteL(ea, dest);
		}
	}

	void CPU68000::SWAPw()
	{
		DWORD& reg = m_reg.DATA[GetOpcodeRegisterIndex()];
		WORD temp = GetLWord(reg);
		SetLWord(reg, GetHWord(reg));
		SetHWord(reg, temp);

		AdjustNZ(reg);
		SetFlag(FLAG_VC, false);
	}

	// CLR <ea>
	void CPU68000::CLRb()
	{
		m_eaMode = GetEAMode(m_opcode);

		const BYTE src = 0;
		AdjustNZ(src);

		if (m_eaMode == EAMode::DRegDirect)
		{
			m_reg.GetDATAb(GetOpcodeRegisterIndex()) = src;
		}
		else
		{
			ADDRESS ea = GetEA(OP_BYTE, EAMode::GroupDataAlt);

			// A Memory destination is read before it is written to
			BYTE dest = ReadB(ea);
			dest = src;
			WriteB(ea, dest);
		}
	}
	void CPU68000::CLRw()
	{
		m_eaMode = GetEAMode(m_opcode);

		WORD src = 0;
		AdjustNZ(src);
		SetFlag(FLAG_VC, false);

		if (m_eaMode == EAMode::DRegDirect)
		{
			m_reg.GetDATAw(GetOpcodeRegisterIndex()) = src;
		}
		else
		{
			ADDRESS ea = GetEA(OP_WORD, EAMode::GroupDataAlt);

			// A Memory destination is read before it is written to
			WORD dest = ReadW(ea);
			dest = src;
			WriteW(ea, dest);
		}
	}
	void CPU68000::CLRl()
	{
		m_eaMode = GetEAMode(m_opcode);

		DWORD src = 0;
		AdjustNZ(src);

		if (m_eaMode == EAMode::DRegDirect)
		{
			m_reg.DATA[GetOpcodeRegisterIndex()] = src;
		}
		else
		{
			ADDRESS ea = GetEA(OP_LONG, EAMode::GroupDataAlt);

			// A Memory destination is read before it is written to
			DWORD dest = ReadL(ea);
			dest = src;
			WriteL(ea, dest);
		}
	}

	// Shift, Rotate
	void CPU68000::SHIFT()
	{
		// Shift direction
		bool left = GetBit(m_opcode, 8);

		// Bits 11-9 Contain either operation type (for mem shifts),
		// A data register index, or an immediate value
		int reg_imm_op = (m_opcode >> 9) & 7;

		// Mem or register
		if (Mask_SHIFT_MEM.IsMatch(m_opcode))
		{
			// <SHIFTop>.w <ea>

			// Determine the operation (Bits 11-9)
			switch (reg_imm_op)
			{
			case 0: left ? ASLw()  : ASRw(); break;
			case 1: left ? LSLw()  : LSRw(); break;
			case 2: left ? ROXLw() : ROXRw(); break;
			case 3: left ? ROLw()  : RORw(); break;
			default: IllegalInstruction();
			}
		}
		else // Register Shifts
		{
			// [  15-12  |       11-9       |     8     | 7-6  |   5   |    4-3    |   2-0    ]
			// [ 1 1 1 0 | Count / Register | direction | Size | i / r | operation | register ]

			int rotCount;
			// Rotation amount can be in instruction, or in data register.
			// This is determined with bit 5
			if (GetBit(m_opcode, 5)) // <Shiftop>.size Dx, Dy
			{
				// Register shift count (Dx mod 64)
				rotCount = m_reg.DATA[reg_imm_op] % 64;
			}
			else // <SHIFTop>.size #<data>, Dy
			{
				// Immediate shift count (where 0 means 8)
				rotCount = reg_imm_op ? reg_imm_op : 8;
			}

			//
			int operation = (m_opcode >> 3) & 3;

			const int reg = GetOpcodeRegisterIndex();

			// Get size of operation
			switch ((m_opcode >> 6) & 3)
			{
			case 0: SHIFTb(m_reg.GetDATAb(reg), rotCount, left, operation); break;
			case 1: SHIFTw(m_reg.GetDATAw(reg), rotCount, left, operation); break;
			case 2: SHIFTl(m_reg.DATA[reg], rotCount, left, operation); break;

			default: // 0b11 already handled, it's the <ea> version above
				NODEFAULT;
			}
		}
	}

	void CPU68000::SHIFTb(BYTE& dest, int count, bool left, int operation)
	{
		NotImplementedOpcode("SHIFTb, Dn");
	}
	void CPU68000::SHIFTw(WORD& dest, int count, bool left, int operation)
	{
		switch (operation)
		{
		case 0: left ? ASLw(dest, count) : ASRw(dest, count); break;
		case 1: left ? LSLw(dest, count) : LSRw(dest, count); break;
		case 2: left ? ROXLw(dest, count) : ROXRw(dest, count); break;
		case 3: left ? ROLw(dest, count) : RORw(dest, count); break;
		default:
			NODEFAULT;
		}
	}
	void CPU68000::SHIFTl(DWORD& dest, int count, bool left, int operation)
	{
		switch (operation)
		{
		case 0: left ? ASLl(dest, count)  : ASRl(dest, count); break;
		case 1: left ? LSLl(dest, count)  : LSRl(dest, count); break;
		case 2: left ? ROXLl(dest, count) : ROXRl(dest, count); break;
		case 3: left ? ROLl(dest, count)  : RORl(dest, count); break;
		default:
			NODEFAULT;
		}
	}

	void CPU68000::ASLw(WORD& dest, int count) { NotImplementedOpcode("ADL.w n,Dy"); }
	void CPU68000::ASRw(WORD& dest, int count)
	{
		bool sign = GetMSB(dest);
		bool carry = false;

		for (int i = 0; i < count; ++i)
		{
			carry = GetLSB(dest);

			dest >>= 1;
			SetMSB(dest, sign);
		}

		SetFlag(FLAG_V, false); // MSB never changes for ASR
		SetFlag(FLAG_C, carry);
		if (count)
		{
			SetFlag(FLAG_X, carry);
		}

		AdjustNZ(dest);
	}
	void CPU68000::LSLw(WORD& dest, int count) { NotImplementedOpcode("LSL.w n,Dy"); }
	void CPU68000::LSRw(WORD& dest, int count)
	{
		bool carry = false;

		for (int i = 0; i < count; ++i)
		{
			carry = GetLSB(dest);

			dest >>= 1;
		}

		SetFlag(FLAG_V, false); // Always cleared for LSR
		SetFlag(FLAG_C, carry);
		if (count)
		{
			SetFlag(FLAG_X, carry);
		}

		AdjustNZ(dest);
	}
	void CPU68000::ROXLw(WORD& dest, int count) { NotImplementedOpcode("ROXL.w n,Dy"); }
	void CPU68000::ROXRw(WORD& dest, int count) { NotImplementedOpcode("ROXR.w n,Dy"); }
	void CPU68000::ROLw(WORD& dest, int count) { NotImplementedOpcode("ROL.w n,Dy"); }
	void CPU68000::RORw(WORD& dest, int count) { NotImplementedOpcode("ROR.w n,Dy"); }

	void CPU68000::ASLl(DWORD& dest, int count) { NotImplementedOpcode("ADL.l n,Dy"); }
	void CPU68000::ASRl(DWORD& dest, int count) { NotImplementedOpcode("ASR.l n,Dy"); }
	void CPU68000::LSLl(DWORD& dest, int count) { NotImplementedOpcode("LSL.l n,Dy"); }
	void CPU68000::LSRl(DWORD& dest, int count)
	{
		bool lsb = GetLSB(dest);
		dest >>= count;

		AdjustNZ(dest);
		if (count)
		{
			SetFlag(FLAG_CX, lsb);
		}
		else
		{
			SetFlag(FLAG_C, false);
		}
	}
	void CPU68000::ROXLl(DWORD& dest, int count) { NotImplementedOpcode("ROXL.l n,Dy"); }
	void CPU68000::ROXRl(DWORD& dest, int count) { NotImplementedOpcode("ROXR.l n,Dy"); }
	void CPU68000::ROLl(DWORD& dest, int count) { NotImplementedOpcode("ROL.l n,Dy"); }
	void CPU68000::RORl(DWORD& dest, int count) { NotImplementedOpcode("ROR.l n,Dy"); }


	// Add Dn, <ea>
	void CPU68000::ADDbToEA(BYTE src)
	{
		NotImplementedOpcode("ADD.b Dn -> <ea>");
	}
	void CPU68000::ADDwToEA(WORD src)
	{
		NotImplementedOpcode("ADD.b Dn -> <ea>");
	}
	void CPU68000::ADDlToEA(DWORD src)
	{
		NotImplementedOpcode("ADD.b Dn -> <ea>");
	}

	void CPU68000::ADDQb(BYTE imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			ADDb(m_reg.GetDATAb(GetOpcodeRegisterIndex()), imm);
		}
		else
		{
			ADDRESS ea = GetEA(OP_BYTE, EAMode::GroupDataAlt);
			BYTE dest = ReadB(ea);
			ADDb(dest, imm);
			WriteB(ea, dest);
		}
	}
	void CPU68000::ADDQw(WORD imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			ADDw(m_reg.GetDATAw(GetOpcodeRegisterIndex()), imm);
		}
		else if (m_eaMode == EAMode::ARegDirect)
		{
			m_reg.ADDR[GetOpcodeRegisterIndex()] += imm;
		}
		else
		{
			ADDRESS ea = GetEA(OP_WORD, EAMode::GroupDataAddrAlt);
			WORD dest = ReadW(ea);
			ADDw(dest, imm);
			WriteW(ea, dest);
		}
	}
	void CPU68000::ADDQl(DWORD imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			ADDl(m_reg.DATA[GetOpcodeRegisterIndex()], imm);
		}
		else if (m_eaMode == EAMode::ARegDirect)
		{
			m_reg.ADDR[GetOpcodeRegisterIndex()] += imm;
		}
		else
		{
			ADDRESS ea = GetEA(OP_LONG, EAMode::GroupDataAddrAlt);
			DWORD dest = ReadL(ea);
			ADDl(dest, imm);
			WriteL(ea, dest);
		}
	}

	void CPU68000::SUBQb(BYTE imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SUBb(m_reg.GetDATAb(GetOpcodeRegisterIndex()), imm);
		}
		else
		{
			ADDRESS ea = GetEA(OP_BYTE, EAMode::GroupDataAlt);
			BYTE dest = ReadB(ea);
			SUBb(dest, imm);
			WriteB(ea, dest);
		}
	}
	void CPU68000::SUBQw(WORD imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SUBw(m_reg.GetDATAw(GetOpcodeRegisterIndex()), imm);
		}
		else if (m_eaMode == EAMode::ARegDirect)
		{
			m_reg.ADDR[GetOpcodeRegisterIndex()] -= imm;
		}
		else
		{
			ADDRESS ea = GetEA(OP_WORD, EAMode::GroupDataAddrAlt);
			WORD dest = ReadW(ea);
			SUBw(dest, imm);
			WriteW(ea, dest);
		}
	}
	void CPU68000::SUBQl(DWORD imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SUBl(m_reg.DATA[GetOpcodeRegisterIndex()], imm);
		}
		else if (m_eaMode == EAMode::ARegDirect)
		{
			m_reg.ADDR[GetOpcodeRegisterIndex()] -= imm;
		}
		else
		{
			ADDRESS ea = GetEA(OP_LONG, EAMode::GroupDataAddrAlt);
			DWORD dest = ReadL(ea);
			SUBl(dest, imm);
			WriteL(ea, dest);
		}
	}

	void CPU68000::ADDb(BYTE& dest, BYTE src)
	{
		BYTE oldDest = dest;

		WORD temp = dest + src;

		dest = (BYTE)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU68000::ADDw(WORD& dest, WORD src)
	{
		WORD oldDest = dest;

		DWORD temp = dest + src;

		dest = (WORD)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > 0xFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU68000::ADDl(DWORD& dest, DWORD src)
	{
		DWORD oldDest = dest;

		QWORD temp = (uint64_t)dest + src;

		dest = (DWORD)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > 0xFFFFFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU68000::SUBb(BYTE& dest, BYTE src, FLAG carryFlag)
	{
		BYTE oldDest = dest;

		WORD temp = dest - src;

		dest = (BYTE)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	void CPU68000::SUBw(WORD& dest, WORD src, FLAG carryFlag)
	{
		WORD oldDest = dest;

		DWORD temp = dest - src;

		dest = (WORD)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > 0xFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	void CPU68000::SUBl(DWORD& dest, DWORD src, FLAG carryFlag)
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
