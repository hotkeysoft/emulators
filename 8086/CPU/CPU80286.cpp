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

	const char* Selector::ToString() const
	{
		static char buf[32];
		sprintf(buf, "IDX[%04x] RPL[%04x] TI[%d]", GetIndex(), GetRPL(), GetTI());
		return buf;
	}

	const char* SegmentDescriptor::ToString() const
	{
		static char buf[64];
		sprintf(buf, "BASE[%08x] LIMIT[%04x] ACCESS[%02x]", base, limit, access);
		return buf;
	}

	const char* ExplicitRegister::ToString() const
	{
		static char buf[16];
		sprintf(buf, "[%08x][%04x]", base, limit);
		return buf;
	}

	const char* InterruptDescriptor::ToString() const
	{
		static char buf[64];
		sprintf(buf, "SEL[%s] OFF[%04x] DPL[%d] GATE[%s]",
			selector.ToString(), offset, GetDPL(), 
			GetGateType() == GateType::INTERRUPT ? "INT" : "TRP");
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

	ADDRESS CPU80286::GetAddress(SegmentOffset segoff) const
	{
		if (IsProtectedMode())
		{
			Selector selector(segoff.segment);

			// TODO: Shortcut for segment registers
			SegmentDescriptor dest = selector.GetTI() ? LoadSegmentLocal(selector) : LoadSegmentGlobal(selector);

			if (segoff.offset > dest.limit)
			{
				// TODO
				LogPrintf(LOG_ERROR, segoff.ToString());
				LogPrintf(LOG_ERROR, dest.ToString());

				throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
			}

			return dest.base + segoff.offset;
		}
		else
		{
			return S2A(segoff.segment, segoff.offset);
		}
	}

	ADDRESS CPU80286::GetCurrentAddress() const
	{
		if (IsProtectedMode())
		{
			// TODO: Checks
			return m_cs.base + m_reg[REG16::IP];
		}
		else
		{
			return S2A(m_reg[REG16::CS], m_reg[REG16::IP]);
		}
	}

	InterruptDescriptor CPU80286::GetInterruptDescriptor(BYTE interrupt) const
	{
		LogPrintf(LOG_DEBUG, "GetInterruptDescriptor[%d]", interrupt);

		if (interrupt >= m_idt.limit)
		{
			// TODO
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}
		ADDRESS address = m_idt.base + (interrupt * 8);

		InterruptDescriptor desc;
		desc.offset = m_memory.Read16(address);
		desc.selector = m_memory.Read16(address + 2);
		desc.flags = m_memory.Read8(address + 5);

		if (!desc.IsGateTypeValid())
		{
			// TODO
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}

		return desc;
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

	SegmentDescriptor CPU80286::LoadSegmentLocal(Selector selector) const
	{
		LogPrintf(LOG_WARNING, "LoadSegmentLocal[%d]", selector);
		throw std::exception("LoadSegmentLocaL: Not implemented");
	}

	SegmentDescriptor CPU80286::LoadSegmentGlobal(Selector selector) const
	{
		LogPrintf(LOG_DEBUG, "LoadSegmentGlobal[%d]", selector);

		if (selector.GetIndex() >= m_gdt.limit)
		{
			// TODO
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}
		ADDRESS address = m_gdt.base + (selector.GetIndex() * 8);

		SegmentDescriptor desc;
		desc.limit = m_memory.Read16(address);

		WORD baseL = m_memory.Read16(address + 2);
		emul::SetLWord(desc.base, baseL);
		WORD baseH = m_memory.Read8(address + 4);
		emul::SetHWord(desc.base, baseH);

		desc.access = m_memory.Read8(address + 5);

		return desc;
	}
	void CPU80286::UpdateTranslationRegister(SegmentTranslationRegister& dest, Selector selector, SegmentDescriptor desc)
	{
		dest.size = desc.limit;
		dest.base = desc.base;
		dest.access = desc.access;
		dest.selector = selector;
	}

	void CPU80286::ProtectedMode()
	{
		// Preinitialize Segment Address Translation Registers to match 
		// existing real mode segments
		m_cs.selector = 0;
		m_cs.size = 0xFFFF;
		m_cs.base = S2A(m_reg[REG16::CS]);
		m_cs.access = 0; // TODO: Adjust when protection/checks are added

		m_ds.selector = 0;
		m_ds.size = 0xFFFF;
		m_ds.base = S2A(m_reg[REG16::DS]);
		m_ds.access = 0; // TODO: Adjust when protection/checks are added

		m_es.selector = 0;
		m_es.size = 0xFFFF;
		m_es.base = S2A(m_reg[REG16::DS]);
		m_es.access = 0; // TODO: Adjust when protection/checks are added

		m_ss.selector = 0;
		m_ss.size = 0xFFFF;
		m_ss.base = S2A(m_reg[REG16::SS]);
		m_ss.access = 0; // TODO: Adjust when protection/checks are added

		// TODO: Since there is no instruction prefetch, the next instruction
		// has to be fetched through the memory.
		// Get the selector in the next jmp command.
		BYTE instruction = FetchByte();
		if (instruction != 0xEA)
		{
			throw std::exception("CPU80286::ModeSwitch: Expected jmpfar");
		}
		
		WORD offset = FetchWord();
		WORD segment = FetchWord();
		// Put IP back where we were
		m_reg[REG16::IP] -= 5;

		m_cs.selector = segment;
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
			throw std::exception("Not implemented");
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
		dest.Write(m_msw);
	}

	void CPU80286::LMSW(Mem16& source)
	{
		LogPrintf(LOG_WARNING, "LMSW");
		WORD value = source.Read();
		SetBitMask(value, MSW_RESERVED_ON, true);

		bool protectedMode = IsProtectedMode();
		if (protectedMode)
		{
			// Don't allow going back to real mode
			value |= MSW_PE;
		}

		MSW newMSW = (MSW)value;
		if (!protectedMode && (newMSW & MSW::MSW_PE))
		{
			LogPrintf(LOG_WARNING, "Switching to Protected Mode");
			ProtectedMode();
		}

		// Don't actually set the MSW until we made the switch: ProtectedMode need
		// access to real mode access functions before the transition
		m_msw = (MSW)value;
	}

	void CPU80286::LAR(BYTE regrm)
	{
		LogPrintf(LOG_ERROR, "LAR rw,ew 14,16 (noreal)");
		throw std::exception("Not implemented");
	}
	void CPU80286::LSL(BYTE regrm)
	{
		LogPrintf(LOG_ERROR, "LSL rw,ew 14,16 (noreal)");
		throw std::exception("Not implemented");
	}
	void CPU80286::LOADALL()
	{
		LogPrintf(LOG_ERROR, "LOADALL 195 (undocumented)(real)");
		throw std::exception("Not implemented");
	}
	void CPU80286::CLTS()
	{
		LogPrintf(LOG_ERROR, "CLTS 2 (real)");
		throw std::exception("Not implemented");
	}

	void CPU80286::INT(BYTE interrupt)
	{
		if (IsProtectedMode())
		{
			LogPrintf(LOG_WARNING, "INT(%02xh)[Protected Mode]", interrupt);

			InterruptDescriptor desc = GetInterruptDescriptor(interrupt);
			LogPrintf(LOG_WARNING, desc.ToString());

			// TODO: From 8086, adjust
			PUSHF();
			PUSH(m_reg[REG16::CS]);
			PUSH(m_reg[inRep ? REG16::_REP_IP : REG16::IP]);
			if (inRep)
			{
				inRep = false;
			}

			SetFlag(FLAG_T, false);
			CLI();

			m_reg[REG16::CS] = desc.selector;
			m_reg[REG16::IP] = desc.offset;
		}
		else
		{
			CPU80186::INT(interrupt);
		}
	}
}
