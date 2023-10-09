#include "stdafx.h"
#include "CPU6809.h"

using cpuInfo::Opcode;

namespace emul
{
	CPU6809::CPU6809(Memory& memory) : CPU6800(CPUID_6809, memory), Logger(CPUID_6809)
	{
		FLAG_RESERVED_ON = (FLAG)0;
		m_sub16SetCarry = true;
	}

	void CPU6809::Init()
	{
		CPU6800::Init();

		m_opcodes[0x00] = [=]() { MEMDirectOp(&CPU6809::NEG); }; // NEG direct
		m_opcodes[0x01] = [=]() { MEMDirectOp(&CPU6809::NEG); }; // NEG direct (undocumented)
		m_opcodes[0x02] = [=]() { MEMDirectOp(&CPU6809::XNC); }; // XNC direct (undocumented)
		m_opcodes[0x03] = [=]() { MEMDirectOp(&CPU6809::COM); }; // COM direct
		m_opcodes[0x04] = [=]() { MEMDirectOp(&CPU6809::LSR); }; // LSR direct
		m_opcodes[0x05] = [=]() { MEMDirectOp(&CPU6809::LSR); }; // LSR direct (undocumented)
		m_opcodes[0x06] = [=]() { MEMDirectOp(&CPU6809::ROR); }; // ROR direct
		m_opcodes[0x07] = [=]() { MEMDirectOp(&CPU6809::ASR); }; // ASR direct
		m_opcodes[0x08] = [=]() { MEMDirectOp(&CPU6809::ASL); }; // ASL direct
		m_opcodes[0x09] = [=]() { MEMDirectOp(&CPU6809::ROL); }; // ROL direct
		m_opcodes[0x0A] = [=]() { MEMDirectOp(&CPU6809::DEC); }; // DEC direct
		m_opcodes[0x0B] = [=]() { MEMDirectOp(&CPU6809::XDEC); }; // XDEC direct (undocumented)
		m_opcodes[0x0C] = [=]() { MEMDirectOp(&CPU6809::INC); }; // INC direct
		m_opcodes[0x0D] = [=]() { TST(GetMemDirectByte()); }; // TST direct
		m_opcodes[0x0E] = [=]() { JMP(GetDirect()); }; // JMP direct
		m_opcodes[0x0F] = [=]() { MEMDirectOp(&CPU6809::CLR); }; // CLR direct

		m_opcodes[0x10] = [=]() { ExecPage2(FetchByte()); }; // Page 2 sub intructions
		m_opcodes[0x11] = [=]() { ExecPage3(FetchByte()); }; // Page 3 sub intructions
		m_opcodes[0x12] = [=]() { }; // NOP
		m_opcodes[0x16] = [=]() { LBRA(true); }; // LBRA
		m_opcodes[0x17] = [=]() { LBSR(); }; // LBSR
		m_opcodes[0x1A] = [=]() { m_flags |= FetchByte(); }; // ORCC imm
		m_opcodes[0x1B] = [=]() {}; // NOP (undocumented)
		m_opcodes[0x1C] = [=]() { m_flags &= FetchByte(); }; // ANDCC imm
		m_opcodes[0x1D] = [=]() { SEX(); }; // SEX
		m_opcodes[0x1E] = [=]() { EXG(FetchByte()); }; // EXG r0,r1
		m_opcodes[0x1F] = [=]() { TFR(FetchByte()); }; // TFR r0,r1

		m_opcodes[0x21] = [=]() { BRA(false); }; // BRN

		m_opcodes[0x30] = [=]() { LEA(m_reg.X, true); }; // LEAX
		m_opcodes[0x31] = [=]() { LEA(m_reg6809.Y, true); }; // LEAY
		m_opcodes[0x32] = [=]() { LEA(m_reg.SP, false); m_nmiEnabled = true; }; // LEAS
		m_opcodes[0x33] = [=]() { LEA(m_reg6809.USP, false); }; // LEAU
		m_opcodes[0x34] = [=]() { PSH(STACK::SSP, FetchByte()); }; // PSHS
		m_opcodes[0x35] = [=]() { PUL(STACK::SSP, FetchByte()); }; // PULS
		m_opcodes[0x36] = [=]() { PSH(STACK::USP, FetchByte()); }; // PSHU
		m_opcodes[0x37] = [=]() { PUL(STACK::USP, FetchByte()); }; // PULU
		m_opcodes[0x38] = [=]() { m_flags &= FetchByte(); }; // ANDCC imm (undocumented)
		m_opcodes[0x39] = [=]() { RTS(); }; // RTS
		m_opcodes[0x3A] = [=]() { m_reg.X += m_reg.ab.B; }; // ABX
		m_opcodes[0x3B] = [=]() { RTI(); }; // RTI
		m_opcodes[0x3D] = [=]() { MUL(); }; // MUL
		m_opcodes[0x3E] = [=]() { XRES(); }; // XRES (undocumented)
		m_opcodes[0x3F] = [=]() { SWI(1); }; // SWI

		m_opcodes[0x41] = [=]() { NEG(m_reg.ab.A); }; // NEGA (undocumented)
		m_opcodes[0x42] = [=]() { XNC(m_reg.ab.A); }; // XNCA (undocumented)
		m_opcodes[0x45] = [=]() { LSR(m_reg.ab.A); }; // LSRA (undocumented)
		m_opcodes[0x4B] = [=]() { XDEC(m_reg.ab.A); }; // XDECA (undocumented)
		m_opcodes[0x4E] = [=]() { XCLR(m_reg.ab.A); }; // XCLRA (undocumented)

		m_opcodes[0x51] = [=]() { NEG(m_reg.ab.B); }; // NEGA (undocumented)
		m_opcodes[0x52] = [=]() { XNC(m_reg.ab.B); }; // XNCB (undocumented)
		m_opcodes[0x55] = [=]() { LSR(m_reg.ab.B); }; // LSRB (undocumented)
		m_opcodes[0x5B] = [=]() { XDEC(m_reg.ab.B); }; // XDECB (undocumented)
		m_opcodes[0x5E] = [=]() { XCLR(m_reg.ab.B); }; // XCLRB (undocumented)

		m_opcodes[0x61] = [=]() { MEMIndexedOp(&CPU6809::NEG); }; // NEG indexed (undocumented)
		m_opcodes[0x62] = [=]() { MEMIndexedOp(&CPU6809::XNC); }; // XNC indexed (undocumented)
		m_opcodes[0x65] = [=]() { MEMIndexedOp(&CPU6809::LSR); }; // LSR indexed (undocumented)
		m_opcodes[0x6B] = [=]() { MEMIndexedOp(&CPU6809::XDEC); }; // XDEC indexed (undocumented)
		m_opcodes[0x6D] = [=]() { TST(GetMemIndexedByte()); }; // TST indirect
		m_opcodes[0x6E] = [=]() { JMP(GetIndexed()); }; // JMP indexed

		m_opcodes[0x71] = [=]() { MEMExtendedOp(&CPU6809::NEG); }; // NEG extended (undocumented)
		m_opcodes[0x72] = [=]() { MEMExtendedOp(&CPU6809::XNC); }; // NEG extended (undocumented)
		m_opcodes[0x7B] = [=]() { MEMExtendedOp(&CPU6809::XDEC); }; // XDEC extended (undocumented)

		m_opcodes[0x83] = [=]() { SUB16(m_reg.D, FetchWord()); }; // SUBD imm
		m_opcodes[0x8E] = [=]() { LD16(m_reg.X, FetchWord()); }; // LDX imm

		m_opcodes[0x93] = [=]() { SUB16(m_reg.D, GetMemDirectWord()); }; // SUBD direct
		m_opcodes[0x9D] = [=]() { JSR(GetDirect()); }; // JSR direct
		m_opcodes[0x9E] = [=]() { LD16(m_reg.X, GetMemDirectWord()); }; // LDX direct
		m_opcodes[0x9F] = [=]() { ST16(GetDirect(),m_reg.X); }; // STU direct

		m_opcodes[0xA3] = [=]() { SUB16(m_reg.D, GetMemIndexedWord()); }; // SUBD indexed
		m_opcodes[0xAE] = [=]() { LD16(m_reg.X, GetMemIndexedWord()); }; // LDX indexed
		m_opcodes[0xAF] = [=]() { ST16(GetIndexed(), m_reg.X); }; // STU indexed

		m_opcodes[0xB3] = [=]() { SUB16(m_reg.D, GetMemExtendedWord()); }; // SUBD extended
		m_opcodes[0xBE] = [=]() { LD16(m_reg.X, GetMemExtendedWord()); }; // LDX extended
		m_opcodes[0xBF] = [=]() { ST16(GetExtended(), m_reg.X); }; // STU extended

		m_opcodes[0xC3] = [=]() { ADD16(m_reg.D, FetchWord()); }; // ADDD imm
		m_opcodes[0xCC] = [=]() { LD16(m_reg.D, FetchWord()); }; // LDD imm
		m_opcodes[0xCE] = [=]() { LD16(m_reg6809.USP, FetchWord()); }; // LDU imm

		m_opcodes[0xD3] = [=]() { ADD16(m_reg.D, GetMemDirectWord()); }; // ADDD direct
		m_opcodes[0xDC] = [=]() { LD16(m_reg.D, GetMemDirectWord()); }; // LDD direct
		m_opcodes[0xDD] = [=]() { ST16(GetDirect(), m_reg.D); }; // STD direct
		m_opcodes[0xDE] = [=]() { LD16(m_reg6809.USP, GetMemDirectWord()); }; // LDU direct
		m_opcodes[0xDF] = [=]() { ST16(GetDirect(), m_reg6809.USP); }; // STU direct

		m_opcodes[0xE3] = [=]() { ADD16(m_reg.D, GetMemIndexedWord()); }; // ADDD indexed
		m_opcodes[0xEC] = [=]() { LD16(m_reg.D, GetMemIndexedWord()); }; // LDD indexed
		m_opcodes[0xED] = [=]() { ST16(GetIndexed(), m_reg.D); }; // STD indexed
		m_opcodes[0xEE] = [=]() { LD16(m_reg6809.USP, GetMemIndexedWord()); }; // LDU indexed
		m_opcodes[0xEF] = [=]() { ST16(GetIndexed(), m_reg6809.USP); }; // STU indexed

		m_opcodes[0xF3] = [=]() { ADD16(m_reg.D, GetMemExtendedWord()); }; // ADDD extended
		m_opcodes[0xFC] = [=]() { LD16(m_reg.D, GetMemExtendedWord()); }; // LDD extended
		m_opcodes[0xFD] = [=]() { ST16(GetExtended(), m_reg.D); }; // STD extended
		m_opcodes[0xFE] = [=]() { LD16(m_reg6809.USP, GetMemExtendedWord()); }; // LDU extended
		m_opcodes[0xFF] = [=]() { ST16(GetExtended(), m_reg6809.USP); }; // STU extended

		InitPage2(); // Prefix 0x10
		InitPage3(); // Prefix 0x11
	}

	// Prefix 0x10
	void CPU6809::InitPage2()
	{
		// Start with main opcode table for fallthrough
		m_opcodesPage2 = m_opcodes;

		// Override with actual page 2 opcodes

		m_opcodesPage2[0x20] = [=]() { LBRA(true); }; // LBRA (undocumented)
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
		m_opcodesPage2[0x8C] = [=]() { CMP16(m_reg6809.Y, FetchWord()); }; // CMPY imm
		m_opcodesPage2[0x8E] = [=]() { LD16(m_reg6809.Y, FetchWord()); }; // LDY imm

		m_opcodesPage2[0x93] = [=]() { CMP16(m_reg.D, GetMemDirectWord()); }; // CMPD direct
		m_opcodesPage2[0x9C] = [=]() { CMP16(m_reg6809.Y, GetMemDirectWord()); }; // CMPY direct
		m_opcodesPage2[0x9E] = [=]() { LD16(m_reg6809.Y, GetMemDirectWord()); }; // LDY direct
		m_opcodesPage2[0x9F] = [=]() { ST16(GetDirect(), m_reg6809.Y); }; // STY direct

		m_opcodesPage2[0xA3] = [=]() { CMP16(m_reg.D, GetMemIndexedWord()); }; // CMPD indexed
		m_opcodesPage2[0xAC] = [=]() { CMP16(m_reg6809.Y, GetMemIndexedWord()); }; // CMPY indexed
		m_opcodesPage2[0xAE] = [=]() { LD16(m_reg6809.Y, GetMemIndexedWord()); }; // LDY indexed
		m_opcodesPage2[0xAF] = [=]() { ST16(GetIndexed(), m_reg6809.Y); }; // STY indexed

		m_opcodesPage2[0xB3] = [=]() { CMP16(m_reg.D, GetMemExtendedWord()); }; // CMPD extended
		m_opcodesPage2[0xBC] = [=]() { CMP16(m_reg6809.Y, GetMemExtendedWord()); }; // CMPY extended
		m_opcodesPage2[0xBE] = [=]() { LD16(m_reg6809.Y, GetMemExtendedWord()); }; // LDY extended
		m_opcodesPage2[0xBF] = [=]() { ST16(GetExtended(), m_reg6809.Y); }; // STY extended

		m_opcodesPage2[0xCE] = [=]() { LD16(m_reg.SP, FetchWord()); m_nmiEnabled = true; }; // LDS imm
		m_opcodesPage2[0xDE] = [=]() { LD16(m_reg.SP, GetMemDirectWord()); m_nmiEnabled = true; }; // LDS direct
		m_opcodesPage2[0xDF] = [=]() { ST16(GetDirect(), m_reg.SP); }; // STS direct

		m_opcodesPage2[0xEE] = [=]() { LD16(m_reg.SP, GetMemIndexedWord()); m_nmiEnabled = true; }; // LDS indexed
		m_opcodesPage2[0xEF] = [=]() { ST16(GetIndexed(), m_reg.SP); }; // STS indexed

		m_opcodesPage2[0xFE] = [=]() { LD16(m_reg.SP, GetMemExtendedWord()); m_nmiEnabled = true; }; // LDS extended
		m_opcodesPage2[0xFF] = [=]() { ST16(GetExtended(), m_reg.SP); }; // STS extended
	}

	// Prefix 0x11
	void CPU6809::InitPage3()
	{
		// Start with main opcode table for fallthrough
		m_opcodesPage3 = m_opcodes;

		// Override actual page 3 opcodes

		m_opcodesPage3[0x3F] = [=]() { SWI(3); };

		m_opcodesPage3[0x83] = [=]() { CMP16(m_reg6809.USP, FetchWord()); }; // CMPU imm
		m_opcodesPage3[0x8C] = [=]() { CMP16(m_reg.SP, FetchWord()); }; // CMPS imm

		m_opcodesPage3[0x93] = [=]() { CMP16(m_reg6809.USP, GetMemDirectWord()); }; // CMPU direct
		m_opcodesPage3[0x9C] = [=]() { CMP16(m_reg.SP, GetMemDirectWord()); }; // CMPS direct

		m_opcodesPage3[0xA3] = [=]() { CMP16(m_reg6809.USP, GetMemIndexedWord()); }; // CMPU indexed
		m_opcodesPage3[0xAC] = [=]() { CMP16(m_reg.SP, GetMemIndexedWord()); }; // CMPS indexed

		m_opcodesPage3[0xB3] = [=]() { CMP16(m_reg6809.USP, GetMemExtendedWord()); }; // CMPU extended
		m_opcodesPage3[0xBC] = [=]() { CMP16(m_reg.SP, GetMemExtendedWord()); }; // CMPS extended
	}

	void CPU6809::Reset()
	{
		CPU6800::Reset();

		m_firq = false;
	}

	void CPU6809::MEMDirectOp(std::function<void(CPU6809*, BYTE&)> func)
	{
		const ADDRESS dest = GetDirect();
		BYTE value = MemRead8(dest);
		func(this, value);
		MemWrite8(dest, value);
	}

	void CPU6809::MEMIndexedOp(std::function<void(CPU6809*, BYTE&)> func)
	{
		const ADDRESS dest = GetIndexed();
		BYTE value = MemRead8(dest);
		func(this, value);
		MemWrite8(dest, value);
	}

	void CPU6809::MEMExtendedOp(std::function<void(CPU6809*, BYTE&)> func)
	{
		const ADDRESS dest = GetExtended();
		BYTE value = MemRead8(dest);
		func(this, value);
		MemWrite8(dest, value);
	}

	WORD& CPU6809::GetIndexedRegister(BYTE idx)
	{
		switch ((idx >> 5) & 3)
		{
		case 0: return m_reg.X;
		case 1: return m_reg6809.Y;
		case 2: return m_reg6809.USP;
		case 3: return m_reg.SP;
		default:
			throw std::exception("not possible");
		}
	}

	// Get effective address for indexed mode
	ADDRESS CPU6809::GetIndexed()
	{
		BYTE idx = FetchByte();
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
			TICK1();
		}
		else switch (idx & 0b1111)
		{
		// Constant Offset from R
		case 0b0100: ea = reg; break; // ,R + 0 Offset
		case 0b1000: ea = reg + FetchSignedByte(); TICK1(); break; // ,R + 8 bit offset
		case 0b1001: ea = reg + FetchSignedWord(); TICKn(3); break; // ,R + 16 bit offset

		// Accumulator Offset from R
		case 0b0101: ea = reg + (SBYTE)m_reg.ab.B; TICK1(); break; // ,R + B Offset
		case 0b0110: ea = reg + (SBYTE)m_reg.ab.A; TICK1(); break; // ,R + A Offset
		case 0b1011: ea = reg + (SWORD)m_reg.D; TICKn(4); break; // ,R + D offset

		// Auto Increment/Decrement of R
		case 0b0000: ea = reg++; TICKn(2); break; // ,R+
		case 0b0001: ea = reg; reg += 2; TICKn(3); break; // ,R++
		case 0b0010: ea = --reg; break; TICKn(2); // ,-R
		case 0b0011: reg -= 2; ea = reg; TICKn(3); break; // ,--R

		// Constant Offset from PC
		case 0b1100: ea = m_programCounter + FetchSignedByte(); TICK1(); break; // ,PC + 8 bit offset
		case 0b1101: ea = m_programCounter + FetchSignedWord(); TICKn(5); break; // ,PC + 16 bit offset

		// Extended indirect
		case 0b1111: ea = FetchWord(); TICKn(2); break;// [,Address]

		// 0b0111: n/a
		// 0b1010: n/a
		// 0b1110: n/a

		default:
			LogPrintf(LOG_ERROR, "Invalid addressing mode (%02X)", idx);
			throw std::exception("Invalid addressing mode");
		}

		// Cleanup ea
		ea &= 0xFFFF;

		if (indirect)
		{
			// TODO: 'illegal' modes
			TICKn(3);
			return MemRead16(ea);
		}

		return ea;
	}

	void CPU6809::Dump()
	{
		LogPrintf(LOG_ERROR, "AB = %04X", m_reg.D);
		LogPrintf(LOG_ERROR, "X  = %04X", m_reg.X);
		LogPrintf(LOG_ERROR, "Y  = %04X", m_reg6809.Y);
		LogPrintf(LOG_ERROR, "Flags EFHINZVC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_flags));
		LogPrintf(LOG_ERROR, "S  = %04X", m_reg.SP);
		LogPrintf(LOG_ERROR, "U  = %04X", m_reg6809.USP);
		LogPrintf(LOG_ERROR, "DP = %02X", m_reg6809.DP);
		LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
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
		if (m_nmiEnabled && m_nmi.IsLatched())
		{
			LogPrintf(LOG_INFO, "[%zu] NMI", emul::g_ticks);

			SetFlag(FLAG_E, true);   // Complete state saved
			PSH(STACK::SSP, REGS_ALL); // Save all registers
			SetFlag(FLAG_F, true);   // Disable FIRQ
			SetFlag(FLAG_I, true);   // Disable IRQ

			m_PC = MemRead16(ADDR_NMI);
			m_nmi.ResetLatch();
		}
		else if (m_firq && !GetFlag(FLAG_F))
		{
			LogPrintf(LOG_INFO, "[%zu] FIRQ", emul::g_ticks);

			PSH(STACK::SSP, REGS_PC); // Save program counter
			SetFlag(FLAG_E, false); // Incomplete state saved
			PSH(STACK::SSP, REGS_CC); // Save flags
			SetFlag(FLAG_F, true);  // Disable FIRQ
			SetFlag(FLAG_I, true);  // Disable IRQ

			m_PC = MemRead16(ADDR_FIRQ);
		}
		else if (m_irq && !GetFlag(FLAG_I))
		{
			LogPrintf(LOG_INFO, "[%zu] IRQ", emul::g_ticks);

			SetFlag(FLAG_E, true);   // Complete state saved
			PSH(STACK::SSP, REGS_ALL); // Save all registers
			SetFlag(FLAG_I, true);   // Disable IRQ

			m_PC = MemRead16(ADDR_IRQ);
		}
	}

	void CPU6809::push(BYTE value)
	{
		TICK1();
		WORD& sp = *m_currStack;

		MemWrite8(--sp, value);
	}

	BYTE CPU6809::pop()
	{
		TICK1();
		WORD& sp = *m_currStack;

		return MemRead8(sp++);
	}

	void CPU6809::PSH(STACK s, BYTE regs)
	{
		SetStack(s);

		if (GetBit(regs, 7))
		{
			push(GetLByte(m_PC));
			push(GetHByte(m_PC));
		}
		if (GetBit(regs, 6))
		{
			// Select the 'other' stack
			const WORD& sp = (s == STACK::SSP) ? m_reg6809.USP : m_reg.SP;

			push(GetLByte(sp));
			push(GetHByte(sp));
		}
		if (GetBit(regs, 5))
		{
			push(GetLByte(m_reg6809.Y));
			push(GetHByte(m_reg6809.Y));
		}
		if (GetBit(regs, 4))
		{
			push(GetLByte(m_reg.X));
			push(GetHByte(m_reg.X));
		}
		if (GetBit(regs, 3))
		{
			push(m_reg6809.DP);
		}
		if (GetBit(regs, 2))
		{
			push(m_reg.ab.B);
		}
		if (GetBit(regs, 1))
		{
			push(m_reg.ab.A);
		}
		if (GetBit(regs, 0))
		{
			push(m_flags);
		}
	}
	void CPU6809::PUL(STACK s, BYTE regs)
	{
		SetStack(s);

		if (GetBit(regs, 0))
		{
			m_flags = pop();
		}
		if (GetBit(regs, 1))
		{
			m_reg.ab.A = pop();
		}
		if (GetBit(regs, 2))
		{
			m_reg.ab.B = pop();
		}
		if (GetBit(regs, 3))
		{
			m_reg6809.DP = pop();
		}
		if (GetBit(regs, 4))
		{
			SetHByte(m_reg.X, pop());
			SetLByte(m_reg.X, pop());
		}
		if (GetBit(regs, 5))
		{
			SetHByte(m_reg6809.Y, pop());
			SetLByte(m_reg6809.Y, pop());
		}
		if (GetBit(regs, 6))
		{
			// Select the 'other' stack
			WORD& sp = (s == STACK::SSP) ? m_reg6809.USP : m_reg.SP;

			SetHByte(sp, pop());
			SetLByte(sp, pop());
		}
		if (GetBit(regs, 7))
		{
			SetHByte(m_PC, pop());
			SetLByte(m_PC, pop());
		}
	}

	// Branching
	void CPU6809::LBRA(bool condition)
	{
		SWORD rel = FetchSignedWord();
		if (condition == true)
		{
			m_PC += rel;
			TICKT3();
		}
	}

	void CPU6809::JSR(ADDRESS dest)
	{
		PSH(STACK::SSP, REGS_PC);
		m_programCounter = dest;
	}

	void CPU6809::BSR()
	{
		SBYTE rel = FetchSignedByte();

		PSH(STACK::SSP, REGS_PC);

		m_PC += rel;
	}

	void CPU6809::LBSR()
	{
		SWORD rel = FetchSignedWord();

		PSH(STACK::SSP, REGS_PC);

		m_PC += rel;
	}

	void CPU6809::RTS()
	{
		PUL(STACK::SSP, REGS_PC);
	}

	void CPU6809::RTI()
	{
		// Depending on E flag, pop either all registers or just CC + PC
		PUL(STACK::SSP, GetFlag(FLAG_E) ? REGS_ALL : REGS_RTI);
	}

	void CPU6809::SWI(BYTE swi)
	{
		SetFlag(FLAG_E, true);

		// Push all registers on the system stack
		PSH(STACK::SSP, REGS_ALL);

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

		m_PC = MemRead16(vect);
	}

	void CPU6809::XRES()
	{
		// Push all registers on the system stack
		PSH(STACK::SSP, REGS_ALL);

		m_PC = MemRead16(ADDR_SWI2);
	}

	void CPU6809::LEA(WORD& dest, bool setZero)
	{
		dest = GetIndexed();

		if (setZero)
		{
			SetFlag(FLAG_Z, dest == 0);
		}
	}

	WORD CPU6809::GetReg(RegCode code) const
	{
		switch (code)
		{
		case RegCode::D: return m_reg.D;
		case RegCode::X: return m_reg.X;
		case RegCode::Y: return m_reg6809.Y;
		case RegCode::U: return m_reg6809.USP;
		case RegCode::S: return m_reg.SP;
		case RegCode::PC: return m_programCounter;

		case RegCode::A: return MakeWord(0xFF, m_reg.ab.A);
		case RegCode::B: return MakeWord(0xFF, m_reg.ab.B);
		case RegCode::CC: return MakeWord(m_flags, m_flags);
		case RegCode::DP: return MakeWord(m_reg6809.DP, m_reg6809.DP);

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
		case RegCode::CC: return m_flags;
		case RegCode::DP: return m_reg6809.DP;

		default: return m_reg6809.void8;
		}
	}

	WORD& CPU6809::GetReg16(RegCode code)
	{
		assert(GetBit((BYTE)code, 3) == false);

		switch (code)
		{
		case RegCode::D:  return m_reg.D;
		case RegCode::X:  return m_reg.X;
		case RegCode::Y:  return m_reg6809.Y;
		case RegCode::U:  return m_reg6809.USP;
		case RegCode::S:  return m_reg.SP;
		case RegCode::PC: return m_PC;

		default: return m_reg6809.void16;
		}
	}

	void CPU6809::TFR(BYTE sd)
	{
		const WORD source = GetReg(GetSourceRegCode(sd));

		const RegCode destCode = GetDestRegCode(sd);

		if (isDestRegWide(sd))
		{
			WORD& dest = GetReg16(destCode);
			dest = source;

			if (IsStackRegister(dest))
			{
				m_nmiEnabled = true;
			}
		}
		else
		{
			GetReg8(destCode) = GetLByte(source);
		}
	}

	void CPU6809::EXG(BYTE sd)
	{
		const RegCode sourceCode = GetSourceRegCode(sd);
		const RegCode destCode = GetDestRegCode(sd);

		const WORD temp = GetReg(destCode);

		switch (GetRegsSize(sd))
		{
		case RegSize::BB: // 8 bit -> 8 bit
		{
			BYTE& source = GetReg8(sourceCode);
			BYTE& dest = GetReg8(destCode);

			dest = source;
			source = GetLByte(temp);
			break;
		}
		case RegSize::BW: // 8 bit -> 16 bit
			LogPrintf(LOG_ERROR, "EGX 8->16 not implemented");
			UnknownOpcode();
			break;
		case RegSize::WB: // 16 bit -> 8 bit
			LogPrintf(LOG_ERROR, "EGX 16->8 not implemented");
			UnknownOpcode();
			break;
		case RegSize::WW: // 16 bit -> 16 bit
		{
			WORD& source = GetReg16(sourceCode);
			WORD& dest = GetReg16(destCode);

			dest = source;
			source = temp;

			if (IsStackRegister(dest) || IsStackRegister(source))
			{
				m_nmiEnabled = true;
			}
			break;
		}
		default:
			throw std::exception("not possible");
		}
	}

	void CPU6809::XCLR(BYTE& dest)
	{
		dest = 0;
		AdjustNZ(dest);
		SetFlag(FLAG_V, false);
	}

	void CPU6809::XDEC(BYTE& dest)
	{
		SetFlag(FLAG_C, dest == 0);
		DEC(dest);
	}

	void CPU6809::SEX()
	{
		m_reg.ab.A = GetMSB(m_reg.ab.B) ? 0xFF : 0;
		AdjustNZ(m_reg.D);
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

		to["pc"] = m_programCounter;

		to["d"] = m_reg.D;
		to["s"] = m_reg.SP;
		to["u"] = m_reg6809.USP;
		to["x"] = m_reg.X;
		to["y"] = m_reg6809.Y;

		to["dp"] = m_reg6809.DP;
		to["flags"] = m_flags;
	}
	void CPU6809::Deserialize(const json& from)
	{
		m_opcode = from["opcode"];

		m_nmiEnabled = from["nmiEnabled"];
		m_nmi.Deserialize(from["nmi"]);
		m_firq = from["firq"];
		m_irq = from["irq"];

		m_programCounter = from["pc"];

		m_reg.D = from["d"];
		m_reg.SP = from["s"];
		m_reg6809.USP = from["u"];
		m_reg.X = from["x"];
		m_reg6809.Y = from["y"];

		m_reg6809.DP = from["dp"];
		m_flags = from["flags"];
	}
}
