#pragma once

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

#define PRINTF_BIN_PATTERN_INT32 \
    PRINTF_BIN_PATTERN_INT16             PRINTF_BIN_PATTERN_INT16
#define PRINTF_BYTE_TO_BIN_INT32(i) \
    PRINTF_BYTE_TO_BIN_INT16((i) >> 16), PRINTF_BYTE_TO_BIN_INT16(i)

namespace emul
{
	extern size_t g_ticks;

	typedef uint8_t BYTE;
	typedef uint16_t WORD;
	typedef uint32_t DWORD;

	typedef uint32_t ADDRESS;

	inline ADDRESS S2A(const WORD segment, const WORD offset = 0)
	{
		return ((segment << 4) + offset);
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

	inline void SetLByte(WORD& out, const BYTE low) { out &= 0xFF00; out |= low; }
	inline void SetHByte(WORD& out, const BYTE hi) { out &= 0x00FF; out |= (hi << 8); }

	inline WORD GetLWord(const DWORD d) { return WORD(d & 0x0000FFFF); };
	inline WORD GetHWord(const DWORD d) { return WORD(d >> 16); };

	inline void SetLWord(DWORD& out, const WORD low) { out &= 0xFFFF0000; out |= low; }
	inline void SetHWord(DWORD& out, const WORD hi) { out &= 0x0000FFFF; out |= (hi << 16); }

	inline WORD MakeWord(const BYTE h, const BYTE l) { return (((WORD)h) << 8) + l; };
	inline DWORD MakeDword(const WORD h, const WORD l) { return (((DWORD)h) << 16) + l; }

	inline BYTE LowestSetBit(const BYTE b) { return b & (-b); }
	inline bool IsPowerOf2(const DWORD d) { return d && !(d & (d - 1)); }
	inline bool IsPowerOf2(const WORD w) { return w && !(w & (w - 1)); }
	inline bool IsPowerOf2(const BYTE b) { return b && !(b & (b - 1)); }
	inline BYTE LogBase2(BYTE b) { BYTE r = 0; while (b >>= 1) r++; return r; }

	inline WORD Widen(const BYTE b)
	{
		return WORD((int16_t)((int8_t)b));
	}

	inline DWORD Widen(const WORD w)
	{
		return DWORD((int32_t)((int16_t)w));
	}

	inline bool GetBit(const BYTE b, const BYTE bit)
	{
		return b & (1 << bit);
	}

	inline bool GetBit(const WORD b, const BYTE bit)
	{
		return b & (1 << bit);
	}

	inline bool GetBit(const DWORD b, const BYTE bit)
	{
		return b & (1 << bit);
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
	inline void SetBitMask(BYTE& out, const BYTE mask, bool state)
	{
		out ^= (-(int8_t)state ^ out) & mask;
	}

	inline void SetBitMask(WORD& out, const WORD mask, bool state)
	{
		out ^= (-(int16_t)state ^ out) & mask;
	}

	inline void SetBitMask(DWORD& out, const DWORD mask, bool state)
	{
		out ^= (-(int32_t)state ^ out) & mask;
	}

	inline void SetBit(BYTE& out, const BYTE bit, bool state)
	{
		out ^= (-(int8_t)state ^ out) & (1 << bit);
	}

	inline void SetBit(WORD& out, const BYTE bit, bool state)
	{
		out ^= (-(int16_t)state ^ out) & (1 << bit);
	}

	inline void SetBit(DWORD& out, const BYTE bit, bool state)
	{
		out ^= (-(int32_t)state ^ out) & (1 << bit);
	}

	// https://graphics.stanford.edu/~seander/bithacks.html#ParityParallel
	inline bool IsParityOdd(BYTE b)
	{
		b ^= b >> 4;
		b &= 0xf;
		return (0x6996 >> b) & 1;
	}
	inline bool IsParityEven(BYTE b) { return !IsParityOdd(b); }

	// Doesn't work if a and b point to same memory location
	inline void Swap(BYTE& a, BYTE& b)
	{
		a ^= b;
		b ^= a;
		a ^= b;
	}

}
