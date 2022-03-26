#include "stdafx.h"

#include "MemoryBlock.h"

namespace emul
{
	const DWORD MaxBlockSize = 1024 * 1024;
	const WORD BlockGranularity = 4096;

	MemoryBlock::MemoryBlock(const char* id, MemoryType type) :
		Logger(id),
		m_id(id ? id : "?"),
		m_type(type)
	{
		EnableLog(LOG_WARNING);
	}

	MemoryBlock::MemoryBlock(const char* id, DWORD size, MemoryType type) : MemoryBlock(id, type)
	{
		Alloc(size);
	}

	MemoryBlock::MemoryBlock(const char* id, const std::vector<BYTE>data, MemoryType type /*=RAM*/) :
		MemoryBlock(id, (DWORD)data.size(), type)
	{
		memcpy(m_data, &(data[0]), data.size());
	}

	MemoryBlock::MemoryBlock(const MemoryBlock& block) :
		Logger("MEMBLK"),
		m_id(block.m_id),
		m_size(block.m_size),
		m_type(block.m_type)
	{
		EnableLog(LOG_WARNING);
		m_data = new BYTE[m_size];
		memcpy(m_data, block.m_data, m_size);
	}

	MemoryBlock::~MemoryBlock()
	{
		delete[] m_data;
	}

	DWORD MemoryBlock::RoundBlockSize(DWORD size)
	{
		DWORD newSize = ((size + BlockGranularity - 1) / BlockGranularity) * BlockGranularity;
		if (newSize != size)
		{
			LogPrintf(Logger::LOG_WARNING, "Rounding block size [%d] -> [%d]", size, newSize);
		}
		return newSize;
	}

	void MemoryBlock::Alloc(DWORD size)
	{
		if (m_data)
		{
			delete[] m_data;
			m_data = nullptr;
			m_size = 0;
		}

		if (size == 0 || size > MaxBlockSize)
		{
			throw std::exception("Invalid block size");
		}

		m_size = RoundBlockSize(size);
		m_data = new BYTE[m_size];

		Clear();
	}

	void MemoryBlock::Clear(BYTE filler)
	{
		memset(m_data, filler, m_size);
	}

	BYTE MemoryBlock::read(ADDRESS offset)
	{
		assert(offset < m_size);
		return m_data[offset];
	}

	void MemoryBlock::write(ADDRESS offset, BYTE data)
	{
		assert(offset < m_size);
		if (m_type == MemoryType::RAM)
		{
			m_data[offset] = data;
		}
	}

	bool MemoryBlock::LoadFromFile(const char* file, WORD offset)
	{
		LogPrintf(LOG_INFO, "LoadBinary: loading %s at offset %04Xh", file, offset);

		if (offset >= m_size)
		{
			throw std::exception("LoadBinary: offset out of range");
		}

		FILE* f = fopen(file, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadBinary: error opening binary file");
			return false;
		}

		size_t bytesRead = fread(m_data+offset, sizeof(BYTE), m_size-offset, f);
		if (bytesRead < 1)
		{
			LogPrintf(LOG_ERROR, "LoadBinary: error reading binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "LoadBinary: read %d bytes to memory block at offset %04Xh", bytesRead, offset);
		}

		fclose(f);
		return true;
	}

	bool MemoryBlock::LoadOddEven(const char* file, OddEven oddEven)
	{
		LogPrintf(LOG_INFO, "LoadBinary: loading %s [%d]", file, oddEven == OddEven::ODD ? "ODD" : "EVEN");

		WORD halfSize = m_size / 2;
		std::vector<BYTE> tempBuf(halfSize);

		FILE* f = fopen(file, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadBinary: error opening binary file");
			return false;
		}

		size_t bytesRead = fread(&tempBuf[0], sizeof(BYTE), halfSize, f);
		if (bytesRead < 1)
		{
			LogPrintf(LOG_ERROR, "LoadBinary: error reading binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "LoadBinary: read %d bytes to memory block (%s addresses)", oddEven == OddEven::ODD ? "ODD" : "EVEN");
		}
		fclose(f);

		// 
		for (auto i = 0; i < halfSize; ++i)
		{
			m_data[(2 * i) + (int)oddEven] = tempBuf[i];
		}

		return true;
	}

	bool MemoryBlock::Dump(ADDRESS offset, DWORD len, const char* outFile) const
	{
		if (!len)
		{
			len = m_size;
		}

		LogPrintf(LOG_INFO, "Dump: dumping block size=%d to %s", m_size, outFile);

		FILE* f = fopen(outFile, "wb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "Dump: error opening binary file");
			return false;
		}

		len = std::min(len, m_size - offset);
		size_t bytesWritten = fwrite(m_data+offset, sizeof(char), len, f);
		if (bytesWritten != len)
		{
			LogPrintf(LOG_ERROR, "Dump: error writing binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "Dump: wrote %d bytes to file", bytesWritten);
		}

		fclose(f);
		return true;
	}

}
