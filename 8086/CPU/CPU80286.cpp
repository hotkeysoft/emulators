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

	const char* AccessRights::ToString() const
	{
		static char buf[48];
		sprintf(buf, "P[%d] DPL[%d] [%s] TYPE[%x]", IsPresent(), GetDPL(), IsControl() ? "CONTROL" : "CODE/DATA", access & 0xF);
		return buf;
	}

	const char* Selector::ToString() const
	{
		static char buf[32];
		sprintf(buf, "IDX[%04x] RPL[%04x] TI[%d]", GetIndex(), GetRPL(), GetTI());
		return buf;
	}

	const char* SegmentDescriptor::ToString() const
	{
		static char buf[80];
		sprintf(buf, "BASE[%08x] LIMIT[%04x] ACCESS[%s]", base, limit, access.ToString());
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
		// Even with generous alignment, shouldn't be more than 16 bytes
		// With Visual Studio 2019 x64, we get 12.
		// We have to explose if larger than 16 because that's the space
		// we keep between segment registers in m_regs
		if (sizeof(SegmentTranslationRegister) > 16)
		{
			throw std::exception("Fatal: sizeof(SegmentTranslationRegister) > 16");
		}

		CPU80186::Init();

		Mem16::CheckAlignment(true);

		FLAG_RESERVED_ON = FLAG(FLAG_R1);
		// Real mode: iopl & nested task bits are locked to zero (and R15 apparently)
		FLAG_RESERVED_OFF = FLAG(FLAG_R3 | FLAG_R5 | FLAG_IOPL0 | FLAG_IOPL1 | FLAG_NT | FLAG_R15);

		// PUSH ES
		m_opcodes[0x06] = [=]() { PUSHSegReg(SEGREG::ES); };
		// POP ES
		m_opcodes[0x07] = [=]() { POPSegReg(SEGREG::ES); };
		// PUSH CS
		m_opcodes[0x0E] = [=]() { PUSHSegReg(SEGREG::CS); };
		// Multi 2-3 opcodes instructions
		m_opcodes[0x0F] = [=]() { MultiF0(FetchByte()); };
		// PUSH SS
		m_opcodes[0x16] = [=]() { PUSHSegReg(SEGREG::SS); };
		// POP SS
		m_opcodes[0x17] = [=]() { POPSegReg(SEGREG::SS); };
		// PUSH DS
		m_opcodes[0x1E] = [=]() { PUSHSegReg(SEGREG::DS); };
		// POP DS
		m_opcodes[0x1F] = [=]() { POPSegReg(SEGREG::DS); };
		// PUSH SP - Fixes the "Bug" on 8086/80186 where push sp pushed an already-decremented value
		m_opcodes[0x54] = [=]() { PUSH(m_reg[REG16::SP]); };
		// ARPL (Protected Mode only)
		m_opcodes[0x63] = [=]() { ARPL(GetModRegRM16(FetchByte(), false)); };
		// MOV REG16/MEM16, SEGREG
		m_opcodes[0x8C] = [=]() { MOVfromSegReg(GetModRegRM16(FetchByte(), false, true)); };
		// MOV SEGREG, REG16/MEM16
		m_opcodes[0x8E] = [=]() { MOVtoSegReg(GetModRegRM16(FetchByte(), true, true)); };


	}

	void CPU80286::Reset()
	{
		CPU80186::Reset();

		Selector sel = 0xF000;
		SegmentDescriptor desc = LoadSegmentReal(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
		m_reg.Write16(REG16::IP, 0xFFF0);
		m_msw = MSW_RESET;
	}

	ADDRESS CPU80286::GetAddress(SegmentOffset segoff, MemAccess access) const
	{
		if (!IsProtectedMode())
		{
			access = MemAccess::NONE;
		}

		const SegmentTranslationRegister* seg;

		switch (segoff.segment)
		{
		case SEGREG::CS: seg = &m_cs; break;
		case SEGREG::DS: seg = &m_ds; break;
		case SEGREG::ES: seg = &m_es; break;
		case SEGREG::SS: seg = &m_ss; break;
		default: throw std::exception("Invalid segment register");
		}

		if (IsProtectedMode() && ((seg->size == 0) || (segoff.offset > seg->size)))
		{
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION, seg->selector);
		}

		switch(access)
		{
		case MemAccess::READ:
			if (!seg->access.IsReadable())
			{
				throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION, seg->selector);
			}
			break;
		case MemAccess::WRITE:
			if (!seg->access.IsWritable())
			{
				throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION, seg->selector);
			}
			break;
		default:
			// No check
			break;
		}

		return seg->base + segoff.offset;
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
		LogPrintf(LOG_INFO, "Force A20 line LOW: [%d]", forceLow);

		ADDRESS mask = m_memory.GetAddressMask();
		if (mask == 0)
		{
			LogPrintf(LOG_ERROR, "CPU Not initialized");
			return;
		}

		SetBit(mask, 20, !forceLow);
		m_memory.SetAddressMask(mask);
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
		LogPrintf(LOG_INFO, "CPU Exception -> INT(%d)", e.GetType());
		INT((BYTE)e.GetType());
	}

	SegmentDescriptor CPU80286::LoadSegmentReal(Selector selector) const
	{
		LogPrintf(LOG_DEBUG, "LoadSegmentReal[%d]", selector);
		SegmentDescriptor desc;
		desc.limit = 0xFFFF;
		desc.base = S2A(selector);
		desc.access = 0; // TODO: Adjust when protection/checks are added
		return desc;
	}

	SegmentDescriptor CPU80286::LoadSegmentLocal(Selector selector) const
	{
		LogPrintf(LOG_WARNING, "LoadSegmentLocal[%d]", selector);
		throw std::exception("LoadSegmentLocaL: Not implemented");
	}

	SegmentDescriptor CPU80286::LoadSegmentGlobal(Selector selector) const
	{
		LogPrintf(LOG_DEBUG, "LoadSegmentGlobal[%d]", selector);

		SegmentDescriptor desc;

		if (selector.IsNull())
		{
			return desc;
		}

		if (selector.GetIndex() >= m_gdt.limit)
		{
			// TODO
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}
		ADDRESS address = m_gdt.base + (selector.GetIndex() * 8);

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

	void CPU80286::ARPL(SourceDest16 sd)
	{
		if (!IsProtectedMode())
		{
			InvalidOpcode();
			return;
		}

		LogPrintf(LOG_DEBUG, "ARPL");

		Selector source = sd.source.Read();
		Selector dest = sd.dest.Read();

		if (dest.GetRPL() < source.GetRPL())
		{
			SetFlag(FLAG_Z, true);
			dest.SetRPL(source.GetRPL());
			sd.dest.Write(dest);
		}
		else
		{
			SetFlag(FLAG_Z, false);
		}
	}

	void CPU80286::MultiF0(BYTE op2)
	{
		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP6, GetOPn(op2));

		switch (op2)
		{
		case 0: MultiF000(FetchByte()); break;
		case 1: MultiF001(FetchByte()); break;
		case 2: LAR(GetModRegRM16(FetchByte())); break; // Load Access Rights Byte
		case 3: LSL(GetModRegRM16(FetchByte())); break; // Load Segment Limit
		case 5: LOADALL(); break;
		case 6: CLTS(); break;
		default: InvalidOpcode(); break;
		}
	}

	void CPU80286::MultiF000(BYTE op3)
	{
		if (!IsProtectedMode())
		{
			InvalidOpcode();
			return;
		}

		Mem16 modrm = GetModRM16(op3);

		m_currTiming = &m_info.GetSubOpcodeTiming(Opcode::MULTI::GRP7, GetOPn(op3));

		switch (GetOPn(op3))
		{
		case 0: SLDT(modrm); break; //  Store Local Descriptor Table Register
		case 1: STR(modrm); break;  //  Store Task Register
		case 2: LLDT(modrm); break; //   Load Local Descriptor Table Register
		case 3: LTR(modrm); break;  //   Load Task Register
		case 4: VERR(modrm); break; // Verify Read
		case 5: VERW(modrm); break; // Verify Write
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
		case 0: SGDT(modrm); break; // Store Global Descriptor Table Register
		case 1: SIDT(modrm); break; // Store Interrupt Descriptor Table Register
		case 2: LGDT(modrm); break; //  Load Global Descriptor Table Register
		case 3: LIDT(modrm); break; //  Load Interrupt Descriptor Table Register
		case 4: SMSW(modrm); break; // Store Machine Status Word	
		case 6: LMSW(modrm); break; //  Load Machine Status Word
		default:
			InvalidOpcode();
			break;
		}
	}

	// Store Local Descriptor Table Register
	void CPU80286::SLDT(Mem16& dest)
	{
		LogPrintf(LOG_DEBUG, "SLDT");

		dest.Write(m_ldtr.selector);
	}

	// Store Task Register
	void CPU80286::STR(Mem16& dest)
	{
		LogPrintf(LOG_DEBUG, "STR");
		
		dest.Write(m_task.selector);
	}

	// Load Local Descriptor Table Register
	void CPU80286::LLDT(Mem16& source)
	{
		LogPrintf(LOG_DEBUG, "LLDT");

		if (m_iopl != 0)
		{
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}

		Selector sel = source.Read();

		SegmentDescriptor desc = LoadSegmentGlobal(sel);
		LogPrintf(LOG_DEBUG, desc.ToString());

		if (!sel.IsNull())
		{
			if (!desc.access.IsLDT())
			{
				throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION, sel);
			}
			if (!desc.access.IsPresent())
			{
				throw CPUException(CPUExceptionType::EX_NOT_PRESENT, sel);
			}
		}

		UpdateTranslationRegister(m_ldtr, sel, desc);
	}

	// Load Task Register
	void CPU80286::LTR(Mem16& source)
	{
		LogPrintf(LOG_DEBUG, "LTR");

		if (m_iopl != 0)
		{
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}

		Selector sel = source.Read();

		SegmentDescriptor desc = LoadSegmentGlobal(sel);
		LogPrintf(LOG_DEBUG, desc.ToString());

		if (!sel.IsNull())
		{
			if (!desc.access.IsTask() || desc.access.IsTaskBusy())
			{
				throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION, sel);
			}
			if (!desc.access.IsPresent())
			{
				throw CPUException(CPUExceptionType::EX_NOT_PRESENT, sel);
			}
		}

		UpdateTranslationRegister(m_task, sel, desc);
	}

	// Verify Read
	void CPU80286::VERR(Mem16& source)
	{
		LogPrintf(LOG_DEBUG, "VERR");

		// Result is in zero flag
		SetFlag(FLAG_Z, false);

		Selector sel = source.Read();

		// From here, no error should be thrown, result is in ZF
		try
		{
			// 1. Bound check, done in LoadSegment
			SegmentDescriptor desc = LoadSegment(sel);

			// 2. Must be code or data segment
			if (!desc.access.IsCodeOrData())
			{
				return;
			}

			// 3. Segment must be readable
			if (!desc.access.IsReadable())
			{
				return;
			}

			// 4. Privilege level... TODO

			// Everything checks out
			SetFlag(FLAG_Z, true);
			LogPrintf(LOG_DEBUG, "VERR: OK");
		}
		catch (CPUException)
		{
			// Nothing to do, ZF is alredy cleared
		}
	}

	// Verify Write
	void CPU80286::VERW(Mem16& source)
	{
		LogPrintf(LOG_DEBUG, "VERW");

		// Result is in zero flag
		SetFlag(FLAG_Z, false);

		Selector sel = source.Read();

		// From here, no error should be thrown, result is in ZF
		try
		{
			// 1. Bound check, done in LoadSegment
			SegmentDescriptor desc = LoadSegment(sel);

			// 2. Must be code or data segment
			if (!desc.access.IsCodeOrData())
			{
				return;
			}

			// 3. Segment must be writable
			if (!desc.access.IsWritable())
			{
				return;
			}

			// 4. Privilege level... TODO

			// Everything checks out
			SetFlag(FLAG_Z, true);
			LogPrintf(LOG_DEBUG, "VERW: OK");
		}
		catch (CPUException)
		{
			// Nothing to do, ZF is alredy cleared
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

		LogPrintf(LOG_DEBUG, "SGDT: %s", m_idt.ToString());
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

		LogPrintf(LOG_DEBUG, "SGDT: %s", m_idt.ToString());
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

		LogPrintf(LOG_DEBUG, "LGDT: %s", m_gdt.ToString());
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

		LogPrintf(LOG_DEBUG, "LIDT: %s", m_gdt.ToString());
	}

	void CPU80286::SMSW(Mem16& dest)
	{
		LogPrintf(LOG_DEBUG, "SMSW");
		dest.Write(m_msw);
	}

	void CPU80286::LMSW(Mem16& source)
	{
		LogPrintf(LOG_DEBUG, "LMSW");
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

	// Load Access Rights Byte
	void CPU80286::LAR(SourceDest16 sd)
	{
		LogPrintf(LOG_DEBUG, "LAR");

		// Result is in zero flag
		SetFlag(FLAG_Z, false);

		if (!IsProtectedMode())
		{
			InvalidOpcode();
			return;
		}

		Selector sel = sd.source.Read();

		// From here, no error should be thrown, result is in ZF
		try
		{
			SegmentDescriptor desc = LoadSegment(sel);

			// These check probably need to happen in LoadSegment
			BYTE dpl = desc.access.GetDPL();
			if (dpl < m_iopl || dpl < sel.GetRPL())
			{
				return;
			}

			WORD ret = 0;
			SetHByte(ret, desc.access);
			sd.dest.Write(ret);

			// Everything checks out
			SetFlag(FLAG_Z, true);
			LogPrintf(LOG_DEBUG, "LAR: OK");
		}
		catch (CPUException)
		{
			// Nothing to do, ZF is alredy cleared
		}
	}
	void CPU80286::LSL(SourceDest16 sd)
	{
		LogPrintf(LOG_DEBUG, "LSL");

		// Result is in zero flag
		SetFlag(FLAG_Z, false);

		if (!IsProtectedMode())
		{
			InvalidOpcode();
			return;
		}

		Selector sel = sd.source.Read();

		// From here, no error should be thrown, result is in ZF
		try
		{
			SegmentDescriptor desc = LoadSegment(sel);

			// These check probably need to happen in LoadSegment
			BYTE dpl = desc.access.GetDPL();
			if (dpl < m_iopl || dpl < sel.GetRPL())
			{
				return;
			}

			if (desc.access.IsConforming())
			{
				return;
			}

			WORD ret = desc.limit;
			sd.dest.Write(ret);

			// Everything checks out
			SetFlag(FLAG_Z, true);
			LogPrintf(LOG_DEBUG, "LSL: OK");
		}
		catch (CPUException)
		{
			// Nothing to do, ZF is alredy cleared
		}
	}

	void CPU80286::LoadTranslationDescriptor(SegmentTranslationRegister& dest, Selector selector, ADDRESS base)
	{
		dest.selector = selector;
		WORD baseL = m_memory.Read16(base + 0);
		emul::SetLWord(dest.base, baseL);
		WORD baseH = m_memory.Read8(base + 2);
		emul::SetHWord(dest.base, baseH);
		dest.access = m_memory.Read8(base + 3);
		dest.size = m_memory.Read16(base + 4);
	}

	void CPU80286::LOADALL()
	{
		LogPrintf(LOG_ERROR, "LOADALL (undocumented) [%04x:%04x]", m_reg[REG16::CS], m_reg[REG16::IP]);
		
		// Reads at fixed address 800h
		ADDRESS base = 0x800;

		// check mode change
		m_msw = (MSW)m_memory.Read16(base + 0x04); // TODO mode change
		WORD tReg = m_memory.Read16(base + 0x16);
		SetFlags(m_memory.Read16(base + 0x18)); // TODO
		m_reg[REG16::IP] = m_memory.Read16(base + 0x1A);
		WORD ldtReg = m_memory.Read16(base + 0x1C);
		WORD ds = m_memory.Read16(base + 0x1E);
		WORD ss = m_memory.Read16(base + 0x20);
		WORD cs = m_memory.Read16(base + 0x22);
		WORD es = m_memory.Read16(base + 0x24);
		m_reg[REG16::DI] = m_memory.Read16(base + 0x26);
		m_reg[REG16::SI] = m_memory.Read16(base + 0x28);
		m_reg[REG16::BP] = m_memory.Read16(base + 0x2A);
		m_reg[REG16::SP] = m_memory.Read16(base + 0x2C);
		m_reg[REG16::BX] = m_memory.Read16(base + 0x2E);
		m_reg[REG16::DX] = m_memory.Read16(base + 0x30);
		m_reg[REG16::CX] = m_memory.Read16(base + 0x32);
		m_reg[REG16::AX] = m_memory.Read16(base + 0x34);

		LoadTranslationDescriptor(m_es, es, base + 0x36);
		LoadTranslationDescriptor(m_cs, cs, base + 0x3C);
		LoadTranslationDescriptor(m_ss, ss, base + 0x42);
		LoadTranslationDescriptor(m_ds, ds, base + 0x48);
		
		// GDT
		{
			ADDRESS gdt = base + 0x4E;
			WORD baseL = m_memory.Read16(gdt + 0);
			emul::SetLWord(m_gdt.base, baseL);
			WORD baseH = m_memory.Read8(gdt + 2);
			emul::SetHWord(m_gdt.base, baseH);
			m_gdt.limit = m_memory.Read16(gdt + 4);
		}

		LoadTranslationDescriptor(m_ldtr, ldtReg, base + 0x54);

		// IDT
		{
			ADDRESS idt = base + 0x5A;
			WORD baseL = m_memory.Read16(idt + 0);
			emul::SetLWord(m_idt.base, baseL);
			WORD baseH = m_memory.Read8(idt + 2);
			emul::SetHWord(m_idt.base, baseH);
			m_idt.limit = m_memory.Read16(idt + 4);
		}

		LoadTranslationDescriptor(m_task, tReg, base + 0x60);
	}

	void CPU80286::CLTS()
	{
		LogPrintf(LOG_ERROR, "CLTS 2 (real)");
		throw std::exception("Not implemented");
	}

	void CPU80286::LoadPTR(SEGREG dest, SourceDest16 modRegRm)
	{
		LogPrintf(LOG_DEBUG, "LoadPtr");

		if (modRegRm.source.IsRegister())
		{
			throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
		}

		if (modRegRm.source.GetOffset() == 0xFFFD)
		{
			throw CPUException(CPUExceptionType::EX_GENERAL_PROTECTION);
		}

		if (IsProtectedMode())
		{
			CPU::TICK(15); // TODO: Dynamic timings
		}

		// Target register -> offset
		modRegRm.dest.Write(modRegRm.source.Read());

		// Read segment
		modRegRm.source.Increment();
		Selector sel = modRegRm.source.Read();
		SegmentDescriptor desc = LoadSegment(sel);

		SegmentTranslationRegister& seg = dest == SEGREG::DS ? m_ds : m_es;
		UpdateTranslationRegister(seg, sel, desc);
	}

	void CPU80286::CALLfar()
	{
		WORD offset = FetchWord();
		Selector sel = FetchWord();
		LogPrintf(LOG_DEBUG, "CALLfar %02X|%02X", sel, offset);

		PUSH(m_cs.selector);
		PUSH(REG16::IP);

		m_reg[REG16::IP] = offset;

		SegmentDescriptor desc = LoadSegment(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
	}

	void CPU80286::CALLInter(Mem16 destPtr)
	{
		PUSH(m_cs.selector);
		PUSH(REG16::IP);

		m_reg[REG16::IP] = destPtr.Read();
		destPtr.Increment();
		Selector sel = destPtr.Read();
		LogPrintf(LOG_DEBUG, "CALLInter newCS=%04X, newIP=%04X", sel, m_reg[REG16::IP]);

		SegmentDescriptor desc = LoadSegment(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
	}

	void CPU80286::JMPfar()
	{
		WORD offset = FetchWord();
		Selector sel = FetchWord();
		LogPrintf(LOG_DEBUG, "JMPfar %02X|%02X", sel, offset);

		m_reg[REG16::IP] = offset;

		SegmentDescriptor desc = LoadSegment(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
	}

	void CPU80286::JMPInter(Mem16 destPtr)
	{
		if (destPtr.IsRegister())
		{
			throw CPUException(CPUExceptionType::EX_UNDEFINED_OPCODE);
		}
		m_reg[REG16::IP] = destPtr.Read();
		destPtr.Increment();

		Selector sel = destPtr.Read();
		LogPrintf(LOG_DEBUG, "JMPInter newCS=%04X, newIP=%04X", sel, m_reg[REG16::IP]);

		SegmentDescriptor desc = LoadSegment(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
	}

	void CPU80286::RETFar(bool pop, WORD value)
	{
		LogPrintf(LOG_DEBUG, "RETFar [%s][%d]", pop ? "Pop" : "NoPop", value);

		POP(REG16::IP);
		Selector sel = POP();
		m_reg[REG16::SP] += value;

		SegmentDescriptor desc = LoadSegment(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
	}

	void CPU80286::INT(BYTE interrupt)
	{
		LogPrintf(LOG_DEBUG, "INT (%02xh)", interrupt);

		PUSHF();
		PUSH(m_cs.selector);
		PUSH(m_reg[inRep ? REG16::_REP_IP : REG16::IP]);
		if (inRep)
		{
			inRep = false;
		}

		SetFlag(FLAG_T, false);
		CLI();

		if (IsProtectedMode())
		{
			InterruptDescriptor intDesc = GetInterruptDescriptor(interrupt);
			SegmentDescriptor segDesc = LoadSegment(intDesc.selector);
			UpdateTranslationRegister(m_cs, intDesc.selector, segDesc);

			m_reg[REG16::IP] = intDesc.offset;
		}
		else
		{
			ADDRESS interruptAddress = interrupt * 4;
			Selector sel = m_memory.Read16(interruptAddress + 2);
			SegmentDescriptor desc = LoadSegment(sel);
			UpdateTranslationRegister(m_cs, sel, desc);

			m_reg[REG16::IP] = m_memory.Read16(interruptAddress);
		}
	}

	void CPU80286::IRET()
	{
		LogPrintf(LOG_DEBUG, "IRET");
		POP(REG16::IP);
		Selector sel = POP();
		POPF();

		SegmentDescriptor desc = LoadSegment(sel);
		UpdateTranslationRegister(m_cs, sel, desc);
	}

	void CPU80286::MOVfromSegReg(SourceDest16 sd)
	{
		LogPrintf(LOG_DEBUG, "MOV from SEGREG");

		Selector sel;
		switch (sd.source.GetRegister())
		{
		case REG16::CS: sel = m_cs.selector; break;
		case REG16::DS: sel = m_ds.selector; break;
		case REG16::SS: sel = m_ss.selector; break;
		case REG16::ES: sel = m_es.selector; break;
		default: InvalidOpcode(); break;
		}
		sd.dest.Write(sel);
	}

	void CPU80286::MOVtoSegReg(SourceDest16 sd)
	{
		LogPrintf(LOG_DEBUG, "MOV to SEGREG");

		if (IsProtectedMode())
		{
			CPU::TICK(15); // TODO: Dynamic timings
		}

		Selector sel = sd.source.Read();
		SegmentDescriptor desc = LoadSegment(sel);

		switch (sd.dest.GetRegister())
		{
		case REG16::ES: UpdateTranslationRegister(m_es, sel, desc); break;
		case REG16::DS: UpdateTranslationRegister(m_ds, sel, desc); break;
		case REG16::SS: UpdateTranslationRegister(m_ss, sel, desc); break;
		default: InvalidOpcode(); break;
		}
	}

	void CPU80286::PUSHSegReg(SEGREG segreg)
	{
		LogPrintf(LOG_DEBUG, "PUSH SEGREG");

		switch (segreg)
		{
		case SEGREG::CS: PUSH(m_cs.selector); break;
		case SEGREG::DS: PUSH(m_ds.selector); break;
		case SEGREG::SS: PUSH(m_ss.selector); break;
		case SEGREG::ES: PUSH(m_es.selector); break;
		default: InvalidOpcode(); break;
		}
	}

	void CPU80286::POPSegReg(SEGREG segreg)
	{
		LogPrintf(LOG_DEBUG, "POP SEGREG");

		if (IsProtectedMode())
		{
			CPU::TICK(15); // TODO: Dynamic timings
		}

		Selector sel = POP();
		SegmentDescriptor desc = LoadSegment(sel);

		switch (segreg)
		{
		case SEGREG::ES: UpdateTranslationRegister(m_es, sel, desc); break;
		case SEGREG::DS: UpdateTranslationRegister(m_ds, sel, desc); break;
		case SEGREG::SS: UpdateTranslationRegister(m_ss, sel, desc); break;
		default: InvalidOpcode(); break;
		}
	}
}
