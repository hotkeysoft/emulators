#include "stdafx.h"
#include "Common.h"
#include "CPU80286.h"

using cpuInfo::Opcode;
using cpuInfo::MiscTiming;
using cpuInfo::CPUType;
using cpuInfo::OpcodeTimingType;

using emul::GetLByte;
using emul::GetHByte;

using emul::GetHWord;
using emul::GetLWord;
using emul::SetHWord;
using emul::SetLWord;

namespace emul
{
	static BYTE GetOPn(BYTE opn) { return (opn >> 3) & 7; }


	// Not thread safe
	const char* ExplicitRegister::ToString() const
	{
		static char buf[16];
		sprintf(buf, "[%08x][%04x]", base, limit);
		return buf;
	}

	CPU80286::CPU80286(Memory& memory) : 
		CPU80186(CPUType::i80286, memory),
		Logger("CPU80286")
	{
	}

	void CPU80286::Init()
	{
		CPU80186::Init();

		FLAG_RESERVED_ON = FLAG(FLAG_R1);
		// Real mode: iopl & nested task bits are locked to zero (and R15 apparently)
		FLAG_RESERVED_OFF = FLAG(FLAG_R3 | FLAG_R5 | FLAG_IOPL0 | FLAG_IOPL1 | FLAG_NT | FLAG_R15);

		// TODO: Writes at SEG:FFFF exception 13

		// PUSH SP
		// Fixes the "Bug" on 8086/80186 where push sp pushed an already-decremented value
		m_opcodes[0x54] = [=]() { PUSH(m_reg[REG16::SP]); };

		// Multi 2-3 opcodes instructions
		m_opcodes[0x0F] = [=]() { MultiF0(FetchByte()); };

		// Not recognized in real mode
		m_opcodes[0x63] = [=]() { InvalidOpcode(); };
	}

	void CPU80286::Reset()
	{
		CPU80186::Reset();

		m_reg.Write16(REG16::CS, 0xF000);
		m_reg.Write16(REG16::IP, 0xFFF0);
		m_msw = MSW_RESET;
	}

	void CPU80286::ForceA20Low(bool forceLow)
	{
		LogPrintf(LOG_WARNING, "Force A20 line LOW: [%d]", forceLow);

		ADDRESS mask = m_memory.GetAddressMask();
		if (mask == 0)
		{
			LogPrintf(LOG_ERROR, "CPU Not initialized");
			return;
		}

		SetBit(mask, 20, !forceLow);
		m_memory.SetAddressMask(mask);

		LogPrintf(LOG_WARNING, "New mask=["PRINTF_BIN_PATTERN_INT32"]", PRINTF_BYTE_TO_BIN_INT32(mask));
	}

	void CPU80286::SetFlags(WORD flags)
	{
		SetBitMask(flags, FLAG_RESERVED_OFF, false);
		SetBitMask(flags, FLAG_RESERVED_ON, true);
		m_reg[REG16::FLAGS] = flags;
	}

	void CPU80286::CPUExceptionHandler(CPUException e)
	{
		// TODO, different behavior for different exceptions
		LogPrintf(LOG_WARNING, "CPU Exception -> INT(%d)", e.GetType());
		INT((BYTE)e.GetType());
	}

	void CPU80286::MultiF0(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP6, GetOPn(op2));

		switch (op2)
		{
		case 0: MultiF000(FetchByte()); break;
		case 1: MultiF001(FetchByte()); break;
		case 2: LAR(FetchByte()); break;
		case 3: LSL(FetchByte()); break;
		case 5: LOADALL(); break;
		case 6: CLTS(); break;
		default: InvalidOpcode(); break;
		}
	}

	void CPU80286::MultiF000(BYTE op3)
	{
		Mem16 modrm = GetModRM16(op3);

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP7, GetOPn(op3));

		switch (GetOPn(op3))
		{
		case 0: LogPrintf(LOG_ERROR, "SLDT ew 2,3 (noreal)"); break;
		case 1: LogPrintf(LOG_ERROR, "STR ew 2, 3 (noreal)"); break;
		case 2: LogPrintf(LOG_ERROR, "LLDT ew 17,19 (noreal)"); break;
		case 3: LogPrintf(LOG_ERROR, "LTR ew 17,19 (noreal)"); break;
		case 4: LogPrintf(LOG_ERROR, "VERR ew 14,16 (noreal)"); break;
		case 5: LogPrintf(LOG_ERROR, "VERW ew 14,16 (noreal)"); break;
		default:
			InvalidOpcode();
			break;
		}
	}
	void CPU80286::MultiF001(BYTE op3)
	{
		Mem16 modrm = GetModRM16(op3);

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP8, GetOPn(op3));

		switch (GetOPn(op3))
		{
		case 0: SGDT(modrm); break;
		case 1: SIDT(modrm); break;
		case 2: LGDT(modrm); break; // Load Global Descriptor Table Register
		case 3: LIDT(modrm); break; // Load Interrupt Descriptor Table Register
		case 4: SMSW(modrm); break; // Store Machine Status Word	
		case 6: LMSW(modrm); break; // Load Machine Status Word
		default:
			InvalidOpcode();
			break;
		}
	}

	void CPU80286::SGDT(Mem16& dest)
	{
		if (dest.IsRegister())
		{
			throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
		}

		dest.Write(m_gdt.limit);
		dest.Increment();
		dest.Write(GetLWord(m_gdt.base));
		dest.Increment();
		dest.Write(GetHWord(m_gdt.base));

		LogPrintf(LOG_WARNING, "SGDT: %s", m_idt.ToString());
	}
	void CPU80286::SIDT(Mem16& dest)
	{
		if (dest.IsRegister())
		{
			throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
		}

		dest.Write(m_idt.limit);
		dest.Increment();
		dest.Write(GetLWord(m_idt.base));
		dest.Increment();
		dest.Write(GetHWord(m_idt.base));

		LogPrintf(LOG_WARNING, "SGDT: %s", m_idt.ToString());
	}
	void CPU80286::LGDT(Mem16& source)
	{
		if (source.IsRegister())
		{
			throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
		}

		m_gdt.limit = source.Read();
		source.Increment();
		SetLWord(m_gdt.base, source.Read());
		source.Increment();
		SetHWord(m_gdt.base, source.Read());

		LogPrintf(LOG_WARNING, "LGDT: %s", m_gdt.ToString());
	}
	void CPU80286::LIDT(Mem16& source)
	{
		if (source.IsRegister())
		{
			throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
		}

		m_idt.limit = source.Read();
		source.Increment();
		SetLWord(m_idt.base, source.Read());
		source.Increment();
		SetHWord(m_idt.base, source.Read());

		LogPrintf(LOG_WARNING, "LIDT: %s", m_gdt.ToString());
	}

	void CPU80286::SMSW(Mem16& dest)
	{
		LogPrintf(LOG_WARNING, "SMSW");
	}

	void CPU80286::LMSW(Mem16& source)
	{
		LogPrintf(LOG_WARNING, "LMSW");
		WORD value = source.Read();
		SetBitMask(value, MSW_RESERVED_ON, true);

		if (IsProtectedMode())
		{
			// TODO

			// Don't allow going back to real mode
			value |= MSW_PE;
		}
		m_msw = (MSW)value;
	}

	void CPU80286::LAR(BYTE regrm)
	{
		LogPrintf(LOG_ERROR, "LAR rw,ew 14,16 (noreal)");
		InvalidOpcode();
	}
	void CPU80286::LSL(BYTE regrm)
	{
		LogPrintf(LOG_ERROR, "LSL rw,ew 14,16 (noreal)");
		InvalidOpcode();
	}
	void CPU80286::LOADALL()
	{
		LogPrintf(LOG_ERROR, "LOADALL 195 (undocumented)(real)");
		InvalidOpcode();
	}
	void CPU80286::CLTS()
	{
		LogPrintf(LOG_ERROR, "CLTS 2 (real)");
		InvalidOpcode();
	}
}
