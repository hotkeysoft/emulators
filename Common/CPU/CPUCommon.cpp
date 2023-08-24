#include "stdafx.h"

#include <CPU/CPUCommon.h>

namespace emul
{
	// https://graphics.stanford.edu/~seander/bithacks.html#BitReverseTable
	static constexpr unsigned char BitReverseTable256[256] =
	{
	#   define R2(n)    n,     n + 2*64,     n + 1*64,     n + 3*64
	#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
	#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
		R6(0), R6(2), R6(1), R6(3)
	};

	WORD ReverseBits(WORD in)
	{
		WORD out;
		BYTE* p = (BYTE*)&in;
		SetHByte(out, BitReverseTable256[GetLByte(in)]);
		SetLByte(out, BitReverseTable256[GetHByte(in)]);
		return out;
	}
}