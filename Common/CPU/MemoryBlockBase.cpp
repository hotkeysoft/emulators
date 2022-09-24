#include "stdafx.h"

#include <CPU/MemoryBlockBase.h>

namespace emul
{
	const DWORD MaxBlockSize = 1024 * 1024;
	const WORD BlockGranularity = 4096;

	MemoryBlockBase::MemoryBlockBase(const char* id, DWORD size, MemoryType type) :
		Logger(id),
		m_id(id ? id : "?"),
		m_size(size),
		m_type(type)
	{
		EnableLog(LOG_WARNING);
	}

	DWORD MemoryBlockBase::RoundBlockSize(DWORD size)
	{
		DWORD newSize = ((size + BlockGranularity - 1) / BlockGranularity) * BlockGranularity;
		if (newSize != size)
		{
			LogPrintf(Logger::LOG_WARNING, "Rounding block size [%d] -> [%d]", size, newSize);
		}
		return newSize;
	}
}
