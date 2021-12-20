#include "stdafx.h"
#include "CPU.h"

namespace emul
{
	CPU::CPU(size_t addressBits, Memory& memory, MemoryMap& mmap) : Logger("CPU"), 
		m_state(CPUState::STOP),
		m_memory(memory),
		m_mmap(mmap)
	{
	}

	CPU::~CPU()
	{
	}

	void CPU::Reset()
	{
		m_state = CPUState::STOP;
	}

	void CPU::Run()
	{
		m_state = CPUState::RUN;
		while (Step() && m_state == CPUState::RUN);
	}

	bool CPU::Step()
	{
		try
		{
			m_opTicks = 0;

			// Fetch opcode
			m_state = CPUState::RUN;
			unsigned char opcode = m_memory.Read8(GetCurrentAddress());
			
			// Execute instruction
			Exec(opcode);
		}
		catch (std::exception e)
		{	
			EnableLog(LOG_ERROR);
			LogPrintf(LOG_ERROR, "Error processing instruction at 0x%04X! [%s] Stopping CPU.\n", GetCurrentAddress(), e.what());
			m_state = CPUState::STOP;
		}

		return (m_state == CPUState::RUN);
	}

	void CPU::UnknownOpcode(BYTE opcode)
	{
		LogPrintf(LOG_ERROR, "CPU: Unknown Opcode (0x%02X) at address 0x%04X! Stopping CPU.\n", opcode, GetCurrentAddress());
		m_state = CPUState::STOP;
	}

	bool CPU::IsParityOdd(BYTE b)
	{
		BYTE parity = 0;
		for (size_t i = 0; i < 8; i++)
		{
			parity ^= (b & 1);
			b = b >> 1;
		}

		return (parity != 0);
	}

	void CPU::AddWatch(ADDRESS address, CPUCallbackFunc onCall, CPUCallbackFunc onRet)
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
	void CPU::RemoveWatch(ADDRESS address)
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

	void CPU::OnCall(ADDRESS caller, ADDRESS target)
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
	void CPU::OnReturn(ADDRESS address)
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
}