#include "stdafx.h"
#include "CPU80186.h"

using cpuInfo::CPUType;

namespace emul
{
	CPU80186::CPU80186(Memory& memory) : CPU80186::CPU80186(CPUType::i80186, memory)
	{
	}

	CPU80186::CPU80186(CPUType type, Memory& memory) :
		CPU8086(type, memory),
		Logger("CPU80186")
	{
	}

	void CPU80186::Init()
	{
		CPU8086::Init();

		// TODO: Writes at SEG:FFFF don't wrap around on the 80186

		// Invalid Opcodes
		m_opcodes[0x0F] = [=]() { InvalidOpcode(); };

		m_opcodes[0x63] = [=]() { InvalidOpcode(); };
		m_opcodes[0x64] = [=]() { InvalidOpcode(); };
		m_opcodes[0x65] = [=]() { InvalidOpcode(); };
		m_opcodes[0x66] = [=]() { InvalidOpcode(); };
		m_opcodes[0x67] = [=]() { InvalidOpcode(); };

		m_opcodes[0xD6] = [=]() { InvalidOpcode(); };

		m_opcodes[0xF1] = [=]() { InvalidOpcode(); };

		// PUSHA
		m_opcodes[0x60] = [=]() { PUSHA(); };

		// POPA
		m_opcodes[0x61] = [=]() { POPA(); };
		// BOUND REG16, MEM16
		m_opcodes[0x62] = [=]() { BOUND(FetchByte()); };
		// PUSH IMM16
		m_opcodes[0x68] = [=]() { PUSH(FetchWord()); };
		m_opcodes[0x69] = [=]() { IMULimm16(FetchByte()); };
		// PUSH IMM8 (sign-extend to 16)
		m_opcodes[0x6A] = [=]() { PUSH(Widen(FetchByte())); };
		// IMUL REG16, REG16/MEM16, IMM16
		m_opcodes[0x6B] = [=]() { IMULimm8(FetchByte()); };
		// INS8
		m_opcodes[0x6C] = [=]() { INSB(); };
		// INS16
		m_opcodes[0x6D] = [=]() { INSW(); };
		// OUTS8
		m_opcodes[0x6E] = [=]() { OUTSB(); };
		// OUTS16
		m_opcodes[0x6F] = [=]() { OUTSW(); };

		// ROL/ROR/RCL/RCR/SAL|SHL/SHR/---/SAR
		// REG8/MEM8, IMM8
		m_opcodes[0xC0] = [=]() { SHIFTROT8Imm(FetchByte()); };
		// REG16/MEM16, IMM8
		m_opcodes[0xC1] = [=]() { SHIFTROT16Imm(FetchByte()); };

		// ENTER IMM16, IMM8
		m_opcodes[0xC8] = [=]() { ENTER(); };
		// LEAVE
		m_opcodes[0xC9] = [=]() { LEAVE(); };

		// REG8/MEM8, CL (masked: [0-31])
		m_opcodes[0xD2] = [=]() { SHIFTROT8Multi(FetchByte(), 31); };
		// REG16/MEM16, CL (masked: [0-31])
		m_opcodes[0xD3] = [=]() { SHIFTROT16Multi(FetchByte(), 31); };
	}

	void CPU80186::InvalidOpcode()
	{	
		LogPrintf(LOG_ERROR, "TRAP: Invalid opcode [%02x] @ %08x", m_opcode, GetCurrentAddress());
		INT(6);
	}

	void CPU80186::BOUND(BYTE op2)
	{
		SourceDest16 sd = GetModRegRM16(op2);
		if (sd.source.IsRegister())
		{
			InvalidOpcode();
			return;
		}

		int16_t value = (int16_t)sd.dest.Read();
		int16_t lowBound = (int16_t)sd.source.Read();
		sd.source.Increment();
		int16_t hiBound = (int16_t)sd.source.Read();

		if (value < lowBound || value > hiBound)
		{
			// TODO: Push CS:IP of current instruction?
			INT(5);
		}
	}

	void CPU80186::ENTER()
	{
		WORD disp = FetchWord();
		BYTE level = FetchByte();
		LogPrintf(LOG_DEBUG, "ENTER, data=%04x, level=%02x", disp, level);

		PUSH(REG16::BP);

		WORD framePtr = m_reg[REG16::SP];
		if (level > 0)
		{
			for (int i = 0; i < level - 1; ++i)
			{
				m_reg[REG16::BP] -= 2;
				WORD ptr = m_memory.Read16(S2A(m_reg[REG16::SS], m_reg[REG16::BP]));
				PUSH(ptr);
				TICKT3(); // Add overhead for each level
			}
			PUSH(framePtr);
		}
		m_reg[REG16::BP] = framePtr;
		m_reg[REG16::SP] -= disp;
	}

	void CPU80186::LEAVE()
	{
		LogPrintf(LOG_DEBUG, "LEAVE");
		m_reg[REG16::SP] = m_reg[REG16::BP];
		POP(REG16::BP);
	}

	void CPU80186::PUSHA()
	{
		LogPrintf(LOG_DEBUG, "PUSHA");
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
		LogPrintf(LOG_DEBUG, "POPA");
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

	void CPU80186::INSB()
	{
		LogPrintf(LOG_DEBUG, "INSB, DI=%04X", m_reg[REG16::DI]);

		if (PreREP())
		{
			Mem8 dest = SegmentOffset(m_reg[REG16::ES], m_reg[REG16::DI]);

			BYTE val;
			In(m_reg[REG16::DX], val);
			dest.Write(val);

			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(false);
	}

	void CPU80186::INSW()
	{
		LogPrintf(LOG_DEBUG, "INSW, DI=%04X", m_reg[REG16::DI]);

		if (PreREP())
		{
			Mem16 dest = SegmentOffset(m_reg[REG16::ES], m_reg[REG16::DI]);

			BYTE l, h;
			In(m_reg[REG16::DX], l);
			In(m_reg[REG16::DX] + 1, h);
			dest.Write(MakeWord(h, l));

			IndexIncDec(m_reg[REG16::DI]);
			IndexIncDec(m_reg[REG16::DI]);
		}
		PostREP(false);
	}

	void CPU80186::OUTSB()
	{
		LogPrintf(LOG_DEBUG, "OUTSB, SI=%04X", m_reg[REG16::SI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			BYTE val = m_memory.Read8(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]));

			Out(m_reg[REG16::DX], val);

			IndexIncDec(m_reg[REG16::SI]);
		}
		PostREP(false);
	}

	void CPU80186::OUTSW()
	{
		LogPrintf(LOG_DEBUG, "OUTSW, SI=%04X", m_reg[REG16::SI]);
		if (inSegOverride)
		{
			LogPrintf(LOG_DEBUG, "SEG OVERRIDE %04X", m_reg[REG16::_SEG_O]);
		}

		if (PreREP())
		{
			WORD val = m_memory.Read16(S2A(m_reg[inSegOverride ? REG16::_SEG_O : REG16::DS], m_reg[REG16::SI]));

			Out(m_reg[REG16::DX], emul::GetLByte(val));
			Out(m_reg[REG16::DX] + 1, emul::GetHByte(val));

			IndexIncDec(m_reg[REG16::SI]);
			IndexIncDec(m_reg[REG16::SI]);
		}
		PostREP(false);
	}

}
