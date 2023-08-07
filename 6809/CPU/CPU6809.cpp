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

		m_opcodes[0x03] = [=]() { COMm(GetDirect(FetchByte())); }; // COM direct
		m_opcodes[0x04] = [=]() { LSRm(GetDirect(FetchByte())); }; // LSR direct
		m_opcodes[0x08] = [=]() { ASLm(GetDirect(FetchByte())); }; // ASL direct
		m_opcodes[0x09] = [=]() { ROLm(GetDirect(FetchByte())); }; // ROL direct
		m_opcodes[0x0A] = [=]() { DECm(GetDirect(FetchByte())); }; // DEC direct
		m_opcodes[0x0C] = [=]() { INCm(GetDirect(FetchByte())); }; // INC direct
		m_opcodes[0x0D] = [=]() { TST(GetMemDirectByte()); }; // TST direct
		m_opcodes[0x0E] = [=]() { JMP(GetDirect(FetchByte())); }; // JMP direct
		m_opcodes[0x0F] = [=]() { CLRm(GetDirect(FetchByte())); }; // CLR direct

		m_opcodes[0x10] = [=]() { ExecPage2(FetchByte()); }; // Page 2 sub intructions
		m_opcodes[0x11] = [=]() { ExecPage3(FetchByte()); }; // Page 3 sub intructions
		m_opcodes[0x1A] = [=]() { m_reg.flags |= FetchByte(); }; // ORCC imm
		m_opcodes[0x1C] = [=]() { m_reg.flags &= FetchByte(); }; // ANDCC imm
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

		m_opcodes[0x30] = [=]() { LEA(m_reg.X, true); };
		m_opcodes[0x31] = [=]() { LEA(m_reg.Y, true); };
		m_opcodes[0x32] = [=]() { LEA(m_reg.S, false); };
		m_opcodes[0x33] = [=]() { LEA(m_reg.U, false); };
		m_opcodes[0x34] = [=]() { PSH(STACK::S, FetchByte()); };
		m_opcodes[0x35] = [=]() { PUL(STACK::S, FetchByte()); };
		m_opcodes[0x36] = [=]() { PSH(STACK::U, FetchByte()); };
		m_opcodes[0x37] = [=]() { PUL(STACK::U, FetchByte()); };
		m_opcodes[0x39] = [=]() { RTS(); };
		m_opcodes[0x3D] = [=]() { MUL(); };
		m_opcodes[0x3F] = [=]() { SWI(1); };

		m_opcodes[0x43] = [=]() { COM(m_reg.ab.A); }; // COMA
		m_opcodes[0x44] = [=]() { LSR(m_reg.ab.A); }; // LSRA
		m_opcodes[0x48] = [=]() { ASL(m_reg.ab.A); }; // ASLA
		m_opcodes[0x49] = [=]() { ROL(m_reg.ab.A); }; // ROLA
		m_opcodes[0x4A] = [=]() { DEC(m_reg.ab.A); }; // DECA
		m_opcodes[0x4C] = [=]() { INC(m_reg.ab.A); }; // INCA
		m_opcodes[0x4D] = [=]() { TST(m_reg.ab.A); }; // TSTA
		m_opcodes[0x4F] = [=]() { CLR(m_reg.ab.A); }; // CLRA

		m_opcodes[0x53] = [=]() { COM(m_reg.ab.B); }; // COMB
		m_opcodes[0x54] = [=]() { LSR(m_reg.ab.B); }; // LSRB
		m_opcodes[0x58] = [=]() { ASL(m_reg.ab.B); }; // ASLB
		m_opcodes[0x59] = [=]() { ROL(m_reg.ab.B); }; // ROLB
		m_opcodes[0x5A] = [=]() { DEC(m_reg.ab.B); }; // DECB
		m_opcodes[0x5C] = [=]() { INC(m_reg.ab.B); }; // INCB
		m_opcodes[0x5D] = [=]() { TST(m_reg.ab.B); }; // TSTB
		m_opcodes[0x5F] = [=]() { CLR(m_reg.ab.B); }; // CLRB

		m_opcodes[0x63] = [=]() { COMm(GetIndexed(FetchByte())); }; // COM indexed
		m_opcodes[0x64] = [=]() { LSRm(GetIndexed(FetchByte())); }; // LSR indexed
		m_opcodes[0x68] = [=]() { ASLm(GetIndexed(FetchByte())); }; // ASL indexed
		m_opcodes[0x69] = [=]() { ROLm(GetIndexed(FetchByte())); }; // ROL indexed
		m_opcodes[0x6A] = [=]() { DECm(GetIndexed(FetchByte())); }; // DEC indexed
		m_opcodes[0x6C] = [=]() { INCm(GetIndexed(FetchByte())); }; // INC indexed
		m_opcodes[0x6D] = [=]() { TST(GetMemIndexedByte()); }; // TST indirect
		m_opcodes[0x6E] = [=]() { JMP(GetIndexed(FetchByte())); }; // JMP indexed
		m_opcodes[0x6F] = [=]() { CLRm(GetIndexed(FetchByte())); }; // CLR indexed

		m_opcodes[0x73] = [=]() { COMm(GetExtended()); }; // COM extended
		m_opcodes[0x74] = [=]() { LSRm(GetExtended()); }; // LSR extended
		m_opcodes[0x78] = [=]() { ASLm(GetExtended()); }; // ASL extended
		m_opcodes[0x79] = [=]() { ROLm(GetExtended()); }; // ROL extended
		m_opcodes[0x7A] = [=]() { DECm(GetExtended()); }; // DEC extended
		m_opcodes[0x7C] = [=]() { INCm(GetExtended()); }; // INC extended
		m_opcodes[0x7D] = [=]() { TST(GetMemExtendedByte()); }; // TST extended
		m_opcodes[0x7E] = [=]() { JMP(GetExtended()); }; // JMP extended
		m_opcodes[0x7F] = [=]() { CLRm(GetExtended()); }; // CLR extended

		m_opcodes[0x80] = [=]() { SUB8(m_reg.ab.A, FetchByte()); }; // SUBA imm
		m_opcodes[0x81] = [=]() { CMP8(m_reg.ab.A, FetchByte()); }; // CMPA imm
		m_opcodes[0x83] = [=]() { SUB16(m_reg.D, FetchWord()); }; // SUBD imm
		m_opcodes[0x84] = [=]() { AND(m_reg.ab.A, FetchByte()); }; // ANDA imm
		m_opcodes[0x85] = [=]() { BIT(m_reg.ab.A, FetchByte()); }; // BITA imm
		m_opcodes[0x86] = [=]() { LD8(m_reg.ab.A, FetchByte()); }; // LDA imm
		m_opcodes[0x88] = [=]() { EOR(m_reg.ab.A, FetchByte()); }; // EORA imm
		m_opcodes[0x8A] = [=]() { OR(m_reg.ab.A, FetchByte()); }; // ORA imm
		m_opcodes[0x8B] = [=]() { ADD8(m_reg.ab.A, FetchByte()); }; // ADDA imm
		m_opcodes[0x8C] = [=]() { CMP16(m_reg.X, FetchWord()); }; // CMPX imm
		m_opcodes[0x8D] = [=]() { BSR(); }; // BSR rel
		m_opcodes[0x8E] = [=]() { LD16(m_reg.X, FetchWord()); }; // LDX imm

		m_opcodes[0x90] = [=]() { SUB8(m_reg.ab.A, GetMemDirectByte()); }; // SUBA direct
		m_opcodes[0x91] = [=]() { CMP8(m_reg.ab.A, GetMemDirectByte()); }; // CMPA direct
		m_opcodes[0x93] = [=]() { SUB16(m_reg.D, GetMemDirectWord()); }; // SUBD direct
		m_opcodes[0x94] = [=]() { AND(m_reg.ab.A, GetMemDirectByte()); }; // ANDA direct
		m_opcodes[0x95] = [=]() { BIT(m_reg.ab.A, GetMemDirectByte()); }; // BITA direct
		m_opcodes[0x96] = [=]() { LD8(m_reg.ab.A, GetMemDirectByte()); }; // LDA direct
		m_opcodes[0x97] = [=]() { ST8(GetDirect(FetchByte()), m_reg.ab.A); }; // STA direct
		m_opcodes[0x98] = [=]() { EOR(m_reg.ab.A, GetMemDirectByte()); }; // EORA direct
		m_opcodes[0x9A] = [=]() { OR(m_reg.ab.A, GetMemDirectByte()); }; // EORA direct
		m_opcodes[0x9B] = [=]() { ADD8(m_reg.ab.A, GetMemDirectByte()); }; // ADDA direct
		m_opcodes[0x9C] = [=]() { CMP16(m_reg.X, GetMemDirectWord()); }; // CMPX direct
		m_opcodes[0x9D] = [=]() { JSR(GetDirect(FetchByte())); }; // JSR direct
		m_opcodes[0x9E] = [=]() { LD16(m_reg.X, GetMemDirectWord()); }; // LDX direct
		m_opcodes[0x9F] = [=]() { ST16(GetDirect(FetchByte()),m_reg.X); }; // STU direct

		m_opcodes[0xA0] = [=]() { SUB8(m_reg.ab.A, GetMemIndexedByte()); }; // SUBA indexed
		m_opcodes[0xA1] = [=]() { CMP8(m_reg.ab.A, GetMemIndexedByte()); }; // CMPA indexed
		m_opcodes[0xA3] = [=]() { SUB16(m_reg.D, GetMemIndexedWord()); }; // SUBD indexed
		m_opcodes[0xA4] = [=]() { AND(m_reg.ab.A, GetMemIndexedByte()); }; // AND indexed
		m_opcodes[0xA5] = [=]() { BIT(m_reg.ab.A, GetMemIndexedByte()); }; // BIT indexed
		m_opcodes[0xA6] = [=]() { LD8(m_reg.ab.A, GetMemIndexedByte()); }; // LDA indexed
		m_opcodes[0xA7] = [=]() { ST8(GetIndexed(FetchByte()), m_reg.ab.A); }; // STA indexed
		m_opcodes[0xA8] = [=]() { EOR(m_reg.ab.A, GetMemIndexedByte()); }; // EORA indexed
		m_opcodes[0xAA] = [=]() { OR(m_reg.ab.A, GetMemIndexedByte()); }; // EORA indexed
		m_opcodes[0xAB] = [=]() { ADD8(m_reg.ab.A, GetMemIndexedByte()); }; // ADDA indexed
		m_opcodes[0xAC] = [=]() { CMP16(m_reg.X, GetMemIndexedWord()); }; // CMPX indexed
		m_opcodes[0xAD] = [=]() { JSR(GetIndexed(FetchByte())); }; // JSR indexed
		m_opcodes[0xAE] = [=]() { LD16(m_reg.X, GetMemIndexedWord()); }; // LDX indexed
		m_opcodes[0xAF] = [=]() { ST16(GetIndexed(FetchByte()), m_reg.X); }; // STU indexed

		m_opcodes[0xB0] = [=]() { SUB8(m_reg.ab.A, GetMemExtendedByte()); }; // SUBA extended
		m_opcodes[0xB1] = [=]() { CMP8(m_reg.ab.A, GetMemExtendedByte()); }; // CMPA extended
		m_opcodes[0xB3] = [=]() { SUB16(m_reg.D, GetMemExtendedWord()); }; // SUBD extended
		m_opcodes[0xB4] = [=]() { AND(m_reg.ab.A, GetMemExtendedByte()); }; // ANDA extended
		m_opcodes[0xB5] = [=]() { BIT(m_reg.ab.A, GetMemExtendedByte()); }; // BITA extended
		m_opcodes[0xB6] = [=]() { LD8(m_reg.ab.A, GetMemExtendedByte()); }; // LDA extended
		m_opcodes[0xB7] = [=]() { ST8(GetExtended(), m_reg.ab.A); }; // STA extended
		m_opcodes[0xB8] = [=]() { EOR(m_reg.ab.A, GetMemExtendedByte()); }; // EORA extended
		m_opcodes[0xBA] = [=]() { OR(m_reg.ab.A, GetMemExtendedByte()); }; // EORA extended
		m_opcodes[0xBB] = [=]() { ADD8(m_reg.ab.A, GetMemExtendedByte()); }; // ADDA extended
		m_opcodes[0xBC] = [=]() { CMP16(m_reg.X, GetMemExtendedWord()); }; // CMPX extended
		m_opcodes[0xBD] = [=]() { JSR(GetExtended()); }; // JSR extended
		m_opcodes[0xBE] = [=]() { LD16(m_reg.X, GetMemExtendedWord()); }; // LDX extended
		m_opcodes[0xBF] = [=]() { ST16(GetExtended(), m_reg.X); }; // STU extended

		m_opcodes[0xC0] = [=]() { SUB8(m_reg.ab.B, FetchByte()); }; // SUBB imm
		m_opcodes[0xC1] = [=]() { CMP8(m_reg.ab.B, FetchByte()); }; // CMPB imm
		m_opcodes[0xC3] = [=]() { ADD16(m_reg.D, FetchWord()); }; // ADDD imm
		m_opcodes[0xC4] = [=]() { AND(m_reg.ab.B, FetchByte()); }; // ANDB imm
		m_opcodes[0xC5] = [=]() { BIT(m_reg.ab.B, FetchByte()); }; // BITB imm
		m_opcodes[0xC6] = [=]() { LD8(m_reg.ab.B, FetchByte()); }; // LDB imm
		m_opcodes[0xC8] = [=]() { EOR(m_reg.ab.B, FetchByte()); }; // EORB imm
		m_opcodes[0xCA] = [=]() { OR(m_reg.ab.B, FetchByte()); }; // EORB imm
		m_opcodes[0xCB] = [=]() { ADD8(m_reg.ab.B, FetchByte()); }; // ADDB imm
		m_opcodes[0xCC] = [=]() { LD16(m_reg.D, FetchWord()); }; // LDD imm
		m_opcodes[0xCE] = [=]() { LD16(m_reg.U, FetchWord()); }; // LDU imm

		m_opcodes[0xD0] = [=]() { SUB8(m_reg.ab.B, GetMemDirectByte()); }; // SUBB direct
		m_opcodes[0xD1] = [=]() { CMP8(m_reg.ab.B, GetMemDirectByte()); }; // CMPB direct
		m_opcodes[0xD3] = [=]() { ADD16(m_reg.D, GetMemDirectWord()); }; // ADDD direct
		m_opcodes[0xD4] = [=]() { AND(m_reg.ab.B, GetMemDirectByte()); }; // ANDB direct
		m_opcodes[0xD5] = [=]() { BIT(m_reg.ab.B, GetMemDirectByte()); }; // BITB direct
		m_opcodes[0xD6] = [=]() { LD8(m_reg.ab.B, GetMemDirectByte()); }; // LDB direct
		m_opcodes[0xD7] = [=]() { ST8(GetDirect(FetchByte()), m_reg.ab.B); }; // STB direct
		m_opcodes[0xD8] = [=]() { EOR(m_reg.ab.B, GetMemDirectByte()); }; // EORB direct
		m_opcodes[0xDA] = [=]() { OR(m_reg.ab.B, GetMemDirectByte()); }; // EORB direct
		m_opcodes[0xDB] = [=]() { ADD8(m_reg.ab.B, GetMemDirectByte()); }; // ADDB direct
		m_opcodes[0xDC] = [=]() { LD16(m_reg.D, GetMemDirectWord()); }; // LDD direct
		m_opcodes[0xDD] = [=]() { ST16(GetDirect(FetchByte()), m_reg.D); }; // STD direct
		m_opcodes[0xDE] = [=]() { LD16(m_reg.U, GetMemDirectWord()); }; // LDU direct
		m_opcodes[0xDF] = [=]() { ST16(GetDirect(FetchByte()), m_reg.U); }; // STU direct

		m_opcodes[0xE0] = [=]() { SUB8(m_reg.ab.B, GetMemIndexedByte()); }; // SUBB indexed
		m_opcodes[0xE1] = [=]() { CMP8(m_reg.ab.B, GetMemIndexedByte()); }; // CMPB indexed
		m_opcodes[0xE3] = [=]() { ADD16(m_reg.D, GetMemIndexedWord()); }; // ADDD indexed
		m_opcodes[0xE4] = [=]() { AND(m_reg.ab.B, GetMemIndexedByte()); }; // ANDB indexed
		m_opcodes[0xE5] = [=]() { BIT(m_reg.ab.B, GetMemIndexedByte()); }; // BITB indexed
		m_opcodes[0xE6] = [=]() { LD8(m_reg.ab.B, GetMemIndexedByte()); }; // LDB indexed
		m_opcodes[0xE7] = [=]() { ST8(GetIndexed(FetchByte()), m_reg.ab.B); }; // STB indexed
		m_opcodes[0xE8] = [=]() { EOR(m_reg.ab.B, GetMemIndexedByte()); }; // EORB indexed
		m_opcodes[0xEA] = [=]() { OR(m_reg.ab.B, GetMemIndexedByte()); }; // EORB indexed
		m_opcodes[0xEB] = [=]() { ADD8(m_reg.ab.B, GetMemIndexedByte()); }; // ADDB indexed
		m_opcodes[0xEC] = [=]() { LD16(m_reg.D, GetMemIndexedWord()); }; // LDD indexed
		m_opcodes[0xED] = [=]() { ST16(GetIndexed(FetchByte()), m_reg.D); }; // STD indexed
		m_opcodes[0xEE] = [=]() { LD16(m_reg.U, GetMemIndexedWord()); }; // LDU indexed
		m_opcodes[0xEF] = [=]() { ST16(GetIndexed(FetchByte()), m_reg.U); }; // STU indexed

		m_opcodes[0xF0] = [=]() { SUB8(m_reg.ab.B, GetMemExtendedByte()); }; // SUBB extended
		m_opcodes[0xF1] = [=]() { CMP8(m_reg.ab.B, GetMemExtendedByte()); }; // CMPB extended
		m_opcodes[0xF3] = [=]() { ADD16(m_reg.D, GetMemExtendedWord()); }; // ADDD extended
		m_opcodes[0xF4] = [=]() { AND(m_reg.ab.B, GetMemExtendedByte()); }; // ANDB extended
		m_opcodes[0xF5] = [=]() { BIT(m_reg.ab.B, GetMemExtendedByte()); }; // BITB extended
		m_opcodes[0xF6] = [=]() { LD8(m_reg.ab.B, GetMemExtendedByte()); }; // LDB extended
		m_opcodes[0xF7] = [=]() { ST8(GetExtended(), m_reg.ab.B); }; // STB extended
		m_opcodes[0xF8] = [=]() { EOR(m_reg.ab.B, GetMemExtendedByte()); }; // EORB extended
		m_opcodes[0xFA] = [=]() { OR(m_reg.ab.B, GetMemExtendedByte()); }; // EORB extended
		m_opcodes[0xFB] = [=]() { ADD8(m_reg.ab.B, GetMemExtendedByte()); }; // ADDB extended
		m_opcodes[0xFC] = [=]() { LD16(m_reg.D, GetMemExtendedWord()); }; // LDD extended
		m_opcodes[0xFD] = [=]() { ST16(GetExtended(), m_reg.D); }; // STD extended
		m_opcodes[0xFE] = [=]() { LD16(m_reg.U, GetMemExtendedWord()); }; // LDU extended
		m_opcodes[0xFF] = [=]() { ST16(GetExtended(), m_reg.U); }; // STU extended

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

		m_opcodesPage2[0x3F] = [=]() { SWI(2); };

		m_opcodesPage2[0x83] = [=]() { CMP16(m_reg.D, FetchWord()); }; // CMPD imm
		m_opcodesPage2[0x8C] = [=]() { CMP16(m_reg.Y, FetchWord()); }; // CMPY imm
		m_opcodesPage2[0x8E] = [=]() { LD16(m_reg.Y, FetchWord()); }; // LDY imm

		m_opcodesPage2[0x93] = [=]() { CMP16(m_reg.D, GetMemDirectWord()); }; // CMPD direct
		m_opcodesPage2[0x9C] = [=]() { CMP16(m_reg.Y, GetMemDirectWord()); }; // CMPY direct
		m_opcodesPage2[0x9E] = [=]() { LD16(m_reg.Y, GetMemDirectWord()); }; // LDY direct
		m_opcodesPage2[0x9F] = [=]() { ST16(GetDirect(FetchByte()), m_reg.Y); }; // STY direct

		m_opcodesPage2[0xA3] = [=]() { CMP16(m_reg.D, GetMemIndexedWord()); }; // CMPD indexed
		m_opcodesPage2[0xAC] = [=]() { CMP16(m_reg.Y, GetMemIndexedWord()); }; // CMPY indexed
		m_opcodesPage2[0xAE] = [=]() { LD16(m_reg.Y, GetMemIndexedWord()); }; // LDY indexed
		m_opcodesPage2[0xAF] = [=]() { ST16(GetIndexed(FetchByte()), m_reg.Y); }; // STY indexed

		m_opcodesPage2[0xB3] = [=]() { CMP16(m_reg.D, GetMemExtendedWord()); }; // CMPD extended
		m_opcodesPage2[0xBC] = [=]() { CMP16(m_reg.Y, GetMemExtendedWord()); }; // CMPY extended
		m_opcodesPage2[0xBE] = [=]() { LD16(m_reg.Y, GetMemExtendedWord()); }; // LDY extended
		m_opcodesPage2[0xBF] = [=]() { ST16(GetExtended(), m_reg.Y); }; // STY extended

		m_opcodesPage2[0xCE] = [=]() { LD16(m_reg.S, FetchWord()); }; // LDS imm
		m_opcodesPage2[0xDE] = [=]() { LD16(m_reg.S, GetMemDirectWord()); }; // LDS direct
		m_opcodesPage2[0xDF] = [=]() { ST16(GetDirect(FetchByte()), m_reg.S); }; // STS direct

		m_opcodesPage2[0xEE] = [=]() { LD16(m_reg.S, GetMemIndexedWord()); }; // LDS indexed
		m_opcodesPage2[0xEF] = [=]() { ST16(GetIndexed(FetchByte()), m_reg.S); }; // STS indexed

		m_opcodesPage2[0xFE] = [=]() { LD16(m_reg.S, GetMemExtendedWord()); }; // LDS extended
		m_opcodesPage2[0xFF] = [=]() { ST16(GetExtended(), m_reg.S); }; // STS extended

	}

	// Prefix 0x11
	void CPU6809::InitPage3()
	{
		// Start with main opcode table for fallthrough
		m_opcodesPage3 = m_opcodes;

		// Override actual page 3 opcodes

		m_opcodesPage3[0x3F] = [=]() { SWI(3); };

		m_opcodesPage3[0x83] = [=]() { CMP16(m_reg.U, FetchWord()); }; // CMPU imm
		m_opcodesPage3[0x8C] = [=]() { CMP16(m_reg.S, FetchWord()); }; // CMPS imm

		m_opcodesPage3[0x93] = [=]() { CMP16(m_reg.U, GetMemDirectWord()); }; // CMPU direct
		m_opcodesPage3[0x9C] = [=]() { CMP16(m_reg.S, GetMemDirectWord()); }; // CMPS direct

		m_opcodesPage3[0xA3] = [=]() { CMP16(m_reg.U, GetMemIndexedWord()); }; // CMPU indexed
		m_opcodesPage3[0xAC] = [=]() { CMP16(m_reg.S, GetMemIndexedWord()); }; // CMPS indexed

		m_opcodesPage3[0xB3] = [=]() { CMP16(m_reg.U, GetMemExtendedWord()); }; // CMPU extended
		m_opcodesPage3[0xBC] = [=]() { CMP16(m_reg.S, GetMemExtendedWord()); }; // CMPS extended

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

	BYTE CPU6809::GetMemDirectByte()
	{
		ADDRESS src = GetDirect(FetchByte());
		return m_memory.Read8(src);
	}

	WORD CPU6809::GetMemDirectWord()
	{
		ADDRESS src = GetDirect(FetchByte());
		return m_memory.Read16be(src);
	}

	BYTE CPU6809::GetMemIndexedByte()
	{
		ADDRESS src = GetIndexed(FetchByte());
		return m_memory.Read8(src);
	}
	WORD CPU6809::GetMemIndexedWord()
	{
		ADDRESS src = GetIndexed(FetchByte());
		return m_memory.Read16be(src);
	}

	BYTE CPU6809::GetMemExtendedByte()
	{
		ADDRESS src = GetExtended();
		return m_memory.Read8(src);
	}
	WORD CPU6809::GetMemExtendedWord()
	{
		ADDRESS src = GetExtended();
		return m_memory.Read16be(src);
	}

	WORD& CPU6809::GetIndexedRegister(BYTE idx)
	{
		switch ((idx >> 5) & 3)
		{
		case 0: return m_reg.X;
		case 1: return m_reg.Y;
		case 2: return m_reg.U;
		case 3: return m_reg.S;
		default:
			throw std::exception("not possible");
		}
	}

	// Get effective address for indexed mode
	ADDRESS CPU6809::GetIndexed(BYTE idx)
	{
		WORD& reg = GetIndexedRegister(idx);

		bool indirect = GetBit(idx, 4);

		ADDRESS ea;

		if (!GetMSB(idx)) //,R + 5 bit offset
		{
			// Extend sign bit
			bool& signBit = indirect; // Same bit as indirect flag
			BYTE offset = idx;
			SetBitMask(offset, 0b11100000, signBit);
			ea = reg + (SBYTE)offset;
			indirect = false;
		}
		else switch (idx & 0b1111)
		{
		case 0b0000: ea = reg++; break; // ,R+
		case 0b0001: ea = reg; reg += 2; break; // ,R++
		case 0b0010: ea = --reg; break; // ,-R
		case 0b0011: reg -= 2; ea = reg; break; // ,--R
		case 0b0100: ea = reg; break; // ,R + 0 Offset
		case 0b0101: ea = reg + (SBYTE)m_reg.ab.B; break; // ,R + B Offset
		case 0b0110: ea = reg + (SBYTE)m_reg.ab.A; break; // ,R + A Offset
			// 0b0111: n/a
		case 0b1000: ea = reg + FetchSignedByte(); break; // ,R + 8 bit offset
		case 0b1001: ea = reg + FetchSignedWord(); break; // ,R + 16 bit offset
			// 0b1010: n/a
		case 0b1011: ea = reg + (SWORD)m_reg.D; break; // ,R + D offset
		case 0b1100: ea = m_programCounter + FetchSignedByte(); break; // ,PC + 8 bit offset
		case 0b1101: ea = m_programCounter + FetchSignedWord(); break; // ,PC + 16 bit offset
			// 0b1110: n/a
		case 0b1111: ea = FetchWord(); break;// [,Address]

		default:
			LogPrintf(LOG_ERROR, "Invalid addressing mode (%02X)", idx);
			throw std::exception("Invalid addressing mode");
		}

		// Cleanup ea
		ea &= 0xFFFF;

		if (indirect)
		{
			// TODO: 'illegal' modes
			return m_memory.Read16be(ea);
		}

		return ea;
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


	void CPU6809::PUSH(BYTE value)
	{
		TICK1();
		WORD& sp = *m_currStack;

		m_memory.Write8(--sp, value);
	}

	BYTE CPU6809::POP()
	{
		TICK1();
		WORD& sp = *m_currStack;

		return m_memory.Read8(sp++);
	}

	void CPU6809::PSH(STACK s, BYTE regs)
	{
		SetStack(s);

		if (GetBit(regs, 7))
		{
			PUSH(GetLByte(m_PC));
			PUSH(GetHByte(m_PC));
		}
		if (GetBit(regs, 6))
		{
			// Select the 'other' stack
			const WORD& sp = (s == STACK::S) ? m_reg.U : m_reg.S;

			PUSH(GetLByte(sp));
			PUSH(GetHByte(sp));
		}
		if (GetBit(regs, 5))
		{
			PUSH(GetLByte(m_reg.Y));
			PUSH(GetHByte(m_reg.Y));
		}
		if (GetBit(regs, 4))
		{
			PUSH(GetLByte(m_reg.X));
			PUSH(GetHByte(m_reg.X));
		}
		if (GetBit(regs, 3))
		{
			PUSH(m_reg.DP);
		}
		if (GetBit(regs, 2))
		{
			PUSH(m_reg.ab.B);
		}
		if (GetBit(regs, 1))
		{
			PUSH(m_reg.ab.A);
		}
		if (GetBit(regs, 0))
		{
			PUSH(m_reg.flags);
		}
	}
	void CPU6809::PUL(STACK s, BYTE regs)
	{
		SetStack(s);

		if (GetBit(regs, 0))
		{
			m_reg.flags = POP();
		}
		if (GetBit(regs, 1))
		{
			m_reg.ab.A = POP();
		}
		if (GetBit(regs, 2))
		{
			m_reg.ab.B = POP();
		}
		if (GetBit(regs, 3))
		{
			m_reg.DP = POP();
		}
		if (GetBit(regs, 4))
		{
			SetHByte(m_reg.X, POP());
			SetLByte(m_reg.X, POP());
		}
		if (GetBit(regs, 5))
		{
			SetHByte(m_reg.Y, POP());
			SetLByte(m_reg.Y, POP());
		}
		if (GetBit(regs, 6))
		{
			// Select the 'other' stack
			WORD& sp = (s == STACK::S) ? m_reg.U : m_reg.S;

			SetHByte(sp, POP());
			SetLByte(sp, POP());
		}
		if (GetBit(regs, 7))
		{
			SetHByte(m_PC, POP());
			SetLByte(m_PC, POP());
		}
	}

	// Branching
	void CPU6809::BRA(bool condition)
	{
		SBYTE rel = FetchSignedByte();
		if (condition == true)
		{
			m_programCounter += rel;
			// No time penalty for short branches
		}
	}

	void CPU6809::LBRA(bool condition)
	{
		SWORD rel = FetchSignedWord();
		if (condition == true)
		{
			m_programCounter += rel;
			TICKT3();
		}
	}

	void CPU6809::JSR(ADDRESS dest)
	{
		SetStack(STACK::S);
		PUSH(GetLByte(m_PC));
		PUSH(GetHByte(m_PC));

		m_programCounter = dest;
	}

	void CPU6809::BSR()
	{
		SBYTE rel = FetchSignedByte();

		SetStack(STACK::S);
		PUSH(GetLByte(m_PC));
		PUSH(GetHByte(m_PC));

		m_PC += rel;
	}

	void CPU6809::RTS()
	{
		ADDRESS old = m_programCounter;
		WORD oldS = m_reg.S;

		SetStack(STACK::S);
		SetHByte(m_PC, POP());
		SetLByte(m_PC, POP());
	}

	void CPU6809::SWI(BYTE swi)
	{
		SetFlag(FLAG_E, true);

		// Push all registers on the system stack
		PSH(STACK::S, REGS_ALL);

		ADDRESS vect;
		switch (swi)
		{
		case 1:
			vect = ADDR_SWI;
			SetFlag(FLAG_I, true);
			SetFlag(FLAG_F, true);
			break;
		case 2:
			vect = ADDR_SWI2;
			break;
		case 3:
			vect = ADDR_SWI3;
			break;
		default:
			throw("Not possible");
		}

		m_PC = m_memory.Read16be(vect);
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

	void CPU6809::LEA(WORD& dest, bool setZero)
	{
		dest = GetIndexed(FetchByte());

		if (setZero)
		{
			SetFlag(FLAG_Z, dest == 0);
		}
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

	void CPU6809::ST8(ADDRESS dest, BYTE src)
	{
		m_memory.Write8(dest, src);

		AdjustNZ(src);
		SetFlag(FLAG::FLAG_V, false);
	}

	void CPU6809::ST16(ADDRESS dest, WORD src)
	{
		m_memory.Write16be(dest, src);

		AdjustNZ(src);
		SetFlag(FLAG::FLAG_V, false);
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

	void CPU6809::COM(BYTE& dest)
	{
		dest = ~dest;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
		SetFlag(FLAG_C, false);
	}

	void CPU6809::COMm(ADDRESS dest)
	{
		BYTE value = m_memory.Read8(dest);
		COM(value);
		m_memory.Write8(dest, value);
	}

	void CPU6809::CLR(BYTE& dest)
	{
		dest = 0;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
		SetFlag(FLAG_C, false);
	}

	void CPU6809::CLRm(ADDRESS dest)
	{
		BYTE value = 0;
		m_memory.Write8(dest, value);

		AdjustNZ(value);
		SetFlag(FLAG_V, false);
		SetFlag(FLAG_C, false);
	}

	void CPU6809::ASL(BYTE& dest)
	{
		//TODO: Half carry undefined
		bool carry = GetMSB(dest);
		SetFlag(FLAG_V, GetBit(dest, 6) ^ carry);
		SetFlag(FLAG_C, carry);

		dest <<= 1;

		AdjustNZ(dest);
	}

	void CPU6809::ASLm(ADDRESS dest)
	{
		BYTE value = m_memory.Read8(dest);
		ASL(value);
		m_memory.Write8(dest, value);
	}

	void CPU6809::LSR(BYTE& dest)
	{
		bool lsb = GetLSB(dest);
		dest >>= 1;

		SetFlag(FLAG_C, lsb);
		AdjustNZ(dest);
	}

	void CPU6809::LSRm(ADDRESS dest)
	{
		BYTE value = m_memory.Read8(dest);
		LSR(value);
		m_memory.Write8(dest, value);
	}

	void CPU6809::ROL(BYTE& dest)
	{
		bool oldCarry = GetFlag(FLAG_C);
		bool msb = GetMSB(dest);
		SetFlag(FLAG_V, GetBit(dest, 6) ^ msb);
		SetFlag(FLAG_C, msb);

		dest <<= 1;
		SetBit(dest, 0, oldCarry);

		AdjustNZ(dest);
	}

	void CPU6809::ROLm(ADDRESS dest)
	{
		BYTE value = m_memory.Read8(dest);
		ROL(value);
		m_memory.Write8(dest, value);
	}

	void CPU6809::EOR(BYTE& dest, BYTE src)
	{
		dest ^= src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}
	void CPU6809::OR(BYTE& dest, BYTE src)
	{
		dest |= src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}
	void CPU6809::AND(BYTE& dest, BYTE src)
	{
		dest &= src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	void CPU6809::INC(BYTE& dest)
	{
		SetFlag(FLAG_V, dest == 0b01111111);
		++dest;
		AdjustNZ(dest);
	}
	void CPU6809::INCm(ADDRESS dest)
	{
		BYTE value = m_memory.Read8(dest);
		INC(value);
		m_memory.Write8(dest, value);
	}

	void CPU6809::DEC(BYTE& dest)
	{
		SetFlag(FLAG_V, dest == 0b10000000);
		--dest;
		AdjustNZ(dest);
	}
	void CPU6809::DECm(ADDRESS dest)
	{
		BYTE value = m_memory.Read8(dest);
		DEC(value);
		m_memory.Write8(dest, value);
	}

	void CPU6809::TST(const BYTE dest)
	{
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	void CPU6809::ADD8(BYTE& dest, BYTE src, bool carry)
	{
		BYTE oldDest = dest;

		// AC flag
		BYTE loNibble = (dest & 0x0F) + (src & 0x0F);

		WORD temp = dest + src;
		if (carry)
		{
			temp++;
			loNibble++;
		}

		dest = (BYTE)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_H, (loNibble > 0x0F));
		SetFlag(FLAG_C, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
	}

	void CPU6809::ADD16(WORD& dest, WORD src, bool carry)
	{
		WORD oldDest = dest;

		DWORD temp = dest + src;
		if (carry)
		{
			temp++;
		}

		dest = (WORD)temp;

		AdjustNZ(dest);
		SetFlag(FLAG_C, (temp > 0xFFFF));
		SetFlag(FLAG_V, (GetMSB(oldDest) == GetMSB(src)) && (GetMSB(dest) != GetMSB(src)));
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

	void CPU6809::MUL()
	{
		WORD res = m_reg.ab.A * m_reg.ab.B;

		SetFlag(FLAG_Z, res == 0);
		SetFlag(FLAG_C, GetBit(res, 7));

		m_reg.D = res;
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
