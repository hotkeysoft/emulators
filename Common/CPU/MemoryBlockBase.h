#pragma once

#include <CPU/CPUCommon.h>
#include <vector>

namespace emul
{
	enum class MemoryType { RAM, ROM, IO };
	enum class OddEven { ODD = 1, EVEN = 0 };

	class MemoryBlockBase : public Logger
	{
	public:
		MemoryBlockBase(const char* id, DWORD size, MemoryType type = MemoryType::RAM);
		virtual ~MemoryBlockBase() {}

		const std::string& GetId() const { return m_id; }
		DWORD GetSize() const { return m_size; };
		MemoryType GetType() const { return m_type; };

		virtual void Clear(BYTE filler = 0) {};

		virtual BYTE read(ADDRESS offset) = 0;
		virtual void write(ADDRESS offset, BYTE data) = 0;

	protected:
		DWORD RoundBlockSize(DWORD size);

		std::string m_id;
		DWORD m_size = 0;
		MemoryType m_type;
	};
}