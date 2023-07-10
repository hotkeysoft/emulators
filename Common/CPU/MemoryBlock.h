#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/MemoryBlockBase.h>
#include <vector>

namespace emul
{
	class MemoryBlock : public MemoryBlockBase
	{
	public:
		using RawBlock = std::vector<BYTE>;

		MemoryBlock(const char* id, DWORD size, MemoryType type = MemoryType::RAM);
		MemoryBlock(const char* id, const RawBlock& data, MemoryType type = MemoryType::RAM);
		MemoryBlock(const char* id, MemoryType type = MemoryType::RAM);
		MemoryBlock(const MemoryBlock& block);

		virtual ~MemoryBlock();

		virtual void Clear(BYTE filler = 0) override;

		void Alloc(DWORD size);

		virtual bool LoadFromFile(const char* file, WORD offset = 0);
		virtual bool LoadOddEven(const char* file, OddEven oddEven = OddEven::ODD);

		// Fills a block of memory with raw block of bytes. Writing in ROM is allowed
		virtual bool Fill(ADDRESS offset, const RawBlock& data);

		virtual bool Dump(const char* outFile) const { return Dump(0, m_size, outFile); }
		virtual bool Dump(ADDRESS offset, DWORD len, const char* outFile) const;

		virtual BYTE read(ADDRESS offset) const override;
		virtual void write(ADDRESS offset, BYTE data) override;

		const BYTE* getPtr() const { return m_data; }

	protected:
		BYTE* m_data = nullptr;
	};
}