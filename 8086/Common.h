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

	inline ADDRESS S2A(WORD segment, WORD offset = 0)
	{
		return (segment << 4) + offset;
	}

	inline size_t GetMaxAddress(size_t addressBits)
	{
		return ((uint64_t)1 << addressBits) - 1;
	}

	inline bool CheckAddressRange(ADDRESS address, size_t addressBits)
	{
		return (address <= GetMaxAddress(addressBits));
	}

	inline BYTE GetLowByte(WORD in) { return in & 0xFF; }
	inline BYTE GetHiByte(WORD in) { return in >> 8; }
	inline WORD SetLowByte(WORD& out, const BYTE low) { out &= 0xFF00; out |= low; return out; }
	inline WORD SetHiByte(WORD& out, const BYTE hi) { out &= 0x00FF; out |= (hi << 8); return out; }
	inline BYTE SetBit(BYTE& out, const BYTE bit, bool state)
	{
		if (state)
		{
			out |= (1 << bit);
		}
		else
		{
			out &= ~(1 << bit);
		}
		return out;
	}
}
