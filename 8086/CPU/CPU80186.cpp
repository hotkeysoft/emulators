#include "stdafx.h"
#include "CPU80186.h"

using cpuInfo::CPUType;

namespace emul
{
	CPU80186::CPU80186(Memory& memory) : 
		CPU8086(CPUType::i80186, memory),
		Logger("CPU80186")
	{
	}

	void CPU80186::Init()
	{
		CPU8086::Init();

		// PUSH SP
		// Fixes the "Bug" on 8086/80186 where push sp pushed an already-decremented value
		m_opcodes[0x54] = [=]() { PUSH(m_reg[REG16::SP]); };

		// PUSHA
		m_opcodes[0x60] = [=]() { PUSHA(); };

		// POPA
		m_opcodes[0x61] = [=]() { POPA(); };

		m_opcodes[0x62] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x63] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x64] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x65] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x66] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x67] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x68] = [=]() { PUSH(FetchWord()); };
		m_opcodes[0x69] = [=]() { IMULimm16(FetchByte()); };
		m_opcodes[0x6A] = [=]() { PUSH(Widen(FetchByte())); };
		m_opcodes[0x6B] = [=]() { IMULimm8(FetchByte()); };
		m_opcodes[0x6C] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6D] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6E] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6F] = [=]() { NotImplemented(m_opcode); };

		// ROL/ROR/RCL/RCR/SAL|SHL/SHR/---/SAR
		// REG8/MEM8, IMM8
		m_opcodes[0xC0] = [=]() { SHIFTROT8Imm(FetchByte()); };
		// REG16/MEM16, IMM8
		m_opcodes[0xC1] = [=]() { SHIFTROT16Imm(FetchByte()); };

		// REG8/MEM8, CL (masked: [0-31])
		m_opcodes[0xD2] = [=]() { SHIFTROT8Multi(FetchByte(), 31); };
		// REG16/MEM16, CL (masked: [0-31])
		m_opcodes[0xD3] = [=]() { SHIFTROT16Multi(FetchByte(), 31); };
	}

	void CPU80186::PUSHA()
	{
		WORD originalSP = m_reg[REG16::SP];

		PUSH(REG16::AX);
		PUSH(REG16::CX);
		PUSH(REG16::DX);
		PUSH(REG16::BX);
		PUSH(originalSP);
		PUSH(REG16::BP);
		PUSH(REG16::SI);
		PUSH(REG16::DI);
	}

	void CPU80186::POPA()
	{
		POP(REG16::DI);
		POP(REG16::SI);
		POP(REG16::BP);
		POP(REG16::_T0); // Dummy read, don't put in SP
		POP(REG16::BX);
		POP(REG16::DX);
		POP(REG16::CX);
		POP(REG16::AX);
	}

	void CPU80186::SHIFTROT8Imm(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT8Imm");

		Mem8 dest = GetModRM8(op2);
		BYTE work = dest.Read();

		BYTE count = FetchByte() & 31;

		work = _SHIFTROT8(work, op2, count);
		dest.Write(work);
	}
	void CPU80186::SHIFTROT16Imm(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "SHIFTROT16Imm");

		Mem16 dest = GetModRM16(op2);
		WORD work = dest.Read();

		BYTE count = FetchByte() & 31;

		work = _SHIFTROT16(work, op2, count);
		dest.Write(work);
	}

	void CPU80186::IMULimm8(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "IMULimm8");

		SourceDest16 sd = GetModRegRM16(op2);
		WORD imm = Widen(FetchByte());
		_IMUL16imm(sd, imm);
	}

	void CPU80186::IMULimm16(BYTE op2)
	{
		LogPrintf(LOG_DEBUG, "IMULimm16");

		SourceDest16 sd = GetModRegRM16(op2);
		WORD imm = FetchWord();
		_IMUL16imm(sd, imm);
	}

	void CPU80186::_IMUL16imm(SourceDest16& sd, WORD imm)
	{
		WORD source = sd.source.Read();

		int32_t result = (int16_t)source * (int16_t)(imm);
		LogPrintf(LOG_DEBUG, "IMUL16imm, %d * %d = %d", (int16_t)source, (int16_t)(imm), result);
		WORD resultW = (WORD)result;
		sd.dest.Write(result);

		bool ocFlags = ((result < -32768) || (result > 32767));
		SetFlag(FLAG_O, ocFlags);
		SetFlag(FLAG_C, ocFlags);
		AdjustSign(resultW);
		AdjustZero(resultW);
		AdjustParity(resultW);
	}
}
