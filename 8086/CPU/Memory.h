#pragma once

#include "MemoryBlock.h"
#include "../Common.h"
#include <Logger.h>
#include <vector>
#include <tuple>

namespace emul
{
	struct MemorySlot
	{
		MemoryBlock* block = nullptr;
		ADDRESS base = 0;
	};

	class Memory : public Logger
	{
	public:
		Memory(size_t addressBits);
		virtual ~Memory();

		bool Allocate(MemoryBlock* block, ADDRESS base);
		bool Free(MemoryBlock* block);

		bool MapWindow(ADDRESS source, ADDRESS window, DWORD len);

		BYTE Read8(ADDRESS address) const;
		WORD Read16(ADDRESS address) const;
		void Write8(ADDRESS address, BYTE value);

		void Dump(ADDRESS start, DWORD len, const char* outFile);

		BYTE* GetPtr8(ADDRESS address) const;

		bool LoadBinary(const char* file, ADDRESS baseAddress);

	private:
		static WORD s_uninitialized;

		const size_t m_addressBits;

		std::vector<MemorySlot> m_memory;
	};
}
