#include "stdafx.h"

#include <CPU/MemoryBlockBase.h>

namespace emul
{
	MemoryBlockBase::MemoryBlockBase(const char* id, DWORD size, MemoryType type) :
		Logger(id),
		m_id(id ? id : "?"),
		m_size(size),
		m_type(type)
	{
		EnableLog(LOG_WARNING);
	}

	void MemoryBlockBase::SetBlockGranularity(WORD blockGranularity)
	{
		assert(IsPowerOf2(blockGranularity));
		assert(blockGranularity >= 8);
		s_blockGranularity = blockGranularity;
	}

	DWORD MemoryBlockBase::RoundBlockSize(DWORD size)
	{
		DWORD newSize = emul::RoundPowerOf2(size, s_blockGranularity);
		if (newSize != size)
		{
			LogPrintf(Logger::LOG_WARNING, "Rounding block size [%d] -> [%d]", size, newSize);
		}
		return newSize;
	}
}
