#pragma once
#include <cstdint>

#define PRINTF_BIN_PATTERN_INT8 "%c%c%c%c%c%c%c%c"
#define PRINTF_BYTE_TO_BIN_INT8(i)    \
    (((i) & 0x80ll) ? '1' : '0'), \
    (((i) & 0x40ll) ? '1' : '0'), \
    (((i) & 0x20ll) ? '1' : '0'), \
    (((i) & 0x10ll) ? '1' : '0'), \
    (((i) & 0x08ll) ? '1' : '0'), \
    (((i) & 0x04ll) ? '1' : '0'), \
    (((i) & 0x02ll) ? '1' : '0'), \
    (((i) & 0x01ll) ? '1' : '0')

#define PRINTF_BIN_PATTERN_INT16 \
    PRINTF_BIN_PATTERN_INT8              PRINTF_BIN_PATTERN_INT8
#define PRINTF_BYTE_TO_BIN_INT16(i) \
    PRINTF_BYTE_TO_BIN_INT8((i) >> 8),   PRINTF_BYTE_TO_BIN_INT8(i)

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
