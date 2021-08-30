#include "CPU8086Test.h"
#include <assert.h>

namespace emul
{
	WORD rawAdd8(SourceDest8 sd, bool);
	WORD rawSub8(SourceDest8 sd, bool);
	WORD rawCmp8(SourceDest8 sd, bool);
	WORD rawAdc8(SourceDest8 sd, bool c);
	WORD rawSbb8(SourceDest8 sd, bool b);

	WORD rawAnd8(SourceDest8 sd, bool);
	WORD rawOr8(SourceDest8 sd, bool);
	WORD rawXor8(SourceDest8 sd, bool);
	WORD rawTest8(SourceDest8 sd, bool);

	DWORD rawAdd16(SourceDest16 sd, bool);
	DWORD rawSub16(SourceDest16 sd, bool);
	DWORD rawCmp16(SourceDest16 sd, bool);
	DWORD rawAdc16(SourceDest16 sd, bool c);
	DWORD rawSbb16(SourceDest16 sd, bool b);

	DWORD rawAnd16(SourceDest16 sd, bool);
	DWORD rawOr16(SourceDest16 sd, bool);
	DWORD rawXor16(SourceDest16 sd, bool);
	DWORD rawTest16(SourceDest16 sd, bool);

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
			ATEST8(rawAdd8, 0x00, 0x00, 0x00, false, false, true, false);
			ATEST8(rawAdd8, 0xFF, 0x01, 0x00, false, false, true, true);
			ATEST8(rawAdd8, 0xFF, 0x00, 0xFF, false, true, false, false);
			ATEST8(rawAdd8, 0xFF, 0xFF, 0xFE, false, true, false, true);
			ATEST8(rawAdd8, 0xFF, 0x80, 0x7F, true, false, false, true);
			ATEST8(rawAdd8, 0x80, 0x80, 0x00, true, false, true, true);
			ATEST8(rawAdd8, 0x7F, 0x7F, 0xFE, true, true, false, false);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawAdc8, 0x7F, 0x00, 0x7F, false, false, false, false);
			ATEST8(rawAdc8, 0xFF, 0x7F, 0x7E, false, false, false, true);
			ATEST8(rawAdc8, 0x00, 0x00, 0x00, false, false, true, false);
			ATEST8(rawAdc8, 0xFF, 0x01, 0x00, false, false, true, true);
			ATEST8(rawAdc8, 0xFF, 0x00, 0xFF, false, true, false, false);
			ATEST8(rawAdc8, 0xFF, 0xFF, 0xFE, false, true, false, true);
			ATEST8(rawAdc8, 0xFF, 0x80, 0x7F, true, false, false, true);
			ATEST8(rawAdc8, 0x80, 0x80, 0x00, true, false, true, true);
			ATEST8(rawAdc8, 0x7F, 0x7F, 0xFE, true, true, false, false);

			//      op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8C(rawAdc8, 0x7E, 0x00, 0x7F, false, false, false, false);
			ATEST8C(rawAdc8, 0xFE, 0x7F, 0x7E, false, false, false, true);
			ATEST8C(rawAdc8, 0x00, 0x00, 0x01, false, false, false, false);
			ATEST8C(rawAdc8, 0xFE, 0x01, 0x00, false, false, true, true);
			ATEST8C(rawAdc8, 0xFE, 0x00, 0xFF, false, true, false, false);
			ATEST8C(rawAdc8, 0xFE, 0xFF, 0xFE, false, true, false, true);
			ATEST8C(rawAdc8, 0xFE, 0x80, 0x7F, true, false, false, true);
			ATEST8C(rawAdc8, 0x80, 0x80, 0x01, true, false, false, true);
			ATEST8C(rawAdc8, 0x7E, 0x7F, 0xFE, true, true, false, false);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawSub8, 0xFF, 0xFE, 0x01, false, false, false, false);
			ATEST8(rawSub8, 0x7E, 0xFF, 0x7F, false, false, false, true);
			ATEST8(rawSub8, 0xFF, 0xFF, 0x00, false, false, true, false);
			ATEST8(rawSub8, 0xFF, 0x7F, 0x80, false, true, false, false);
			ATEST8(rawSub8, 0xFE, 0xFF, 0xFF, false, true, false, true);
			ATEST8(rawSub8, 0xFE, 0x7F, 0x7F, true, false, false, false);
			ATEST8(rawSub8, 0x7F, 0xFF, 0x80, true, true, false, true);

			// NEG(n) === 0-n
			ATEST8(rawSub8, 0x00, 0x00, 0x00, false, false, true, false);
			ATEST8(rawSub8, 0x00, 0x01, 0xFF, false, true, false, true);
			ATEST8(rawSub8, 0x00, 0x80, 0x80, true, true, false, true);
			ATEST8(rawSub8, 0x00, 0xFF, 0x01, false, false, false, true);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawSbb8, 0xFF, 0xFE, 0x01, false, false, false, false);
			ATEST8(rawSbb8, 0x7E, 0xFF, 0x7F, false, false, false, true);
			ATEST8(rawSbb8, 0xFF, 0xFF, 0x00, false, false, true, false);
			ATEST8(rawSbb8, 0xFF, 0x7F, 0x80, false, true, false, false);
			ATEST8(rawSbb8, 0xFE, 0xFF, 0xFF, false, true, false, true);
			ATEST8(rawSbb8, 0xFE, 0x7F, 0x7F, true, false, false, false);
			ATEST8(rawSbb8, 0x7F, 0xFF, 0x80, true, true, false, true);

			//      op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8C(rawSbb8, 0x00, 0xFF, 0x00, false, false, true, true);
			ATEST8C(rawSbb8, 0xFF, 0xFE, 0x00, false, false, true, false);
			ATEST8C(rawSbb8, 0x7F, 0xFF, 0x7F, false, false, false, true);
			ATEST8C(rawSbb8, 0xFF, 0xFF, 0xFF, false, true, false, true);
			ATEST8C(rawSbb8, 0xFF, 0x7F, 0x7F, true, false, false, false);
			ATEST8C(rawSbb8, 0x80, 0xFF, 0x80, false, true, false, true);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawAnd8, 0xFF, 0x00, 0x00, false, false, true, false);
			ATEST8(rawAnd8, 0xFF, 0xFF, 0xFF, false, true, false, false);
			ATEST8(rawAnd8, 0x0F, 0x0F, 0x0F, false, false, false, false);

			//     op       A     B     RES   OFLOW  SIGN   ZERO   CARRY
			ATEST8(rawOr8, 0x00, 0x00, 0x00, false, false, true, false);
			ATEST8(rawOr8, 0xFF, 0x00, 0xFF, false, true, false, false);
			ATEST8(rawOr8, 0xFF, 0xFF, 0xFF, false, true, false, false);
			ATEST8(rawOr8, 0x0F, 0xF0, 0xFF, false, true, false, false);

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
			ATEST16(rawAdd16, 0x0000, 0x0000, 0x0000, false, false, true, false);
			ATEST16(rawAdd16, 0xFFFF, 0x0001, 0x0000, false, false, true, true);
			ATEST16(rawAdd16, 0xFFFF, 0x0000, 0xFFFF, false, true, false, false);
			ATEST16(rawAdd16, 0xFFFF, 0xFFFF, 0xFFFE, false, true, false, true);
			ATEST16(rawAdd16, 0xFFFF, 0x8000, 0x7FFF, true, false, false, true);
			ATEST16(rawAdd16, 0x8000, 0x8000, 0x0000, true, false, true, true);
			ATEST16(rawAdd16, 0x7FFF, 0x7FFF, 0xFFFE, true, true, false, false);

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawAdc16, 0x7FFF, 0x0000, 0x7FFF, false, false, false, false);
			ATEST16(rawAdc16, 0xFFFF, 0x7FFF, 0x7FFE, false, false, false, true);
			ATEST16(rawAdc16, 0x0000, 0x0000, 0x0000, false, false, true, false);
			ATEST16(rawAdc16, 0xFFFF, 0x0001, 0x0000, false, false, true, true);
			ATEST16(rawAdc16, 0xFFFF, 0x0000, 0xFFFF, false, true, false, false);
			ATEST16(rawAdc16, 0xFFFF, 0xFFFF, 0xFFFE, false, true, false, true);
			ATEST16(rawAdc16, 0xFFFF, 0x8000, 0x7FFF, true, false, false, true);
			ATEST16(rawAdc16, 0x8000, 0x8000, 0x0000, true, false, true, true);
			ATEST16(rawAdc16, 0x7FFF, 0x7FFF, 0xFFFE, true, true, false, false);

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawSub16, 0xFFFF, 0xFFFE, 0x0001, false, false, false, false);
			ATEST16(rawSub16, 0x7FFE, 0xFFFF, 0x7FFF, false, false, false, true);
			ATEST16(rawSub16, 0xFFFF, 0xFFFF, 0x0000, false, false, true, false);
			ATEST16(rawSub16, 0xFFFF, 0x7FFF, 0x8000, false, true, false, false);
			ATEST16(rawSub16, 0xFFFE, 0xFFFF, 0xFFFF, false, true, false, true);
			ATEST16(rawSub16, 0xFFFE, 0x7FFF, 0x7FFF, true, false, false, false);
			ATEST16(rawSub16, 0x7FFF, 0xFFFF, 0x8000, true, true, false, true);

			//      op        A       B       RES     OFLOW  SIGN   ZERO   CARRY
			ATEST16(rawSbb16, 0xFFFF, 0xFFFE, 0x0001, false, false, false, false);
			ATEST16(rawSbb16, 0x7FFE, 0xFFFF, 0x7FFF, false, false, false, true);
			ATEST16(rawSbb16, 0xFFFF, 0xFFFF, 0x0000, false, false, true, false);
			ATEST16(rawSbb16, 0xFFFF, 0x7FFF, 0x8000, false, true, false, false);
			ATEST16(rawSbb16, 0xFFFE, 0xFFFF, 0xFFFF, false, true, false, true);
			ATEST16(rawSbb16, 0xFFFE, 0x7FFF, 0x7FFF, true, false, false, false);
			ATEST16(rawSbb16, 0x7FFF, 0xFFFF, 0x8000, true, true, false, true);
		}
	}

	#define SHIFTROTTEST8(CLR,F,R,O,C) { flags=(CLR?0:flags); SHIFTROT8(F, 1); assert(regA.hl.l == R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_C) == C); }

	void CPU8086Test::TestShiftRotate()
	{
		// SHIFT/ROTATE8
		{
			const BYTE SHL = 0xE0, SHR = 0xE8, SAR = 0xF8, ROL = 0xC0, ROR = 0xC8, RCL = 0xD0, RCR = 0xD8;

			// SHL
			regA.hl.l = 0x01;
			SHIFTROTTEST8(true, SHL, 0x02, false, false);
			SHIFTROTTEST8(true, SHL, 0x04, false, false);
			SHIFTROTTEST8(true, SHL, 0x08, false, false);
			SHIFTROTTEST8(true, SHL, 0x10, false, false);
			SHIFTROTTEST8(true, SHL, 0x20, false, false);
			SHIFTROTTEST8(true, SHL, 0x40, false, false);
			SHIFTROTTEST8(true, SHL, 0x80, true, false);
			SHIFTROTTEST8(true, SHL, 0x00, true, true);
			SHIFTROTTEST8(true, SHL, 0x00, false, false);

			// SHR
			regA.hl.l = 0x80;
			SHIFTROTTEST8(true, SHR, 0x40, true, false);
			SHIFTROTTEST8(true, SHR, 0x20, false, false);
			SHIFTROTTEST8(true, SHR, 0x10, false, false);
			SHIFTROTTEST8(true, SHR, 0x08, false, false);
			SHIFTROTTEST8(true, SHR, 0x04, false, false);
			SHIFTROTTEST8(true, SHR, 0x02, false, false);
			SHIFTROTTEST8(true, SHR, 0x01, false, false);
			SHIFTROTTEST8(true, SHR, 0x00, false, true);
			SHIFTROTTEST8(true, SHR, 0x00, false, false);

			// SAR
			regA.hl.l = 0x80;
			SHIFTROTTEST8(true, SAR, 0xC0, false, false);
			SHIFTROTTEST8(true, SAR, 0xE0, false, false);
			SHIFTROTTEST8(true, SAR, 0xF0, false, false);
			SHIFTROTTEST8(true, SAR, 0xF8, false, false);
			SHIFTROTTEST8(true, SAR, 0xFC, false, false);
			SHIFTROTTEST8(true, SAR, 0xFE, false, false);
			SHIFTROTTEST8(true, SAR, 0xFF, false, true);
			SHIFTROTTEST8(true, SAR, 0xFF, false, true);

			// ROL
			regA.hl.l = 0x01;
			SHIFTROTTEST8(true, ROL, 0x02, false, false);
			SHIFTROTTEST8(true, ROL, 0x04, false, false);
			SHIFTROTTEST8(true, ROL, 0x08, false, false);
			SHIFTROTTEST8(true, ROL, 0x10, false, false);
			SHIFTROTTEST8(true, ROL, 0x20, false, false);
			SHIFTROTTEST8(true, ROL, 0x40, false, false);
			SHIFTROTTEST8(true, ROL, 0x80, true, false);
			SHIFTROTTEST8(true, ROL, 0x01, true, true);
			SHIFTROTTEST8(true, ROL, 0x02, false, false);

			// ROR
			regA.hl.l = 0x80;
			SHIFTROTTEST8(true, ROR, 0x40, true, false);
			SHIFTROTTEST8(true, ROR, 0x20, false, false);
			SHIFTROTTEST8(true, ROR, 0x10, false, false);
			SHIFTROTTEST8(true, ROR, 0x08, false, false);
			SHIFTROTTEST8(true, ROR, 0x04, false, false);
			SHIFTROTTEST8(true, ROR, 0x02, false, false);
			SHIFTROTTEST8(true, ROR, 0x01, false, false);
			SHIFTROTTEST8(true, ROR, 0x80, true, true);
			SHIFTROTTEST8(true, ROR, 0x40, true, false);

			// RCL
			regA.hl.l = 0x01; flags = 0;
			SHIFTROTTEST8(false, RCL, 0x02, false, false);
			SHIFTROTTEST8(false, RCL, 0x04, false, false);
			SHIFTROTTEST8(false, RCL, 0x08, false, false);
			SHIFTROTTEST8(false, RCL, 0x10, false, false);
			SHIFTROTTEST8(false, RCL, 0x20, false, false);
			SHIFTROTTEST8(false, RCL, 0x40, false, false);
			SHIFTROTTEST8(false, RCL, 0x80, true, false);
			SHIFTROTTEST8(false, RCL, 0x00, true, true);
			SHIFTROTTEST8(false, RCL, 0x01, false, false);
			SHIFTROTTEST8(false, RCL, 0x02, false, false);

			// RCR
			regA.hl.l = 0x80; flags = 0;
			SHIFTROTTEST8(false, RCR, 0x40, true, false);
			SHIFTROTTEST8(false, RCR, 0x20, false, false);
			SHIFTROTTEST8(false, RCR, 0x10, false, false);
			SHIFTROTTEST8(false, RCR, 0x08, false, false);
			SHIFTROTTEST8(false, RCR, 0x04, false, false);
			SHIFTROTTEST8(false, RCR, 0x02, false, false);
			SHIFTROTTEST8(false, RCR, 0x01, false, false);
			SHIFTROTTEST8(false, RCR, 0x00, false, true);
			SHIFTROTTEST8(false, RCR, 0x80, true, false);
			SHIFTROTTEST8(false, RCR, 0x40, true, false);

		}
	}
}