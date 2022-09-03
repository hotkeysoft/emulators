#pragma once

#include <Common.h>
#include <vector>

namespace emul
{
	enum class MemoryType { RAM, ROM };
	enum class OddEven { ODD = 1, EVEN = 0 };

	class MemoryBlock : public Logger
	{
	public:
		MemoryBlock(const char* id, DWORD size, MemoryType type = MemoryType::RAM);
		MemoryBlock(const char* id, const std::vector<BYTE>data, MemoryType type = MemoryType::RAM);
		MemoryBlock(const char* id, MemoryType type = MemoryType::RAM);
		MemoryBlock(const MemoryBlock& block);

		virtual ~MemoryBlock();

		virtual void Clear(BYTE filler = 0);

		const std::string& GetId() const { return m_id; }
		DWORD GetSize() const { return m_size; };
		MemoryType GetType() const { return m_type; };

		void Alloc(DWORD size);

		virtual bool LoadFromFile(const char* file, WORD offset = 0);
		virtual bool LoadOddEven(const char* file, OddEven oddEven = OddEven::ODD);

		virtual bool Dump(const char* outFile) const { return Dump(0, m_size, outFile); }
		virtual bool Dump(ADDRESS offset, DWORD len, const char* outFile) const;

		virtual BYTE read(ADDRESS offset);
		virtual void write(ADDRESS offset, BYTE data);

		const BYTE* getPtr() const { return m_data; }

	protected:
		DWORD RoundBlockSize(DWORD size);

		std::string m_id;
		DWORD m_size = 0;
		MemoryType m_type;

		BYTE* m_data = nullptr;
	};
}