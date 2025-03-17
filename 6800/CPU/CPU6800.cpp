#include "stdafx.h"
#include "CPU6800.h"

using cpuInfo::Opcode;

namespace emul
{
	CPU6800::CPU6800(Memory& memory) : CPU6800(CPUID_6800, memory)
	{
	}

	CPU6800::CPU6800(const char* cpuid, Memory& memory) :
		CPU(CPU6800_ADDRESS_BITS, memory),
		m_info(cpuid),
		Logger(cpuid)
	{
		static_assert(sizeof(m_reg.D) == 2);
		static_assert(sizeof(m_reg.ab.A) == 1);
		static_assert(sizeof(m_reg.ab.B) == 1);

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

		m_opcodes[0x01] = [=]() {}; // NOP
		m_opcodes[0x06] = [=]() { SetFlags(m_reg.ab.A); }; // TAP
		m_opcodes[0x07] = [=]() { m_reg.ab.A = m_flags; }; // TPA
		m_opcodes[0x08] = [=]() { ++m_reg.X; SetFlag(FLAG_Z, m_reg.X == 0); }; // INX
		m_opcodes[0x09] = [=]() { --m_reg.X; SetFlag(FLAG_Z, m_reg.X == 0); }; // DEX
		m_opcodes[0x0A] = [=]() { SetFlag(FLAG_V, false); }; // CLV
		m_opcodes[0x0B] = [=]() { SetFlag(FLAG_V, true); }; // SEV
		m_opcodes[0x0C] = [=]() { SetFlag(FLAG_C, false); }; // CLC
		m_opcodes[0x0D] = [=]() { SetFlag(FLAG_C, true); }; // SEC
		m_opcodes[0x0E] = [=]() { m_clearIntMask = true; }; // CLI
		m_opcodes[0x0F] = [=]() { SetFlag(FLAG_I, true); }; // SEI
		m_opcodes[0x10] = [=]() { SUB8(m_reg.ab.A, m_reg.ab.B); }; // SBA
		m_opcodes[0x11] = [=]() { CMP8(m_reg.ab.A, m_reg.ab.B); }; // CBA
		m_opcodes[0x16] = [=]() { m_reg.ab.B = m_reg.ab.A; AdjustNZ(m_reg.ab.B); SetFlag(FLAG_V, false); }; // TAB
		m_opcodes[0x17] = [=]() { m_reg.ab.A = m_reg.ab.B; AdjustNZ(m_reg.ab.A); SetFlag(FLAG_V, false); }; // TBA
		//TODO m_opcodes[0x19] = [=]() {}; // DAA
		m_opcodes[0x1B] = [=]() { ADD8(m_reg.ab.A, m_reg.ab.B); }; // ABA

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

		m_opcodes[0x30] = [=]() { m_reg.X = m_reg.SP + 1; }; // TSX
		m_opcodes[0x31] = [=]() { ++m_reg.SP; }; // INS
		m_opcodes[0x32] = [=]() { m_reg.ab.A = popB(); }; // PUL A
		m_opcodes[0x33] = [=]() { m_reg.ab.B = popB(); }; // PUL B
		m_opcodes[0x34] = [=]() { --m_reg.SP; }; // DES
		m_opcodes[0x35] = [=]() { m_reg.SP = m_reg.X - 1; }; // TXS
		m_opcodes[0x36] = [=]() { pushB(m_reg.ab.A); }; // PSH A
		m_opcodes[0x37] = [=]() { pushB(m_reg.ab.B); }; // PSH B
		m_opcodes[0x39] = [=]() { RTS(); }; // RTS
		m_opcodes[0x3B] = [=]() { RTI(); }; // RTI
		m_opcodes[0x3E] = [=]() { WAI(); }; // WAI
		m_opcodes[0x3F] = [=]() { SWI(); }; // SWI

		m_opcodes[0x40] = [=]() { NEG(m_reg.ab.A); }; // NEG A
		m_opcodes[0x43] = [=]() { COM(m_reg.ab.A); }; // COM A
		m_opcodes[0x44] = [=]() { LSR(m_reg.ab.A); }; // LSR A
		m_opcodes[0x46] = [=]() { ROR(m_reg.ab.A); }; // ROR A
		m_opcodes[0x47] = [=]() { ASR(m_reg.ab.A); }; // ASR A
		m_opcodes[0x48] = [=]() { ASL(m_reg.ab.A); }; // ASL A
		m_opcodes[0x49] = [=]() { ROL(m_reg.ab.A); }; // ROL A
		m_opcodes[0x4A] = [=]() { DEC(m_reg.ab.A); }; // DEC A
		m_opcodes[0x4C] = [=]() { INC(m_reg.ab.A); }; // INC A
		m_opcodes[0x4D] = [=]() { TST(m_reg.ab.A); }; // TST A
		m_opcodes[0x4F] = [=]() { CLR(m_reg.ab.A); }; // CLR A

		m_opcodes[0x50] = [=]() { NEG(m_reg.ab.B); }; // NEG B
		m_opcodes[0x53] = [=]() { COM(m_reg.ab.B); }; // COM B
		m_opcodes[0x54] = [=]() { LSR(m_reg.ab.B); }; // LSR B
		m_opcodes[0x56] = [=]() { ROR(m_reg.ab.B); }; // ROR B
		m_opcodes[0x57] = [=]() { ASR(m_reg.ab.B); }; // ASR B
		m_opcodes[0x58] = [=]() { ASL(m_reg.ab.B); }; // ASL B
		m_opcodes[0x59] = [=]() { ROL(m_reg.ab.B); }; // ROL B
		m_opcodes[0x5A] = [=]() { DEC(m_reg.ab.B); }; // DEC B
		m_opcodes[0x5C] = [=]() { INC(m_reg.ab.B); }; // INC B
		m_opcodes[0x5D] = [=]() { TST(m_reg.ab.B); }; // TST B
		m_opcodes[0x5F] = [=]() { CLR(m_reg.ab.B); }; // CLR B

		m_opcodes[0x60] = [=]() { MEMIndexedOp(&CPU6800::NEG); }; // NEG idx
		m_opcodes[0x63] = [=]() { MEMIndexedOp(&CPU6800::COM); }; // COM idx
		m_opcodes[0x64] = [=]() { MEMIndexedOp(&CPU6800::LSR); }; // LSR idx
		m_opcodes[0x66] = [=]() { MEMIndexedOp(&CPU6800::ROR); }; // ROR idx
		m_opcodes[0x67] = [=]() { MEMIndexedOp(&CPU6800::ASR); }; // ASR idx
		m_opcodes[0x68] = [=]() { MEMIndexedOp(&CPU6800::ASL); }; // ASL idx
		m_opcodes[0x69] = [=]() { MEMIndexedOp(&CPU6800::ROL); }; // ROL idx
		m_opcodes[0x6A] = [=]() { MEMIndexedOp(&CPU6800::DEC); }; // DEC idx
		m_opcodes[0x6C] = [=]() { MEMIndexedOp(&CPU6800::INC); }; // INC idx
		m_opcodes[0x6D] = [=]() { TST(GetMemIndexedByte()); }; // TST idx
		m_opcodes[0x6E] = [=]() { JMP(GetIndexed()); }; // JMP idx
		m_opcodes[0x6F] = [=]() { MEMIndexedOp(&CPU6800::CLR); }; // CLR idx

		m_opcodes[0x70] = [=]() { MEMExtendedOp(&CPU6800::NEG); }; // NEG ext
		m_opcodes[0x73] = [=]() { MEMExtendedOp(&CPU6800::COM); }; // COM ext
		m_opcodes[0x74] = [=]() { MEMExtendedOp(&CPU6800::LSR); }; // LSR ext
		m_opcodes[0x76] = [=]() { MEMExtendedOp(&CPU6800::ROR); }; // ROR ext
		m_opcodes[0x77] = [=]() { MEMExtendedOp(&CPU6800::ASR); }; // ASR ext
		m_opcodes[0x78] = [=]() { MEMExtendedOp(&CPU6800::ASL); }; // ASL ext
		m_opcodes[0x79] = [=]() { MEMExtendedOp(&CPU6800::ROL); }; // ROL ext
		m_opcodes[0x7A] = [=]() { MEMExtendedOp(&CPU6800::DEC); }; // DEC ext
		m_opcodes[0x7C] = [=]() { MEMExtendedOp(&CPU6800::INC); }; // INC ext
		m_opcodes[0x7D] = [=]() { TST(GetMemExtendedByte()); }; // TST ext
		m_opcodes[0x7E] = [=]() { JMP(GetExtended()); }; // JMP ext
		m_opcodes[0x7F] = [=]() { MEMExtendedOp(&CPU6800::CLR); }; // CLR ext

		m_opcodes[0x80] = [=]() { SUB8(m_reg.ab.A, FetchByte()); }; // SUB A imm
		m_opcodes[0x81] = [=]() { CMP8(m_reg.ab.A, FetchByte()); }; // CMP A imm
		m_opcodes[0x82] = [=]() { SUB8(m_reg.ab.A, FetchByte(), GetFlag(FLAG_C)); }; // SBC A imm
		m_opcodes[0x84] = [=]() { AND(m_reg.ab.A, FetchByte()); }; // AND A imm
		m_opcodes[0x85] = [=]() { BIT(m_reg.ab.A, FetchByte()); }; // BIT A imm
		m_opcodes[0x86] = [=]() { LD8(m_reg.ab.A, FetchByte()); }; // LDA A imm
		m_opcodes[0x88] = [=]() { EOR(m_reg.ab.A, FetchByte()); }; // EOR A imm
		m_opcodes[0x89] = [=]() { ADD8(m_reg.ab.A, FetchByte(), GetFlag(FLAG_C)); }; // ADC A imm
		m_opcodes[0x8A] = [=]() { OR(m_reg.ab.A, FetchByte()); }; // ORA A imm
		m_opcodes[0x8B] = [=]() { ADD8(m_reg.ab.A, FetchByte()); }; // ADD A imm
		m_opcodes[0x8C] = [=]() { CMP16(m_reg.X, FetchWord()); }; // CPX imm
		m_opcodes[0x8D] = [=]() { BSR(); }; // BSR rel
		m_opcodes[0x8E] = [=]() { LD16(m_reg.SP, FetchWord()); }; // LDS imm

		m_opcodes[0x90] = [=]() { SUB8(m_reg.ab.A, GetMemDirectByte()); }; // SUB A dir
		m_opcodes[0x91] = [=]() { CMP8(m_reg.ab.A, GetMemDirectByte()); }; // CMP A dir
		m_opcodes[0x92] = [=]() { SUB8(m_reg.ab.A, GetMemDirectByte(), GetFlag(FLAG_C)); }; // SBC A dir
		m_opcodes[0x94] = [=]() { AND(m_reg.ab.A, GetMemDirectByte()); }; // AND A dir
		m_opcodes[0x95] = [=]() { BIT(m_reg.ab.A, GetMemDirectByte()); }; // BIT A dir
		m_opcodes[0x96] = [=]() { LD8(m_reg.ab.A, GetMemDirectByte()); }; // LDA A dir
		m_opcodes[0x97] = [=]() { ST8(GetDirect(), m_reg.ab.A); }; // STA A dir
		m_opcodes[0x98] = [=]() { EOR(m_reg.ab.A, GetMemDirectByte()); }; // EOR A dir
		m_opcodes[0x99] = [=]() { ADD8(m_reg.ab.A, GetMemDirectByte(), GetFlag(FLAG_C)); }; // ADC A dir
		m_opcodes[0x9A] = [=]() { OR(m_reg.ab.A, GetMemDirectByte()); }; // ORA A dir
		m_opcodes[0x9B] = [=]() { ADD8(m_reg.ab.A, GetMemDirectByte()); }; // ADD A dir
		m_opcodes[0x9C] = [=]() { CMP16(m_reg.X, GetMemDirectWord()); }; // CPX dir
		m_opcodes[0x9E] = [=]() { LD16(m_reg.SP, GetMemDirectWord()); }; // LDS dir
		m_opcodes[0x9F] = [=]() { ST16(GetDirect(), m_reg.SP); }; // STS dir

		m_opcodes[0xA0] = [=]() { SUB8(m_reg.ab.A, GetMemIndexedByte()); }; // SUB A idx
		m_opcodes[0xA1] = [=]() { CMP8(m_reg.ab.A, GetMemIndexedByte()); }; // CMP A idx
		m_opcodes[0xA2] = [=]() { SUB8(m_reg.ab.A, GetMemIndexedByte(), GetFlag(FLAG_C)); }; // SBC A idx
		m_opcodes[0xA4] = [=]() { AND(m_reg.ab.A, GetMemIndexedByte()); }; // AND A idx
		m_opcodes[0xA5] = [=]() { BIT(m_reg.ab.A, GetMemIndexedByte()); }; // BIT A idx
		m_opcodes[0xA6] = [=]() { LD8(m_reg.ab.A, GetMemIndexedByte()); }; // LDA A idx
		m_opcodes[0xA7] = [=]() { ST8(GetIndexed(), m_reg.ab.A); }; // STA A idx
		m_opcodes[0xA8] = [=]() { EOR(m_reg.ab.A, GetMemIndexedByte()); }; // EOR A idx
		m_opcodes[0xA9] = [=]() { ADD8(m_reg.ab.A, GetMemIndexedByte(), GetFlag(FLAG_C)); }; // ADC A idx
		m_opcodes[0xAA] = [=]() { OR(m_reg.ab.A, GetMemIndexedByte()); }; // ORA A idx
		m_opcodes[0xAB] = [=]() { ADD8(m_reg.ab.A, GetMemIndexedByte()); }; // ADD A idx
		m_opcodes[0xAC] = [=]() { CMP16(m_reg.X, GetMemIndexedWord()); }; // CPX idx
		m_opcodes[0xAD] = [=]() { JSR(GetIndexed()); }; // JSR idx
		m_opcodes[0xAE] = [=]() { LD16(m_reg.SP, GetMemIndexedWord()); }; // LDS idx
		m_opcodes[0xAF] = [=]() { ST16(GetIndexed(), m_reg.SP); }; // STS idx

		m_opcodes[0xB0] = [=]() { SUB8(m_reg.ab.A, GetMemExtendedByte()); }; // SUB A ext
		m_opcodes[0xB1] = [=]() { CMP8(m_reg.ab.A, GetMemExtendedByte()); }; // CMP A ext
		m_opcodes[0xB2] = [=]() { SUB8(m_reg.ab.A, GetMemExtendedByte(), GetFlag(FLAG_C)); }; // SBC A ext
		m_opcodes[0xB4] = [=]() { AND(m_reg.ab.A, GetMemExtendedByte()); }; // AND A ext
		m_opcodes[0xB5] = [=]() { BIT(m_reg.ab.A, GetMemExtendedByte()); }; // BIT A ext
		m_opcodes[0xB6] = [=]() { LD8(m_reg.ab.A, GetMemExtendedByte()); }; // LDA A ext
		m_opcodes[0xB7] = [=]() { ST8(GetExtended(), m_reg.ab.A); }; // STA A ext
		m_opcodes[0xB8] = [=]() { EOR(m_reg.ab.A, GetMemExtendedByte()); }; // EOR A ext
		m_opcodes[0xB9] = [=]() { ADD8(m_reg.ab.A, GetMemExtendedByte(), GetFlag(FLAG_C)); }; // ADC A ext
		m_opcodes[0xBA] = [=]() { OR(m_reg.ab.A, GetMemExtendedByte()); }; // ORA A ext
		m_opcodes[0xBB] = [=]() { ADD8(m_reg.ab.A, GetMemExtendedByte()); }; // ADD A ext
		m_opcodes[0xBC] = [=]() { CMP16(m_reg.X, GetMemExtendedWord()); }; // CPX ext
		m_opcodes[0xBD] = [=]() { JSR(GetExtended()); }; // JSR ext
		m_opcodes[0xBE] = [=]() { LD16(m_reg.SP, GetMemExtendedWord()); }; // LDS ext
		m_opcodes[0xBF] = [=]() { ST16(GetExtended(), m_reg.SP); }; // STS ext

		m_opcodes[0xC0] = [=]() { SUB8(m_reg.ab.B, FetchByte()); }; // SUB B imm
		m_opcodes[0xC1] = [=]() { CMP8(m_reg.ab.B, FetchByte()); }; // CMP B imm
		m_opcodes[0xC2] = [=]() { SUB8(m_reg.ab.B, FetchByte(), GetFlag(FLAG_C)); }; // SBC B imm
		m_opcodes[0xC4] = [=]() { AND(m_reg.ab.B, FetchByte()); }; // AND B imm
		m_opcodes[0xC5] = [=]() { BIT(m_reg.ab.B, FetchByte()); }; // BIT B imm
		m_opcodes[0xC6] = [=]() { LD8(m_reg.ab.B, FetchByte()); }; // LDA B imm
		m_opcodes[0xC8] = [=]() { EOR(m_reg.ab.B, FetchByte()); }; // EOR B imm
		m_opcodes[0xC9] = [=]() { ADD8(m_reg.ab.B, FetchByte(), GetFlag(FLAG_C)); }; // ADC B imm
		m_opcodes[0xCA] = [=]() { OR(m_reg.ab.B, FetchByte()); }; // ORA B imm
		m_opcodes[0xCB] = [=]() { ADD8(m_reg.ab.B, FetchByte()); }; // ADD B imm
		m_opcodes[0xCE] = [=]() { LD16(m_reg.X, FetchWord()); }; // LDX imm

		m_opcodes[0xD0] = [=]() { SUB8(m_reg.ab.B, GetMemDirectByte()); }; // SUB B dir
		m_opcodes[0xD1] = [=]() { CMP8(m_reg.ab.B, GetMemDirectByte()); }; // CMP B dir
		m_opcodes[0xD2] = [=]() { SUB8(m_reg.ab.B, GetMemDirectByte(), GetFlag(FLAG_C)); }; // SBC B dir
		m_opcodes[0xD4] = [=]() { AND(m_reg.ab.B, GetMemDirectByte()); }; // AND B dir
		m_opcodes[0xD5] = [=]() { BIT(m_reg.ab.B, GetMemDirectByte()); }; // BIT B dir
		m_opcodes[0xD6] = [=]() { LD8(m_reg.ab.B, GetMemDirectByte()); }; // LDA B dir
		m_opcodes[0xD7] = [=]() { ST8(GetDirect(), m_reg.ab.B); }; // STA B dir
		m_opcodes[0xD8] = [=]() { EOR(m_reg.ab.B, GetMemDirectByte()); }; // EOR B dir
		m_opcodes[0xD9] = [=]() { ADD8(m_reg.ab.B, GetMemDirectByte(), GetFlag(FLAG_C)); }; // ADC B dir
		m_opcodes[0xDA] = [=]() { OR(m_reg.ab.B, GetMemDirectByte()); }; // ORA B dir
		m_opcodes[0xDB] = [=]() { ADD8(m_reg.ab.B, GetMemDirectByte()); }; // ADD B dir
		m_opcodes[0xDE] = [=]() { LD16(m_reg.X, GetMemDirectWord()); }; // LDX dir
		m_opcodes[0xDF] = [=]() { ST16(GetDirect(), m_reg.X); }; // STX dir

		m_opcodes[0xE0] = [=]() { SUB8(m_reg.ab.B, GetMemIndexedByte()); }; // SUB B idx
		m_opcodes[0xE1] = [=]() { CMP8(m_reg.ab.B, GetMemIndexedByte()); }; // CMP B idx
		m_opcodes[0xE2] = [=]() { SUB8(m_reg.ab.B, GetMemIndexedByte(), GetFlag(FLAG_C)); }; // SBC B idx
		m_opcodes[0xE4] = [=]() { AND(m_reg.ab.B, GetMemIndexedByte()); }; // AND B idx
		m_opcodes[0xE5] = [=]() { BIT(m_reg.ab.B, GetMemIndexedByte()); }; // BIT B idx
		m_opcodes[0xE6] = [=]() { LD8(m_reg.ab.B, GetMemIndexedByte()); }; // LDA B idx
		m_opcodes[0xE7] = [=]() { ST8(GetIndexed(), m_reg.ab.B); }; // STA B idx
		m_opcodes[0xE8] = [=]() { EOR(m_reg.ab.B, GetMemIndexedByte()); }; // EOR B idx
		m_opcodes[0xE9] = [=]() { ADD8(m_reg.ab.B, GetMemIndexedByte(), GetFlag(FLAG_C)); }; // ADC B idx
		m_opcodes[0xEA] = [=]() { OR(m_reg.ab.B, GetMemIndexedByte()); }; // ORA B idx
		m_opcodes[0xEB] = [=]() { ADD8(m_reg.ab.B, GetMemIndexedByte()); }; // ADD B idx
		m_opcodes[0xEE] = [=]() { LD16(m_reg.X, GetMemIndexedWord()); }; // LDX idx
		m_opcodes[0xEF] = [=]() { ST16(GetIndexed(), m_reg.X); }; // STX idx

		m_opcodes[0xF0] = [=]() { SUB8(m_reg.ab.B, GetMemExtendedByte()); }; // SUB B ext
		m_opcodes[0xF1] = [=]() { CMP8(m_reg.ab.B, GetMemExtendedByte()); }; // CMP B ext
		m_opcodes[0xF2] = [=]() { SUB8(m_reg.ab.B, GetMemExtendedByte(), GetFlag(FLAG_C)); }; // SBC B ext
		m_opcodes[0xF4] = [=]() { AND(m_reg.ab.B, GetMemExtendedByte()); }; // AND B ext
		m_opcodes[0xF5] = [=]() { BIT(m_reg.ab.B, GetMemExtendedByte()); }; // BIT B ext
		m_opcodes[0xF6] = [=]() { LD8(m_reg.ab.B, GetMemExtendedByte()); }; // LDA B ext
		m_opcodes[0xF7] = [=]() { ST8(GetExtended(), m_reg.ab.B); }; // STA B ext
		m_opcodes[0xF8] = [=]() { EOR(m_reg.ab.B, GetMemExtendedByte()); }; // EOR B ext
		m_opcodes[0xF9] = [=]() { ADD8(m_reg.ab.B, GetMemExtendedByte(), GetFlag(FLAG_C)); }; // ADC B ext
		m_opcodes[0xFA] = [=]() { OR(m_reg.ab.B, GetMemExtendedByte()); }; // ORA B ext
		m_opcodes[0xFB] = [=]() { ADD8(m_reg.ab.B, GetMemExtendedByte()); }; // ADD B ext
		m_opcodes[0xFE] = [=]() { LD16(m_reg.X, GetMemExtendedWord()); }; // LDX ext
		m_opcodes[0xFF] = [=]() { ST16(GetExtended(), m_reg.X); }; // STX ext
	}

	CPU6800::~CPU6800()
	{
	}

	void CPU6800::Reset()
	{
		CPU::Reset();

		// Read reset vector
		ADDRESS resetVector = MemRead16(ADDR_RESET);
		LogPrintf(LOG_INFO, "Reset vector: %04X", resetVector);
		m_programCounter = resetVector;

		ClearFlags(m_flags);
		SetFlag(FLAG_I, true);

		m_clearIntMask = false;
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
		m_flags = f;
	}

	BYTE CPU6800::FetchByte()
	{
		BYTE b = MemRead8(GetCurrentAddress());
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

	void CPU6800::MEMDirectOp(std::function<void(CPU6800*, BYTE&)> func)
	{
		const ADDRESS dest = GetDirect();
		BYTE value = MemRead8(dest);
		func(this, value);
		MemWrite8(dest, value);
	}

	void CPU6800::MEMIndexedOp(std::function<void(CPU6800*, BYTE&)> func)
	{
		const ADDRESS dest = GetIndexed();
		BYTE value = MemRead8(dest);
		func(this, value);
		MemWrite8(dest, value);
	}

	void CPU6800::MEMExtendedOp(std::function<void(CPU6800*, BYTE&)> func)
	{
		const ADDRESS dest = GetExtended();
		BYTE value = MemRead8(dest);
		func(this, value);
		MemWrite8(dest, value);
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
		LogPrintf(LOG_ERROR, "A = %02X", m_reg.ab.A);
		LogPrintf(LOG_ERROR, "B = %02X", m_reg.ab.B);
		LogPrintf(LOG_ERROR, "X = %02X", m_reg.X);
		LogPrintf(LOG_ERROR, "Flags --HINZVC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_flags));
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

		bool clearIntMask = m_clearIntMask;

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

		if (clearIntMask)
		{
			m_clearIntMask = false;
			SetFlag(FLAG_I, false);
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
			m_programCounter = MemRead16(vector);
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
		pushW(m_reg.X);
		pushB(m_reg.ab.A);
		pushB(m_reg.ab.B);
		pushB(m_flags);
	}

	void CPU6800::popAll()
	{
		SetFlags(popB());
		m_reg.ab.B = popB();
		m_reg.ab.A = popB();
		m_reg.X = popW();
		m_programCounter = popW();
	}

	void CPU6800::pushB(BYTE value)
	{
		MemWrite8(--m_reg.SP, value);
	}

	void CPU6800::pushW(WORD value)
	{
		pushB(GetLByte(value));
		pushB(GetHByte(value));
	}

	BYTE CPU6800::popB()
	{
		return MemRead8(m_reg.SP++);
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

	void CPU6800::BSR()
	{
		SBYTE rel = FetchSignedByte();

		pushW(m_PC);

		m_PC += rel;
	}

	void CPU6800::RTS()
	{
		m_programCounter = popW();
	}

	void CPU6800::RTI()
	{
		popAll();
	}

	void CPU6800::WAI()
	{
		pushAll();
		Halt();
	}

	void CPU6800::SWI()
	{
		pushAll();
		SetFlag(FLAG_I, true);   // Disable IRQ
		m_programCounter = MemRead16(ADDR_SWI);
	}

	void CPU6800::LD8(BYTE& dest, BYTE src)
	{
		dest = src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	void CPU6800::LD16(WORD& dest, WORD src)
	{
		dest = src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	void CPU6800::ST8(ADDRESS dest, BYTE src)
	{
		MemWrite8(dest, src);

		AdjustNZ(src);
		SetFlag(FLAG_V, false);
	}

	void CPU6800::ST16(ADDRESS dest, WORD src)
	{
		MemWrite16(dest, src);

		AdjustNZ(src);
		SetFlag(FLAG_V, false);
	}

	void CPU6800::COM(BYTE& dest)
	{
		dest = ~dest;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
		SetFlag(FLAG_C, true);
	}

	void CPU6800::CLR(BYTE& dest)
	{
		dest = 0;
		SetFlag(FLAG_C, false);
		SetFlag(FLAG_V, false);
		AdjustNZ(dest);
	}

	void CPU6800::ASL(BYTE& dest)
	{
		bool carry = GetMSB(dest);
		dest <<= 1;

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, GetMSB(dest) ^ carry);
		AdjustNZ(dest);
	}

	void CPU6800::ASR(BYTE& dest)
	{
		bool sign = GetMSB(dest);
		bool carry = GetLSB(dest);
		dest >>= 1;
		SetBit(dest, 7, sign);

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, GetMSB(dest) ^ carry);
		AdjustNZ(dest);
	}

	void CPU6800::LSR(BYTE& dest)
	{
		bool carry = GetLSB(dest);
		dest >>= 1;

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, GetMSB(dest) ^ carry);
		AdjustNZ(dest);
	}

	void CPU6800::ROL(BYTE& dest)
	{
		bool oldCarry = GetFlag(FLAG_C);
		bool carry = GetMSB(dest);
		dest <<= 1;
		SetBit(dest, 0, oldCarry);

		SetFlag(FLAG_C, carry);
		SetFlag(FLAG_V, GetMSB(dest) ^ carry);
		AdjustNZ(dest);
	}

	void CPU6800::ROR(BYTE& dest)
	{
		bool oldCarry = GetFlag(FLAG_C);
		bool carry = GetLSB(dest);
		dest >>= 1;
		SetFlag(FLAG_C, carry);
		SetBit(dest, 7, oldCarry);
		SetFlag(FLAG_V, GetMSB(dest) ^ carry);

		AdjustNZ(dest);
	}

	void CPU6800::EOR(BYTE& dest, BYTE src)
	{
		dest ^= src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}
	void CPU6800::OR(BYTE& dest, BYTE src)
	{
		dest |= src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}
	void CPU6800::AND(BYTE& dest, BYTE src)
	{
		dest &= src;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	void CPU6800::INC(BYTE& dest)
	{
		SetFlag(FLAG_V, dest == 0b01111111);
		++dest;
		AdjustNZ(dest);
	}

	void CPU6800::DEC(BYTE& dest)
	{
		SetFlag(FLAG_V, dest == 0b10000000);
		--dest;
		AdjustNZ(dest);
	}

	void CPU6800::TST(const BYTE dest)
	{
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	// Get 'MSB' of low nibble (bit 3)
	inline bool GetHMSB(BYTE nib) { return GetBit(nib, 3); }

	void CPU6800::ADD8(BYTE& dest, BYTE src, bool carry)
	{
		BYTE res = dest + src + carry;

		AdjustNZ(res);
		SetFlag(FLAG_C, (GetMSB(dest) && GetMSB(src)) || (!GetMSB(res) && (GetMSB(src) || GetMSB(dest))));
		SetFlag(FLAG_H, (GetHMSB(dest) && GetHMSB(src)) || (!GetHMSB(res) && (GetHMSB(src) || GetHMSB(dest))));
		SetFlag(FLAG_V, (GetMSB(dest) == GetMSB(src)) && (GetMSB(res) != GetMSB(src)));

		dest = res;
	}

	void CPU6800::ADD16(WORD& dest, WORD src, bool carry)
	{
		WORD res = dest + src + carry;

		AdjustNZ(res);
		SetFlag(FLAG_C, (GetMSB(dest) && GetMSB(src)) || (!GetMSB(res) && (GetMSB(src) || GetMSB(dest))));
		SetFlag(FLAG_V, (GetMSB(dest) == GetMSB(src)) && (GetMSB(res) != GetMSB(src)));

		dest = res;
	}

	void CPU6800::SUB8(BYTE& dest, BYTE src, bool borrow)
	{
		BYTE res = dest - src - borrow;

		AdjustNZ(res);
		SetFlag(FLAG_C, (GetMSB(res) && GetMSB(src)) || (!GetMSB(dest) && (GetMSB(src) || GetMSB(res))));
		SetFlag(FLAG_V, (GetMSB(dest) != GetMSB(src)) && (GetMSB(res) == GetMSB(src)));

		dest = res;
	}

	void CPU6800::SUB16(WORD& dest, WORD src, bool borrow)
	{
		WORD res = dest - src - borrow;

		AdjustNZ(res);

		if (m_sub16SetCarry)
		{
			SetFlag(FLAG_C, (GetMSB(res) && GetMSB(src)) || (!GetMSB(dest) && (GetMSB(src) || GetMSB(res))));
		}
		SetFlag(FLAG_V, (GetMSB(dest) != GetMSB(src)) && (GetMSB(res) == GetMSB(src)));

		dest = res;
	}

	void CPU6800::NEG(BYTE& dest)
	{
		BYTE tempDest = 0;
		SUB8(tempDest, dest);
		dest = tempDest;
	}

	void CPU6800::Serialize(json& to)
	{
		to["opcode"] = m_opcode;

		m_nmi.Serialize(to["nmi"]);
		to["irq"] = m_irq;

		to["a"] = m_reg.ab.A;
		to["b"] = m_reg.ab.B;
		to["ix"] = m_reg.X;
		to["sp"] = m_reg.SP;
		to["pc"] = m_programCounter;
		to["flags"] = m_flags;
	}
	void CPU6800::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];

		m_nmi.Deserialize(from["nmi"]);
		m_irq = from["irq"];

		m_reg.ab.A = from["a"];
		m_reg.ab.B = from["b"];
		m_reg.X = from["ix"];
		m_reg.SP = from["sp"];
		m_programCounter = from["pc"];
		m_flags = from["flags"];
	}
}
