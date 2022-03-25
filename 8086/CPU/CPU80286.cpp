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

		// TODO: Writes at SEG:FFFF exception 13

		// PUSH SP
		// Fixes the "Bug" on 8086/80186 where push sp pushed an already-decremented value
		m_opcodes[0x54] = [=]() { PUSH(m_reg[REG16::SP]); };
	}

	void CPU80286::Reset()
	{
		CPU80186::Reset();

		m_reg.Write16(REG16::CS, 0xF000);
		m_reg.Write16(REG16::IP, 0xFFF0);
	}
}
