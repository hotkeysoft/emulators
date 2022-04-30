#include "stdafx.h"
#include "CPUZ80.h"

using cpuInfo::Opcode;
using cpuInfo::CPUType;

namespace emul
{
	CPUZ80::CPUZ80(Memory& memory, Interrupts& interrupts) : CPUZ80(CPUType::z80, memory, interrupts)
	{
	}

	CPUZ80::CPUZ80(cpuInfo::CPUType type, Memory& memory, Interrupts& interrupts) :
		Logger("Z80"),
		CPU8080(type, memory, interrupts)
	{
	}

	void CPUZ80::Init()
	{
		CPU8080::Init();
	}

	void CPUZ80::Reset()
	{
		CPU8080::Reset();
	}


}