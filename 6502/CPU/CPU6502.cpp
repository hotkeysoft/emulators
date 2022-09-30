#include "stdafx.h"
#include "CPU6502.h"

using cpuInfo::Opcode;

namespace emul
{
	CPU6502::CPU6502(Memory& memory) : CPU6502("6502", memory)
	{
	}


	CPU6502::CPU6502(const char* cpuid, Memory& memory) :
		CPU(CPU6502_ADDRESS_BITS, memory),
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

	void CPU6502::Init()
	{
		m_opcodes.resize(256);
		std::fill(m_opcodes.begin(), m_opcodes.end(), [=]() { UnknownOpcode(); });

		// ------------------------------
		// Logical and Arithmetic Commands
		// ------------------------------

		// ORA A:=A or {adr}, NZ
		m_opcodes[0x09] = [=]() { ORA(FetchByte()); }; // imm
		m_opcodes[0x05] = [=]() { MEMopR(&CPU6502::ORA, GetZP()); }; // zp
		m_opcodes[0x15] = [=]() { MEMopR(&CPU6502::ORA, GetZPX()); }; // zpx
		m_opcodes[0x01] = [=]() { MEMopR(&CPU6502::ORA, GetINDX()); }; // izx
		m_opcodes[0x11] = [=]() { MEMopR(&CPU6502::ORA, GetINDY()); }; // izy
		m_opcodes[0x0D] = [=]() { MEMopR(&CPU6502::ORA, GetABS()); }; // abs
		m_opcodes[0x1D] = [=]() { MEMopR(&CPU6502::ORA, GetABSX()); }; // abx
		m_opcodes[0x19] = [=]() { MEMopR(&CPU6502::ORA, GetABSY()); }; // aby

		// AND A:=A and {adr}, NZ
		m_opcodes[0x29] = [=]() { AND(FetchByte()); }; // imm
		m_opcodes[0x25] = [=]() { MEMopR(&CPU6502::AND, GetZP()); }; // zp
		m_opcodes[0x35] = [=]() { MEMopR(&CPU6502::AND, GetZPX()); }; // zpx
		m_opcodes[0x21] = [=]() { MEMopR(&CPU6502::AND, GetINDX()); }; // izx
		m_opcodes[0x31] = [=]() { MEMopR(&CPU6502::AND, GetINDY()); }; // izy
		m_opcodes[0x2D] = [=]() { MEMopR(&CPU6502::AND, GetABS()); }; // abs
		m_opcodes[0x3D] = [=]() { MEMopR(&CPU6502::AND, GetABSX()); }; // abx
		m_opcodes[0x39] = [=]() { MEMopR(&CPU6502::AND, GetABSY()); }; // aby

		// EOR A:=A xor {adr}, NZ
		m_opcodes[0x49] = [=]() { EOR(FetchByte()); }; // imm
		m_opcodes[0x45] = [=]() { MEMopR(&CPU6502::EOR, GetZP()); }; // zp
		m_opcodes[0x55] = [=]() { MEMopR(&CPU6502::EOR, GetZPX()); }; // zpx
		m_opcodes[0x41] = [=]() { MEMopR(&CPU6502::EOR, GetINDX()); }; // izx
		m_opcodes[0x51] = [=]() { MEMopR(&CPU6502::EOR, GetINDY()); }; // izy
		m_opcodes[0x4D] = [=]() { MEMopR(&CPU6502::EOR, GetABS()); }; // abs
		m_opcodes[0x5D] = [=]() { MEMopR(&CPU6502::EOR, GetABSX()); }; // abx
		m_opcodes[0x59] = [=]() { MEMopR(&CPU6502::EOR, GetABSY()); }; // aby

		// ADC A:=A + {adr}, NVZC
		m_opcodes[0x69] = [=]() { ADC(FetchByte()); }; // imm
		m_opcodes[0x65] = [=]() { MEMopR(&CPU6502::ADC, GetZP()); }; // zp
		m_opcodes[0x75] = [=]() { MEMopR(&CPU6502::ADC, GetZPX()); }; // zpx
		m_opcodes[0x61] = [=]() { MEMopR(&CPU6502::ADC, GetINDX()); }; // izx
		m_opcodes[0x71] = [=]() { MEMopR(&CPU6502::ADC, GetINDY()); }; // izy
		m_opcodes[0x6D] = [=]() { MEMopR(&CPU6502::ADC, GetABS()); }; // abs
		m_opcodes[0x7D] = [=]() { MEMopR(&CPU6502::ADC, GetABSX()); }; // abx
		m_opcodes[0x79] = [=]() { MEMopR(&CPU6502::ADC, GetABSY()); }; // aby

		// SBC A:=A - {adr}, NVZC
		m_opcodes[0xE9] = [=]() { SBC(FetchByte()); }; // imm
		m_opcodes[0xE5] = [=]() { MEMopR(&CPU6502::SBC, GetZP()); }; // zp
		m_opcodes[0xF5] = [=]() { MEMopR(&CPU6502::SBC, GetZPX()); }; // zpx
		m_opcodes[0xE1] = [=]() { MEMopR(&CPU6502::SBC, GetINDX()); }; // izx
		m_opcodes[0xF1] = [=]() { MEMopR(&CPU6502::SBC, GetINDY()); }; // izy
		m_opcodes[0xED] = [=]() { MEMopR(&CPU6502::SBC, GetABS()); }; // abs
		m_opcodes[0xFD] = [=]() { MEMopR(&CPU6502::SBC, GetABSX()); }; // abx
		m_opcodes[0xF9] = [=]() { MEMopR(&CPU6502::SBC, GetABSY()); }; // aby

		// CMP A - {adr}, NZC
		m_opcodes[0xC9] = [=]() { CMPA(FetchByte()); }; // imm
		m_opcodes[0xC5] = [=]() { MEMopR(&CPU6502::CMPA, GetZP()); }; // zp
		m_opcodes[0xD5] = [=]() { MEMopR(&CPU6502::CMPA, GetZPX()); }; // zpx
		m_opcodes[0xC1] = [=]() { MEMopR(&CPU6502::CMPA, GetINDX()); }; // izx
		m_opcodes[0xD1] = [=]() { MEMopR(&CPU6502::CMPA, GetINDY()); }; // izy
		m_opcodes[0xCD] = [=]() { MEMopR(&CPU6502::CMPA, GetABS()); }; // abs
		m_opcodes[0xDD] = [=]() { MEMopR(&CPU6502::CMPA, GetABSX()); }; // abx
		m_opcodes[0xD9] = [=]() { MEMopR(&CPU6502::CMPA, GetABSY()); }; // aby

		// CPX
		m_opcodes[0xE0] = [=]() { CMPX(FetchByte()); }; // imm
		m_opcodes[0xE4] = [=]() { MEMopR(&CPU6502::CMPX, GetZP()); }; // zp
		m_opcodes[0xEC] = [=]() { MEMopR(&CPU6502::CMPX, GetABS()); }; // abs

		// CPY
		m_opcodes[0xC0] = [=]() { CMPY(FetchByte()); }; // imm
		m_opcodes[0xC4] = [=]() { MEMopR(&CPU6502::CMPY, GetZP()); }; // zp
		m_opcodes[0xCC] = [=]() { MEMopR(&CPU6502::CMPY, GetABS()); }; // abs

		// DEC {adr}:={adr}-1, NZ
		m_opcodes[0xC6] = [=]() { MEMopRMW(&CPU6502::DEC, GetZP()); }; // zp
		m_opcodes[0xD6] = [=]() { MEMopRMW(&CPU6502::DEC, GetZPX()); }; // zpx
		m_opcodes[0xCE] = [=]() { MEMopRMW(&CPU6502::DEC, GetABS()); }; // abs
		m_opcodes[0xDE] = [=]() { MEMopRMW(&CPU6502::DEC, GetABSX()); }; // abx

		// DEX X:=X-1, NZ
		m_opcodes[0xCA] = [=]() { DEC(m_reg.X); }; // imp

		// DEY Y:=Y-1, NZ
		m_opcodes[0x88] = [=]() { DEC(m_reg.Y); }; // imp

		// INC {adr}:={adr}+1, NZ
		m_opcodes[0xE6] = [=]() { MEMopRMW(&CPU6502::INC, GetZP());  }; // zp
		m_opcodes[0xF6] = [=]() { MEMopRMW(&CPU6502::INC, GetZPX()); }; // zpx
		m_opcodes[0xEE] = [=]() { MEMopRMW(&CPU6502::INC, GetABS()); }; // abs
		m_opcodes[0xFE] = [=]() { MEMopRMW(&CPU6502::INC, GetABSX()); }; // abx

		// INX X:=X+1, NZ
		m_opcodes[0xE8] = [=]() { INC(m_reg.X); }; // imp

		// INY Y:=Y+1, NZ
		m_opcodes[0xC8] = [=]() { INC(m_reg.Y); }; // imp

		// ASL {adr}:={adr}*2, NZC
		m_opcodes[0x0A] = [=]() { ASL(m_reg.A); }; // imp
		m_opcodes[0x06] = [=]() { MEMopRMW(&CPU6502::ASL, GetZP()); }; // zp
		m_opcodes[0x16] = [=]() { MEMopRMW(&CPU6502::ASL, GetZPX()); }; // zpx
		m_opcodes[0x0E] = [=]() { MEMopRMW(&CPU6502::ASL, GetABS()); }; // abs
		m_opcodes[0x1E] = [=]() { MEMopRMW(&CPU6502::ASL, GetABSX()); }; // abx

		// ROL {adr}:={adr}*2+C, NZC
		m_opcodes[0x2A] = [=]() { ROL(m_reg.A); }; // imp
		m_opcodes[0x26] = [=]() { MEMopRMW(&CPU6502::ROL, GetZP()); }; // zp
		m_opcodes[0x36] = [=]() { MEMopRMW(&CPU6502::ROL, GetZPX()); }; // zpx
		m_opcodes[0x2E] = [=]() { MEMopRMW(&CPU6502::ROL, GetABS()); }; // abs
		m_opcodes[0x3E] = [=]() { MEMopRMW(&CPU6502::ROL, GetABSX()); }; // abx

		// LSR {adr}:={adr}/2, NZC
		m_opcodes[0x4A] = [=]() { LSR(m_reg.A); }; // imp
		m_opcodes[0x46] = [=]() { MEMopRMW(&CPU6502::LSR, GetZP()); }; // zp
		m_opcodes[0x56] = [=]() { MEMopRMW(&CPU6502::LSR, GetZPX()); }; // zpx
		m_opcodes[0x4E] = [=]() { MEMopRMW(&CPU6502::LSR, GetABS()); }; // abs
		m_opcodes[0x5E] = [=]() { MEMopRMW(&CPU6502::LSR, GetABSX()); }; // abx

		// ROR {adr}:={adr}/2+(C*128), NZC
		m_opcodes[0x6A] = [=]() { ROR(m_reg.A); }; // imp
		m_opcodes[0x66] = [=]() { MEMopRMW(&CPU6502::ROR, GetZP()); }; // zp
		m_opcodes[0x76] = [=]() { MEMopRMW(&CPU6502::ROR, GetZPX()); }; // zpx
		m_opcodes[0x6E] = [=]() { MEMopRMW(&CPU6502::ROR, GetABS()); }; // abs
		m_opcodes[0x7E] = [=]() { MEMopRMW(&CPU6502::ROR, GetABSX()); }; // abx

		// ------------------------------
		// Move Commands
		// ------------------------------

		// LDA A:={adr}, NZ
		m_opcodes[0xA9] = [=]() { LD(m_reg.A, FetchByte()); }; // imm
		m_opcodes[0xA5] = [=]() { LDmem(m_reg.A, GetZP()); }; // zp
		m_opcodes[0xB5] = [=]() { LDmem(m_reg.A, GetZPX()); }; // zpx
		m_opcodes[0xA1] = [=]() { LDmem(m_reg.A, GetINDX()); }; // izx
		m_opcodes[0xB1] = [=]() { LDmem(m_reg.A, GetINDY()); }; // izy
		m_opcodes[0xAD] = [=]() { LDmem(m_reg.A, GetABS()); }; // abs
		m_opcodes[0xBD] = [=]() { LDmem(m_reg.A, GetABSX()); }; // abx
		m_opcodes[0xB9] = [=]() { LDmem(m_reg.A, GetABSY()); }; // aby

		// STA {adr}:=A
		m_opcodes[0x85] = [=]() { STmem(GetZP(), m_reg.A); }; // zp
		m_opcodes[0x95] = [=]() { STmem(GetZPX(), m_reg.A); }; // zpx
		m_opcodes[0x81] = [=]() { STmem(GetINDX(), m_reg.A); }; // izx
		m_opcodes[0x91] = [=]() { STmem(GetINDY(), m_reg.A); }; // izy
		m_opcodes[0x8D] = [=]() { STmem(GetABS(), m_reg.A); }; // abs
		m_opcodes[0x9D] = [=]() { STmem(GetABSX(), m_reg.A); }; // abx
		m_opcodes[0x99] = [=]() { STmem(GetABSY(), m_reg.A); }; // aby

		// LDX X:={adr}, NZ
		m_opcodes[0xA2] = [=]() { LD(m_reg.X, FetchByte()); }; // imm
		m_opcodes[0xA6] = [=]() { LDmem(m_reg.X, GetZP()); }; // zp
		m_opcodes[0xB6] = [=]() { LDmem(m_reg.X, GetZPY()); }; // zpy
		m_opcodes[0xAE] = [=]() { LDmem(m_reg.X, GetABS()); }; // abs
		m_opcodes[0xBE] = [=]() { LDmem(m_reg.X, GetABSY()); }; // aby

		// STX {adr}:=X
		m_opcodes[0x86] = [=]() { STmem(GetZP(), m_reg.X); }; // zp
		m_opcodes[0x96] = [=]() { STmem(GetZPY(), m_reg.X); }; // zpy
		m_opcodes[0x8E] = [=]() { STmem(GetABS(), m_reg.X); }; // abs

		// LDY X:={adr}, NZ
		m_opcodes[0xA0] = [=]() { LD(m_reg.Y, FetchByte()); }; // imm
		m_opcodes[0xA4] = [=]() { LDmem(m_reg.Y, GetZP()); }; // zp
		m_opcodes[0xB4] = [=]() { LDmem(m_reg.Y, GetZPX()); }; // zpx
		m_opcodes[0xAC] = [=]() { LDmem(m_reg.Y, GetABS()); }; // abs
		m_opcodes[0xBC] = [=]() { LDmem(m_reg.Y, GetABSX()); }; // abx

		// STY {adr}:=X
		m_opcodes[0x84] = [=]() { STmem(GetZP(), m_reg.Y);  }; // zp
		m_opcodes[0x94] = [=]() { STmem(GetZPX(), m_reg.Y);  }; // zpx
		m_opcodes[0x8C] = [=]() { STmem(GetABS(), m_reg.Y); }; // abs

		// TAX X:=A, XZ
		m_opcodes[0xAA] = [=]() { LD(m_reg.X, m_reg.A); }; // imp

		// TXA A:=X, NZ
		m_opcodes[0x8A] = [=]() { LD(m_reg.A, m_reg.X); }; // imp

		// TAY Y:=A, NZ
		m_opcodes[0xA8] = [=]() { LD(m_reg.Y, m_reg.A); }; // imp

		// TYA A:=Y, NZ
		m_opcodes[0x98] = [=]() { LD(m_reg.A, m_reg.Y); }; // imp

		// TSX X:=S, NZ
		m_opcodes[0xBA] = [=]() { LD(m_reg.X, m_reg.SP); }; // imp

		// TXS S:=X
		m_opcodes[0x9A] = [=]() { m_reg.SP = m_reg.X; }; // imp

		// PLA A:=+(S), NZ
		m_opcodes[0x68] = [=]() { PLA(); }; // imp

		// PHA (S):=A
		m_opcodes[0x48] = [=]() { PHA(); }; // imp

		// PLP P:=+(S), NVDIZC
		m_opcodes[0x28] = [=]() { PLP(); }; // imp

		// PHP (S)-:=P
		m_opcodes[0x08] = [=]() { PHP(); }; // imp

		// ------------------------------
		// Jump Commands
		// ------------------------------

		m_opcodes[0x10] = [=]() { BRANCHif(GetFlag(FLAG_N) == false); }; // BPL
		m_opcodes[0x30] = [=]() { BRANCHif(GetFlag(FLAG_N) == true);  }; // BMI
		m_opcodes[0x50] = [=]() { BRANCHif(GetFlag(FLAG_V) == false); }; // BVC
		m_opcodes[0x70] = [=]() { BRANCHif(GetFlag(FLAG_V) == true);  }; // BVS
		m_opcodes[0x90] = [=]() { BRANCHif(GetFlag(FLAG_C) == false); }; // BCC
		m_opcodes[0xB0] = [=]() { BRANCHif(GetFlag(FLAG_C) == true);  }; // BCS
		m_opcodes[0xD0] = [=]() { BRANCHif(GetFlag(FLAG_Z) == false); }; // BNE
		m_opcodes[0xF0] = [=]() { BRANCHif(GetFlag(FLAG_Z) == true);  }; // BEQ

		// BRK (S)-:=PC,P PC:=($FFFE)
		m_opcodes[0x00] = [=]() { BRK(); };

		// RTI P,PC:=+(S)
		m_opcodes[0x40] = [=]() { RTI(); };

		// JSR (S)-:=PC, PC:={adr}
		m_opcodes[0x20] = [=]() { JSR(FetchWord()); };

		// RTS PC:=+(S)
		m_opcodes[0x60] = [=]() { RTS(); };

		// JMP PC:={adr}
		m_opcodes[0x4C] = [=]() { JMP(GetABS()); }; // abs
		m_opcodes[0x6C] = [=]() { JMP(GetIND()); }; // ind

		// BIT N:=b7, V:=b6, Z:=A&{adr}
		m_opcodes[0x24] = [=]() { BIT(GetZP()); }; // zp
		m_opcodes[0x2C] = [=]() { BIT(GetABS()); }; // abs

		// ------------------------------
		// Flag Commands
		// ------------------------------

		// CARRY
		m_opcodes[0x18] = [=]() { SetFlag(FLAG_C, false); }; // CLC
		m_opcodes[0x38] = [=]() { SetFlag(FLAG_C, true); };  // SEC

		// DECIMAL
		m_opcodes[0xD8] = [=]() { SetFlag(FLAG_D, false); }; // CLD
		m_opcodes[0xF8] = [=]() { SetFlag(FLAG_D, true); };  // SED

		// INTERRUPT
		m_opcodes[0x58] = [=]() { SetFlag(FLAG_I, false); }; // CLI
		m_opcodes[0x78] = [=]() { SetFlag(FLAG_I, true); };  // SEI

		// OVERFLOW
		m_opcodes[0xB8] = [=]() { SetFlag(FLAG_V, false); }; // CLV

		// NOP
		m_opcodes[0xEA] = [=]() { }; // NOP
	}

	CPU6502::~CPU6502()
	{
	}

	void CPU6502::Reset()
	{
		CPU::Reset();

		// TODO: Check if values are reset
		m_reg.A = 0;
		m_reg.X = 0;
		m_reg.Y = 0;
		m_reg.SP = 0;

		// Read reset vector
		ADDRESS resetVector = m_memory.Read16(ADDR_RESET);
		LogPrintf(LOG_INFO, "Reset vector: %04X", resetVector);
		m_programCounter = resetVector;

		ClearFlags(m_reg.flags);
	}

	void CPU6502::Reset(ADDRESS overrideAddress)
	{
		Reset();
		m_programCounter = overrideAddress;
	}

	void CPU6502::ClearFlags(BYTE& flags)
	{
		flags = FLAG_RESERVED_ON;
	}

	void CPU6502::SetFlags(BYTE f)
	{
		SetBitMask(f, FLAG_RESERVED_ON, true);
		m_reg.flags = f;
	}

	BYTE CPU6502::FetchByte()
	{
		BYTE b = m_memory.Read8(GetCurrentAddress());
		++m_programCounter;
		m_programCounter &= 0xFFFF;
		return b;
	}

	bool CPU6502::Step()
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

	void CPU6502::Dump()
	{
		LogPrintf(LOG_ERROR, "A = %02X", m_reg.A);
		LogPrintf(LOG_ERROR, "X = %02X", m_reg.X);
		LogPrintf(LOG_ERROR, "Y = %02X", m_reg.Y);
		LogPrintf(LOG_ERROR, "Flags NV-BDIZC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_reg.flags));
		LogPrintf(LOG_ERROR, "SP = %04X", m_reg.SP);
		LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
	}

	void CPU6502::UnknownOpcode()
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X", m_opcode, m_programCounter);
		Dump();
		throw std::exception("Unknown opcode");
	}

	void CPU6502::Exec(BYTE opcode)
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

	void CPU6502::Interrupt()
	{
		if (m_interruptsEnabled)
		{
		}

		// TODO: Not implemented
	}

	ADDRESS CPU6502::GetIND()
	{
		// Indirect mode doesn't handle page boundary crossing, low byte wraps around
		BYTE lInd = FetchByte();
		BYTE hInd = FetchByte();

		BYTE l = m_memory.Read8(MakeWord(hInd, lInd));
		BYTE h = m_memory.Read8(MakeWord(hInd, lInd + 1));

		return MakeWord(h, l);
	}

	// Address of data is ZP(n+X, n+X+1)
	ADDRESS CPU6502::GetINDX()
	{
		ADDRESS addr = FetchByte() + m_reg.X;

		// Cast to BYTE to wrap around in zero page in case we cross a page boundary
		BYTE l = m_memory.Read8(BYTE(addr));
		BYTE h = m_memory.Read8(BYTE(addr + 1));

		return MakeWord(h, l);
	}

	// Address of data is Y + address at ZP(n, n+1)
	ADDRESS CPU6502::GetINDY()
	{
		ADDRESS addr = FetchByte();

		// Cast to BYTE to wrap around in zero page in case we cross a page boundary
		BYTE l = m_memory.Read8(BYTE(addr));
		BYTE h = m_memory.Read8(BYTE(addr + 1));

		// TODO: 1 cycle penalty for boundary crossing
		return MakeWord(h, l) + m_reg.Y;
	}

	void CPU6502::MEMopRMW(std::function<void(CPU6502*, BYTE&)> func, ADDRESS dest)
	{
		BYTE temp = m_memory.Read8(dest);
		func(this, temp);
		m_memory.Write8(dest, temp);
	}

	void CPU6502::MEMopR(std::function<void(CPU6502*, BYTE)> func, ADDRESS oper)
	{
		BYTE temp = m_memory.Read8(oper);
		func(this, temp);
	}

	void CPU6502::AdjustNZ(BYTE val)
	{
		SetFlag(FLAG_N, GetBit(val, 7));
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU6502::PUSH(BYTE val)
	{
		m_memory.Write8(GetSP(), val);
		m_reg.SP--;
	}
	BYTE CPU6502::POP()
	{
		m_reg.SP++;
		return m_memory.Read8(GetSP());
	}

	void CPU6502::LD(BYTE& dest, BYTE src)
	{
		dest = src;
		AdjustNZ(dest);
	}

	void CPU6502::LDmem(BYTE& dest, ADDRESS src)
	{
		dest = m_memory.Read8(src);
		AdjustNZ(dest);
	}

	void CPU6502::STmem(ADDRESS dest, BYTE src)
	{
		m_memory.Write8(dest, src);
	}

	void CPU6502::PLA()
	{
		m_reg.A = POP();
		AdjustNZ(m_reg.A);
	}

	void CPU6502::PHA()
	{
		PUSH(m_reg.A);
	}

	void CPU6502::PLP()
	{
		// State of B and 5 flags is not changed
		bool oldR5 = GetFlag(_FLAG_R5);
		bool oldB = GetFlag(FLAG_B);

		m_reg.flags = POP();
		SetFlag(_FLAG_R5, oldR5);
		SetFlag(FLAG_B, oldB);
	}

	void CPU6502::PHP()
	{
		BYTE flags = m_reg.flags;

		// B and R5 are always pushed as set
		flags |= (_FLAG_R5 | FLAG_B);
		PUSH(flags);
	}

	void CPU6502::BRANCHif(bool cond)
	{
		// Sign expansion of relative address
		WORD rel = Widen(FetchByte());

		if (cond)
		{
			m_programCounter += rel;
		}
	}

	void CPU6502::JSR(ADDRESS dest)
	{
		PUSH(GetHByte(m_programCounter - 1));
		PUSH(GetLByte(m_programCounter - 1));
		m_programCounter = dest;
	}

	void CPU6502::RTS()
	{
		BYTE l = POP();
		BYTE h = POP();
		m_programCounter = MakeWord(h, l);
		++m_programCounter;
	}

	void CPU6502::JMP(ADDRESS dest)
	{
		m_programCounter = dest;
	}

	void CPU6502::BIT(ADDRESS src)
	{
		BYTE val = m_memory.Read8(src);

		SetFlag(FLAG_N, GetBit(val, 7));
		SetFlag(FLAG_V, GetBit(val, 6));

		val &= m_reg.A;
		SetFlag(FLAG_Z, val == 0);
	}

	void CPU6502::BRK()
	{
		FetchByte(); // Ignore padding byte

		// Push return address
		PUSH(GetHByte(m_programCounter));
		PUSH(GetLByte(m_programCounter));

		// Push flags
		PHP();

		SetFlag(FLAG_B, true);
		SetFlag(FLAG_I, true);

		ADDRESS irqVector = m_memory.Read16(ADDR_IRQ);
		LogPrintf(LOG_DEBUG, "IRQ vector: %04X", irqVector);
		m_programCounter = irqVector;
	}
	void CPU6502::RTI()
	{
		// Restore flags
		PLP();

		// Restore return address
		BYTE l = POP();
		BYTE h = POP();
		m_programCounter = MakeWord(h, l);
	}

	void CPU6502::ORA(BYTE oper)
	{
		m_reg.A |= oper;
		AdjustNZ(m_reg.A);

	}
	void CPU6502::AND(BYTE oper)
	{
		m_reg.A &= oper;
		AdjustNZ(m_reg.A);
	}
	void CPU6502::EOR(BYTE oper)
	{
		m_reg.A ^= oper;
		AdjustNZ(m_reg.A);
	}

	BYTE CPU6502::addSubBinary(BYTE oper, bool carry)
	{
		BYTE oldA = m_reg.A;
		WORD temp = oldA + oper;
		if (carry)
			++temp;

		BYTE ret = (BYTE)temp;

		AdjustNZ(ret);
		SetFlag(FLAG_C, (temp > 0xFF));
		SetFlag(FLAG_V, (GetMSB(oldA) == GetMSB(oper)) && (GetMSB(ret) != GetMSB(oper)));
		return ret;
	}

	BYTE CPU6502::addBCD(BYTE oper, bool carry)
	{
		BYTE oldA = m_reg.A;

		BYTE loNibble = (m_reg.A & 0x0F) + (oper & 0x0F) + (carry ? 1 : 0);
		BYTE hiNibble = (m_reg.A >> 4) + (oper >> 4);

		// Zero is set like in binary addition
		SetFlag(FLAG_Z, ((loNibble | hiNibble) & 0x0F) == 0);

		// Low nibble BCD adjust
		if (loNibble > 9)
		{
			loNibble += 6;
		}

		// Add low nibble carry
		if (loNibble > 0x0F)
		{
			++hiNibble;
		}

		// negative and overflow flags are set at this point
		bool msbResult = GetBit(hiNibble, 3);
		SetFlag(FLAG_N, msbResult);
		SetFlag(FLAG_V, (GetMSB(oldA) == GetMSB(oper)) && (msbResult != GetMSB(oper)));

		// High nibble BCD adjust
		if (hiNibble > 9)
		{
			hiNibble += 6;
		}

		// Carry set at this point
		SetFlag(FLAG_C, (hiNibble > 0x0F));

		// Combine nibbles
		BYTE ret = (hiNibble << 4) | (loNibble & 0x0F);
		return ret;
	}

	BYTE CPU6502::subBCD(BYTE oper, bool carry)
	{
		// Flags are set like binary mode, so do binary substraction first and ignore the result
		addSubBinary(~oper, carry);

		BYTE oldA = m_reg.A;

		BYTE loNibble = (m_reg.A & 0x0F) - (oper & 0x0F) - (carry ? 0 : 1);

		// Low nibble BCD adjust
		if (loNibble > 15)
		{
			loNibble -= 6;
		}

		BYTE hiNibble = (m_reg.A >> 4) - (oper >> 4);

		// Add low nibble carry
		if (loNibble > 0x0F)
		{
			--hiNibble;
		}

		// High nibble BCD adjust
		if (hiNibble > 15)
		{
			hiNibble -= 6;
		}

		// Combine nibbles
		BYTE ret = (hiNibble << 4) | (loNibble & 0x0F);
		return ret;
	}

	void CPU6502::ADC(BYTE oper)
	{
		bool carry = GetFlag(FLAG_C);
		m_reg.A = GetFlag(FLAG_D) ? addBCD(oper, carry) : addSubBinary(oper, carry);
	}
	void CPU6502::SBC(BYTE oper)
	{
		bool carry = GetFlag(FLAG_C);
		m_reg.A = GetFlag(FLAG_D) ? subBCD(oper, carry) : addSubBinary(~oper, carry);
	}

	void CPU6502::cmp(BYTE reg, BYTE oper)
	{
		WORD temp = reg - oper;

		AdjustNZ((BYTE)temp);
		SetFlag(FLAG_C, !(temp > 0xFF));
	}

	void CPU6502::INC(BYTE& dest)
	{
		++dest;
		AdjustNZ(dest);
	}
	void CPU6502::DEC(BYTE& dest)
	{
		--dest;
		AdjustNZ(dest);
	}

	void CPU6502::ASL(BYTE& dest)
	{
		SetFlag(FLAG_C, GetBit(dest, 7));
		dest <<= 1;
		AdjustNZ(dest);
	}

	void CPU6502::ROL(BYTE& dest)
	{
		bool carry = GetFlag(FLAG_C);
		SetFlag(FLAG_C, GetBit(dest, 7));
		dest <<= 1;
		SetBit(dest, 0, carry);
		AdjustNZ(dest);
	}

	void CPU6502::LSR(BYTE& dest)
	{
		SetFlag(FLAG_C, GetBit(dest, 0));
		dest >>= 1;
		AdjustNZ(dest);
	}

	void CPU6502::ROR(BYTE& dest)
	{
		bool carry = GetFlag(FLAG_C);
		SetFlag(FLAG_C, GetBit(dest, 0));
		dest >>= 1;
		SetBit(dest, 7, carry);
		AdjustNZ(dest);
	}
}
