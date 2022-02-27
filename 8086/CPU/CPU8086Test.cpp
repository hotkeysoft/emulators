#include "CPU8086Test.h"
#include <assert.h>

namespace emul
{
	WORD rawAdd8(BYTE&, const BYTE, bool);
	WORD rawSub8(BYTE&, const BYTE, bool);
	WORD rawCmp8(BYTE&, const BYTE, bool);
	WORD rawAdc8(BYTE&, const BYTE, bool c);
	WORD rawSbb8(BYTE&, const BYTE, bool b);

	WORD rawAnd8(BYTE&, const BYTE, bool);
	WORD rawOr8(BYTE&, const BYTE, bool);
	WORD rawXor8(BYTE&, const BYTE, bool);
	WORD rawTest8(BYTE&, const BYTE, bool);

	DWORD rawAdd16(WORD&, const WORD, bool);
	DWORD rawSub16(WORD&, const WORD, bool);
	DWORD rawCmp16(WORD&, const WORD, bool);
	DWORD rawAdc16(WORD&, const WORD, bool c);
	DWORD rawSbb16(WORD&, const WORD, bool b);

	DWORD rawAnd16(WORD&, const WORD, bool);
	DWORD rawOr16(WORD&, const WORD, bool);
	DWORD rawXor16(WORD&, const WORD, bool);
	DWORD rawTest16(WORD&, const WORD, bool);

	// Arithmetic functions internal tests
	#define ATEST8(F,A,B,R,O,S,Z,C) { m_reg[REG16::FLAGS]=0; m_reg[REG8::DL]=A; m_reg[REG8::AL]=B; Arithmetic8(sd8, F); assert(sd8.dest.Read()==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define ATEST16(F,A,B,R,O,S,Z,C) { m_reg[REG16::FLAGS]=0; m_reg[REG16::DX]=A; m_reg[REG16::AX]=B; Arithmetic16(sd16, F); assert(sd16.dest.Read()==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define ATEST8C(F,A,B,R,O,S,Z,C) { m_reg[REG16::FLAGS]=FLAG_C; m_reg[REG8::DL]=A; m_reg[REG8::AL]=B; Arithmetic8(sd8, F); assert(sd8.dest.Read()==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define ATEST16C(F,A,B,R,O,S,Z,C) { m_reg[REG16::FLAGS]=FLAG_C; m_reg[REG16::DX]=A; m_reg[REG16::AX]=B; Arithmetic16(sd16, F); assert(sd16.dest.Read()==R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }

	#define REG_RWR(REG,O,W) { assert(m_reg[REG]==O); m_reg[REG]=W; assert(m_reg[REG]==W);}
	#define REG_XHL(R,RX,RH,RL) { m_reg[RX]=R; assert(m_reg[RH]==GetHByte(R)); assert(m_reg[RL]==GetLByte(R)); }

	void CPU8086Test::TestRegisters()
	{
		LogPrintf(LOG_INFO, "TestRegisters");

		LogPrintf(LOG_INFO, "Reg8");
		m_reg.Clear(0xFF);

		REG_RWR(REG8::AH, 0xFF, 0x00); 
		REG_RWR(REG8::AH, 0x00, 0x55);

		REG_RWR(REG8::AL, 0xFF, 0x00); 
		REG_RWR(REG8::AL, 0x00, 0x55);

		REG_RWR(REG8::BH, 0xFF, 0x00); 
		REG_RWR(REG8::BH, 0x00, 0x55);

		REG_RWR(REG8::BL, 0xFF, 0x00); 
		REG_RWR(REG8::BL, 0x00, 0x55);

		REG_RWR(REG8::CH, 0xFF, 0x00); 
		REG_RWR(REG8::CH, 0x00, 0x55);

		REG_RWR(REG8::CL, 0xFF, 0x00); 
		REG_RWR(REG8::CL, 0x00, 0x55);

		REG_RWR(REG8::DH, 0xFF, 0x00); 
		REG_RWR(REG8::DH, 0x00, 0x55);

		REG_RWR(REG8::DL, 0xFF, 0x00); 
		REG_RWR(REG8::DL, 0x00, 0x55);

		REG_RWR(REG8::_T0, 0xFF, 0x00);
		REG_RWR(REG8::_T0, 0x00, 0x55);

		LogPrintf(LOG_INFO, "Reg16");
		m_reg.Clear(0xFF);

		REG_RWR(REG16::AX, 0xFFFF, 0x0000);
		REG_RWR(REG16::AX, 0x0000, 0x55AA);

		REG_RWR(REG16::BX, 0xFFFF, 0x0000);
		REG_RWR(REG16::BX, 0x0000, 0x55AA);

		REG_RWR(REG16::CX, 0xFFFF, 0x0000);
		REG_RWR(REG16::CX, 0x0000, 0x55AA);

		REG_RWR(REG16::DX, 0xFFFF, 0x0000);
		REG_RWR(REG16::DX, 0x0000, 0x55AA);

		REG_RWR(REG16::SP, 0xFFFF, 0x0000);
		REG_RWR(REG16::SP, 0x0000, 0x55AA);

		REG_RWR(REG16::BP, 0xFFFF, 0x0000);
		REG_RWR(REG16::BP, 0x0000, 0x55AA);

		REG_RWR(REG16::SI, 0xFFFF, 0x0000);
		REG_RWR(REG16::SI, 0x0000, 0x55AA);

		REG_RWR(REG16::DI, 0xFFFF, 0x0000);
		REG_RWR(REG16::DI, 0x0000, 0x55AA);

		REG_RWR(REG16::CS, 0xFFFF, 0x0000);
		REG_RWR(REG16::CS, 0x0000, 0x55AA);

		REG_RWR(REG16::DS, 0xFFFF, 0x0000);
		REG_RWR(REG16::DS, 0x0000, 0x55AA);

		REG_RWR(REG16::SS, 0xFFFf, 0x0000);
		REG_RWR(REG16::SS, 0x0000, 0x55AA);

		REG_RWR(REG16::ES, 0xFFFF, 0x0000);
		REG_RWR(REG16::ES, 0x0000, 0x55AA);

		REG_RWR(REG16::IP, 0xFFFF, 0x0000);
		REG_RWR(REG16::IP, 0x0000, 0x55AA);

		REG_RWR(REG16::FLAGS, 0xFFFF, 0x0000);
		REG_RWR(REG16::FLAGS, 0x0000, 0x55AA);

		REG_RWR(REG16::_REP_IP, 0xFFFF, 0x0000);
		REG_RWR(REG16::_REP_IP, 0x0000, 0x55AA);

		REG_RWR(REG16::_SEG_O, 0xFFFF, 0x0000);
		REG_RWR(REG16::_SEG_O, 0x0000, 0x55AA);

		REG_RWR(REG16::_T0, 0xFFFF, 0x0000);
		REG_RWR(REG16::_T0, 0x0000, 0x55AA);

		LogPrintf(LOG_INFO, "Reg16<->8");
		m_reg.Clear(0x00);

		REG_XHL(0x1234, REG16::AX, REG8::AH, REG8::AL);
		REG_XHL(0x5678, REG16::BX, REG8::BH, REG8::BL);
		REG_XHL(0x9ABC, REG16::CX, REG8::CH, REG8::CL);
		REG_XHL(0xDEF0, REG16::DX, REG8::DH, REG8::DL);
	}

	void CPU8086Test::TestArithmetic()
	{
		LogPrintf(LOG_INFO, "TestArithmetic");
		// Arithmetic8
		{
			SourceDest8 sd8;
			sd8.source = REG8::AL;
			sd8.dest = REG8::DL;

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
			SourceDest16 sd16;
			sd16.source = REG16::AX;
			sd16.dest = REG16::DX;

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

	#define SHIFTROTTEST8(CLR,F,N,R,O,S,Z,C) { m_reg[REG16::FLAGS]=(CLR?0:m_reg[REG16::FLAGS]); SHIFTROT8(F, N); assert(m_reg[REG8::AL] == R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }
	#define SHIFTROTTEST8noO(CLR,F,N,R,S,Z,C) { m_reg[REG16::FLAGS]=(CLR?0:m_reg[REG16::FLAGS]); SHIFTROT8(F, N); assert(m_reg[REG8::AL] == R); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }

	#define SHIFTROTTEST16(CLR,F,N,R,O,S,Z,C) { m_reg[REG16::FLAGS]=(CLR?0:m_reg[REG16::FLAGS]); SHIFTROT16(F, N); assert(m_reg[REG16::AX] == R); assert(GetFlag(FLAG_O) == O); assert(GetFlag(FLAG_S) == S); assert(GetFlag(FLAG_Z) == Z); assert(GetFlag(FLAG_C) == C); }


	void CPU8086Test::TestShiftRotate()
	{
		LogPrintf(LOG_INFO, "TestShiftRotate");
		// SHIFT/ROTATE8
		{
			const BYTE SHL = 0xE0, SHR = 0xE8, SAR = 0xF8, ROL = 0xC0, ROR = 0xC8, RCL = 0xD0, RCR = 0xD8;

			// N=1

			// SHL
			m_reg[REG8::AL] = 0x01;
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
			m_reg[REG8::AL] = 0x80;
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
			m_reg[REG8::AL] = 0b10000000;
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
			m_reg[REG8::AL] = 0b01000000;
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
			m_reg[REG8::AL] = 0x01;
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
			m_reg[REG8::AL] = 0x80;
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
			m_reg[REG8::AL] = 0x01; m_reg[REG16::FLAGS] = 0;
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
			m_reg[REG8::AL] = 0x80; m_reg[REG16::FLAGS] = 0;
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

			//                                          clrf  op   n  RES         OFLOW  SIGN   ZERO   CARRY

			//// SHL
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, SHL, 0, 0b10101010, false, true,  false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, SHL, 0, 0b01010101, false, false, false, false);

			//// SHR
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, SHR, 0, 0b10101010, false, true,  false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, SHR, 0, 0b01010101, false, false, false, false);

			//// SAR
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, SAR, 0, 0b10101010, false, true,  false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, SAR, 0, 0b01010101, false, false, false, false);

			//// ROL
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, ROL, 0, 0b10101010, false, false, false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, ROL, 0, 0b01010101, false, false, false, false);

			//// ROR
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, ROR, 0, 0b10101010, false, false, false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, ROR, 0, 0b01010101, false, false, false, false);

			//// RCL
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, RCL, 0, 0b10101010, false, false,  false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, RCL, 0, 0b01010101, false, false, false, false);

			//// RCR
			//m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8(true, RCR, 0, 0b10101010, false, false,  false, false);
			//m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8(true, RCR, 0, 0b01010101, false, false, false, false);

			//                                             clrf  op   n  RES         SIGN   ZERO   CARRY
			// N=7 (OF Undefined for n>1)
			// SHL
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, SHL, 7, 0b00000000, false, true,  true);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, SHL, 7, 0b10000000, true,  false, false);

			// SHR
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, SHR, 7, 0b00000001, false, false, false);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, SHR, 7, 0b00000000, false, true,  true);

			// SAR
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, SAR, 7, 0b11111111, true,  false, false);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, SAR, 7, 0b00000000, false, true,  true);

			// ROL
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, ROL, 7, 0b01010101, false, false, true);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, ROL, 7, 0b10101010, false, false, false);

			// ROR
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, ROR, 7, 0b01010101, false, false, false);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, ROR, 7, 0b10101010, false, false, true);

			// RCL
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, RCL, 7, 0b00101010, false, false, true);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, RCL, 7, 0b10010101, false, false, false);
			
			// RCR
			m_reg[REG8::AL] = 0b10101010; SHIFTROTTEST8noO(true, RCR, 7, 0b10101001, false, false, false);
			m_reg[REG8::AL] = 0b01010101; SHIFTROTTEST8noO(true, RCR, 7, 0b01010100, false, false, true);

		}

		// SHIFT/ROTATE16
		{
			const BYTE SHL = 0xE0, SHR = 0xE8, SAR = 0xF8, ROL = 0xC0, ROR = 0xC8, RCL = 0xD0, RCR = 0xD8;

			//                                        clrf  op   n    RES     OFLOW  SIGN   ZERO   CARRY
			//m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 0,   0xC8A7, false, false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 1,   0x914E, false, false, false, true);
			
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 15,  0xB229, false, false, false, true);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 16,  0x6453, true,  false, false, true);

			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 17,  0xC8A7, true,  false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 18,  0x914E, false, false, false, true);
			
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 29,  0x7645, false, false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 30,  0xEC8A, true,  false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 31,  0xD914, false, false, false, true);

			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 32,  0xC8A7, false, false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 33,  0x914E, false, false, false, true);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 34,  0x229D, true, false, false, true);

			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 253, 0x7645, false, false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 254, 0xEC8A, true,  false, false, false);
			m_reg[REG16::AX] = 0xC8A7; SHIFTROTTEST16(true, RCL, 255, 0xD914, false, false, false, true);
		}
	}
}