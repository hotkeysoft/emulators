#pragma once
#include <cstdint>

namespace emul
{
	typedef uint8_t BYTE;
	typedef uint16_t WORD;
	typedef uint32_t DWORD;

	typedef uint32_t ADDRESS;

	inline size_t GetMaxAddress(size_t addressBits)
	{
		return ((uint64_t)1 << addressBits) - 1;
	}

	inline bool CheckAddressRange(ADDRESS address, size_t addressBits)
	{
		return (address <= GetMaxAddress(addressBits));
	}
}
