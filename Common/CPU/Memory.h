#pragma once

#include <Serializable.h>
#include <CPU/MemoryBlockBase.h>
#include <CPU/MemoryBlock.h>
#include <vector>
#include <set>

namespace emul
{
	struct MemorySlot
	{
		MemoryBlockBase* block = nullptr;
		ADDRESS base = 0;
	};

	class Memory : public Logger, public Serializable
	{
	public:
		// Recommended blockGranularity:
		//  - 1024 for 16 bit memory addresses
		//  - 4096 for 20-24 bit memory addresses
		Memory(WORD blockGranularity);
		virtual ~Memory();

		void Init(size_t addressBits);

		bool Allocate(MemoryBlockBase* block, ADDRESS base, DWORD len = (DWORD)-1);
		bool Free(MemoryBlockBase* block);

		bool MapWindow(ADDRESS source, ADDRESS window, DWORD len);

		const MemorySlot& FindBlock(ADDRESS address) const { return m_memory[address / m_blockGranularity]; }

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
		MemoryBlockBase* FindBlock(const char* id) const;

		using MemoryBlocks = std::vector<std::tuple<ADDRESS, MemoryBlock>>;
		void AddROMBlock(MemoryBlocks& out, ADDRESS addr, std::vector<BYTE>& buffer) const;

	private:
		size_t m_addressBits = 0;
		ADDRESS m_addressMask = 0;

		const WORD m_blockGranularity;

		static WORD s_uninitialized;

		std::vector<MemorySlot> m_memory;
		std::set<MemoryBlockBase*> m_blocks;
	};
}
