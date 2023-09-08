#include "stdafx.h"
#include "CPU68000.h"

using cpuInfo::Opcode;

namespace emul::cpu68k
{
	static const BitMaskW Mask_SWAP("000xxx");
	static const BitMaskW Mask_EXT("000xxx");
	static const BitMaskW Mask_DB("001xxx");
	static const BitMaskW Mask_CMPM("001xxx");
	static const BitMaskW Mask_EXG("00xxxx");
	static const BitMaskW Mask_BCD("00xxxx");
	static const BitMaskW Mask_X("00xxxx");
	static const BitMaskW Mask_CCR("00111100");
	static const BitMaskW Mask_SR("01111100");
	static const BitMaskW Mask_MOVEA   ("001xxxxxx");
	static const BitMaskW Mask_MOVEP   ("1xx001xxx");
	static const BitMaskW Mask_SHIFT_MEM("11xxxxxx");
	static const BitMaskW Mask_ILLEGAL    ("111100");

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
		m_opcodes[0b0001] = [=]() { MOVE<BYTE>(); }; // MOVE.b
		m_opcodes[0b0010] = [=]() { Mask_MOVEA.IsMatch(m_opcode) ? MOVEA<DWORD>() : MOVE<DWORD>(); }; // MOVE.l, MOVEA.l
		m_opcodes[0b0011] = [=]() { Mask_MOVEA.IsMatch(m_opcode) ? MOVEA<WORD>()  : MOVE<WORD>(); }; // MOVE.w, MOVEA.w
		m_opcodes[0b0100] = [=]() { Exec(SubOpcodeGroup::b0100, GetSubopcode6()); }; // Misc
		m_opcodes[0b0101] = [=]() { Exec(SubOpcodeGroup::b0101, GetSubopcode6()); }; // ADDQ,SUBQ,Scc,DBcc
		m_opcodes[0b0110] = [=]() { Exec(SubOpcodeGroup::b0110, GetSubopcode4()); }; // BRA,BSR,Bcc
		m_opcodes[0b0111] = [=]() { MOVEQ(); };
		m_opcodes[0b1000] = [=]() { Exec(SubOpcodeGroup::b1000, GetSubopcode6()); }; // DIVU,DIVS,SBCD,OR
		m_opcodes[0b1001] = [=]() { Exec(SubOpcodeGroup::b1001, GetSubopcode6()); }; // SUB,SUBX,SUBA
		m_opcodes[0b1010] = [=]() { Exception(VECTOR::Line1010Emulator); };
		m_opcodes[0b1011] = [=]() { Exec(SubOpcodeGroup::b1011, GetSubopcode6()); }; // EOR,CMPM,CMP,CMPA
		m_opcodes[0b1100] = [=]() { Exec(SubOpcodeGroup::b1100, GetSubopcode6()); }; // MULU,MULS,ABCD,EXG,AND
		m_opcodes[0b1101] = [=]() { Exec(SubOpcodeGroup::b1101, GetSubopcode6()); }; // ADD,ADDX,ADDA
		m_opcodes[0b1110] = [=]() { SHIFT(); }; // Shift, Rotate
		m_opcodes[0b1111] = [=]() { Exception(VECTOR::Line1111Emulator); };

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

		table[000] = [=]() { Mask_CCR.IsMatch(m_opcode) ? ORIbToCCR() : ORI<BYTE>(); }; // ORI #imm, <ea>
		table[001] = [=]() { Mask_SR.IsMatch(m_opcode)  ? ORIwToSR()  : ORI<WORD>(); };
		table[002] = [=]() { ORI<DWORD>(); };

		table[004] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D0w) : BTST(m_reg.D0b); };
		table[005] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D0)  : BCHG(m_reg.D0b); };
		table[006] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D0w) : BCLR(m_reg.D0b); };
		table[007] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D0)  : BSET(m_reg.D0b); };

		table[010] = [=]() { Mask_CCR.IsMatch(m_opcode) ? ANDIbToCCR() : ANDI<BYTE>(); }; // ANDI #imm, <ea>
		table[011] = [=]() { Mask_SR.IsMatch(m_opcode)  ? ANDIwToSR()  : ANDI<WORD>(); };
		table[012] = [=]() { ANDI<DWORD>(); };

		table[014] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D1w) : BTST(m_reg.D1b); };
		table[015] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D1)  : BCHG(m_reg.D1b); };
		table[016] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D1w) : BCLR(m_reg.D1b); };
		table[017] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D1)  : BSET(m_reg.D1b); };

		table[020] = [=]() { SUBI<BYTE>(); };
		table[021] = [=]() { SUBI<WORD>(); };
		table[022] = [=]() { SUBI<DWORD>(); };

		table[024] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D2w) : BTST(m_reg.D2b); };
		table[025] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D2)  : BCHG(m_reg.D2b); };
		table[026] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D2w) : BCLR(m_reg.D2b); };
		table[027] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D2)  : BSET(m_reg.D2b); };

		table[030] = [=]() { ADDI<BYTE>(); };
		table[031] = [=]() { ADDI<WORD>(); };
		table[032] = [=]() { ADDI<DWORD>(); };

		table[034] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D3w) : BTST(m_reg.D3b); };
		table[035] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D3)  : BCHG(m_reg.D3b); };
		table[036] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D3w) : BCLR(m_reg.D3b); };
		table[037] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D3)  : BSET(m_reg.D3b); };

		table[040] = [=]() { BTSTimm(); };
		table[041] = [=]() { BCHGimm(); };
		table[042] = [=]() { BCLRimm(); };
		table[043] = [=]() { BSETimm(); };

		table[044] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D4w) : BTST(m_reg.D4b); };
		table[045] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D4)  : BCHG(m_reg.D4b); };
		table[046] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D4w) : BCLR(m_reg.D4b); };
		table[047] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D4)  : BSET(m_reg.D4b); };

		table[050] = [=]() { Mask_CCR.IsMatch(m_opcode) ? EORIbToCCR() : EORI<BYTE>(); }; // EORI #imm, <ea>
		table[051] = [=]() { Mask_SR.IsMatch(m_opcode) ? EORIwToSR() : EORI<WORD>(); };
		table[052] = [=]() { EORI<DWORD>(); };

		table[054] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D5w) : BTST(m_reg.D5b); };
		table[055] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D5)  : BCHG(m_reg.D5b); };
		table[056] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D5w) : BCLR(m_reg.D5b); };
		table[057] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D5)  : BSET(m_reg.D5b); };

		// CMPI #<data>, <ea>
		table[060] = [=]() { BYTE src = FetchByte(); CMP(GetEAValue<BYTE>(EAMode::GroupDataAlt), src); };
		table[061] = [=]() { WORD src = FetchWord(); CMP(GetEAValue<WORD>(EAMode::GroupDataAlt), src); };
		table[062] = [=]() { DWORD src = FetchLong(); CMP(GetEAValue<DWORD>(EAMode::GroupDataAlt), src); };

		table[064] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D6w) : BTST(m_reg.D6b); };
		table[065] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D6)  : BCHG(m_reg.D6b); };
		table[066] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D6w) : BCLR(m_reg.D6b); };
		table[067] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D6)  : BSET(m_reg.D6b); };

		table[074] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPwToReg(m_reg.D7w) : BTST(m_reg.D7b); };
		table[075] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ?   MOVEPlToReg(m_reg.D7)  : BCHG(m_reg.D7b); };
		table[076] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPwFromReg(m_reg.D7w) : BCLR(m_reg.D7b); };
		table[077] = [=]() { Mask_MOVEP.IsMatch(m_opcode) ? MOVEPlFromReg(m_reg.D7)  : BSET(m_reg.D7b); };

	}

	// b0100: Misc
	void CPU68000::InitGroupB0100(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[000] = [=]() { NEGX<BYTE>(); }; // NEGX.b <ea>
		table[001] = [=]() { NEGX<WORD>(); }; // NEGX.w <ea>
		table[002] = [=]() { NEGX<DWORD>(); }; // NEGX.l <ea>

		table[003] = [=]() { MOVEwFromSR(); }; // MOVE.w SR,<ea>
		table[006] = [=]() { CHK((SWORD)m_reg.D0w); }; // CHK.w <ea>,D0
		table[007] = [=]() { LEA(m_reg.A0); }; // LEA <ea>,A0

		table[010] = [=]() { CLR<BYTE>(); }; // CLR.b <ea>
		table[011] = [=]() { CLR<WORD>(); }; // CLR.w <ea>
		table[012] = [=]() { CLR<DWORD>(); }; // CLR.l <ea>

		table[016] = [=]() { CHK((SWORD)m_reg.D1w); }; // CHK.w <ea>,D1
		table[017] = [=]() { LEA(m_reg.A1); }; // LEA <ea>,A1

		table[020] = [=]() { NEG<BYTE>(); }; // NEG.b <ea>
		table[021] = [=]() { NEG<WORD>(); }; // NEG.w <ea>
		table[022] = [=]() { NEG<DWORD>(); }; // NEG.l <ea>
		table[023] = [=]() { MOVEwToCCR(GetEAValue<WORD>(EAMode::GroupData)); };

		table[026] = [=]() { CHK((SWORD)m_reg.D2w); }; // CHK.w <ea>,D2
		table[027] = [=]() { LEA(m_reg.A2); }; // LEA <ea>,A2

		table[030] = [=]() { NOT<BYTE>(); }; // NOT.b <ea>
		table[031] = [=]() { NOT<WORD>(); }; // NOT.b <ea>
		table[032] = [=]() { NOT<DWORD>(); }; // NOT.b <ea>

		// MOVE <ea>, SR
		table[033] = [=]() { MOVEwToSR(GetEAValue<WORD>(EAMode::GroupData)); };

		table[036] = [=]() { CHK((SWORD)m_reg.D3w); }; // CHK.w <ea>,D3
		table[037] = [=]() { LEA(m_reg.A3); }; // LEA <ea>,A3

		table[040] = [=]() { NBCDb(); }; // NBCD.b <ea>
		table[041] = [=]() { Mask_SWAP.IsMatch(m_opcode) ? SWAPw() : PEAl(); }; // SWAP, PEAl
		table[042] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTw() : MOVEMToEA<WORD>(FetchWord()); }; // MOVEM <register list>, <ea>
		table[043] = [=]() { Mask_EXT.IsMatch(m_opcode) ? EXTl() : MOVEMToEA<DWORD>(FetchWord()); }; // MOVEM <register list>, <ea>

		table[046] = [=]() { CHK((SWORD)m_reg.D4w); }; // CHK.w <ea>,D4
		table[047] = [=]() { LEA(m_reg.A4); }; // LEA <ea>,A4

		table[050] = [=]() { TST<BYTE>(); }; // TST.b <ea>
		table[051] = [=]() { TST<WORD>(); }; // TST.w <ea>
		table[052] = [=]() { TST<DWORD>(); }; // TST.l <ea>
		table[053] = [=]() { Mask_ILLEGAL.IsMatch(m_opcode) ? IllegalInstruction() : TASb(); }; // TAS.b <ea>

		table[056] = [=]() { CHK((SWORD)m_reg.D5w); }; // CHK.w <ea>,D5
		table[057] = [=]() { LEA(m_reg.A5); }; // LEA <ea>,A5

		// MOVEM <ea>, <register list>
		table[062] = [=]() { MOVEMFromEA<WORD>(FetchWord()); };
		table[063] = [=]() { MOVEMFromEA<DWORD>(FetchWord()); };

		table[066] = [=]() { CHK((SWORD)m_reg.D6w); }; // CHK.w <ea>,D6
		table[067] = [=]() { LEA(m_reg.A6); }; // LEA <ea>,A6

		table[076] = [=]() { CHK((SWORD)m_reg.D7w); }; // CHK.w <ea>,D7
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

		// D6
		table[000] = [=]() { OR(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D0
		table[001] = [=]() { OR(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D0
		table[002] = [=]() { OR(m_reg.D0, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D0
		table[003] = [=]() { DIVUw(m_reg.D0); }; // DIVU.w <ea>, D0
		table[004] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D0b); }; // OR.b D0,<ea>
		table[005] = [=]() { ORToEA(m_reg.D0w); }; // OR.w D0,<ea>
		table[006] = [=]() { ORToEA(m_reg.D0 ); }; // OR.l D0,<ea>
		table[007] = [=]() { DIVSw(m_reg.D0); }; // DIVS.w <ea>, D0

		// D1
		table[010] = [=]() { OR(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D1
		table[011] = [=]() { OR(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D1
		table[012] = [=]() { OR(m_reg.D1, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D1
		table[013] = [=]() { DIVUw(m_reg.D1); }; // DIVU.w <ea>, D1
		table[014] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D1b); }; // OR.b D1,<ea>
		table[015] = [=]() { ORToEA(m_reg.D1w); }; // OR.w D1,<ea>
		table[016] = [=]() { ORToEA(m_reg.D1); }; // OR.l D1,<ea>
		table[017] = [=]() { DIVSw(m_reg.D1); }; // DIVS.w <ea>, D1

		// D2
		table[020] = [=]() { OR(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D2
		table[021] = [=]() { OR(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D2
		table[022] = [=]() { OR(m_reg.D2, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D2
		table[023] = [=]() { DIVUw(m_reg.D2); }; // DIVU.w <ea>, D2
		table[024] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D2b); }; // OR.b D2,<ea>
		table[025] = [=]() { ORToEA(m_reg.D2w); }; // OR.w D2,<ea>
		table[026] = [=]() { ORToEA(m_reg.D2); }; // OR.l D2,<ea>
		table[027] = [=]() { DIVSw(m_reg.D2); }; // DIVS.w <ea>, D2

		// D3
		table[030] = [=]() { OR(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D3
		table[031] = [=]() { OR(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D3
		table[032] = [=]() { OR(m_reg.D3, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D3
		table[033] = [=]() { DIVUw(m_reg.D3); }; // DIVU.w <ea>, D3
		table[034] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D3b); }; // OR.b D3,<ea>
		table[035] = [=]() { ORToEA(m_reg.D3w); }; // OR.w D3,<ea>
		table[036] = [=]() { ORToEA(m_reg.D3); }; // OR.l D3,<ea>
		table[037] = [=]() { DIVSw(m_reg.D3); }; // DIVS.w <ea>, D3

		// D4
		table[040] = [=]() { OR(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D4
		table[041] = [=]() { OR(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D4
		table[042] = [=]() { OR(m_reg.D4, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D4
		table[043] = [=]() { DIVUw(m_reg.D4); }; // DIVU.w <ea>, D4
		table[044] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D4b); }; // OR.b D4,<ea>
		table[045] = [=]() { ORToEA(m_reg.D4w); }; // OR.w D4,<ea>
		table[046] = [=]() { ORToEA(m_reg.D4); }; // OR.l D4,<ea>
		table[047] = [=]() { DIVSw(m_reg.D4); }; // DIVS.w <ea>, D4

		// D5
		table[050] = [=]() { OR(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D5
		table[051] = [=]() { OR(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D5
		table[052] = [=]() { OR(m_reg.D5, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D5
		table[053] = [=]() { DIVUw(m_reg.D5); }; // DIVU.w <ea>, D5
		table[054] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D5b); }; // OR.b D5,<ea>
		table[055] = [=]() { ORToEA(m_reg.D5w); }; // OR.w D5,<ea>
		table[056] = [=]() { ORToEA(m_reg.D5); }; // OR.l D5,<ea>
		table[057] = [=]() { DIVSw(m_reg.D5); }; // DIVS.w <ea>, D5

		// D6
		table[060] = [=]() { OR(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D6
		table[061] = [=]() { OR(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D6
		table[062] = [=]() { OR(m_reg.D6, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D6
		table[063] = [=]() { DIVUw(m_reg.D6); }; // DIVU.w <ea>, D6
		table[064] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D6b); }; // OR.b D6,<ea>
		table[065] = [=]() { ORToEA(m_reg.D6w); }; // OR.w D6,<ea>
		table[066] = [=]() { ORToEA(m_reg.D6); }; // OR.l D6,<ea>
		table[067] = [=]() { DIVSw(m_reg.D6); }; // DIVS.w <ea>, D6

		// D7
		table[070] = [=]() { OR(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // OR.b <ea>,D7
		table[071] = [=]() { OR(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupData)); }; // OR.w <ea>,D7
		table[072] = [=]() { OR(m_reg.D7, GetEAValue<DWORD>(EAMode::GroupData)); }; // OR.l <ea>,D7
		table[073] = [=]() { DIVUw(m_reg.D7); }; // DIVU.w <ea>, D7
		table[074] = [=]() { Mask_BCD.IsMatch(m_opcode) ? SBCDb() : ORToEA(m_reg.D7b); }; // OR.b D7,<ea>
		table[075] = [=]() { ORToEA(m_reg.D7w); }; // OR.w D7,<ea>
		table[076] = [=]() { ORToEA(m_reg.D7); }; // OR.l D7,<ea>
		table[077] = [=]() { DIVSw(m_reg.D7); }; // DIVS.w <ea>, D7
	}

	// b1001: SUB,SUBX,SUBA
	void CPU68000::InitGroupB1001(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0/A0
		table[000] = [=]() { SUB(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D0
		table[001] = [=]() { SUB(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D0
		table[002] = [=]() { SUB(m_reg.D0,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D0
		table[003] = [=]() { SUBA(m_reg.A0, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A0
		table[004] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D0b); }; // SUB.b D0,<ea>
		table[005] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D0w); }; // SUB.w D0,<ea>
		table[006] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D0); };  // SUB.l D0,<ea>
		table[007] = [=]() { SUBA(m_reg.A0, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A0

		// D1/A1
		table[010] = [=]() { SUB(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D1
		table[011] = [=]() { SUB(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D1
		table[012] = [=]() { SUB(m_reg.D1,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D1
		table[013] = [=]() { SUBA(m_reg.A1, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A1
		table[014] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D1b); }; // SUB.b D1,<ea>
		table[015] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D1w); }; // SUB.w D1,<ea>
		table[016] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D1); };  // SUB.l D1,<ea>
		table[017] = [=]() { SUBA(m_reg.A1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A1

		// D2/A2
		table[020] = [=]() { SUB(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D2
		table[021] = [=]() { SUB(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D2
		table[022] = [=]() { SUB(m_reg.D2,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D2
		table[023] = [=]() { SUBA(m_reg.A2, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A2
		table[024] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D2b); }; // SUB.b D2,<ea>
		table[025] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D2w); }; // SUB.w D2,<ea>
		table[026] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D2); };  // SUB.l D2,<ea>
		table[027] = [=]() { SUBA(m_reg.A2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A2

		// D3/A3
		table[030] = [=]() { SUB(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D3
		table[031] = [=]() { SUB(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D3
		table[032] = [=]() { SUB(m_reg.D3,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D3
		table[033] = [=]() { SUBA(m_reg.A3, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A3
		table[034] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D3b); }; // SUB.b D3,<ea>
		table[035] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D3w); }; // SUB.w D3,<ea>
		table[036] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D3); };  // SUB.l D3,<ea>
		table[037] = [=]() { SUBA(m_reg.A3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A3

		// D4/A4
		table[040] = [=]() { SUB(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D4
		table[041] = [=]() { SUB(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D4
		table[042] = [=]() { SUB(m_reg.D4,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D4
		table[043] = [=]() { SUBA(m_reg.A4, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A4
		table[044] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D4b); }; // SUB.b D4,<ea>
		table[045] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D4w); }; // SUB.w D4,<ea>
		table[046] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D4); };  // SUB.l D4,<ea>
		table[047] = [=]() { SUBA(m_reg.A4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A4

		// D5/A5
		table[050] = [=]() { SUB(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D5
		table[051] = [=]() { SUB(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D5
		table[052] = [=]() { SUB(m_reg.D5,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D5
		table[053] = [=]() { SUBA(m_reg.A5, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A5
		table[054] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D5b); }; // SUB.b D5,<ea>
		table[055] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D5w); }; // SUB.w D5,<ea>
		table[056] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D5); };  // SUB.l D5,<ea>
		table[057] = [=]() { SUBA(m_reg.A5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A5

		// D6/A6
		table[060] = [=]() { SUB(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D6
		table[061] = [=]() { SUB(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D6
		table[062] = [=]() { SUB(m_reg.D6,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D6
		table[063] = [=]() { SUBA(m_reg.A6, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A6
		table[064] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D6b); }; // SUB.b D6,<ea>
		table[065] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D6w); }; // SUB.w D6,<ea>
		table[066] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D6); };  // SUB.l D6,<ea>
		table[067] = [=]() { SUBA(m_reg.A6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUBA.l <ea>,A6

		// D7/A7
		table[070] = [=]() { SUB(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // SUB.b <ea>,D7
		table[071] = [=]() { SUB(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUB.w <ea>,D7
		table[072] = [=]() { SUB(m_reg.D7,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // SUB.l <ea>,D7
		table[073] = [=]() { SUBA(m_reg.A7, GetEAValue<WORD>(EAMode::GroupAll)); }; // SUBA.w <ea>,A7
		table[074] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<BYTE>() : SUBToEA(m_reg.D7b); }; // SUB.b D7,<ea>
		table[075] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<WORD>() : SUBToEA(m_reg.D7w); }; // SUB.w D7,<ea>
		table[076] = [=]() { Mask_X.IsMatch(m_opcode) ? SUBX<DWORD>() : SUBToEA(m_reg.D7); };  // SUB.l D7,<ea>
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
		table[003] = [=]() { CMP(m_reg.A0, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A0
		table[004] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D0b); }; // EOR.b D0,{idx}
		table[005] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D0w); }; // EOR.b D0,{idx}
		table[006] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D0); }; // EOR.b D0,{idx}
		table[007] = [=]() { CMP(m_reg.A0, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A0

		// D1
		table[010] = [=]() { CMP(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D1
		table[011] = [=]() { CMP(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D1
		table[012] = [=]() { CMP(m_reg.D1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D1
		table[013] = [=]() { CMP(m_reg.A1, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A1
		table[014] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D1b); }; // EOR.b D1,{idx}
		table[015] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D1w); }; // EOR.b D1,{idx}
		table[016] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D1); }; // EOR.b D1,{idx}
		table[017] = [=]() { CMP(m_reg.A1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A1

		// D2
		table[020] = [=]() { CMP(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D2
		table[021] = [=]() { CMP(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D2
		table[022] = [=]() { CMP(m_reg.D2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D2
		table[023] = [=]() { CMP(m_reg.A2, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A2
		table[024] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D2b); }; // EOR.b D2,{idx}
		table[025] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D2w); }; // EOR.b D2,{idx}
		table[026] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D2); }; // EOR.b D2,{idx}
		table[027] = [=]() { CMP(m_reg.A2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A2

		// D3
		table[030] = [=]() { CMP(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D3
		table[031] = [=]() { CMP(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D3
		table[032] = [=]() { CMP(m_reg.D3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D3
		table[033] = [=]() { CMP(m_reg.A3, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A3
		table[034] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D3b); }; // EOR.b D3,{idx}
		table[035] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D3w); }; // EOR.b D3,{idx}
		table[036] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D3); }; // EOR.b D3,{idx}
		table[037] = [=]() { CMP(m_reg.A3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A3

		// D4
		table[040] = [=]() { CMP(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D4
		table[041] = [=]() { CMP(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D4
		table[042] = [=]() { CMP(m_reg.D4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D4
		table[043] = [=]() { CMP(m_reg.A4, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A4
		table[044] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D4b); }; // EOR.b D4,{idx}
		table[045] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D4w); }; // EOR.b D4,{idx}
		table[046] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D4); }; // EOR.b D4,{idx}
		table[047] = [=]() { CMP(m_reg.A4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A4

		// D5
		table[050] = [=]() { CMP(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D5
		table[051] = [=]() { CMP(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D5
		table[052] = [=]() { CMP(m_reg.D5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D5
		table[053] = [=]() { CMP(m_reg.A5, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A5
		table[054] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D5b); }; // EOR.b D5,{idx}
		table[055] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D5w); }; // EOR.b D5,{idx}
		table[056] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D5); }; // EOR.b D5,{idx}
		table[057] = [=]() { CMP(m_reg.A5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A5

		// D6
		table[060] = [=]() { CMP(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D6
		table[061] = [=]() { CMP(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D6
		table[062] = [=]() { CMP(m_reg.D6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D6
		table[063] = [=]() { CMP(m_reg.A6, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A6
		table[064] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D6b); }; // EOR.b D6,{idx}
		table[065] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D6w); }; // EOR.b D6,{idx}
		table[066] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D6); }; // EOR.b D6,{idx}
		table[067] = [=]() { CMP(m_reg.A6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A6

		// D7
		table[070] = [=]() { CMP(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // CMP.b <ea>,D7
		table[071] = [=]() { CMP(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupAll)); }; // CMP.w <ea>,D7
		table[072] = [=]() { CMP(m_reg.D7, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMP.l <ea>,D7
		table[073] = [=]() { CMP(m_reg.A7, Widen(GetEAValue<WORD>(EAMode::GroupAll))); }; // CMPA.w <ea>,A7
		table[074] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<BYTE>() : EORToEA<BYTE>(m_reg.D7b); }; // EOR.b D7,{idx}
		table[075] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<WORD>() : EORToEA<WORD>(m_reg.D7w); }; // EOR.b D7,{idx}
		table[076] = [=]() { Mask_CMPM.IsMatch(m_opcode) ? CMPM<DWORD>() : EORToEA<DWORD>(m_reg.D7); }; // EOR.b D7,{idx}
		table[077] = [=]() { CMP(m_reg.A7, GetEAValue<DWORD>(EAMode::GroupAll)); }; // CMPA.l <ea>,A7
	}

	// b1100: MULU,MULS,ABCD,EXG,AND
	void CPU68000::InitGroupB1100(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { AND(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D0
		table[001] = [=]() { AND(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D0
		table[002] = [=]() { AND(m_reg.D0,  GetEAValue<DWORD>(EAMode::GroupData)); }; // AND.l <ea>,D0
		table[003] = [=]() { MULUw(m_reg.D0); }; // MULU.w <ea>, D0
		table[004] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D0b); }; // AND.b D0,<ea>
		table[005] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D0w); }; // AND.b D0,<ea>
		table[006] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D0); };  // AND.b D0,<ea>
		table[007] = [=]() { MULSw(m_reg.D0); }; // MULS.w <ea>, D0

		// D1
		table[010] = [=]() { AND(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D1
		table[011] = [=]() { AND(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D1
		table[012] = [=]() { AND(m_reg.D1,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D1
		table[013] = [=]() { MULUw(m_reg.D1); }; // MULU.w <ea>, D1
		table[014] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D1b); }; // AND.b D1,<ea>
		table[015] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D1w); }; // AND.b D1,<ea>
		table[016] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D1); };  // AND.b D1,<ea>
		table[017] = [=]() { MULSw(m_reg.D1); }; // MULS.w <ea>, D1

		// D2
		table[020] = [=]() { AND(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D2
		table[021] = [=]() { AND(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D2
		table[022] = [=]() { AND(m_reg.D2,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D2
		table[023] = [=]() { MULUw(m_reg.D2); }; // MULU.w <ea>, D2
		table[024] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D2b); }; // AND.b D2,<ea>
		table[025] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D2w); }; // AND.b D2,<ea>
		table[026] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D2); };  // AND.b D2,<ea>
		table[027] = [=]() { MULSw(m_reg.D2); }; // MULS.w <ea>, D2

		// D3
		table[030] = [=]() { AND(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D3
		table[031] = [=]() { AND(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D3
		table[032] = [=]() { AND(m_reg.D3,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D3
		table[033] = [=]() { MULUw(m_reg.D3); }; // MULU.w <ea>, D3
		table[034] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D3b); }; // AND.b D3,<ea>
		table[035] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D3w); }; // AND.b D3,<ea>
		table[036] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D3); };  // AND.b D3,<ea>
		table[037] = [=]() { MULSw(m_reg.D3); }; // MULS.w <ea>, D3

		// D4
		table[040] = [=]() { AND(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D4
		table[041] = [=]() { AND(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D4
		table[042] = [=]() { AND(m_reg.D4,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D4
		table[043] = [=]() { MULUw(m_reg.D4); }; // MULU.w <ea>, D4
		table[044] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D4b); }; // AND.b D4,<ea>
		table[045] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D4w); }; // AND.b D4,<ea>
		table[046] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D4); };  // AND.b D4,<ea>
		table[047] = [=]() { MULSw(m_reg.D4); }; // MULS.w <ea>, D4

		// D5
		table[050] = [=]() { AND(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D5
		table[051] = [=]() { AND(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D5
		table[052] = [=]() { AND(m_reg.D5,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D5
		table[053] = [=]() { MULUw(m_reg.D5); }; // MULU.w <ea>, D5
		table[054] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D5b); }; // AND.b D5,<ea>
		table[055] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D5w); }; // AND.b D5,<ea>
		table[056] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D5); };  // AND.b D5,<ea>
		table[057] = [=]() { MULSw(m_reg.D5); }; // MULS.w <ea>, D5

		// D6
		table[060] = [=]() { AND(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D6
		table[061] = [=]() { AND(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D6
		table[062] = [=]() { AND(m_reg.D6,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D6
		table[063] = [=]() { MULUw(m_reg.D6); }; // MULU.w <ea>, D6
		table[064] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D6b); }; // AND.b D6,<ea>
		table[065] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D6w); }; // AND.b D6,<ea>
		table[066] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D6); };  // AND.b D6,<ea>
		table[067] = [=]() { MULSw(m_reg.D6); }; // MULS.w <ea>, D6

		// D7
		table[070] = [=]() { AND(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // AND.b <ea>,D7
		table[071] = [=]() { AND(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupData)); }; // AND.w <ea>,D7
		table[072] = [=]() { AND(m_reg.D7,  GetEAValue<DWORD>(EAMode::GroupData)); };  // AND.l <ea>,D7
		table[073] = [=]() { MULUw(m_reg.D7); }; // MULU.w <ea>, D7
		table[074] = [=]() { Mask_BCD.IsMatch(m_opcode) ? ABCDb() : ANDToEA<BYTE>(m_reg.D7b); }; // AND.b D7,<ea>
		table[075] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<WORD>(m_reg.D7w); }; // AND.b D7,<ea>
		table[076] = [=]() { Mask_EXG.IsMatch(m_opcode)  ? EXGl()  : ANDToEA<DWORD>(m_reg.D7); };  // AND.b D7,<ea>
		table[077] = [=]() { MULSw(m_reg.D7); }; // MULS.w <ea>, D7
	}

	// b1101: ADD,ADDX,ADDA
	void CPU68000::InitGroupB1101(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		// D0
		table[000] = [=]() { ADD(m_reg.D0b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D0
		table[001] = [=]() { ADD(m_reg.D0w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D0
		table[002] = [=]() { ADD(m_reg.D0,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D0
		table[003] = [=]() { ADDA(m_reg.A0, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A0
		table[004] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D0b); }; // ADD.b D0,<ea>
		table[005] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D0w); }; // ADD.w D0,<ea>
		table[006] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D0); };  // ADD.l D0,<ea>
		table[007] = [=]() { ADDA(m_reg.A0, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A0

		// D1
		table[010] = [=]() { ADD(m_reg.D1b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D1
		table[011] = [=]() { ADD(m_reg.D1w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D1
		table[012] = [=]() { ADD(m_reg.D1,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D1
		table[013] = [=]() { ADDA(m_reg.A1, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A1
		table[014] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D1b); }; // ADD.b D1,<ea>
		table[015] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D1w); }; // ADD.w D1,<ea>
		table[016] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D1); };  // ADD.l D1,<ea>
		table[017] = [=]() { ADDA(m_reg.A1, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A1

		// D2
		table[020] = [=]() { ADD(m_reg.D2b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D2
		table[021] = [=]() { ADD(m_reg.D2w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D2
		table[022] = [=]() { ADD(m_reg.D2,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D2
		table[023] = [=]() { ADDA(m_reg.A2, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A2
		table[024] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D2b); }; // ADD.b D2,<ea>
		table[025] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D2w); }; // ADD.w D2,<ea>
		table[026] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D2); };  // ADD.l D2,<ea>
		table[027] = [=]() { ADDA(m_reg.A2, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A2

		// D3
		table[030] = [=]() { ADD(m_reg.D3b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D3
		table[031] = [=]() { ADD(m_reg.D3w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D3
		table[032] = [=]() { ADD(m_reg.D3,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D3
		table[033] = [=]() { ADDA(m_reg.A3, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A3
		table[034] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D3b); }; // ADD.b D3,<ea>
		table[035] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D3w); }; // ADD.w D3,<ea>
		table[036] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D3); };  // ADD.l D3,<ea>
		table[037] = [=]() { ADDA(m_reg.A3, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A3

		// D4
		table[040] = [=]() { ADD(m_reg.D4b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D4
		table[041] = [=]() { ADD(m_reg.D4w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D4
		table[042] = [=]() { ADD(m_reg.D4,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D4
		table[043] = [=]() { ADDA(m_reg.A4, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A4
		table[044] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D4b); }; // ADD.b D4,<ea>
		table[045] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D4w); }; // ADD.w D4,<ea>
		table[046] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D4); };  // ADD.l D4,<ea>
		table[047] = [=]() { ADDA(m_reg.A4, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A4

		// D5
		table[050] = [=]() { ADD(m_reg.D5b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D5
		table[051] = [=]() { ADD(m_reg.D5w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D5
		table[052] = [=]() { ADD(m_reg.D5,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D5
		table[053] = [=]() { ADDA(m_reg.A5, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A5
		table[054] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D5b); }; // ADD.b D5,<ea>
		table[055] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D5w); }; // ADD.w D5,<ea>
		table[056] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D5); };  // ADD.l D5,<ea>
		table[057] = [=]() { ADDA(m_reg.A5, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A5

		// D6
		table[060] = [=]() { ADD(m_reg.D6b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D6
		table[061] = [=]() { ADD(m_reg.D6w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D6
		table[062] = [=]() { ADD(m_reg.D6,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D6
		table[063] = [=]() { ADDA(m_reg.A6, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A6
		table[064] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D6b); }; // ADD.b D6,<ea>
		table[065] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D6w); }; // ADD.w D6,<ea>
		table[066] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D6); };  // ADD.l D6,<ea>
		table[067] = [=]() { ADDA(m_reg.A6, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A6

		// D7
		table[070] = [=]() { ADD(m_reg.D7b, GetEAValue<BYTE>(EAMode::GroupData)); }; // ADD.b <ea>,D7
		table[071] = [=]() { ADD(m_reg.D7w, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADD.w <ea>,D7
		table[072] = [=]() { ADD(m_reg.D7,  GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADD.l <ea>,D7
		table[073] = [=]() { ADDA(m_reg.A7, GetEAValue<WORD>(EAMode::GroupAll)); }; // ADDA.w <ea>,A7
		table[074] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<BYTE>() : ADDToEA(m_reg.D7b); }; // ADD.b D7,<ea>
		table[075] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<WORD>() : ADDToEA(m_reg.D7w); }; // ADD.w D7,<ea>
		table[076] = [=]() { Mask_X.IsMatch(m_opcode) ? ADDX<DWORD>() : ADDToEA(m_reg.D7); };  // ADD.l D7,<ea>
		table[077] = [=]() { ADDA(m_reg.A7, GetEAValue<DWORD>(EAMode::GroupAll)); }; // ADDA.l <ea>,A7
	}

	void CPU68000::InitGroupMisc(OpcodeTable& table, size_t size)
	{
		InitTable(table, size);

		table[020] = [=]() { LINKw(m_reg.A0); }; // LINKw A0, #<disp>
		table[021] = [=]() { LINKw(m_reg.A1); }; // LINKw A1, #<disp>
		table[022] = [=]() { LINKw(m_reg.A2); }; // LINKw A2, #<disp>
		table[023] = [=]() { LINKw(m_reg.A3); }; // LINKw A3, #<disp>
		table[024] = [=]() { LINKw(m_reg.A4); }; // LINKw A4, #<disp>
		table[025] = [=]() { LINKw(m_reg.A5); }; // LINKw A5, #<disp>
		table[026] = [=]() { LINKw(m_reg.A6); }; // LINKw A6, #<disp>
		table[027] = [=]() { LINKw(m_reg.A7); }; // LINKw A7, #<disp>

		table[030] = [=]() { UNLK(m_reg.A0); }; // UNLK A0
		table[031] = [=]() { UNLK(m_reg.A1); }; // UNLK A1
		table[032] = [=]() { UNLK(m_reg.A2); }; // UNLK A2
		table[033] = [=]() { UNLK(m_reg.A3); }; // UNLK A3
		table[034] = [=]() { UNLK(m_reg.A4); }; // UNLK A4
		table[035] = [=]() { UNLK(m_reg.A5); }; // UNLK A5
		table[036] = [=]() { UNLK(m_reg.A6); }; // UNLK A6
		table[037] = [=]() { UNLK(m_reg.A7); }; // UNLK A7

		table[040] = [=]() { MOVEtoUSP(m_reg.A0); }; // MOVE.l A0, USP
		table[041] = [=]() { MOVEtoUSP(m_reg.A1); }; // MOVE.l A1, USP
		table[042] = [=]() { MOVEtoUSP(m_reg.A2); }; // MOVE.l A2, USP
		table[043] = [=]() { MOVEtoUSP(m_reg.A3); }; // MOVE.l A3, USP
		table[044] = [=]() { MOVEtoUSP(m_reg.A4); }; // MOVE.l A4, USP
		table[045] = [=]() { MOVEtoUSP(m_reg.A5); }; // MOVE.l A5, USP
		table[046] = [=]() { MOVEtoUSP(m_reg.A6); }; // MOVE.l A6, USP
		table[047] = [=]() { MOVEtoUSP(m_reg.A7); }; // MOVE.l A7, USP

		table[050] = [=]() { MOVEfromUSP(m_reg.A0); }; // MOVE.l USP, A0
		table[051] = [=]() { MOVEfromUSP(m_reg.A1); }; // MOVE.l USP, A1
		table[052] = [=]() { MOVEfromUSP(m_reg.A2); }; // MOVE.l USP, A2
		table[053] = [=]() { MOVEfromUSP(m_reg.A3); }; // MOVE.l USP, A3
		table[054] = [=]() { MOVEfromUSP(m_reg.A4); }; // MOVE.l USP, A4
		table[055] = [=]() { MOVEfromUSP(m_reg.A5); }; // MOVE.l USP, A5
		table[056] = [=]() { MOVEfromUSP(m_reg.A6); }; // MOVE.l USP, A6
		table[057] = [=]() { MOVEfromUSP(m_reg.A7); }; // MOVE.l USP, A7

		table[061] = [=]() { /* nothing */ }; // NOP
		table[063] = [=]() { RTE(); }; // RTE
		table[065] = [=]() { RTS(); }; // RTS
		table[066] = [=]() { if (GetFlag(FLAG_V)) Exception(VECTOR::TRAPV_Instruction); }; // TRAPV
		table[067] = [=]() { RTR(); }; // RTR
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
		SetInterruptLevel(0);
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

	void CPU68000::SetCC(BYTE f)
	{
		// Don't touch upper byte
		WORD newFlags = (m_reg.flags & 0xFF00);
		SetFlags(newFlags | f);
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
			m_lastAddress = GetCurrentAddress();

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

	void CPU68000::Exception(VECTOR v)
	{
		// TODO: Temp
		bool fatal = false;

		switch (v)
		{
		case VECTOR::Line1010Emulator:
		case VECTOR::Line1111Emulator:
		{
			const auto trapItem = m_trapList.find(m_opcode);
			std::string trapName = "";
			if (trapItem != m_trapList.end())
				trapName = trapItem->second;
			m_programCounter -= 2; // Point to beginning of instruction
			LogPrintf(LOG_INFO, "CPU: Exception (%d)[%04X][%s] at address 0x%08X", v, m_opcode, trapName.c_str(), m_programCounter);
			break;
		}
		case VECTOR::CHK_Instruction:
			LogPrintf(LOG_INFO, "CPU: Exception (%d)[CHECK] at address 0x%08X", v, m_programCounter);
			break;
		case VECTOR::TRAPV_Instruction:
			LogPrintf(LOG_INFO, "CPU: Exception (%d)[TRAPV] at address 0x%08X", v, m_programCounter);
			break;
		default:
			LogPrintf(LOG_WARNING, "CPU: Exception (%d) at address 0x%08X", v, m_programCounter);
			throw std::exception("Unimplemented Exception");
			break;
		}

		WORD flags = m_reg.flags;

		SetFlag(FLAG_S, true); // Set supervisor mode
		SetFlag(FLAG_T, false); // Clear trace flag

		ADDRESS handler = Read<DWORD>(GetVectorAddress(v));
		PUSHl(m_programCounter);
		PUSHw(flags);

		m_programCounter = handler;
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
		if (m_interruptLevel <= GetInterruptMask())
		{
			return;
		}

		WORD flags = m_reg.flags;

		SetFlag(FLAG_S, true); // Set supervisor mode
		SetFlag(FLAG_T, false); // Clear trace flag
		SetInterruptMask(m_interruptLevel);

		ADDRESS handler = Read<DWORD>(GetIntVectorAddress(m_interruptLevel));
		PUSHl(m_programCounter);
		PUSHw(flags);

		m_programCounter = handler;
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

		// Byte operations on stack pointer increment by word in -(An) or (An)+ modes
		int incrModeSize = ((size == 1) && (&An == &m_reg.A7)) ? 2 : size;

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
			An += incrModeSize;
			timePenalty = 4;
			break;

			// Address Register Indirect with Predecrement, AN -= N, EA=(An)
		case EAMode::ARegIndirectPredec:
			An -= incrModeSize;
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
		bool wordLong = GetBit(extWord, 11);
		if (wordLong)
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
		dest = GetEA<DWORD>(EAMode::GroupControl);
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

	template<typename SIZE>
	void CPU68000::MOVE()
	{
		constexpr EAMode mode = (sizeof(SIZE) == 1) ? EAMode::GroupData : EAMode::GroupAll;
		SIZE source = GetEAValue<SIZE>(mode);

		AdjustNZ(source);
		SetFlag(FLAG_VC, false);

		// For destination, need to shuffle the bits
		// (opcode order is: 0|0| size | DestReg|DestMode | SrcMode|SrcReg)
		const WORD destMode = (m_opcode >> 3) & 0b111000;
		const WORD destReg = (m_opcode >> 9) & 0b000111;

		// dest == Data reg
		if (destMode == 0)
		{
			m_reg.GetDATA<SIZE>(destReg) = source;
		}
		else // Dest is an address
		{
			m_opcode = destMode | destReg;
			ADDRESS dest = GetEA<SIZE>(EAMode::GroupDataAlt);
			Write<SIZE>(dest, source);
		}
	}

	template <typename SIZE>
	void CPU68000::MOVEA()
	{
		SIZE source = GetEAValue<SIZE>(EAMode::GroupAll);

		const WORD destReg = (m_opcode >> 9) & 7;

		m_reg.ADDR[destReg] = Widen(source);
	}

	void CPU68000::MOVEwToCCR(WORD src)
	{
		WORD newFlags = m_reg.flags;
		SetLByte(newFlags, (BYTE)src); // Upper byte of src is ignored

		SetFlags(newFlags);
	}

	void CPU68000::MOVEwToSR(WORD src)
	{
		Privileged();

		SetFlags(src);
	}

	void CPU68000::MOVEwFromSR()
	{
		Privileged();

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			m_reg.DATA[GetOpcodeRegisterIndex()] = m_reg.flags;
		}
		else
		{
			ADDRESS dest = GetEA<WORD>(EAMode::GroupDataAlt);
			Write<WORD>(dest, m_reg.flags);
		}
	}
	void CPU68000::MOVEfromUSP(DWORD& dest)
	{
		Privileged();

		dest = GetUSP();
	}
	void CPU68000::MOVEtoUSP(DWORD src)
	{
		Privileged();

		GetUSP() = src;
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
		while(regs)
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
			++dest;
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
		while(regs)
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
			src += direction;
		}
	}

	// MOVEP.w d(Ay), Dx
	void CPU68000::MOVEPwToReg(WORD& dest)
	{
		const int addrIndex = m_opcode & 7;
		const WORD disp = FetchWord();
		const ADDRESS src = m_reg.ADDR[addrIndex] + Widen(disp);

		const BYTE hByte = Read<BYTE>(src);
		const BYTE lByte = Read<BYTE>(src + 2);

		dest = MakeWord(hByte, lByte);
	}

	// MOVEP.l d(Ay), Dx
	void CPU68000::MOVEPlToReg(DWORD& dest)
	{
		const int addrIndex = m_opcode & 7;
		const WORD disp = FetchWord();
		const ADDRESS src = m_reg.ADDR[addrIndex] + Widen(disp);

		WORD hWord, lWord;

		// High word
		{
			const BYTE hByte = Read<BYTE>(src + 0);
			const BYTE lByte = Read<BYTE>(src + 2);
			hWord = MakeWord(hByte, lByte);
		}

		// Low word
		{
			const BYTE hByte = Read<BYTE>(src + 4);
			const BYTE lByte = Read<BYTE>(src + 6);
			lWord = MakeWord(hByte, lByte);
		}

		dest = MakeDword(hWord, lWord);
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

	// MOVEP.l Dx, d(Ay)
	void CPU68000::MOVEPlFromReg(DWORD src)
	{
		const int addrIndex = m_opcode & 7;
		const WORD disp = FetchWord();
		const ADDRESS dest = m_reg.ADDR[addrIndex] + Widen(disp);

		// High word
		{
			const WORD hWord = GetHWord(src);
			Write<BYTE>(dest + 0, GetHByte(hWord));
			Write<BYTE>(dest + 2, GetLByte(hWord));
		}

		// Low word
		{
			const WORD lWord = GetLWord(src);
			Write<BYTE>(dest + 4, GetHByte(lWord));
			Write<BYTE>(dest + 6, GetLByte(lWord));
		}
	}

	void CPU68000::EXGl()
	{
		int yIndex = GetOpcodeRegisterIndex();
		int xIndex = (m_opcode >> 9) & 7;

		int opmode = (m_opcode >> 3) & 0b11111;

		DWORD* yReg;
		DWORD* xReg;

		switch (opmode)
		{
		case 0b01000: // Dx, Dy
			yReg = m_reg.DATA + yIndex;
			xReg = m_reg.DATA + xIndex;
			break;
		case 0b01001: // Ax, Ay
			yReg = m_reg.ADDR + yIndex;
			xReg = m_reg.ADDR + xIndex;
			break;
		case 0b10001: // Dx, Ay
			yReg = m_reg.ADDR + yIndex;
			xReg = m_reg.DATA + xIndex;
			break;
		default:
			NODEFAULT;
		}

		DWORD temp = *yReg;
		*yReg = *xReg;
		*xReg = temp;
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
		// Save PC here because it can be incremented by FetchWord()
		ADDRESS pc = m_programCounter;

		ADDRESS disp = DoubleWiden(GetLByte(m_opcode));
		if (disp == 0)
		{
			disp = Widen(FetchWord());
		}

		PUSHl(m_programCounter);

		m_programCounter = pc + disp;
		m_programCounter &= ADDRESS_MASK;
	}

	void CPU68000::JSR()
	{
		DWORD addr = GetEA<DWORD>(EAMode::GroupControl);

		PUSHl(m_programCounter);

		m_programCounter = addr;
	}

	void CPU68000::RTS()
	{
		m_programCounter = POPl();
	}

	void CPU68000::RTE()
	{
		Privileged();
		SetFlags(POPw());
		m_programCounter = POPl();
	}

	void CPU68000::RTR()
	{
		Privileged();
		SetCC(GetLByte(POPw()));
		m_programCounter = POPl();
	}

	WORD CPU68000::POPw()
	{
		WORD val = Read<WORD>(m_reg.SP);
		m_reg.SP += 2;
		return val;
	}
	DWORD CPU68000::POPl()
	{
		DWORD val = Read<DWORD>(m_reg.SP);
		m_reg.SP += 4;
		return val;
	}

	void CPU68000::PUSHw(WORD src)
	{
		m_reg.SP -= 2;
		Write(m_reg.SP, src);
	}
	void CPU68000::PUSHl(DWORD src)
	{
		m_reg.SP -= 4;
		Write(m_reg.SP, src);
	}

	void CPU68000::PEAl()
	{
		PUSHl(GetEA<DWORD>(EAMode::GroupControl));
	}

	void CPU68000::LINKw(DWORD& dest)
	{
		DWORD displacement = Widen(FetchWord());

		PUSHl(dest);
		dest = m_reg.SP;
		m_reg.SP += displacement; // TODO: odd number?
	}

	void CPU68000::UNLK(DWORD& dest)
	{
		m_reg.SP = dest; // TODO: odd number?
		dest = POPl();
	}

	// Scc.b <ea>
	void CPU68000::Sccb(bool cond)
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SetLByte(m_reg.DATA[GetOpcodeRegisterIndex()], cond ? 0xFF : 0);
		}
		else
		{
			ADDRESS addr = GetEA<BYTE>(EAMode::GroupDataAlt);
			BYTE dest = Read<BYTE>(addr);

			dest = cond ? 0xFF : 0;

			Write<BYTE>(addr, dest);
		}
	}

	// TAS.b <ea>
	void CPU68000::TASb()
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			BYTE& reg = m_reg.GetDATAb(GetOpcodeRegisterIndex());
			AdjustNZ(reg);
			SetMSB(reg, true);
		}
		else
		{
			ADDRESS addr = GetEA<BYTE>(EAMode::GroupDataAlt);
			BYTE dest = Read<BYTE>(addr);

			AdjustNZ(dest);
			SetMSB(dest, true);

			Write<BYTE>(addr, dest);
		}
		SetFlag(FLAG_VC, false);
	}

	// BTST (bitNumber: imm/Dn), <ea>
	void CPU68000::BitTst(BYTE bitNumber)
	{
		m_eaMode = GetEAMode(m_opcode);

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
			BYTE src = GetEAValue<BYTE>(EAMode::GroupData);
			testBit = GetBit(src, bitNumber % 8);
		}
		SetFlag(FLAG_Z, !testBit);
	}

	// B[CHG|CLR|SET] (bitcount: imm/Dn), <ea>
	void CPU68000::BitOps(BYTE bitNumber, BitOp bitOp)
	{
		m_eaMode = GetEAMode(m_opcode);

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
			ADDRESS addr = GetEA<BYTE>(EAMode::GroupDataAlt);
			BYTE dest = Read<BYTE>(addr);

			testBit = GetBit(dest, bitNumber);
			bool newBit = (bitOp == BitOp::SET) ? 1 : ((bitOp == BitOp::CLEAR) ? 0 : !testBit);
			SetBit(dest, bitNumber, newBit);

			Write<BYTE>(addr, dest);
		}
		SetFlag(FLAG_Z, !testBit);
	}

	// NOT <ea>
	template<typename SIZE>
	void CPU68000::NOT()
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());
			dest = ~dest;
			AdjustNZ(dest);
		}
		else
		{
			ADDRESS addr = GetEA<SIZE>(EAMode::GroupDataAlt);
			SIZE dest = Read<SIZE>(addr);
			dest = ~dest;
			Write<SIZE>(addr, dest);
			AdjustNZ(dest);
		}
		SetFlag(FLAG_VC, false);
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
	template<typename SIZE>
	void CPU68000::ANDToEA(SIZE src)
	{
		ADDRESS ea = GetEA<SIZE>(EAMode::GroupMemAlt);

		SIZE dest = Read<SIZE>(ea);
		AND<SIZE>(dest, src);
		Write<SIZE>(ea, dest);
	}

	void CPU68000::ANDIbToCCR()
	{
		WORD imm = 0xFF00 | FetchByte(); // No change to upper byte
		WORD newFlags = m_reg.flags & imm;
		SetFlags(newFlags);
	}

	void CPU68000::ANDIwToSR()
	{
		Privileged();

		WORD imm = FetchWord();
		WORD newFlags = m_reg.flags & imm;

		SetFlags(newFlags);
	}

	template<typename SIZE>
	void CPU68000::OR(SIZE& dest, SIZE src)
	{
		dest |= src;

		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	// ORI #imm, <ea>
	template<typename SIZE>
	void CPU68000::ORI()
	{
		SIZE imm = Fetch<SIZE>();

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());
			OR<SIZE>(dest, imm);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			SIZE dest = Read<SIZE>(ea);
			OR<SIZE>(dest, imm);
			Write<SIZE>(ea, dest);
		}
	}

	// OR Dn, <ea>
	template<typename SIZE>
	void CPU68000::ORToEA(SIZE src)
	{
		ADDRESS ea = GetEA<SIZE>(EAMode::GroupMemAlt);

		SIZE dest = Read<SIZE>(ea);
		OR<SIZE>(dest, src);
		Write<SIZE>(ea, dest);
	}

	void CPU68000::ORIbToCCR()
	{
		WORD imm = FetchByte(); // No change to upper byte
		WORD newFlags = m_reg.flags | imm;
		SetFlags(newFlags);
	}

	void CPU68000::ORIwToSR()
	{
		Privileged();

		WORD imm = FetchWord();
		WORD newFlags = m_reg.flags | imm;
		SetFlags(newFlags);
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

	// EOR Dn,<ea>
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

	void CPU68000::EORIbToCCR()
	{
		WORD imm = FetchByte(); // No change to upper byte
		WORD newFlags = m_reg.flags ^ imm;
		SetFlags(newFlags);
	}

	void CPU68000::EORIwToSR()
	{
		Privileged();

		WORD imm = FetchWord();
		WORD newFlags = m_reg.flags ^ imm;
		SetFlags(newFlags);
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

	void CPU68000::EXTw()
	{
		DWORD& reg = m_reg.DATA[GetOpcodeRegisterIndex()];
		reg = DoubleWiden(GetLByte(reg));
		AdjustNZ(reg);
		SetFlag(FLAG_VC, false);
	}
	void CPU68000::EXTl()
	{
		DWORD& reg = m_reg.DATA[GetOpcodeRegisterIndex()];
		reg = Widen(GetLWord(reg));
		AdjustNZ(reg);
		SetFlag(FLAG_VC, false);
	}

	// CHK <ea>, Dn
	void CPU68000::CHK(SWORD src)
	{
		SWORD upperBound = (SWORD)GetEAValue<WORD>(EAMode::GroupData);

		if (src < 0)
		{
			SetFlag(FLAG_N, true);
			Exception(VECTOR::CHK_Instruction);
		}
		else if (src > upperBound)
		{
			SetFlag(FLAG_N, false);
			Exception(VECTOR::CHK_Instruction);
		}
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
		bool carry = false;

		for (int i = 0; i < count; ++i)
		{
			carry = GetMSB(dest);
			dest <<= 1;
			SetLSB(dest, carry);
		}

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, false); // Always cleared for ROL

		AdjustNZ(dest);
	}
	template<typename SIZE>
	void CPU68000::ROR(SIZE& dest, int count)
	{
		bool carry = false;

		for (int i = 0; i < count; ++i)
		{
			carry = GetLSB(dest);
			dest >>= 1;
			SetMSB(dest, carry);
		}

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, false); // Always cleared for ROR

		AdjustNZ(dest);
	}

	template<typename SIZE>
	void CPU68000::ROXL(SIZE& dest, int count)
	{
		bool carry = false;
		bool extend = GetFlag(FLAG_X);

		for (int i = 0; i < count; ++i)
		{
			carry = GetMSB(dest);
			dest <<= 1;
			SetLSB(dest, extend);
			extend = carry;
		}

		SetFlag(FLAG_C, count ? carry : extend);
		SetFlag(FLAG_V, false); // Always cleared for ROXL
		if (count)
		{
			SetFlag(FLAG_X, extend);
		}

		AdjustNZ(dest);
	}
	template<typename SIZE>
	void CPU68000::ROXR(SIZE& dest, int count)
	{
		bool carry = false;
		bool extend = GetFlag(FLAG_X);

		for (int i = 0; i < count; ++i)
		{
			carry = GetLSB(dest);
			dest >>= 1;
			SetMSB(dest, extend);
			extend = carry;
		}

		SetFlag(FLAG_C, count ? carry : extend);
		SetFlag(FLAG_V, false); // Always cleared for ROXR
		if (count)
		{
			SetFlag(FLAG_X, extend);
		}

		AdjustNZ(dest);
	}

	void CPU68000::ASLw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		ASL(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::ASRw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		ASR(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::LSLw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		LSL(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::LSRw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		LSR(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::ROXLw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		ROXL(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::ROXRw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		ROXR(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::ROLw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		ROL(dest, 1);
		Write<WORD>(ea, dest);
	}
	void CPU68000::RORw()
	{
		ADDRESS ea = GetEA<WORD>(EAMode::GroupMemAlt);
		WORD dest = Read<WORD>(ea);
		ROR(dest, 1);
		Write<WORD>(ea, dest);
	}

	template<typename SIZE>
	void CPU68000::ADD(SIZE& dest, SIZE src, bool carry)
	{
		constexpr size_t size_max = std::numeric_limits<SIZE>::max();

		SIZE oldDest = dest;

		QWORD temp = (QWORD)dest + src + carry;

		dest = (SIZE)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_CX, (temp > size_max));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	// ADD Dn, <ea>
	template<typename SIZE>
	void CPU68000::ADDToEA(SIZE src)
	{
		ADDRESS ea = GetEA<SIZE>(EAMode::GroupMemAlt);
		SIZE dest = Read<SIZE>(ea);
		ADD<SIZE>(dest, src);
		Write<SIZE>(ea, dest);
	}

	// ADDQ #<data>, <ea>
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
			// Behaves like ADDA
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

	// ADDI #<data>, <ea>
	template<typename SIZE>
	void CPU68000::ADDI()
	{
		SIZE src = Fetch<SIZE>();

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			ADD<SIZE>(m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex()), src);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);
			SIZE dest = Read<SIZE>(ea);
			ADD<SIZE>(dest, src);
			Write<SIZE>(ea, dest);
		}
	}

	template<typename SIZE>
	void CPU68000::ADDX()
	{
		// ADDX/SUBX/NEGX leave the Z flag unchanged
		// if the result is zero, so we may need to
		// restore it at the end.
		bool zFlag = GetFlag(FLAG_Z);
		bool restoreZFlag = false;

		bool regMem = GetBit(m_opcode, 3);

		int srcIndex = GetOpcodeRegisterIndex();
		int destIndex = (m_opcode >> 9) & 7;

		if (!regMem) // ADDX Dy, Dx
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(destIndex);
			SIZE src = m_reg.GetDATA<SIZE>(srcIndex);

			ADD<SIZE>(dest, src, GetFlag(FLAG_X));
			restoreZFlag = (dest == 0);
		}
		else // ADDX -(Ay), -(Ax)
		{
			constexpr int size = sizeof(SIZE);

			DWORD& destAddr = m_reg.ADDR[destIndex];
			DWORD& srcAddr = m_reg.ADDR[srcIndex];

			// TODO: Order? Important if same register (for sub)
			destAddr -= size;
			SIZE dest = Read<SIZE>(destAddr);
			srcAddr -= size;
			SIZE src = Read<SIZE>(srcAddr);

			ADD<SIZE>(dest, src, GetFlag(FLAG_X));
			Write<SIZE>(destAddr, dest);
			restoreZFlag = (dest == 0);
		}

		if (restoreZFlag)
		{
			SetFlag(FLAG_Z, zFlag);
		}
	}

	template<typename SIZE>
	void CPU68000::SUB(SIZE& dest, SIZE src, FLAG carryFlag, bool borrow)
	{
		constexpr size_t size_max = std::numeric_limits<SIZE>::max();

		SIZE oldDest = dest;

		QWORD temp = (QWORD)dest - src - borrow;

		dest = (SIZE)temp;

		AdjustNZ(dest);
		SetFlag(carryFlag, (temp > size_max));
		SetFlag(FLAG_V, (GetMSB(oldDest) != GetMSB(src)) && (GetMSB(dest) == GetMSB(src)));
	}

	// SUB Dn, <ea>
	template<typename SIZE>
	void CPU68000::SUBToEA(SIZE src)
	{
		ADDRESS ea = GetEA<SIZE>(EAMode::GroupMemAlt);
		SIZE dest = Read<SIZE>(ea);
		SUB<SIZE>(dest, src);
		Write<SIZE>(ea, dest);
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

	// SUBI #<data>, <ea>
	template<typename SIZE>
	void CPU68000::SUBI()
	{
		SIZE src = Fetch<SIZE>();

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SUB<SIZE>(m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex()), src);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);
			SIZE dest = Read<SIZE>(ea);
			SUB<SIZE>(dest, src);
			Write<SIZE>(ea, dest);
		}
	}

	template<typename SIZE>
	void CPU68000::SUBX()
	{
		// ADDX/SUBX/NEGX leave the Z flag unchanged
		// if the result is zero, so we may need to
		// restore it at the end.
		bool zFlag = GetFlag(FLAG_Z);
		bool restoreZFlag = false;

		bool regMem = GetBit(m_opcode, 3);

		int srcIndex = GetOpcodeRegisterIndex();
		int destIndex = (m_opcode >> 9) & 7;

		if (!regMem) // SUB Dy, Dx
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(destIndex);
			SIZE src = m_reg.GetDATA<SIZE>(srcIndex);

			SUB<SIZE>(dest, src, FLAG_CX, GetFlag(FLAG_X));
			restoreZFlag = (dest == 0);
		}
		else // SUB -(Ay), -(Ax)
		{
			constexpr int size = sizeof(SIZE);

			DWORD& destAddr = m_reg.ADDR[destIndex];
			DWORD& srcAddr = m_reg.ADDR[srcIndex];

			// TODO: Order? Important if same register (for sub)
			destAddr -= size;
			SIZE dest = Read<SIZE>(destAddr);
			srcAddr -= size;
			SIZE src = Read<SIZE>(srcAddr);

			SUB<SIZE>(dest, src, FLAG_CX, GetFlag(FLAG_X));
			Write<SIZE>(destAddr, dest);
			restoreZFlag = (dest == 0);
		}

		if (restoreZFlag)
		{
			SetFlag(FLAG_Z, zFlag);
		}
	}

	template<typename SIZE>
	void CPU68000::CMPM()
	{
		constexpr int size = sizeof(SIZE);

		DWORD& sourceReg = m_reg.ADDR[GetOpcodeRegisterIndex()];
		DWORD& destReg = m_reg.ADDR[(m_opcode >> 9) & 7];

		// TODO: Order of read/increment is important if source and
		// dest are same register. Can't find doc about this.
		SIZE src = Read<SIZE>(sourceReg);
		sourceReg += size;

		SIZE dest = Read<SIZE>(destReg);
		destReg += size;

		CMP<SIZE>(dest, src);
	}

	// NEG <ea> ( same as 0 - dest )
	template<typename SIZE>
	void CPU68000::NEG()
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());

			SIZE negated = 0;
			SUB<SIZE>(negated, dest);
			dest = negated;
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			SIZE dest = Read<SIZE>(ea);

			SIZE negated = 0;
			SUB<SIZE>(negated, dest);

			Write<SIZE>(ea, negated);
		}
	}

	// NEGX <ea> ( same as 0 - dest - X )
	template<typename SIZE>
	void CPU68000::NEGX()
	{
		// ADDX/SUBX/NEGX leave the Z flag unchanged
		// if the result is zero, so we may need to
		// restore it at the end.
		bool zFlag = GetFlag(FLAG_Z);
		bool restoreZFlag = false;

		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			SIZE& dest = m_reg.GetDATA<SIZE>(GetOpcodeRegisterIndex());

			SIZE negated = 0;
			SUB<SIZE>(negated, dest, FLAG_CX, GetFlag(FLAG_X));
			dest = negated;
			restoreZFlag = (dest == 0);
		}
		else
		{
			ADDRESS ea = GetEA<SIZE>(EAMode::GroupDataAlt);

			SIZE dest = Read<SIZE>(ea);

			SIZE negated = 0;
			SUB<SIZE>(negated, dest, FLAG_CX, GetFlag(FLAG_X));

			Write<SIZE>(ea, negated);
			restoreZFlag = (dest == 0);
		}

		if (restoreZFlag)
		{
			SetFlag(FLAG_Z, zFlag);
		}
	}

	void CPU68000::ABCD(BYTE& dest, BYTE src, bool carry)
	{
		BYTE lowNibble = (dest & 0x0F) + (src & 0x0F) + carry;
		bool halfCarry = (lowNibble > 0x0F);
		lowNibble &= 0x0F;

		int adjust = 0;
		// Low nibble BCD adjust
		if (lowNibble > 9 || halfCarry)
		{
			adjust = 6;
		}

		// Uncorrected add
		WORD uncorrected = dest + src + carry;

		if (uncorrected > 0x99)
		{
			adjust |= 0x60;
		}

		bool oldMSB = GetBit(uncorrected, 7);
		uncorrected += adjust;
		bool newMSB = GetBit(uncorrected, 7);
		SetFlag(FLAG_V, !oldMSB && newMSB);
		SetFlag(FLAG_CX, (uncorrected > 0x9F));
		SetFlag(FLAG_N, newMSB);

		dest = (BYTE)uncorrected;


		// Don't change Z flag if zero (like the ADDX/etc functions)
		if (dest)
		{
			SetFlag(FLAG_Z, false);
		}
	}

	void CPU68000::ABCDb()
	{
		bool regMem = GetBit(m_opcode, 3);

		int srcIndex = GetOpcodeRegisterIndex();
		int destIndex = (m_opcode >> 9) & 7;

		if (!regMem) // ABCD Dy, Dx
		{
			BYTE& dest = m_reg.GetDATA<BYTE>(destIndex);
			BYTE src = m_reg.GetDATA<BYTE>(srcIndex);

			ABCD(dest, src, GetFlag(FLAG_X));
		}
		else // ABCD -(Ay), -(Ax)
		{
			DWORD& destAddr = m_reg.ADDR[destIndex];
			DWORD& srcAddr = m_reg.ADDR[srcIndex];

			// TODO: Order? Important if same register (for sub)
			BYTE dest = Read<BYTE>(--destAddr);
			BYTE src = Read<BYTE>(--srcAddr);

			ABCD(dest, src, GetFlag(FLAG_X));
			Write<BYTE>(destAddr, dest);
		}
	}

	void CPU68000::SBCD(BYTE& dest, BYTE src, bool borrow)
	{
		BYTE lowNibble = (dest & 0x0F) - (src & 0x0F) - borrow;
		bool halfCarry = (lowNibble > 0x0F);
		lowNibble &= 0x0F;

		int adjust = 0;
		// Low nibble BCD adjust
		if (lowNibble > 15 || halfCarry)
		{
			adjust = 6;
		}

		// Uncorrected substract
		WORD uncorrected = dest - src - borrow;

		if (uncorrected > 0xFF)
		{
			adjust |= 0x60;
		}

		bool oldMSB = GetBit(uncorrected, 7);
		uncorrected -= adjust;
		bool newMSB = GetBit(uncorrected, 7);
		SetFlag(FLAG_V, oldMSB && !newMSB);
		SetFlag(FLAG_CX, (uncorrected > 0xFF));
		SetFlag(FLAG_N, newMSB);

		dest = (BYTE)uncorrected;

		// Don't change Z flag if zero (like the ADDX/etc functions)
		if (dest)
		{
			SetFlag(FLAG_Z, false);
		}
	}

	void CPU68000::SBCDb()
	{
		bool regMem = GetBit(m_opcode, 3);

		int srcIndex = GetOpcodeRegisterIndex();
		int destIndex = (m_opcode >> 9) & 7;

		if (!regMem) // SBCD Dy, Dx
		{
			BYTE& dest = m_reg.GetDATA<BYTE>(destIndex);
			BYTE src = m_reg.GetDATA<BYTE>(srcIndex);

			SBCD(dest, src, GetFlag(FLAG_X));
		}
		else // SBCD -(Ay), -(Ax)
		{
			DWORD& destAddr = m_reg.ADDR[destIndex];
			DWORD& srcAddr = m_reg.ADDR[srcIndex];

			// TODO: Order? Important if same register (for sub)
			BYTE dest = Read<BYTE>(--destAddr);
			BYTE src = Read<BYTE>(--srcAddr);

			SBCD(dest, src, GetFlag(FLAG_X));
			Write<BYTE>(destAddr, dest);
		}
	}

	void CPU68000::NBCD(BYTE& dest, bool borrow)
	{
		BYTE negated = 0;
		SBCD(negated, dest, borrow);
		dest = negated;
	}

	void CPU68000::NBCDb()
	{
		m_eaMode = GetEAMode(m_opcode);

		if (m_eaMode == EAMode::DRegDirect)
		{
			BYTE& dest = m_reg.GetDATA<BYTE>(GetOpcodeRegisterIndex());

			NBCD(dest, GetFlag(FLAG_X));
		}
		else // NBCD <ea>
		{
			ADDRESS ea = GetEA<BYTE>(EAMode::GroupDataAlt);

			BYTE dest = Read<BYTE>(ea);

			NBCD(dest, GetFlag(FLAG_X));

			Write<BYTE>(ea, dest);
		}
	}

	void CPU68000::MULUw(DWORD& dest)
	{
		const WORD op1 = GetEAValue<WORD>(EAMode::GroupData);
		const WORD op2 = GetLWord(dest);

		dest = op1 * op2;
		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	void CPU68000::MULSw(DWORD& dest)
	{
		const SWORD op1 = (SWORD)GetEAValue<WORD>(EAMode::GroupData);
		const SWORD op2 = (SWORD)GetLWord(dest);

		const int32_t res = op1 * op2;
		dest = (DWORD)res;
		AdjustNZ(dest);
		SetFlag(FLAG_VC, false);
	}

	void CPU68000::DIVUw(DWORD& dest)
	{
		WORD src = GetEAValue<WORD>(EAMode::GroupData);

		if (src == 0)
		{
			Exception(VECTOR::ZeroDivide);
		}

		DWORD quotient = dest / src;
		WORD remainder = dest % src;

		// overflow
		if (quotient > 0xFFFF)
		{
			SetFlag(FLAG_V, true);
		}
		else
		{
			SetLWord(dest, (WORD)quotient);
			SetHWord(dest, remainder);

			AdjustNZ((WORD)quotient);
			SetFlag(FLAG_VC, false);
		}
	}

	void CPU68000::DIVSw(DWORD& dest)
	{
		int16_t src = (int16_t)GetEAValue<WORD>(EAMode::GroupData);

		if (src == 0)
		{
			Exception(VECTOR::ZeroDivide);
		}

		// TODO: Triple check this
		int32_t quotient = dest / src;
		int32_t remainder = abs((int32_t)(dest % src)); // Modulo sign is not defined
		if (quotient < 0)
		{
			remainder = -remainder; // Sign of the remainder is same as the dividend
		}

		// overflow
		if ((quotient > 32767) || (quotient < -32768))
		{
			SetFlag(FLAG_V, true);
		}
		else
		{
			SetLWord(dest, (WORD)quotient);
			SetHWord(dest, (WORD)remainder);

			AdjustNZ((WORD)quotient);
			SetFlag(FLAG_VC, false);
		}
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
