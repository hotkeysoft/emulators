#include "stdafx.h"
#include "CPU8086.h"

namespace emul
{
	CPU8086::CPU8086(Memory& memory, MemoryMap& mmap)
		: CPU(memory, mmap), Logger("CPU8086")
	{
		//	AddOpcode(0100, (OPCodeFunction)(&CPU8086::MOVrr));	// B,B

	}

	CPU8086::~CPU8086()
	{

	}

	void CPU8086::AddDevice(PortConnector& ports)
	{
		return m_ports.Connect(ports);
	}

	void CPU8086::Reset()
	{
		CPU::Reset();
	}

	void CPU8086::Dump()
	{
		//	LogPrintf(LOG_DEBUG, "PC = %04X\n", m_programCounter);
		LogPrintf(LOG_DEBUG, "\n");
	}

}