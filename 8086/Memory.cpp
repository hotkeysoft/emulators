#include "stdafx.h"
#include "Memory.h"

namespace emul
{
	Memory::Memory(size_t addressBits) : Logger("MEM"),
		m_addressBits(addressBits),
		m_currBlock(NULL),
		m_currMin(0),
		m_currMax(0)
	{
	}

	Memory::~Memory()
	{

	}

	bool Memory::Allocate(MemoryBlock* block)
	{
		LogPrintf(LOG_INFO, "Request to allocate block at %X, size = %d bytes", block->GetBaseAddress(), block->GetSize());

		if (!CheckAddressRange(block->GetBaseAddress(), m_addressBits) || 
			!CheckAddressRange(block->GetBaseAddress() + block->GetSize() - 1, m_addressBits))
		{
			LogPrintf(LOG_ERROR, "Address out of range: block at %X, size = %d bytes", block->GetBaseAddress(), block->GetSize());
			LogPrintf(LOG_ERROR, "CPU Max address: %X", GetMaxAddress(m_addressBits));
			return false;
		}

		MemoryBlock* overlap = FindOverlap(block);
		if (overlap != NULL)
		{
			LogPrintf(LOG_ERROR, "Found overlap: block at %X, size = %d bytes", overlap->GetBaseAddress(), overlap->GetSize());
			return false;
		}

		m_memory.push_back(block);

		return true;
	}

	bool Memory::Free(MemoryBlock* block)
	{
		LogPrintf(LOG_INFO, "Freeing block at %X", block->GetBaseAddress());

		m_memory.remove(block);

		return true;
	}

	void Memory::Read(ADDRESS address, BYTE& value)
	{
		LogPrintf(LOG_INFO, "Read(%X)", address);

		MemoryBlock* block = NULL;

		if (m_currBlock && address >= m_currMin && address <= m_currMax)
		{
			block = m_currBlock;
		}
		else
		{
			MemoryBlock* newBlock = FindBlock(address);
			if (newBlock)
			{
				block = newBlock;
			}
			else
			{
				LogPrintf(LOG_ERROR, "Reading unallocated memory space (%X)", address);
			}
		}

		if (block)
		{
			value = block->read(address);
		}
		else
		{
			throw std::exception();
		}
	}

	BYTE* Memory::GetPtr8(ADDRESS address)
	{
		LogPrintf(LOG_INFO, "GetPtr8(%X)", address);

		MemoryBlock* block = NULL;

		if (m_currBlock && address >= m_currMin && address <= m_currMax)
		{
			block = m_currBlock;
		}
		else
		{
			MemoryBlock* newBlock = FindBlock(address);
			if (newBlock)
			{
				block = newBlock;
			}
			else
			{
				LogPrintf(LOG_ERROR, "Reading unallocated memory space (%X)", address);
			}
		}

		if (block)
		{
			return block->getPtr(address);
		}
		else
		{
			throw std::exception();
		}
	}

	void Memory::Write(ADDRESS address, BYTE value)
	{
		LogPrintf(LOG_INFO, "Write(%X, %X)", address, value);

		MemoryBlock* block = NULL;

		if (m_currBlock && address >= m_currMin && address <= m_currMax)
		{
			LogPrintf(LOG_INFO, "\tUsing cached block.");
			block = m_currBlock;
		}
		else
		{
			MemoryBlock* newBlock = FindBlock(address);
			if (newBlock)
			{
				LogPrintf(LOG_INFO, "\tNew block put in cache.");
				block = newBlock;
			}
			else
			{
				LogPrintf(LOG_ERROR, "Writing unallocated memory space (%X)", address);
			}
		}

		if (block)
		{
			if (block->GetType() == MemoryType::ROM)
			{
				LogPrintf(LOG_ERROR, "Attempting to write in ROM block at %X, size = %d bytes", block->GetBaseAddress(), block->GetSize());
				throw std::exception();
			}
			else
			{
				block->write(address, value);
			}
		}
		else
		{
			throw std::exception();
		}
	}

	MemoryBlock* Memory::FindBlock(ADDRESS address)
	{
		MemoryListType::const_iterator i;

		for (i = m_memory.begin(); i != m_memory.end(); i++)
		{
			ADDRESS currMin = (*i)->GetBaseAddress();
			ADDRESS currMax = currMin + (*i)->GetSize() - 1;

			if (address >= currMin && address <= currMax)
			{
				m_currBlock = *i;
				m_currMin = m_currBlock->GetBaseAddress();
				m_currMax = m_currMin + m_currBlock->GetSize() - 1;
				return *i;
			}
		}

		return NULL;
	}

	MemoryBlock* Memory::FindOverlap(const MemoryBlock* block)
	{
		ADDRESS min = block->GetBaseAddress();
		ADDRESS max = min + block->GetSize() - 1;

		MemoryListType::const_iterator i;

		for (i = m_memory.begin(); i != m_memory.end(); i++)
		{
			ADDRESS currMin = (*i)->GetBaseAddress();
			ADDRESS currMax = currMin + (*i)->GetSize() - 1;

			if ((min >= currMin && min <= currMax) || (max >= currMin && max <= currMax))
			{
				return *i;
			}
		}

		return NULL;
	}
}