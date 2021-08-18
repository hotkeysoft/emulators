#pragma once

#include "MemoryBlock.h"
#include "Common.h"
#include "Logger.h"
#include <list>

namespace emul
{
	typedef std::list<MemoryBlock*> MemoryListType;

	class Memory : public Logger
	{
	public:
		Memory(size_t addressBits);
		virtual ~Memory();

		bool Allocate(MemoryBlock* block);
		bool Free(MemoryBlock* block);

		void Read(ADDRESS address, BYTE& value);
		void Write(ADDRESS address, BYTE value);

	private:
		const size_t m_addressBits;

		MemoryListType m_memory;

		MemoryBlock* FindBlock(ADDRESS address);
		MemoryBlock* FindOverlap(const MemoryBlock* block);

		MemoryBlock* m_currBlock;
		ADDRESS m_currMin, m_currMax;
	};
}
