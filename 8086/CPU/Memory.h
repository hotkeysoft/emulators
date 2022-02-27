#pragma once

#include "../Serializable.h"
#include "MemoryBlock.h"
#include "../Common.h"
#include <Logger.h>
#include <vector>
#include <set>

namespace emul
{
	struct MemorySlot
	{
		MemoryBlock* block = nullptr;
		ADDRESS base = 0;
	};

	class Memory : public Logger, public Serializable
	{
	public:
		Memory(size_t addressBits);
		virtual ~Memory();

		bool Allocate(MemoryBlock* block, ADDRESS base, DWORD len = (DWORD)-1);
		bool Free(MemoryBlock* block);

		bool MapWindow(ADDRESS source, ADDRESS window, DWORD len);

		BYTE Read8(ADDRESS address) const;
		WORD Read16(ADDRESS address) const;
		void Write8(ADDRESS address, BYTE value);

		void Clear(BYTE filler = 0);

		void Dump(ADDRESS start, DWORD len, const char* outFile);

		bool LoadBinary(const char* file, ADDRESS baseAddress);

		virtual void Serialize(json& to);
		virtual void Deserialize(json& from);

	protected:
		MemoryBlock* FindBlock(const char* id) const;

	private:
		static WORD s_uninitialized;

		const size_t m_addressBits;

		std::vector<MemorySlot> m_memory;
		std::set<MemoryBlock*> m_blocks;
	};
}
