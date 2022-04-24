#pragma once

#include "../Serializable.h"
#include "MemoryBlock.h"
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
		Memory();
		virtual ~Memory();

		void Init(size_t addressBits);

		bool Allocate(MemoryBlock* block, ADDRESS base, DWORD len = (DWORD)-1);
		bool Free(MemoryBlock* block);

		bool MapWindow(ADDRESS source, ADDRESS window, DWORD len);

		BYTE Read8(ADDRESS address) const;
		WORD Read16(ADDRESS address) const;
		void Write8(ADDRESS address, BYTE value);
		void Write16(ADDRESS address, WORD value);

		void Clear(BYTE filler = 0);

		void Dump(ADDRESS start, DWORD len, const char* outFile);

		bool LoadBinary(const char* file, ADDRESS baseAddress);

		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

		void SetAddressMask(ADDRESS mask) { m_addressMask = mask; }
		ADDRESS GetAddressMask() const { return m_addressMask; }

	protected:
		MemoryBlock* FindBlock(const char* id) const;

		using MemoryBlocks = std::vector<std::tuple<ADDRESS, MemoryBlock>>;
		void AddROMBlock(MemoryBlocks& out, ADDRESS addr, std::vector<BYTE>& buffer) const;

	private:
		size_t m_addressBits = 0;
		ADDRESS m_addressMask = 0;

		static WORD s_uninitialized;

		std::vector<MemorySlot> m_memory;
		std::set<MemoryBlock*> m_blocks;
	};
}
