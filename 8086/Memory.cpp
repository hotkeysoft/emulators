#include "stdafx.h"
#include "Memory.h"

namespace emul
{
	WORD Memory::s_uninitialized = 0xF00F;

	const WORD BlockGranularity = 4096;

	Memory::Memory(size_t addressBits) : Logger("MEM"),
		m_addressBits(addressBits)
	{
		size_t memSlots = ((uint64_t)1 << addressBits) / BlockGranularity;
		LogPrintf(LOG_INFO, "BlockGranularity: %d, Allocating %d memory slots", BlockGranularity, memSlots);

		m_memory.reserve(memSlots);
		for (size_t i = 0; i < memSlots; ++i)
		{
			m_memory.push_back(MemorySlot());
		}
	}

	Memory::~Memory()
	{

	}

	bool Memory::Allocate(MemoryBlock* block, ADDRESS base)
	{
		LogPrintf(LOG_INFO, "Request to allocate block [%s] at %X, size = %d bytes", block->GetId().c_str(), base, block->GetSize());

		if (!CheckAddressRange(base, m_addressBits) ||
			!CheckAddressRange(base + (DWORD)block->GetSize() - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", base, block->GetSize());
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		int nbSlots = block->GetSize() / BlockGranularity;
		int minSlot = base / BlockGranularity;
		LogPrintf(LOG_DEBUG, "Using %d slots, first slot = %02Xh", nbSlots, minSlot);

		for (size_t i = 0; i < nbSlots; ++i)
		{
			MemorySlot& slot = m_memory[minSlot + i];
			if (slot.block)
			{
				LogPrintf(LOG_WARNING, "Slot [%x] already allocated to [%s], replacing with block [%s]", i,
					slot.block->GetId().c_str(),
					block->GetId().c_str());
			}
			m_memory[minSlot + i] = {block, base};
		}

		return true;
	}

	bool Memory::Free(MemoryBlock* block)
	{
		LogPrintf(LOG_INFO, "Freeing block [%s]", block->GetId().c_str());

		for (size_t i = 0; i < m_memory.size(); ++i)
		{
			MemorySlot& slot = m_memory[i];
			MemoryBlock* oldBlock = slot.block;
			if (oldBlock == block)
			{
				slot = { nullptr, 0 };
			}
		}

		return true;
	}

	bool Memory::MapWindow(ADDRESS source, ADDRESS window, DWORD len)
	{
		LogPrintf(LOG_WARNING, "Request to map memory [%X]-[%X] to window [%X]-[%X], window size: %d bytes",
			source, source + len - 1,
			window, window + len - 1,
			len);

		if (len % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Window size [%d] is not a multiple of [%d]", len, BlockGranularity);
			return false;
		}

		if (source % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Source base address [%d] is not aligned to [%d] bytes boundary", source, BlockGranularity);
			return false;
		}

		if (window % BlockGranularity != 0)
		{
			LogPrintf(LOG_ERROR, "Window base address [%d] is not aligned to [%d] bytes boundary", window, BlockGranularity);
			return false;
		}

		if (!CheckAddressRange(source, m_addressBits) ||
			!CheckAddressRange(source + len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", source, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		if (!CheckAddressRange(window, m_addressBits) ||
			!CheckAddressRange(window + len - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", source, len);
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		int nbSlots = len / BlockGranularity;
		int sourceBaseSlot = source / BlockGranularity;
		int windowBaseSlot = window / BlockGranularity;
		LogPrintf(LOG_INFO, "Copying %d slots, base slot = %02Xh -> %02Xh", nbSlots, sourceBaseSlot, windowBaseSlot);

		for (size_t i = 0; i < nbSlots; ++i)
		{			
			MemorySlot slot = m_memory[sourceBaseSlot + i];
			slot.base += (window - source);

			m_memory[windowBaseSlot + i] = slot;
		}

		return true;
	}
	

	BYTE* Memory::GetPtr8(ADDRESS address)
	{
		MemorySlot& slot = m_memory[address / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			return block->getPtr8(address - slot.base);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Reading unallocated memory space (%X)", address);
			s_uninitialized = 0xF00F; // Reset 'initialized' value in case someone wrote in it
			return (BYTE*)&Memory::s_uninitialized;
		}
	}

	WORD* Memory::GetPtr16(ADDRESS address)
	{
		MemorySlot& slot = m_memory[address / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			return block->getPtr16(address - slot.base);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Reading unallocated memory space (%X)", address);
			s_uninitialized = 0xF00F; // Reset 'initialized' value in case someone wrote in it
			return &Memory::s_uninitialized;
		}
	}

	bool Memory::LoadBinary(const char* file, ADDRESS baseAddress)
	{
		MemorySlot& slot = m_memory[baseAddress / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (!block)
		{
			// TODO: Create block
			LogPrintf(LOG_ERROR, "LoadBinary: No memory allocated at address %04X", baseAddress);
			return false;
		}
		return block->LoadBinary(file, baseAddress - slot.base);
	}

	void Memory::Read(ADDRESS address, BYTE& value)
	{
		BYTE* mem = GetPtr8(address);
		value = *mem;
	}

	void Memory::Write(ADDRESS address, BYTE value)
	{
		MemorySlot& slot = m_memory[address / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			block->write(address - slot.base, value);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Writing unallocated memory space (%X)", address);
		}
	}

	void Memory::Dump(ADDRESS start, DWORD len, const char* outFile)
	{
		MemorySlot& slot = m_memory[start / BlockGranularity];
		MemoryBlock* block = slot.block;

		if (block)
		{
			block->Dump(start - slot.base, len, outFile);
		}
		else
		{
			LogPrintf(LOG_WARNING, "No block allocated at address (%X)", start);
		}

	}
}