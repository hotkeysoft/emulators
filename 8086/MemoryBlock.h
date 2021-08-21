#pragma once

#include "Common.h"
#include "Logger.h"
#include <vector>

namespace emul
{
	const size_t MaxBlockSize = 65536;
	enum class MemoryType { RAM, ROM };

	class MemoryBlock : public Logger
	{
	public:
		MemoryBlock(ADDRESS baseAddress, size_t size, MemoryType type = MemoryType::RAM);
		MemoryBlock(ADDRESS baseAddress, const std::vector<BYTE>data, MemoryType type = MemoryType::RAM);
		MemoryBlock(const MemoryBlock& block);

		virtual ~MemoryBlock();

		void Clear(BYTE filler = 0xFF);

		ADDRESS GetBaseAddress() const { return m_baseAddress; };
		size_t GetSize() const { return m_size; };
		MemoryType GetType() const { return m_type; };

		bool LoadBinary(const char* file);

		virtual BYTE read(ADDRESS address);
		virtual BYTE* getPtr8(ADDRESS address);
		virtual WORD* getPtr16(ADDRESS address);
		virtual void write(ADDRESS address, char data);

	protected:
		ADDRESS m_baseAddress;
		size_t m_size;
		MemoryType m_type;

		BYTE* m_data;

		BYTE m_invalid;
	};
}