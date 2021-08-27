#include "stdafx.h"
#include "MemoryBlock.h"
#include <memory.h>
#include <fstream>

namespace emul
{
	MemoryBlock::MemoryBlock(ADDRESS baseAddress, size_t size, MemoryType type) :
		Logger("MEMBLK"),
		m_baseAddress(baseAddress),
		m_size(size),
		m_type(type)
	{
		m_invalid = 0xFA;

		m_data = new BYTE[m_size];

		Clear();
	}

	MemoryBlock::MemoryBlock(ADDRESS baseAddress, const std::vector<BYTE>data, MemoryType type /*=RAM*/) :
		Logger("MEMBLK"),
		m_baseAddress(baseAddress),
		m_type(type)
	{
		if (data.size() == 0 || data.size() > MaxBlockSize)
			throw std::exception("Invalid block size");
		m_size = data.size();

		m_invalid = 0xFA;

		m_data = new BYTE[m_size];

		for (size_t i = 0; i < m_size; i++)
			m_data[i] = data[i];
	}

	MemoryBlock::MemoryBlock(const MemoryBlock& block) :
		Logger("MEMBLK"),
		m_baseAddress(block.m_baseAddress),
		m_size(block.m_size),
		m_invalid(block.m_invalid),
		m_type(block.m_type)
	{
		m_data = new BYTE[m_size];
		memcpy(m_data, block.m_data, m_size);
	}

	MemoryBlock::~MemoryBlock()
	{
		delete[] m_data;
	}

	void MemoryBlock::Clear(BYTE filler)
	{
		memset(m_data, filler, m_size);
	}

	BYTE MemoryBlock::read(ADDRESS address)
	{
		if (address < m_baseAddress || address >= m_baseAddress + m_size)
			return m_invalid;
		else
			return m_data[address - m_baseAddress];
	}

	BYTE* MemoryBlock::getPtr8(ADDRESS address)
	{
		if (address < m_baseAddress || address >= m_baseAddress + m_size)
			return 0;
		else
			return m_data+(address - m_baseAddress);
	}

	WORD* MemoryBlock::getPtr16(ADDRESS address)
	{
		if (address < m_baseAddress || address + 1 >= m_baseAddress + m_size)
		{
			throw std::exception("Not implemented: can not cross memory block boundary with WORD access");
			return 0;
		}
		else
		{
			return (WORD*)(m_data + (address - m_baseAddress));
		}
	}

	void MemoryBlock::write(ADDRESS address, char data)
	{
		if (address < m_baseAddress || address >= m_baseAddress + m_size)
			return;

		m_data[address - m_baseAddress] = data;
	}

	bool MemoryBlock::LoadBinary(const char* file)
	{
		LogPrintf(LOG_INFO, "LoadBinary: loading %s", file);

		FILE* f = fopen(file, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadBinary: error opening binary file");
			return false;
		}

		size_t bytesRead = fread(m_data, sizeof(char), m_size, f);
		if (bytesRead < 1)
		{
			LogPrintf(LOG_ERROR, "LoadBinary: error reading binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "LoadBinary: read %d bytes to memory block", m_size);
		}

		fclose(f);
		return true;
	}


}
