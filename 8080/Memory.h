#pragma once

#include "MemoryBlock.h"
#include "Common.h"
#include "Logger.h"
#include <list>

typedef std::list<MemoryBlock *> MemoryListType;

class Memory : public Logger
{
public:
	Memory();
	virtual ~Memory();

	bool Allocate(MemoryBlock *block);
	bool Free(MemoryBlock *block);

	void Read(WORD address, BYTE &value);
	void Write(WORD address, BYTE value);

private:
	MemoryListType m_memory;

	MemoryBlock *FindBlock(WORD address);
	MemoryBlock *FindOverlap(const MemoryBlock *block);

	MemoryBlock *m_currBlock;
	WORD m_currMin, m_currMax;
};
