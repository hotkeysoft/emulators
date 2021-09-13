#pragma once

#include "Common.h"
#include "Logger.h"
#include <vector>

namespace emul
{
	enum class MemoryType { RAM, ROM };

	class MemoryBlock : public Logger
	{
	public:
		MemoryBlock(const char* id, DWORD size, MemoryType type = MemoryType::RAM);
		MemoryBlock(const char* id, const std::vector<BYTE>data, MemoryType type = MemoryType::RAM);
		MemoryBlock(const MemoryBlock& block);

		virtual ~MemoryBlock();

		void Clear(BYTE filler = 0xFF);

		const std::string& GetId() const { return m_id; }
		DWORD GetSize() const { return m_size; };
		MemoryType GetType() const { return m_type; };

		bool LoadBinary(const char* file, WORD offset = 0);

		bool Dump(ADDRESS offset, DWORD len, const char* outFile);

		virtual BYTE read(ADDRESS offset);
		virtual BYTE* getPtr8(ADDRESS offset);
		virtual WORD* getPtr16(ADDRESS offset);
		virtual void write(ADDRESS offset, char data);

	protected:
		DWORD RoundBlockSize(DWORD size);

		std::string m_id;
		DWORD m_size;
		MemoryType m_type;

		BYTE* m_data;
	};
}