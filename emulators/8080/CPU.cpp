#include "stdafx.h"
#include "CPU.h"

CPU::CPU(Memory &memory, MemoryMap &mmap) : Logger("CPU"), m_memory(memory), m_mmap(mmap)
{
	for (int i=0; i<256; i++)
	{
		m_opcodesTable[i] = &CPU::UnknownOpcode;
	}
}

CPU::~CPU()
{
}

void CPU::Reset()
{
	m_programCounter = 0;
	m_state = STOP;
	m_timeTicks = 0;
}

void CPU::Run()
{
	m_state = RUN;
	while (Step() && m_state == RUN);
}

bool CPU::Step()
{
	try
	{
		// Fetch opcode
		unsigned char opcode;
		m_state = RUN;
		m_memory.Read(m_programCounter, opcode);

		// Execute instruction
		(this->*m_opcodesTable[opcode])(opcode);
	}
	catch (std::exception e)
	{		
		LogPrintf(LOG_ERROR, "Error processing instruction at 0x%04X! Stopping CPU.\n", m_programCounter);
		m_state = STOP;
	}

	return (m_state == RUN);
}

void CPU::DumpUnassignedOpcodes()
{
	for (int i = 0; i < 256; ++i)
	{
		if (m_opcodesTable[i] == &CPU::UnknownOpcode) 
		{
			LogPrintf(LOG_INFO, "Unassigned: 0x%02X\t0%03o\n", i, i);
		}
	}
}

void CPU::AddOpcode(BYTE opcode, OPCodeFunction f)
{
	if (m_opcodesTable[opcode] != &CPU::UnknownOpcode)
	{
		LogPrintf(LOG_ERROR, "CPU: Opcode (0x%02X) already defined!\n", opcode);
	}

	m_opcodesTable[opcode] = f;
}

void CPU::UnknownOpcode(BYTE opcode)
{
	LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X! Stopping CPU.\n", opcode, m_programCounter);
	m_state = STOP;
}

bool CPU::isParityOdd(BYTE b)
{
	BYTE parity = 0;
	for (int i=0; i<8; i++)
	{
		parity ^= (b&1);
		b = b >> 1;
	}

	return (parity != 0);
}

void CPU::AddWatch(WORD address, CPUCallbackFunc onCall, CPUCallbackFunc onRet)
{
	LogPrintf(LOG_INFO, "Adding watch at address 0x%04X", address);
	WatchItem item;
	item.addr = address;
	item.onCall = onCall;
	item.onRet = onRet;
	m_callWatches[address] = item;
}
void CPU::AddWatch(const char* label, CPUCallbackFunc onCall, CPUCallbackFunc onRet)
{
	MemoryMapItem* item = m_mmap.Get(label);
	if (item)
	{
		AddWatch(item->address, onCall, onRet);
	}
	else
	{
		LogPrintf(LOG_WARNING, "AddWatch: Undefined label %s", label);
	}
}
void CPU::RemoveWatch(WORD address)
{
	LogPrintf(LOG_INFO, "Removing watch at address 0x%04X", address);
	m_callWatches.erase(address);
	m_returnWatches.erase(address);
}
void CPU::RemoveWatch(const char* label)
{
	MemoryMapItem* item = m_mmap.Get(label);
	if (item)
	{
		RemoveWatch(item->address);
	}
	else
	{
		LogPrintf(LOG_WARNING, "RemoveWatch: Undefined label %s", label);
	}
}

void CPU::OnCall(WORD caller, WORD target)
{
	auto it = m_callWatches.find(target);
	if (it != m_callWatches.end())
	{
		WatchItem& item = it->second;
		if (item.onCall)
		{
			m_returnWatches[caller] = item;
			item.onCall(this, target);
		}
	}
}
void CPU::OnReturn(WORD address)
{
	auto it = m_returnWatches.find(address);
	if (it != m_returnWatches.end())
	{
		WatchItem& item = it->second;
		if (item.onRet)
		{
			item.onRet(this, address);
		}
		m_returnWatches.erase(it);
	}
}