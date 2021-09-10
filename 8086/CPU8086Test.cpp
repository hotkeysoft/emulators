#include "CPU8086Test.h"
#include <assert.h>

namespace emul
{
	WORD rawAdd8(BYTE&, const BYTE&, bool);
	WORD rawSub8(BYTE&, const BYTE&, bool);
	WORD rawCmp8(BYTE&, const BYTE&, bool);
	WORD rawAdc8(BYTE&, const BYTE&, bool c);
	WORD rawSbb8(BYTE&, const BYTE&, bool b);

	WORD rawAnd8(BYTE&, const BYTE&, bool);
	WORD rawOr8(BYTE&, const BYTE&, bool);
	WORD rawXor8(BYTE&, const BYTE&, bool);
	WORD rawTest8(BYTE&, const BYTE&, bool);

	DWORD rawAdd16(WORD&, const WORD&, bool);
	DWORD rawSub16(WORD&, const WORD&, bool);
	DWORD rawCmp16(WORD&, const WORD&, bool);
	DWORD rawAdc16(WORD&, const WORD&, bool c);
	DWORD rawSbb16(WORD&, const WORD&, bool b);

	DWORD rawAnd16(WORD&, const WORD&, bool);
	DWORD rawOr16(WORD&, const WORD&, bool);
	DWORD rawXor16(WORD&, const WORD&, bool);
	DWORD rawTest16(WORD&, const WORD&, bool);

	// Arithmetic functions internal tests
	#define ATEST8(F,A,B,R,O,S,Z,C) { flags=0; d8=A; s8=B; Arithmetic8(sd8, F); assert(*sd8.dest==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define ATEST16(F,A,B,R,O,S,Z,C) { flags=0; d16=A; s16=B; Arithmetic16(sd16, F); assert(*sd16.dest==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define ATEST8C(F,A,B,R,O,S,Z,C) { flags=FLAG_C; d8=A; s8=B; Arithmetic8(sd8, F); assert(*sd8.dest==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define ATEST16C(F,A,B,R,O,S,Z,C) { flags=FLAG_C; d16=A; s16=B; Arithmetic16(sd16, F); assert(*sd16.dest==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }

	void CPU8086Test::TestArithmetic()
	{
		// Arithmetic8
		{
			BYTE s8;
			BYTE d8;
			SourceDest8 sd8;
			sd8.source = &s8;
			sd8.dest = &d8;

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawAdd8, 0x7F, 0x00, 0x7F, false, false, false, false);
			ATEST8(rawAdd8, 0xFF, 0x7F, 0x7E, false, false, false, true);
			ATEST8(rawAdd8, 0x00, 0x00, 0x00, false, false, true,  false);
			ATEST8(rawAdd8, 0xFF, 0x01, 0x00, false, false, true,  true);
			ATEST8(rawAdd8, 0xFF, 0x00, 0xFF, false, true,  false, false);
			ATEST8(rawAdd8, 0xFF, 0xFF, 0xFE, false, true,  false, true);
			ATEST8(rawAdd8, 0xFF, 0x80, 0x7F, true,  false, false, true);
			ATEST8(rawAdd8, 0x80, 0x80, 0x00, true,  false, true,  true);
			ATEST8(rawAdd8, 0x7F, 0x7F, 0xFE, true,  true,  false, false);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawAdc8, 0x7F, 0x00, 0x7F, false, false, false, false);
			ATEST8(rawAdc8, 0xFF, 0x7F, 0x7E, false, false, false, true);
			ATEST8(rawAdc8, 0x00, 0x00, 0x00, false, false, true,  false);
			ATEST8(rawAdc8, 0xFF, 0x01, 0x00, false, false, true,  true);
			ATEST8(rawAdc8, 0xFF, 0x00, 0xFF, false, true,  false, false);
			ATEST8(rawAdc8, 0xFF, 0xFF, 0xFE, false, true,  false, true);
			ATEST8(rawAdc8, 0xFF, 0x80, 0x7F, true,  false, false, true);
			ATEST8(rawAdc8, 0x80, 0x80, 0x00, true,  false, true,  true);
			ATEST8(rawAdc8, 0x7F, 0x7F, 0xFE, true,  true,  false, false);

			//      op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8C(rawAdc8, 0x7E, 0x00, 0x7F, false, false, false, false);
			ATEST8C(rawAdc8, 0xFE, 0x7F, 0x7E, false, false, false, true);
			ATEST8C(rawAdc8, 0x00, 0x00, 0x01, false, false, false, false);
			ATEST8C(rawAdc8, 0xFE, 0x01, 0x00, false, false, true,  true);
			ATEST8C(rawAdc8, 0xFE, 0x00, 0xFF, false, true,  false, false);
			ATEST8C(rawAdc8, 0xFE, 0xFF, 0xFE, false, true,  false, true);
			ATEST8C(rawAdc8, 0xFE, 0x80, 0x7F, true,  false, false, true);
			ATEST8C(rawAdc8, 0x80, 0x80, 0x01, true,  false, false, true);
			ATEST8C(rawAdc8, 0x7E, 0x7F, 0xFE, true,  true,  false, false);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawSub8, 0xFF, 0xFE, 0x01, false, false, false, false);
			ATEST8(rawSub8, 0x7E, 0xFF, 0x7F, false, false, false, true);
			ATEST8(rawSub8, 0xFF, 0xFF, 0x00, false, false, true,  false);
			ATEST8(rawSub8, 0xFF, 0x7F, 0x80, false, true,  false, false);
			ATEST8(rawSub8, 0xFE, 0xFF, 0xFF, false, true,  false, true);
			ATEST8(rawSub8, 0xFE, 0x7F, 0x7F, true,  false, false, false);
			ATEST8(rawSub8, 0x7F, 0xFF, 0x80, true,  true,  false, true);

			// NEG(n) === 0-n
			ATEST8(rawSub8, 0x00, 0x00, 0x00, false, false, true, false);
			ATEST8(rawSub8, 0x00, 0x01, 0xFF, false, true,  false, true);
			ATEST8(rawSub8, 0x00, 0x80, 0x80, true,  true,  false, true);
			ATEST8(rawSub8, 0x00, 0xFF, 0x01, false, false, false, true);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawSbb8, 0xFF, 0xFE, 0x01, false, false, false, false);
			ATEST8(rawSbb8, 0x7E, 0xFF, 0x7F, false, false, false, true);
			ATEST8(rawSbb8, 0xFF, 0xFF, 0x00, false, false, true,  false);
			ATEST8(rawSbb8, 0xFF, 0x7F, 0x80, false, true,  false, false);
			ATEST8(rawSbb8, 0xFE, 0xFF, 0xFF, false, true,  false, true);
			ATEST8(rawSbb8, 0xFE, 0x7F, 0x7F, true,  false, false, false);
			ATEST8(rawSbb8, 0x7F, 0xFF, 0x80, true,  true,  false, true);

			//      op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8C(rawSbb8, 0x00, 0xFF, 0x00, false, false, true,  true);
			ATEST8C(rawSbb8, 0xFF, 0xFE, 0x00, false, false, true,  false);
			ATEST8C(rawSbb8, 0x7F, 0xFF, 0x7F, false, false, false, true);
			ATEST8C(rawSbb8, 0xFF, 0xFF, 0xFF, false, true,  false, true);
			ATEST8C(rawSbb8, 0xFF, 0x7F, 0x7F, true,  false, false, false);
			ATEST8C(rawSbb8, 0x80, 0xFF, 0x80, false, true,  false, true);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawAnd8, 0xFF, 0x00, 0x00, false, false, true,  false);
			ATEST8(rawAnd8, 0xFF, 0xFF, 0xFF, false, true,  false, false);
			ATEST8(rawAnd8, 0x0F, 0x0F, 0x0F, false, false, false, false);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawOr8, 0x00, 0x00, 0x00, false, false, true,  false);
			ATEST8(rawOr8, 0xFF, 0x00, 0xFF, false, true,  false, false);
			ATEST8(rawOr8, 0xFF, 0xFF, 0xFF, false, true,  false, false);
			ATEST8(rawOr8, 0x0F, 0xF0, 0xFF, false, true,  false, false);

			//			ATEST8(rawXor8, 0xFF, 0xFF, 0x00, false, false, true, false);

		}

		// Arithmetic16
		{
			WORD s16;
			WORD d16;
			SourceDest16 sd16;
			sd16.source = &s16;
			sd16.dest = &d16;

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawAdd16, 0x7FFF, 0x0000, 0x7FFF, false, false, false, false);
			ATEST16(rawAdd16, 0xFFFF, 0x7FFF, 0x7FFE, false, false, false, true);
			ATEST16(rawAdd16, 0x0000, 0x0000, 0x0000, false, false, true,  false);
			ATEST16(rawAdd16, 0xFFFF, 0x0001, 0x0000, false, false, true,  true);
			ATEST16(rawAdd16, 0xFFFF, 0x0000, 0xFFFF, false, true,  false, false);
			ATEST16(rawAdd16, 0xFFFF, 0xFFFF, 0xFFFE, false, true,  false, true);
			ATEST16(rawAdd16, 0xFFFF, 0x8000, 0x7FFF, true,  false, false, true);
			ATEST16(rawAdd16, 0x8000, 0x8000, 0x0000, true,  false, true,  true);
			ATEST16(rawAdd16, 0x7FFF, 0x7FFF, 0xFFFE, true,  true,  false, false);

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawAdc16, 0x7FFF, 0x0000, 0x7FFF, false, false, false, false);
			ATEST16(rawAdc16, 0xFFFF, 0x7FFF, 0x7FFE, false, false, false, true);
			ATEST16(rawAdc16, 0x0000, 0x0000, 0x0000, false, false, true,  false);
			ATEST16(rawAdc16, 0xFFFF, 0x0001, 0x0000, false, false, true,  true);
			ATEST16(rawAdc16, 0xFFFF, 0x0000, 0xFFFF, false, true,  false, false);
			ATEST16(rawAdc16, 0xFFFF, 0xFFFF, 0xFFFE, false, true,  false, true);
			ATEST16(rawAdc16, 0xFFFF, 0x8000, 0x7FFF, true,  false, false, true);
			ATEST16(rawAdc16, 0x8000, 0x8000, 0x0000, true,  false, true,  true);
			ATEST16(rawAdc16, 0x7FFF, 0x7FFF, 0xFFFE, true,  true,  false, false);

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawSub16, 0xFFFF, 0xFFFE, 0x0001, false, false, false, false);
			ATEST16(rawSub16, 0x7FFE, 0xFFFF, 0x7FFF, false, false, false, true);
			ATEST16(rawSub16, 0xFFFF, 0xFFFF, 0x0000, false, false, true,  false);
			ATEST16(rawSub16, 0xFFFF, 0x7FFF, 0x8000, false, true,  false, false);
			ATEST16(rawSub16, 0xFFFE, 0xFFFF, 0xFFFF, false, true,  false, true);
			ATEST16(rawSub16, 0xFFFE, 0x7FFF, 0x7FFF, true,  false, false, false);
			ATEST16(rawSub16, 0x7FFF, 0xFFFF, 0x8000, true,  true,  false, true);

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawSbb16, 0xFFFF, 0xFFFE, 0x0001, false, false, false, false);
			ATEST16(rawSbb16, 0x7FFE, 0xFFFF, 0x7FFF, false, false, false, true);
			ATEST16(rawSbb16, 0xFFFF, 0xFFFF, 0x0000, false, false, true,  false);
			ATEST16(rawSbb16, 0xFFFF, 0x7FFF, 0x8000, false, true,  false, false);
			ATEST16(rawSbb16, 0xFFFE, 0xFFFF, 0xFFFF, false, true,  false, true);
			ATEST16(rawSbb16, 0xFFFE, 0x7FFF, 0x7FFF, true,  false, false, false);
			ATEST16(rawSbb16, 0x7FFF, 0xFFFF, 0x8000, true,  true,  false, true);
		}
	}

	#define SHIFTROTTEST8(CLR,F,N,R,O,S,Z,C) { flags=(CLR?0:flags); SHIFTROT8(F, N); assert(regA.hl.l == R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define SHIFTROTTEST8noO(CLR,F,N,R,S,Z,C) { flags=(CLR?0:flags); SHIFTROT8(F, N); assert(regA.hl.l == R); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }

	#define SHIFTROTTEST16(CLR,F,N,R,O,S,Z,C) { flags=(CLR?0:flags); SHIFTROT16(F, N); assert(regA.x == R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }


	void CPU8086Test::TestShiftRotate()
	{
		// SHIFT/ROTATE8
		{
			const BYTE SHL = 0xE0, SHR = 0xE8, SAR = 0xF8, ROL = 0xC0, ROR = 0xC8, RCL = 0xD0, RCR = 0xD8;

			// N=1

			// SHL
			regA.hl.l = 0x01;
			//            clrf  op   n  RES   OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(true, SHL, 1, 0x02, false, false, false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x04, false, false, false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x08, false, false, false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x10, false, false, false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x20, false, false, false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x40, false, false, false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x80, true,  true,  false, false);
			SHIFTROTTEST8(true, SHL, 1, 0x00, true,  false, true, true);
			SHIFTROTTEST8(true, SHL, 1, 0x00, false, false, true, false);

			// SHR
			regA.hl.l = 0x80;
			//            clrf  op   n  RES   OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(true, SHR, 1, 0x40, true,  false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x20, false, false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x10, false, false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x08, false, false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x04, false, false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x02, false, false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x01, false, false, false, false);
			SHIFTROTTEST8(true, SHR, 1, 0x00, false, false, true,  true);
			SHIFTROTTEST8(true, SHR, 1, 0x00, false, false, true,  false);

			// SAR (negative)
			regA.hl.l = 0b10000000;
			//            clrf  op   n  RES         OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(true, SAR, 1, 0b11000000, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11100000, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11110000, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11111000, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11111100, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11111110, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11111111, false, true,  false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b11111111, false, true,  false, true);

			// SAR (positive)
			regA.hl.l = 0b01000000;
			//            clrf  op   n  RES         OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(true, SAR, 1, 0b00100000, false, false, false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b00010000, false, false, false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b00001000, false, false, false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b00000100, false, false, false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b00000010, false, false, false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b00000001, false, false, false, false);
			SHIFTROTTEST8(true, SAR, 1, 0b00000000, false, false, true,  true);
			SHIFTROTTEST8(true, SAR, 1, 0b00000000, false, false, true,  false);

			// ROL
			regA.hl.l = 0x01;
			//            clrf  op   n  RES   OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(true, ROL, 1, 0x02, false, false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x04, false, false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x08, false, false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x10, false, false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x20, false, false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x40, false, false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x80, true,  false, false, false);
			SHIFTROTTEST8(true, ROL, 1, 0x01, true,  false, false, true);
			SHIFTROTTEST8(true, ROL, 1, 0x02, false, false, false, false);

			// ROR
			regA.hl.l = 0x80;
			//            clrf  op   n  RES   OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(true, ROR, 1, 0x40, true,  false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x20, false, false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x10, false, false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x08, false, false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x04, false, false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x02, false, false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x01, false, false, false, false);
			SHIFTROTTEST8(true, ROR, 1, 0x80, true,  false, false, true);
			SHIFTROTTEST8(true, ROR, 1, 0x40, true,  false, false, false);

			// RCL
			regA.hl.l = 0x01; flags = 0;
			//            clrf   op   n  RES   OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(false, RCL, 1, 0x02, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x04, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x08, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x10, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x20, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x40, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x80, true,  false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x00, true,  false, false, true);
			SHIFTROTTEST8(false, RCL, 1, 0x01, false, false, false, false);
			SHIFTROTTEST8(false, RCL, 1, 0x02, false, false, false, false);

			// RCR
			regA.hl.l = 0x80; flags = 0;
			//            clrf   op   n  RES   OFLOW  SIGN   ZERO   CARRY
			SHIFTROTTEST8(false, RCR, 1, 0x40, true,  false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x20, false, false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x10, false, false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x08, false, false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x04, false, false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x02, false, false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x01, false, false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x00, false, false, false,  true);
			SHIFTROTTEST8(false, RCR, 1, 0x80, true,  false, false, false);
			SHIFTROTTEST8(false, RCR, 1, 0x40, true,  false, false, false);

			// N=0

			//                                    clrf  op   n  RES         OFLOW  SIGN   ZERO   CARRY

			// SHL
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, SHL, 0, 0b10101010, false, true,  false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, SHL, 0, 0b01010101, false, false, false, false);

			// SHR
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, SHR, 0, 0b10101010, false, true,  false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, SHR, 0, 0b01010101, false, false, false, false);

			// SAR
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, SAR, 0, 0b10101010, false, true,  false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, SAR, 0, 0b01010101, false, false, false, false);

			// ROL
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, ROL, 0, 0b10101010, false, false, false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, ROL, 0, 0b01010101, false, false, false, false);

			// ROR
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, ROR, 0, 0b10101010, false, false, false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, ROR, 0, 0b01010101, false, false, false, false);

			// RCL
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, RCL, 0, 0b10101010, false, false,  false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, RCL, 0, 0b01010101, false, false, false, false);

			// RCR
			regA.hl.l = 0b10101010; SHIFTROTTEST8(true, RCR, 0, 0b10101010, false, false,  false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8(true, RCR, 0, 0b01010101, false, false, false, false);

			//                                       clrf  op   n  RES         SIGN   ZERO   CARRY

			// N=7 (OF Undefined for n>1)
			// SHL
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, SHL, 7, 0b00000000, false, true,  true);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, SHL, 7, 0b10000000, true,  false, false);

			// SHR
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, SHR, 7, 0b00000001, false, false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, SHR, 7, 0b00000000, false, true,  true);

			// SAR
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, SAR, 7, 0b11111111, true,  false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, SAR, 7, 0b00000000, false, true,  true);

			// ROL
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, ROL, 7, 0b01010101, false, false, true);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, ROL, 7, 0b10101010, false, false, false);

			// ROR
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, ROR, 7, 0b01010101, false, false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, ROR, 7, 0b10101010, false, false, true);

			// RCL
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, RCL, 7, 0b00101010, false, false, true);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, RCL, 7, 0b10010101, false, false, false);
			
			// RCR
			regA.hl.l = 0b10101010; SHIFTROTTEST8noO(true, RCR, 7, 0b10101001, false, false, false);
			regA.hl.l = 0b01010101; SHIFTROTTEST8noO(true, RCR, 7, 0b01010100, false, false, true);

		}

		// SHIFT/ROTATE16
		{
			const BYTE SHL = 0xE0, SHR = 0xE8, SAR = 0xF8, ROL = 0xC0, ROR = 0xC8, RCL = 0xD0, RCR = 0xD8;

			//                              clrf  op   n    RES     OFLOW  SIGN   ZERO   CARRY
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 0,   0xC8A7, false, false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 1,   0x914E, false, false, false, true);
			
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 15,  0xB229, false, false, false, true);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 16,  0x6453, true,  false, false, true);

			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 17,  0xC8A7, true,  false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 18,  0x914E, false, false, false, true);
			
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 29,  0x7645, false, false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 30,  0xEC8A, true,  false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 31,  0xD914, false, false, false, true);

			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 32,  0xC8A7, false, false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 33,  0x914E, false, false, false, true);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 34,  0x229D, true, false, false, true);

			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 253, 0x7645, false, false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 254, 0xEC8A, true,  false, false, false);
			regA.x = 0xC8A7; SHIFTROTTEST16(true, RCL, 255, 0xD914, false, false, false, true);
		}
	}
}