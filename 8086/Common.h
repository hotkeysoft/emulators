#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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
	extern size_t g_ticks;

	typedef uint8_t BYTE;
	typedef uint16_t WORD;
	typedef uint32_t DWORD;

	typedef uint32_t ADDRESS;

	inline ADDRESS S2A(const WORD segment, const WORD offset = 0)
	{
		return ((segment << 4) + offset)&0xFFFFF;
	}

	inline size_t GetMaxAddress(const size_t addressBits)
	{
		return ((uint64_t)1 << addressBits) - 1;
	}

	inline bool CheckAddressRange(const ADDRESS address, const size_t addressBits)
	{
		return (address <= GetMaxAddress(addressBits));
	}

	inline bool GetLSB(const BYTE b) { return b & 1; }
	inline bool GetMSB(const BYTE b) { return b & 128; }

	inline bool GetLSB(const WORD b) { return b & 1; }
	inline bool GetMSB(const WORD b) { return b & 32768; }

	inline BYTE GetLByte(const WORD w) { return BYTE(w); };
	inline BYTE GetHByte(const WORD w) { return BYTE(w >> 8); };

	inline WORD SetLByte(WORD& out, const BYTE low) { out &= 0xFF00; out |= low; return out; }
	inline WORD SetHByte(WORD& out, const BYTE hi) { out &= 0x00FF; out |= (hi << 8); return out; }

	inline WORD GetLWord(const DWORD d) { return WORD(d & 0x0000FFFF); };
	inline WORD GetHWord(const DWORD d) { return WORD(d >> 16); };

	inline WORD MakeWord(const BYTE h, const BYTE l) { return (((WORD)h) << 8) + l; };
	inline DWORD MakeDword(const WORD h, const WORD l) { return (((DWORD)h) << 16) + l; }

	inline BYTE LowestSetBit(const BYTE b) { return b & (-b); }
	inline bool IsPowerOf2(const DWORD d) { return d && !(d & (d - 1)); }
	inline bool IsPowerOf2(const WORD w) { return w && !(w & (w - 1)); }
	inline bool IsPowerOf2(const BYTE b) { return b && !(b & (b - 1)); }
	inline BYTE LogBase2(BYTE b) { BYTE r = 0; while (b >>= 1) r++; return r; }

	inline WORD Widen(const BYTE b)
	{
		WORD w = b;
		if (GetMSB(b))
		{
			w |= 0xFF00;
		}
		return w;
	}
	inline DWORD Widen(const WORD w)
	{
		DWORD dw = w;
		if (GetMSB(w))
		{
			dw |= 0xFFFF0000;
		}
		return dw;
	}

	inline bool GetBit(const BYTE b, const BYTE bit)
	{
		return b & (1 << bit);
	}

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
