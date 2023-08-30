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
	static const BitMaskW Mask_ANDI_CCR("00111100");
	static const BitMaskW Mask_ANDI_SR("01111100");
	static const BitMaskW Mask_EORI_CCR("00111100");
	static const BitMaskW Mask_EORI_SR("01111100");

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

		m_opcodes[0b0000] = [=]() { Exec(SubOpcodeGroup::b0000, GetSubopcode6()); }; // Immediate,Bit,MOVEP
		m_opcodes[0b0001] = [=]() { MOVEb(); }; // MOVE.b
		m_opcodes[0b0010] = [=]() { MOVEl(); }; // MOVE.l, MOVEA.l
		m_opcodes[0b0011] = [=]() { MOVEw(); }; // MOVE.w, MOVEA.w
		m_opcodes[0b0100] = [=]() { Exec(SubOpcodeGroup::b0100, GetSubopcode6()); }; // Misc
		m_opcodes[0b0101] = [=]() { Exec(SubOpcodeGroup::b0101, GetSubopcode6()); }; // ADDQ,SUBQ,Scc,DBcc
		m_opcodes[0b0110] = [=]() { Exec(SubOpcodeGroup::b0110, GetSubopcode4()); }; // BRA,BSR,Bcc
		m_opcodes[0b0111] = [=]() { MOVEQ(); };
		m_opcodes[0b1000] = [=]() { Exec(SubOpcodeGroup::b1000, GetSubopcode6()); }; // DIVU,DIVS,SBCD,OR
		m_opcodes[0b1001] = [=]() { Exec(SubOpcodeGroup::b1001, GetSubopcode6()); }; // SUB,SUBX,SUBA
		m_opcodes[0b1010] = [=]() { IllegalInstruction(); };
		m_opcodes[0b1011] = [=]() { Exec(SubOpcodeGroup::b1011, GetSubopcode6()); }; // EOR,CMPM,CMP,CMPA
		m_opcodes[0b1100] = [=]() { Exec(SubOpcodeGroup::b1100, GetSubopcode6()); }; // MULU,MULS,ABCD,EXG,AND
		m_opcodes[0b1101] = [=]() { Exec(SubOpcodeGroup::b1101, GetSubopcode6()); }; // ADD,ADDX,ADDA
		m_opcodes[0b1110] = [=]() { SHIFT(); }; // Shift, Rotate
		m_opcodes[0b1111] = [=]() { IllegalInstruction(); };

		InitGroupB0000(m_subOpcodes[(int)SubOpcodeGroup::b0000], 64);
		InitGroupB0100(m_subOpcodes[(int)SubOpcodeGroup::b0100], 64);
		InitGroupB0101(m_subOpcodes[(int)SubOpcodeGroup::b0101], 64);
		InitGroupB0110(m_subOpcodes[(int)SubOpcodeGroup::b0110], 16);
		InitGroupB1000(m_subOpcodes[(int)SubOpcodeGroup::b1000], 64);
		InitGroupB1001(m_subOpcodes[(int)SubOpcodeGroup::b1001], 64);
		InitGroupB1011(m_subOpcodes[(int)SubOpcodeGroup::b1011], 64);
		InitGroupB1100(m_subOpcodes[(int)SubOpcodeGroup::b1100], 64);
		InitGroupB1101(m_subOpcodes[(int)SubOpcodeGroup::b1101], 64);
		InitGroupMisc(m_subOpcodes[(int)SubOpcodeGroup::misc], 56);
	}

	void CPU68000::InitTable(OpcodeTable& table, size_t size)
	{
		table.resize(size);
		std::fill(table.begin(), table.end(), [=]() { IllegalInstruction(); });
	}

	// b0000: Immediate,Bit,MOVEP
	void CPU68000::InitGroupB0000(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[004] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D0w) : BTST(m_reg.D0); };
		table[005] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D0)  : BCHG(m_reg.D0); };
		table[006] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D0w) : BCLR(m_reg.D0); };
		table[007] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D0)  : BSET(m_reg.D0); };

		table[010] = [=]() { Mask_ANDI_CCR.IsMatch(m_opcode) ? ANDIbToCCR() : ANDI<BYTE>(); }; // ANDI #imm, <ea>
		table[011] = [=]() { Mask_ANDI_SR.IsMatch(m_opcode) ? ANDIwToSR() : ANDI<WORD>(); };
		table[012] = [=]() { ANDI<DWORD>(); };

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

		table[050] = [=]() { Mask_EORI_CCR.IsMatch(m_opcode) ? EORIbToCCR() : EORI<BYTE>(); }; // EORI #imm, <ea>
		table[051] = [=]() { Mask_EORI_SR.IsMatch(m_opcode) ? EORIwToSR() : EORI<WORD>(); };
		table[052] = [=]() { EORI<DWORD>(); };

		table[054] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D5w) : BTST(m_reg.D5); };
		table[055] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D5)  : BCHG(m_reg.D5); };
		table[056] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D5w) : BCLR(m_reg.D5); };
		table[057] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D5)  : BSET(m_reg.D5); };

		// CMPI #<data>, <ea>
		table[060] = [=]() { BYTE src = FetchByte(); CMP(GetEAValue<BYTE>(EAMode::GroupDataAlt), src); };
		table[061] = [=]() { WORD src = FetchWord(); CMP(GetEAValue<WORD>(EAMode::GroupDataAlt), src); };
		table[062] = [=]() { DWORD src = FetchLong(); CMP(GetEAValue<DWORD>(EAMode::GroupDataAlt), src); };

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
	void CPU68000::InitGroupB0100(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[007] = [=]() { LEA(m_reg.A0); }; // LEA <ea>,A0

		table[010] = [=]() { CLR<BYTE>(); }; // CLR.b <ea>
		table[011] = [=]() { CLR<WORD>(); }; // CLR.w <ea>
		table[012] = [=]() { CLR<DWORD>(); }; // CLR.l <ea>

		table[017] = [=]() { LEA(m_reg.A1); }; // LEA <ea>,A1
		table[027] = [=]() { LEA(m_reg.A2); }; // LEA <ea>,A2

		// MOVE <ea>, SR
		table[033] = [=]() { Privileged(); MOVE_w_toSR(GetEAValue<WORD>(EAMode::GroupData)); };

		table[037] = [=]() { LEA(m_reg.A3); }; // LEA <ea>,A3

		table[041] = [=]() { Mask_SWAP.IsMatch(m_opcode) ? SWAPw() : NotImplementedOpcode("PEA.l {idx}"); }; // SWAP
		table[042] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEMToEA<WORD>(FetchWord()); }; // MOVEM <register list>, <ea>
		table[043] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEMToEA<DWORD>(FetchWord()); }; // MOVEM <register list>, <ea>

		table[047] = [=]() { LEA(m_reg.A4); }; // LEA <ea>,A4

		table[050] = [=]() { TST<BYTE>(); }; // TST.b <ea>
		table[051] = [=]() { TST<WORD>(); }; // TST.w <ea>
		table[052] = [=]() { TST<DWORD>(); }; // TST.l <ea>

		table[057] = [=]() { LEA(m_reg.A5); }; // LEA <ea>,A5

		// MOVEM <ea>, <register list>
		table[062] = [=]() { MOVEMFromEA<WORD>(FetchWord()); };
		table[063] = [=]() { MOVEMFromEA<DWORD>(FetchWord()); };

		table[067] = [=]() { LEA(m_reg.A6); }; // LEA <ea>,A6
		table[077] = [=]() { LEA(m_reg.A7); }; // LEA <ea>,A7

		table[071] = [=]() { Exec(SubOpcodeGroup::misc, GetSubopcodeLow6()); };
		table[072] = [=]() { JSR(); }; // JSR <ea>
		table[073] = [=]() { JMP(); }; // JMP <ea>
	}

	// b0101: ADDQ,SUBQ,Scc,DBcc
	void CPU68000::InitGroupB0101(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// 0 (=8)
		table[000] = [=]() { ADDQ<BYTE>(8); }; // ADDQ.b #8, <ea>
		table[001] = [=]() { ADDQ<WORD>(8); }; // ADDQ.w #8, <ea>
		table[002] = [=]() { ADDQ<DWORD>(8); }; // ADDQ.l #8, <ea>
		table[003] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(true) : Sccb(true); }; // ST.b <ea> / DBT.w Dn,disp
		table[004] = [=]() { SUBQ<BYTE>(8); }; // SUBQ.b #8, <ea>
		table[005] = [=]() { SUBQ<WORD>(8); }; // SUBQ.w #8, <ea>
		table[006] = [=]() { SUBQ<DWORD>(8); }; // SUBQ.l #8, <ea>
		table[007] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(false) : Sccb(false); }; // SF.b <ea> / DBF.w Dn,disp

		// 1
		table[010] = [=]() { ADDQ<BYTE>(1); }; // ADDQ.b #1, <ea>
		table[011] = [=]() { ADDQ<WORD>(1); }; // ADDQ.w #1, <ea>
		table[012] = [=]() { ADDQ<DWORD>(1); }; // ADDQ.l #1, <ea>
		table[013] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagHI()) : Sccb(FlagHI()); }; // SHI.b <ea> / DBHI.w Dn,disp
		table[014] = [=]() { SUBQ<BYTE>(1); }; // SUBQ.b #1, <ea>
		table[015] = [=]() { SUBQ<WORD>(1); }; // SUBQ.w #1, <ea>
		table[016] = [=]() { SUBQ<DWORD>(1); }; // SUBQ.l #1, <ea>
		table[017] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagLS()) : Sccb(FlagLS()); }; // STLS.b <ea> / DBLS.w Dn,disp

		// 2
		table[020] = [=]() { ADDQ<BYTE>(2); }; // ADDQ.b #2, <ea>
		table[021] = [=]() { ADDQ<WORD>(2); }; // ADDQ.w #2, <ea>
		table[022] = [=]() { ADDQ<DWORD>(2); }; // ADDQ.l #2, <ea>
		table[023] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagCC()) : Sccb(FlagCC()); }; // SCC.b <ea> / DBCC.w Dn,disp
		table[024] = [=]() { SUBQ<BYTE>(2); }; // SUBQ.b #2, <ea>
		table[025] = [=]() { SUBQ<WORD>(2); }; // SUBQ.w #2, <ea>
		table[026] = [=]() { SUBQ<DWORD>(2); }; // SUBQ.l #2, <ea>
		table[027] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagCS()) : Sccb(FlagCS()); }; // SCS.b <ea> / DBCS.w Dn,disp

		// 3
		table[030] = [=]() { ADDQ<BYTE>(3); }; // ADDQ.b #3, <ea>
		table[031] = [=]() { ADDQ<WORD>(3); }; // ADDQ.w #3, <ea>
		table[032] = [=]() { ADDQ<DWORD>(3); }; // ADDQ.l #3, <ea>
		table[033] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagNE()) : Sccb(FlagNE()); }; // SNE.b <ea> / DBNE.w Dn,disp
		table[034] = [=]() { SUBQ<BYTE>(3); }; // SUBQ.b #3, <ea>
		table[035] = [=]() { SUBQ<WORD>(3); }; // SUBQ.w #3, <ea>
		table[036] = [=]() { SUBQ<DWORD>(3); }; // SUBQ.l #3, <ea>
		table[037] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagEQ()) : Sccb(FlagEQ()); }; // SEQ.b <ea> / DBEQ.w Dn,disp

		// 4
		table[040] = [=]() { ADDQ<BYTE>(4); }; // ADDQ.b #4, <ea>
		table[041] = [=]() { ADDQ<WORD>(4); }; // ADDQ.w #4, <ea>
		table[042] = [=]() { ADDQ<DWORD>(4); }; // ADDQ.l #4, <ea>
		table[043] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagVC()) : Sccb(FlagVC()); }; // SVC.b <ea> / DBVC.w Dn,disp
		table[044] = [=]() { SUBQ<BYTE>(4); }; // SUBQ.b #4, <ea>
		table[045] = [=]() { SUBQ<WORD>(4); }; // SUBQ.w #4, <ea>
		table[046] = [=]() { SUBQ<DWORD>(4); }; // SUBQ.l #4, <ea>
		table[047] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagVS()) : Sccb(FlagVS()); }; // SVS.b <ea> / DBVS.w Dn,disp

		// 5
		table[050] = [=]() { ADDQ<BYTE>(5); }; // ADDQ.b #5, <ea>
		table[051] = [=]() { ADDQ<WORD>(5); }; // ADDQ.w #5, <ea>
		table[052] = [=]() { ADDQ<DWORD>(5); }; // ADDQ.l #5, <ea>
		table[053] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagPL()) : Sccb(FlagPL()); }; // SPL.b <ea> / DBPL.w Dn,disp
		table[054] = [=]() { SUBQ<BYTE>(5); }; // SUBQ.b #5, <ea>
		table[055] = [=]() { SUBQ<WORD>(5); }; // SUBQ.w #5, <ea>
		table[056] = [=]() { SUBQ<DWORD>(5); }; // SUBQ.l #5, <ea>
		table[057] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagMI()) : Sccb(FlagMI()); }; // SMI.b <ea> / DBMI.w Dn,disp

		// 6
		table[060] = [=]() { ADDQ<BYTE>(6); }; // ADDQ.b #6, <ea>
		table[061] = [=]() { ADDQ<WORD>(6); }; // ADDQ.w #6, <ea>
		table[062] = [=]() { ADDQ<DWORD>(6); }; // ADDQ.l #6, <ea>
		table[063] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagGE()) : Sccb(FlagGE()); }; // SGE.b <ea> / DBGE.w Dn,disp
		table[064] = [=]() { SUBQ<BYTE>(6); }; // SUBQ.b #6, <ea>
		table[065] = [=]() { SUBQ<WORD>(6); }; // SUBQ.w #6, <ea>
		table[066] = [=]() { SUBQ<DWORD>(6); }; // SUBQ.l #6, <ea>
		table[067] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagLT()) : Sccb(FlagLT()); }; // SLT.b <ea> / DBLT.w Dn,disp

		// 7
		table[070] = [=]() { ADDQ<BYTE>(7); }; // ADDQ.b #7, <ea>
		table[071] = [=]() { ADDQ<WORD>(7); }; // ADDQ.w #7, <ea>
		table[072] = [=]() { ADDQ<DWORD>(7); }; // ADDQ.l #7, <ea>
		table[073] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagGT()) : Sccb(FlagGT()); }; // SGT.b <ea> / DBGT.w Dn,disp
		table[074] = [=]() { SUBQ<BYTE>(7); }; // SUBQ.b #7, <ea>
		table[075] = [=]() { SUBQ<WORD>(7); }; // SUBQ.w #7, <ea>
		table[076] = [=]() { SUBQ<DWORD>(7); }; // SUBQ.l #7, <ea>
		table[073] = [=]() { Mask_DB.IsMatch(m_opcode) ? DBccw(FlagLE()) : Sccb(FlagLE()); }; // SLE.b <ea> / DBLE.w Dn,disp
	}

	// b0110: BRA,BSR,Bcc
	void CPU68000::InitGroupB0110(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[000] = [=]() { BRA(); }; // BRA
		table[001] = [=]() { BSR(); }; // BSR
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
	void CPU68000::InitGroupB1000(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { OR<BYTE>(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D0
		table[001] = [=]() { OR<WORD>(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D0
		table[002] = [=]() { OR<DWORD>(m_reg.D0, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D0

		// D1
		table[010] = [=]() { OR<BYTE>(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D1
		table[011] = [=]() { OR<WORD>(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D1
		table[012] = [=]() { OR<DWORD>(m_reg.D1, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D1

		// D2
		table[020] = [=]() { OR<BYTE>(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D2
		table[021] = [=]() { OR<WORD>(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D2
		table[022] = [=]() { OR<DWORD>(m_reg.D2, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D2

		// D3
		table[030] = [=]() { OR<BYTE>(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D3
		table[031] = [=]() { OR<WORD>(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D3
		table[032] = [=]() { OR<DWORD>(m_reg.D3, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D3

		// D4
		table[040] = [=]() { OR<BYTE>(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D4
		table[041] = [=]() { OR<WORD>(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D4
		table[042] = [=]() { OR<DWORD>(m_reg.D4, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D4

		// D5
		table[050] = [=]() { OR<BYTE>(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D5
		table[051] = [=]() { OR<WORD>(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D5
		table[052] = [=]() { OR<DWORD>(m_reg.D5, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D5

		// D6
		table[060] = [=]() { OR<BYTE>(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D6
		table[061] = [=]() { OR<WORD>(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D6
		table[062] = [=]() { OR<DWORD>(m_reg.D6, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D6

		// D7
		table[070] = [=]() { OR<BYTE>(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D7
		table[071] = [=]() { OR<WORD>(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D7
		table[072] = [=]() { OR<DWORD>(m_reg.D7, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D7
	}

	// b1001: SUB,SUBX,SUBA
	void CPU68000::InitGroupB1001(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// A0
		table[003] = [=]() { SUBA(m_reg.A0, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A0
		table[007] = [=]() { SUBA(m_reg.A0, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A0

		// A1
		table[013] = [=]() { SUBA(m_reg.A1, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A1
		table[017] = [=]() { SUBA(m_reg.A1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A1

		// A2
		table[023] = [=]() { SUBA(m_reg.A2, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A2
		table[027] = [=]() { SUBA(m_reg.A2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A2

		// A3
		table[033] = [=]() { SUBA(m_reg.A3, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A3
		table[037] = [=]() { SUBA(m_reg.A3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A3

		// A4
		table[043] = [=]() { SUBA(m_reg.A4, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A4
		table[047] = [=]() { SUBA(m_reg.A4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A4

		// A5
		table[053] = [=]() { SUBA(m_reg.A5, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A5
		table[057] = [=]() { SUBA(m_reg.A5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A5

		// A6
		table[063] = [=]() { SUBA(m_reg.A6, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A6
		table[067] = [=]() { SUBA(m_reg.A6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A6

		// A7
		table[073] = [=]() { SUBA(m_reg.A7, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A7
		table[077] = [=]() { SUBA(m_reg.A7, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A7
	}

	// b1011: EOR,CMPM,CMP,CMPA
	void CPU68000::InitGroupB1011(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { CMP(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D0
		table[001] = [=]() { CMP(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D0
		table[002] = [=]() { CMP(m_reg.D0, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D0

		table[004] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D0b); }; // EOR.b D0,{idx}
		table[005] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D0w); }; // EOR.b D0,{idx}
		table[006] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D0); }; // EOR.b D0,{idx}

		// D1
		table[010] = [=]() { CMP(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D1
		table[011] = [=]() { CMP(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D1
		table[012] = [=]() { CMP(m_reg.D1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D1

		table[014] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D1b); }; // EOR.b D1,{idx}
		table[015] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D1w); }; // EOR.b D1,{idx}
		table[016] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D1); }; // EOR.b D1,{idx}

		// D2
		table[020] = [=]() { CMP(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D2
		table[021] = [=]() { CMP(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D2
		table[022] = [=]() { CMP(m_reg.D2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D2

		table[024] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D2b); }; // EOR.b D2,{idx}
		table[025] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D2w); }; // EOR.b D2,{idx}
		table[026] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D2); }; // EOR.b D2,{idx}

		// D3
		table[030] = [=]() { CMP(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D3
		table[031] = [=]() { CMP(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D3
		table[032] = [=]() { CMP(m_reg.D3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D3

		table[034] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D3b); }; // EOR.b D3,{idx}
		table[035] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D3w); }; // EOR.b D3,{idx}
		table[036] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D3); }; // EOR.b D3,{idx}

		// D4
		table[040] = [=]() { CMP(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D4
		table[041] = [=]() { CMP(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D4
		table[042] = [=]() { CMP(m_reg.D4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D4

		table[044] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D4b); }; // EOR.b D4,{idx}
		table[045] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D4w); }; // EOR.b D4,{idx}
		table[046] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D4); }; // EOR.b D4,{idx}

		// D5
		table[050] = [=]() { CMP(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D5
		table[051] = [=]() { CMP(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D5
		table[052] = [=]() { CMP(m_reg.D5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D5

		table[054] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D5b); }; // EOR.b D5,{idx}
		table[055] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D5w); }; // EOR.b D5,{idx}
		table[056] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D5); }; // EOR.b D5,{idx}

		// D6
		table[060] = [=]() { CMP(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D6
		table[061] = [=]() { CMP(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D6
		table[062] = [=]() { CMP(m_reg.D6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D6

		table[064] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D6b); }; // EOR.b D6,{idx}
		table[065] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D6w); }; // EOR.b D6,{idx}
		table[066] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D6); }; // EOR.b D6,{idx}

		// D7
		table[070] = [=]() { CMP(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D7
		table[071] = [=]() { CMP(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D7
		table[072] = [=]() { CMP(m_reg.D7, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D7

		table[074] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMb() : EORToEA<BYTE>(m_reg.D7b); }; // EOR.b D7,{idx}
		table[075] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMw() : EORToEA<WORD>(m_reg.D7w); }; // EOR.b D7,{idx}
		table[076] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPMl() : EORToEA<DWORD>(m_reg.D7); }; // EOR.b D7,{idx}
	}

	// b1100: MULU,MULS,ABCD,EXG,AND
	void CPU68000::InitGroupB1100(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { AND(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D0
		table[001] = [=]() { AND(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D0
		table[002] = [=]() { AND(m_reg.D0,  GetEAValue<DWORD>(EAMode::GroupData)); }; // AND.l <ea>,D0
		table[003] = [=]() { MULUw(m_reg.D0); }; // MUL.w <ea>, D0
		table[004] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D0b); }; // AND.b D0,<ea>
		table[005] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D0w); }; // AND.b D0,<ea>
		table[006] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D0); };  // AND.b D0,<ea>

		// D1
		table[010] = [=]() { AND(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D1
		table[011] = [=]() { AND(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D1
		table[012] = [=]() { AND(m_reg.D1,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D1
		table[013] = [=]() { MULUw(m_reg.D1); }; // MUL.w <ea>, D1
		table[014] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D1b); }; // AND.b D1,<ea>
		table[015] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D1w); }; // AND.b D1,<ea>
		table[016] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D1); };  // AND.b D1,<ea>

		// D2
		table[020] = [=]() { AND(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D2
		table[021] = [=]() { AND(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D2
		table[022] = [=]() { AND(m_reg.D2,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D2
		table[023] = [=]() { MULUw(m_reg.D2); }; // MUL.w <ea>, D2
		table[024] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D2b); }; // AND.b D2,<ea>
		table[025] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D2w); }; // AND.b D2,<ea>
		table[026] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D2); };  // AND.b D2,<ea>

		// D3
		table[030] = [=]() { AND(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D3
		table[031] = [=]() { AND(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D3
		table[032] = [=]() { AND(m_reg.D3,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D3
		table[033] = [=]() { MULUw(m_reg.D3); }; // MUL.w <ea>, D3
		table[034] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D3b); }; // AND.b D3,<ea>
		table[035] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D3w); }; // AND.b D3,<ea>
		table[036] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D3); };  // AND.b D3,<ea>

		// D4
		table[040] = [=]() { AND(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D4
		table[041] = [=]() { AND(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D4
		table[042] = [=]() { AND(m_reg.D4,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D4
		table[043] = [=]() { MULUw(m_reg.D4); }; // MUL.w <ea>, D4
		table[044] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D4b); }; // AND.b D4,<ea>
		table[045] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D4w); }; // AND.b D4,<ea>
		table[046] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D4); };  // AND.b D4,<ea>

		// D5
		table[050] = [=]() { AND(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D5
		table[051] = [=]() { AND(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D5
		table[052] = [=]() { AND(m_reg.D5,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D5
		table[053] = [=]() { MULUw(m_reg.D5); }; // MUL.w <ea>, D5
		table[054] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D5b); }; // AND.b D5,<ea>
		table[055] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D5w); }; // AND.b D5,<ea>
		table[056] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D5); };  // AND.b D5,<ea>

		// D6
		table[060] = [=]() { AND(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D6
		table[061] = [=]() { AND(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D6
		table[062] = [=]() { AND(m_reg.D6,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D6
		table[063] = [=]() { MULUw(m_reg.D6); }; // MUL.w <ea>, D6
		table[064] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D6b); }; // AND.b D6,<ea>
		table[065] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D6w); }; // AND.b D6,<ea>
		table[066] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D6); };  // AND.b D6,<ea>

		// D7
		table[070] = [=]() { AND(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D7
		table[071] = [=]() { AND(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D7
		table[072] = [=]() { AND(m_reg.D7,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D7
		table[073] = [=]() { MULUw(m_reg.D7); }; // MUL.w <ea>, D7
		table[074] = [=]() { Mask_ABCD.IsMatch(m_opcode) ? ABCDb() : ANDbToEA(m_reg.D7b); }; // AND.b D7,<ea>
		table[075] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDwToEA(m_reg.D7w); }; // AND.b D7,<ea>
		table[076] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDlToEA(m_reg.D7); };  // AND.b D7,<ea>
	}

	// b1101: ADD,ADDX,ADDA
	void CPU68000::InitGroupB1101(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { ADD(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D0
		table[001] = [=]() { ADD(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D0
		table[002] = [=]() { ADD(m_reg.D0,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D0
		table[003] = [=]() { ADDA(m_reg.A0, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A0
		table[004] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D0b); }; // ADD.b D0,<ea>
		table[005] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D0w); }; // ADD.b D0,<ea>
		table[006] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D0); };  // ADD.b D0,<ea>
		table[007] = [=]() { ADDA(m_reg.A0, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A0

		// D1
		table[010] = [=]() { ADD(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D1
		table[011] = [=]() { ADD(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D1
		table[012] = [=]() { ADD(m_reg.D1,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D1
		table[013] = [=]() { ADDA(m_reg.A1, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A1
		table[014] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D1b); }; // ADD.b D1,<ea>
		table[015] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D1w); }; // ADD.b D1,<ea>
		table[016] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D1); };  // ADD.b D1,<ea>
		table[017] = [=]() { ADDA(m_reg.A1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A1

		// D2
		table[020] = [=]() { ADD(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D2
		table[021] = [=]() { ADD(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D2
		table[022] = [=]() { ADD(m_reg.D2,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D2
		table[023] = [=]() { ADDA(m_reg.A2, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A2
		table[024] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D2b); }; // ADD.b D2,<ea>
		table[025] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D2w); }; // ADD.b D2,<ea>
		table[026] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D2); };  // ADD.b D2,<ea>
		table[027] = [=]() { ADDA(m_reg.A2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A2

		// D3
		table[030] = [=]() { ADD(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D3
		table[031] = [=]() { ADD(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D3
		table[032] = [=]() { ADD(m_reg.D3,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D3
		table[033] = [=]() { ADDA(m_reg.A3, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A3
		table[034] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D3b); }; // ADD.b D3,<ea>
		table[035] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D3w); }; // ADD.b D3,<ea>
		table[036] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D3); };  // ADD.b D3,<ea>
		table[037] = [=]() { ADDA(m_reg.A3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A3

		// D4
		table[040] = [=]() { ADD(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D4
		table[041] = [=]() { ADD(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D4
		table[042] = [=]() { ADD(m_reg.D4,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D4
		table[043] = [=]() { ADDA(m_reg.A4, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A4
		table[044] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D4b); }; // ADD.b D4,<ea>
		table[045] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D4w); }; // ADD.b D4,<ea>
		table[046] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D4); };  // ADD.b D4,<ea>
		table[047] = [=]() { ADDA(m_reg.A4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A4

		// D5
		table[050] = [=]() { ADD(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D5
		table[051] = [=]() { ADD(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D5
		table[052] = [=]() { ADD(m_reg.D5,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D5
		table[053] = [=]() { ADDA(m_reg.A5, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A5
		table[054] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D5b); }; // ADD.b D5,<ea>
		table[055] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D5w); }; // ADD.b D5,<ea>
		table[056] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D5); };  // ADD.b D5,<ea>
		table[057] = [=]() { ADDA(m_reg.A5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A5

		// D6
		table[060] = [=]() { ADD(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D6
		table[061] = [=]() { ADD(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D6
		table[062] = [=]() { ADD(m_reg.D6,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D6
		table[063] = [=]() { ADDA(m_reg.A6, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A6
		table[064] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D6b); }; // ADD.b D6,<ea>
		table[065] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D6w); }; // ADD.b D6,<ea>
		table[066] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D6); };  // ADD.b D6,<ea>
		table[067] = [=]() { ADDA(m_reg.A6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A6

		// D7
		table[070] = [=]() { ADD(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D7
		table[071] = [=]() { ADD(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupData)); }; // ADD.w <ea>,D7
		table[072] = [=]() { ADD(m_reg.D7,  GetEAValue<DWORD>(EAMode::GroupData)); }; // ADD.l <ea>,D7
		table[073] = [=]() { ADDA(m_reg.A7, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A7
		table[074] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXb() : ADDbToEA(m_reg.D7b); }; // ADD.b D7,<ea>
		table[075] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXw() : ADDwToEA(m_reg.D7w); }; // ADD.b D7,<ea>
		table[076] = [=]() { Mask_ADDX.IsMatch(m_opcode) ? ADDXl() : ADDlToEA(m_reg.D7); };  // ADD.b D7,<ea>
		table[077] = [=]() { ADDA(m_reg.A7, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A7
	}

	void CPU68000::InitGroupMisc(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[065] = [=]() { RTS(); }; // RTS
	}

	void CPU68000::Reset()
	{
		CPU::Reset();

		// TODO: Check if values are reset
		m_reg.SSP = 0;
		m_reg.USP = 0;

		std::fill(std::begin(m_reg.DataAddress), std::end(m_reg.DataAddress), 0);

		ADDRESS reset = Read<DWORD>(GetVectorAddress(VECTOR::ResetPC));
		ADDRESS ssp = Read<DWORD>(GetVectorAddress(VECTOR::ResetSSP));

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
		WORD w = Read<WORD>(GetCurrentAddress());
		m_programCounter += 2;
		m_programCounter &= ADDRESS_MASK;
		return w;
	}

	DWORD CPU68000::FetchLong()
	{
		DWORD dw = Read<DWORD>(GetCurrentAddress());
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

	void CPU68000::Exec(SubOpcodeGroup group, WORD subOpcode)
	{
		auto& opFunc = m_subOpcodes[(int)group][subOpcode];
		opFunc();
	}

	void CPU68000::Interrupt()
	{

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

	template<typename SIZE>
	SIZE CPU68000::GetEAValue(EAMode groupCheck)
	{
		constexpr int immTickPenalty = (sizeof(SIZE) == 4) ? 8 : 4;

		m_eaMode = GetEAMode(m_opcode);
		EACheck(groupCheck);

		const int reg = GetOpcodeRegisterIndex();

		switch (GetEAMode(m_opcode))
		{
		case EAMode::DRegDirect:
			return m_reg.GetDATA<SIZE>(reg);
		case EAMode::ARegDirect:
			return m_reg.GetADDR<SIZE>(reg);
		case EAMode::Immediate:
			TICKn(immTickPenalty);
			return Fetch<SIZE>();
		default:
			return Read<SIZE>(rawGetEA<SIZE>());
		}
	}

	template<typename SIZE>
	ADDRESS CPU68000::rawGetEA()
	{
		static_assert(std::is_integral<SIZE>::value, "Integral required.");

		constexpr int size = sizeof(SIZE);
		static_assert(size == 1 || size == 2 || size == 4, "BYTE/WORD/DWORD required.");

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
		case EAMode::ARegIndirect:
			ea = An;
			timePenalty = 4;
			break;

			// Address Register Indirect with Postincrement, EA=(An), An += N
		case EAMode::ARegIndirectPostinc:
			ea = An;
			An += size;
			timePenalty = 4;
			break;

			// Address Register Indirect with Predecrement, AN -= N, EA=(An)
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
			ea = An;
			ea += GetExtensionWordDisp();
			timePenalty = 10;
			break;

			// Absolute Short Address, EA given
		case EAMode::AbsShort:
			ea = Widen(FetchWord());
			timePenalty = 8;
			break;

			// Absolute Long Address, EA given
		case EAMode::AbsLong:
			ea = FetchLong();
			timePenalty = 12;
			break;

			// Program Counter with Displacement, EA = (PC) + d
		case EAMode::PCDisp:
			ea = m_programCounter;
			ea += Widen(FetchWord());
			timePenalty = 8;
			break;

			// Program Counter with Index, EA = (PC) + (Ri) + d
		case EAMode::PCIndex:
			ea = m_programCounter;
			ea += GetExtensionWordDisp();
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

	ADDRESS CPU68000::GetExtensionWordDisp()
	{
		// Extension word:
		// [ 15  | 14 13 12 | 11  | 10-8  |         7-0          ]
		// [ D/A | Register | W/L | 0 0 0 | Displacement Integer ]
		const WORD extWord = FetchWord();

		ADDRESS disp = DoubleWiden(GetLByte(extWord));

		const int regIndex = extWord >> 12;
		DWORD reg = m_reg.DataAddress[regIndex];

		// 1 = long, 0 = sign-extend low word
		bool wideLong = GetBit(extWord, 11);
		if (wideLong)
		{
			disp += reg;
		}
		else
		{
			disp += Widen(GetLWord(reg));
		}

		return disp;
	}

	void CPU68000::LEA(DWORD& dest)
	{
		ADDRESS src = GetEA<DWORD>(EAMode::GroupControl);

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
		BYTE source = GetEAValue<BYTE>(EAMode::GroupData);
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
			ADDRESS dest = GetEA<BYTE>(EAMode::GroupDataAlt);
			Write<BYTE>(dest, source);
		}
	}

	void CPU68000::MOVEw()
	{
		WORD source = GetEAValue<WORD>(EAMode::GroupAll);
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
			ADDRESS dest = GetEA<WORD>(EAMode::GroupDataAlt);
			Write<WORD>(dest, source);
		}
	}

	void CPU68000::MOVEl()
	{
		DWORD source = GetEAValue<DWORD>(EAMode::GroupAll);
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
			ADDRESS dest = GetEA<DWORD>(EAMode::GroupDataAlt);
			Write<DWORD>(dest, source);
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

	template<typename SIZE>
	void CPU68000::MOVEMFromEA(WORD regs)
	{
		constexpr int increment = sizeof(SIZE);
		ADDRESS src = GetEA<SIZE>(EAMode::GroupControlAltPostinc);

		// Set only for postincrement mode, where
		// we need we iterate directly through the address reg
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPostinc) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		if (addrReg)
		{
			// Postincrement was already done in GetEA but we handle things manually here
			// (alternative would be to call GetEA() for each iteration

			// TODO: Check address pos when no bits set
			*addrReg -= increment;
		}

		DWORD* dest = m_reg.DataAddress.data();
		for (int i = 0; i < 16; ++i, ++dest)
		{
			if (GetLSB(regs))
			{
				if (addrReg)
				{
					*dest = Widen(Read<SIZE>(*addrReg));
					*addrReg += increment;
				}
				else
				{
					*dest = Widen(Read<SIZE>(src));
					src += increment;
				}
			}

			regs >>= 1;
		}
	}

	template<typename SIZE>
	void CPU68000::MOVEMToEA(WORD regs)
	{
		constexpr int incdec = sizeof(SIZE);
		ADDRESS dest = GetEA<SIZE>(EAMode::GroupControlAltPredec);

		// Set only for predecrement mode, where
		// we need to update its value at the end
		DWORD* addrReg = (m_eaMode == EAMode::ARegIndirectPredec) ?
			&m_reg.ADDR[GetOpcodeRegisterIndex()] : nullptr;

		if (addrReg)
		{
			// Predecrement was already done in GetEA but we handle things manually here
			// (alternative would be to call GetEA() for each iteration
			// TODO: Check address pos when no bits set
			*addrReg += incdec;
		}

		// Start from the end in predecrement mode
		DWORD* src = m_reg.DataAddress.data() + (addrReg ? 15 : 0);

		// Go backwards through register list in predecrement mode
		int direction = addrReg ? -1 : 1;
		for (int i = 0; i < 16; ++i, src += direction)
		{
			if (GetLSB(regs))
			{
				if (addrReg)
				{
					*addrReg -= incdec;
					Write<SIZE>(*addrReg, SIZE(*src));
				}
				else
				{
					Write<SIZE>(dest, SIZE(*src));
					dest += incdec;
				}
			}

			regs >>= 1;
		}
	}

	// MOVEP.w Dx, d(Ay)
	void CPU68000::MOVEPwFromReg(WORD src)
	{
		const int addrIndex = m_opcode & 7;
		const WORD disp = FetchWord();
		const ADDRESS dest = m_reg.ADDR[addrIndex] + Widen(disp);

		Write<BYTE>(dest, GetHByte(src));
		Write<BYTE>(dest + 2, GetLByte(src));
	}

	void CPU68000::BRA(bool cond)
	{
		ADDRESS rel = DoubleWiden(GetLByte(m_opcode));

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
		DWORD addr = GetEA<DWORD>(EAMode::GroupControl);
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

	void CPU68000::BSR()
	{
		ADDRESS disp = DoubleWiden(GetLByte(m_opcode));
		if (disp == 0)
		{
			disp = Widen(FetchWord());
		}

		PUSH(m_programCounter);

		m_programCounter += disp;
		m_programCounter &= ADDRESS_MASK;
	}

	void CPU68000::JSR()
	{
		DWORD addr = GetEA<DWORD>(EAMode::GroupControl);

		PUSH(m_programCounter);

		m_programCounter = addr;
	}


	void CPU68000::RTS()
	{
		m_programCounter = POP();
	}

	DWORD CPU68000::POP()
	{
		DWORD val = Read<DWORD>(m_reg.SP);
		m_reg.SP += 4;
		return val;
	}

	void CPU68000::PUSH(DWORD src)
	{
		m_reg.SP -= 4;
		Write(m_reg.SP, src);
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
			BYTE src = GetEAValue<BYTE>(EAMode::GroupDataNoImm);
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
			ADDRESS addr = GetEA<BYTE>(EAMode::GroupDataNoImm);
			BYTE dest = Read<BYTE>(addr);

			testBit = GetBit(dest, bitNumber);
			bool newBit = (bitOp == BitOp::SET) ? 1 : ((bitOp == BitOp::CLEAR) ? 0 : !testBit);
			SetBit(dest, bitNumber, newBit);

			Write<BYTE>(addr, dest);
		}
		SetFlag(FLAG_Z, testBit);
	}

	// TST <ea>
	template<typename SIZE>
	void CPU68000::TST()
	{
		SIZE src = GetEAValue<SIZE>(EAMode::GroupDataAlt);
		AdjustNZ(src);
		SetFlag(FLAG_VC, false);
	}

	// AND <ea>, Dn
	template<typename SIZE>
	void CPU68000::AND(SIZE& dest, SIZE src)
	{
		dest &= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	// AND #imm, <ea>
	template<typename SIZE>
	void CPU68000::ANDI()
	{
		SIZE imm = Fetch<SIZE>();

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());
			AND<SIZE>(dest, imm);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			SIZE dest = Read<SIZE>(ea);
			AND<SIZE>(dest, imm);
			Write<SIZE>(ea, dest);
		}
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

	template<typename SIZE>
	void CPU68000::OR(SIZE& dest, SIZE src)
	{
		dest |= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	// EORI #imm, <ea>
	template<typename SIZE>
	void CPU68000::EORI()
	{
		SIZE imm = Fetch<SIZE>();

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());
			EOR<SIZE>(dest, imm);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			SIZE dest = Read<SIZE>(ea);
			EOR<SIZE>(dest, imm);
			Write<SIZE>(ea, dest);
		}
	}

	template<typename SIZE>
	void CPU68000::EOR(SIZE& dest, SIZE src)
	{
		dest ^= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	template<typename SIZE>
	void CPU68000::EORToEA(SIZE src)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());
			EOR<SIZE>(dest, src);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			SIZE dest = Read<SIZE>(ea);
			EOR<SIZE>(dest, src);
			Write<SIZE>(ea, dest);
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
	template<typename SIZE>
	void CPU68000::CLR()
	{
		m_eaMode = GetEAMode(m_opcode);

		const SIZE src = 0;
		AdjustNZ(src);
		SetFlag(FLAG_VC, false);

		if (m_eaMode == EAMode::DRegDirect)
		{
			m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex()) = src;
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			// A Memory destination is read before it is written to
			SIZE dest = Read<SIZE>(ea);
			dest = src;
			Write<SIZE>(ea, dest);
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
			case 0: SHIFT<BYTE>(m_reg.GetDATA<BYTE>(reg), rotCount, left, operation); break;
			case 1: SHIFT<WORD>(m_reg.GetDATA<WORD>(reg), rotCount, left, operation); break;
			case 2: SHIFT<DWORD>(m_reg.GetDATA<DWORD>(reg), rotCount, left, operation); break;

			default: // 0b11 already handled, it's the <ea> version above
				NODEFAULT;
			}
		}
	}

	template<typename SIZE>
	void CPU68000::SHIFT(SIZE& dest, int count, bool left, int operation)
	{
		switch (operation)
		{
		case 0: left ? ASL(dest, count)  : ASR(dest, count); break;
		case 1: left ? LSL(dest, count)  : LSR(dest, count); break;
		case 2: left ? ROXL(dest, count) : ROXR(dest, count); break;
		case 3: left ? ROL(dest, count)  : ROR(dest, count); break;
		default:
			NODEFAULT;
		}
	}

	template<typename SIZE>
	void CPU68000::ASL(SIZE& dest, int count)
	{
		bool carry = false;
		bool msbChanged = false;

		for (int i = 0; i < count; ++i)
		{
			carry = GetMSB(dest);

			dest <<= 1;

			if (GetMSB(dest) != carry)
			{
				msbChanged = true;
			}
		}

		SetFlag(FLAG_V, msbChanged);
		SetFlag(FLAG_C, carry);
		if (count)
		{
			SetFlag(FLAG_X, carry);
		}

		AdjustNZ(dest);
	}
	template<typename SIZE>
	void CPU68000::ASR(SIZE& dest, int count)
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

	template<typename SIZE>
	void CPU68000::LSL(SIZE& dest, int count)
	{
		bool carry = false;

		for (int i = 0; i < count; ++i)
		{
			carry = GetMSB(dest);

			dest <<= 1;
		}

		SetFlag(FLAG_V, false); // Always cleared for LSL
		SetFlag(FLAG_C, carry);
		if (count)
		{
			SetFlag(FLAG_X, carry);
		}

		AdjustNZ(dest);
	}
	template<typename SIZE>
	void CPU68000::LSR(SIZE& dest, int count)
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

	template<typename SIZE>
	void CPU68000::ROL(SIZE& dest, int count)
	{
		bool carry = GetFlag(FLAG_C);

		for (int i = 0; i < count; ++i)
		{
			bool msb = GetMSB(dest);
			dest <<= 1;
			SetLSB(dest, carry);
			carry = msb;
		}

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, false); // Always cleared for ROL

		AdjustNZ(dest);
	}
	template<typename SIZE>
	void CPU68000::ROR(SIZE& dest, int count)
	{
		NotImplementedOpcode("ROR n,Dy");
	}

	template<typename SIZE>
	void CPU68000::ROXL(SIZE& dest, int count)
	{
		NotImplementedOpcode("ROXL n,Dy");
	}
	template<typename SIZE>
	void CPU68000::ROXR(SIZE& dest, int count)
	{
		NotImplementedOpcode("ROXR n,Dy");
	}

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

	template<typename SIZE>
	void CPU68000::ADDQ(SIZE imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			ADD<SIZE>(m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex()), imm);
		}
		// Address Register Direct only for WORD and LONG
		else if ((m_eaMode == EAMode::ARegDirect) && (sizeof(SIZE) > 1))
		{
			// Behaves like SUBA
			ADDA(m_reg.ADDR[GetOpcodeRegisterIndex()], imm);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);
			SIZE dest = Read<SIZE>(ea);
			ADD<SIZE>(dest, imm);
			Write<SIZE>(ea, dest);
		}
	}

	template<typename SIZE>
	void CPU68000::SUBQ(SIZE imm)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SUB<SIZE>(m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex()), imm);
		}
		// Address Register Direct only for WORD and LONG
		else if ((m_eaMode == EAMode::ARegDirect) && (sizeof(SIZE) > 1))
		{
			// Behaves like SUBA
			SUBA(m_reg.ADDR[GetOpcodeRegisterIndex()], imm);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);
			SIZE dest = Read<SIZE>(ea);
			SUB<SIZE>(dest, imm);
			Write<SIZE>(ea, dest);
		}
	}

	template<typename SIZE>
	void CPU68000::ADD(SIZE& dest, SIZE src)
	{
		constexpr size_t size_max = std::numeric_limits<SIZE>::max();

		SIZE oldDest = dest;

		QWORD temp = (QWORD)dest + src;

		dest = (SIZE)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > size_max));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	template<typename SIZE>
	void CPU68000::SUB(SIZE& dest, SIZE src, FLAG carryFlag)
	{
		constexpr size_t size_max = std::numeric_limits<SIZE>::max();

		SIZE oldDest = dest;

		QWORD temp = (QWORD)dest - src;

		dest = (SIZE)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > size_max));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	void CPU68000::MULUw(DWORD& dest)
	{
		WORD op1 = GetEAValue<WORD>(EAMode::GroupData);
		WORD op2 = GetLWord(dest);

		dest = op1 * op2;
		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
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
