#include "stdafx.h"
#include "Memory.h"

Memory::Memory() : Logger("MEM")
{
	m_currBlock = NULL;
	m_currMin = 0;
	m_currMax = 0;
}

Memory::~Memory()
{

}

bool Memory::Allocate(MemoryBlock *block)
{
	LogPrintf(LOG_INFO, "Request to allocate block at %X, size = %d bytes", block->GetBaseAddress(), block->GetSize());

	MemoryBlock *overlap = FindOverlap(block);
	if (overlap != NULL)
	{
		LogPrintf(LOG_ERROR, "Found overlap: block at %X, size = %d bytes", overlap->GetBaseAddress(), overlap->GetSize());
		return false;
	}

	m_memory.push_back(block);

	return true;
}

bool Memory::Free(MemoryBlock *block)
{
	LogPrintf(LOG_INFO, "Freeing block at %X", block->GetBaseAddress());

	m_memory.remove(block);

	return true;
}

void Memory::Read(WORD address, BYTE &value)
{
	LogPrintf(LOG_INFO, "Read(%X)", address);

	MemoryBlock *block = NULL;

	if (m_currBlock && address >= m_currMin && address <= m_currMax)
	{
		LogPrintf(LOG_INFO, "\tUsing cached block. ret = %X", m_currBlock->read(address));
		block = m_currBlock;
	}
	else
	{
		MemoryBlock *newBlock = FindBlock(address);
		if (newBlock)
		{
			LogPrintf(LOG_INFO, "\tNew block put in cache.  ret = %X", newBlock->read(address));
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

void Memory::Write(WORD address, BYTE value)
{
	LogPrintf(LOG_INFO, "Write(%X, %X)", address, value);

	MemoryBlock *block = NULL;

	if (m_currBlock && address >= m_currMin && address <= m_currMax)
	{
		LogPrintf(LOG_INFO, "\tUsing cached block.");
		block = m_currBlock;
	}
	else
	{
		MemoryBlock *newBlock = FindBlock(address);
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
		if (block->GetType() == ROM)
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

MemoryBlock *Memory::FindBlock(WORD address)
{
	MemoryListType::const_iterator i;

	for (i=m_memory.begin(); i!=m_memory.end(); i++)
	{
		WORD currMin = (*i)->GetBaseAddress();
		WORD currMax = currMin + (*i)->GetSize() - 1;

		if (address>=currMin && address<=currMax)
		{
			m_currBlock = *i;
			m_currMin = m_currBlock->GetBaseAddress();
			m_currMax = m_currMin + m_currBlock->GetSize() - 1;
			return *i;
		}
	}

	return NULL;
}

MemoryBlock *Memory::FindOverlap(const MemoryBlock *block)
{
	WORD min = block->GetBaseAddress();
	WORD max = min + block->GetSize() - 1;

	MemoryListType::const_iterator i;

	for (i=m_memory.begin(); i!=m_memory.end(); i++)
	{
		WORD currMin = (*i)->GetBaseAddress();
		WORD currMax = currMin + (*i)->GetSize() - 1;

		if ((min>=currMin && min<=currMax)||(max>=currMin && max<=currMax))
		{
			return *i;
		}
	}

	return NULL;
}
