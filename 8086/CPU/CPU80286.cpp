#include "stdafx.h"
#include "CPU80286.h"

using cpuInfo::CPUType;

namespace emul
{
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

		// 0x0F 0x00 /0 SLDT ew 2,3 (noreal)
		// 0x0F 0x00 /1 STR ew 2,3 (noreal)
		// 0x0F 0x00 /2 LLDT ew 17,19 (noreal)
		// 0x0F 0x00 /3 LTR ew 17,19 (noreal)
		// 0x0F 0x00 /4 VERR ew 14,16 (noreal)
		// 0x0F 0x00 /5 VERW ew 14,16 (noreal)

		// 0x0F 0x01 /0 SGDT m 11 (real)
		// 0x0F 0x01 /1 SIDT m 12 (real)
		// 0x0F 0x01 /2 LGDT m 11 (real)
		// 0x0F 0x01 /3 LIDT m 12 (real)
		// 0x0F 0x01 /4 SMSW ew 2,3 (real)
		// 0x0F 0x01 /6 LMSW ew 3,6 (real)
		// 
		// 0x0F 0x02 LAR 14,16 (noreal)
		// 
		// 0x0F 0x03 /r LSL rw,ew 14,16 (noreal)
		// 
		// 0x0F 0x05 LOADALL 195 (undocumented) (real)
		// 
		// 0x0F 0x06 CLTS 2 (real)

		// Not recognized in real mode
		m_opcodes[0x63] = [=]() { InvalidOpcode(); };

	}

	void CPU80286::Reset()
	{
		CPU80186::Reset();

		m_reg.Write16(REG16::CS, 0xF000);
		m_reg.Write16(REG16::IP, 0xFFF0);
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
}
