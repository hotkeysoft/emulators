
#include "stdafx.h"
#include "CPUInfo.h"
#include "Console.h"

#include <conio.h>
#include <iostream>
#include "TMS1000.h"

void LogCallback(const char *str) {
	fprintf(stderr, str);
}

void TestIMAC(BYTE m, BYTE expect, bool carry) {
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 1;
	TMS1000::g_cpu.A = 0;
	TMS1000::g_cpu.S = false;
	TMS1000::PutRAM(m);
	TMS1000::Exec(0x28); // IMAC
	assert(TMS1000::g_cpu.X == 0);
	assert(TMS1000::g_cpu.Y == 1);
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::GetRAM(TMS1000::GetM(0, 1)) == m);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestDMAN(BYTE m, BYTE expect, bool carry) {
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 2;
	TMS1000::g_cpu.A = 0;
	TMS1000::g_cpu.S = false;
	TMS1000::PutRAM(m);
	TMS1000::Exec(0x2A); // IMAC
	assert(TMS1000::g_cpu.X == 0);
	assert(TMS1000::g_cpu.Y == 2);
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::GetRAM(TMS1000::GetM(0, 2)) == m);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestSAMAN(BYTE m, BYTE a, BYTE res, bool carry) {
	TMS1000::g_cpu.X = 2;
	TMS1000::g_cpu.Y = 15;
	TMS1000::PutRAM(m);
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x27); // SAMAN
	assert(TMS1000::g_cpu.X == 2);
	assert(TMS1000::g_cpu.Y == 15);
	assert(TMS1000::g_cpu.A == res);
	assert(TMS1000::GetRAM(TMS1000::GetM(2, 15)) == m);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestIA(BYTE a, BYTE expect) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x0E); // IA
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();
}

void TestIYC(BYTE y, BYTE expect, bool carry) {
	TMS1000::g_cpu.Y = y;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x2B); // IA
	assert(TMS1000::g_cpu.Y == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestDAN(BYTE a, BYTE expect, bool carry) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x07); // DAN
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestDYN(BYTE a, BYTE expect, bool carry) {
	TMS1000::g_cpu.Y = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x2C); // DYN
	assert(TMS1000::g_cpu.Y == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestA6AAC(BYTE a, BYTE expect, bool carry) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x06); // A6AAC
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestA8AAC(BYTE a, BYTE expect, bool carry) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x01); // A8AAC
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestA10AAC(BYTE a, BYTE expect, bool carry) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x05); // A10AAC
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestCPAIZ(BYTE a, BYTE expect, bool carry) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x2D); // CPAIZ
	assert(TMS1000::g_cpu.A == expect);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestALEM(BYTE a, BYTE m, bool carry) {
	TMS1000::g_cpu.X = 2;
	TMS1000::g_cpu.Y = 10;
	TMS1000::PutRAM(m);
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x29); // ALEM
	assert(TMS1000::g_cpu.X == 2);
	assert(TMS1000::g_cpu.Y == 10);
	assert(TMS1000::g_cpu.A == a);
	assert(TMS1000::GetRAM(TMS1000::GetM(2, 10)) == m);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestALEC(BYTE opCode, BYTE a, bool carry) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode); // ALEC
	assert(TMS1000::g_cpu.A == a);
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestMNEZ(BYTE m, bool status) {
	TMS1000::g_cpu.X = 3;
	TMS1000::g_cpu.Y = 3;
	TMS1000::PutRAM(m);
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x26); // MNEZ
	assert(TMS1000::g_cpu.X == 3);
	assert(TMS1000::g_cpu.Y == 3);
	assert(TMS1000::GetRAM(TMS1000::GetM(3, 3)) == m);
	assert(TMS1000::g_cpu.S == status);
	Console::UpdateStatus();
}

void TestYNEA(BYTE y, BYTE a, bool status) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.Y = y;
	TMS1000::g_cpu.S = false;
	TMS1000::g_cpu.SL = false;

	TMS1000::Exec(0x02); // YNEA
	assert(TMS1000::g_cpu.S == status);
	assert(TMS1000::g_cpu.SL == status);
	Console::UpdateStatus();
}

void TestYNEC(BYTE opCode, BYTE y, bool carry) {
	TMS1000::g_cpu.Y = y & 0x0F;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode); // YNEC
	assert(TMS1000::g_cpu.Y == (y & 0x0F));
	assert(TMS1000::g_cpu.S == carry);
	Console::UpdateStatus();
}

void TestSRBIT(BYTE opCode, BYTE m, BYTE expected) {
	TMS1000::g_cpu.X = 1;
	TMS1000::g_cpu.Y = 2;
	TMS1000::PutRAM(m);
	TMS1000::g_cpu.S = false;
	
	TMS1000::Exec(opCode);
	assert(TMS1000::GetRAM() == expected);
	assert(TMS1000::g_cpu.S == 1);
	Console::UpdateStatus();
}

void TestTBIT(BYTE opCode, BYTE m, bool status) {
	TMS1000::g_cpu.X = 1;
	TMS1000::g_cpu.Y = 3;
	TMS1000::PutRAM(m);
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.S == status);
	Console::UpdateStatus();
}

void TestTCY(BYTE opCode, BYTE y) {
	TMS1000::g_cpu.Y = 0;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.Y == y);
	assert(TMS1000::g_cpu.S == 1);
	Console::UpdateStatus();
}

void TestTCMIY(BYTE opCode, BYTE value) {
	BYTE oldY = TMS1000::g_cpu.Y;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode);
	assert(TMS1000::GetRAM(TMS1000::GetM(TMS1000::g_cpu.X, oldY)) == value);
	assert(TMS1000::g_cpu.Y == ((oldY + 1) & 0x0F));
	assert(TMS1000::g_cpu.S == 1);
	Console::UpdateStatus();
}

void TestKNEZ(BYTE value, bool status) {
	TMS1000::g_cpu.K = value;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x09); // KNEZ
	assert(TMS1000::g_cpu.S == status);
	Console::UpdateStatus();
}

void TestTKA(BYTE value) {
	TMS1000::g_cpu.K = value;
	TMS1000::g_cpu.A = ~value;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x08); // TKA
	assert(TMS1000::g_cpu.A == value);
	assert(TMS1000::g_cpu.S == true);
	Console::UpdateStatus();
}

void TestSETR(BYTE r) {
	TMS1000::g_cpu.R = 0;
	TMS1000::g_cpu.Y = r;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x0D); // SETR
	assert(TMS1000::g_cpu.R & (1 << r));
	assert(TMS1000::g_cpu.S == true);
	Console::UpdateStatus();
}

void TestRSTR(BYTE r) {
	TMS1000::g_cpu.R = 0x3FF;
	TMS1000::g_cpu.Y = r;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x0C); // RSTR
	assert(!(TMS1000::g_cpu.R & (1 << r)));
	assert(TMS1000::g_cpu.S == true);
	Console::UpdateStatus();
}

void TestTDO(BYTE a, bool SL) {
	TMS1000::g_cpu.A = a;
	TMS1000::g_cpu.SL = SL;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x0A); // TDO
	assert(TMS1000::g_cpu.O == ((a <<1) + (SL?1:0)));
	assert(TMS1000::g_cpu.S == true);
	Console::UpdateStatus();
}

void TestCLO() {
	TMS1000::g_cpu.O = 0xFF;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0x0B); // CLO
	assert(TMS1000::g_cpu.O == 0);
	assert(TMS1000::g_cpu.S == true);
	Console::UpdateStatus();
}

void TestLDX(BYTE opCode, BYTE value) {
	TMS1000::g_cpu.X = ~value;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.X == value);
	assert(TMS1000::g_cpu.S == 1);
	Console::UpdateStatus();
}

void TextCOMX(BYTE x, BYTE notX) {
	TMS1000::g_cpu.X = x;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(0);
	assert(TMS1000::g_cpu.X == notX);
	assert(TMS1000::g_cpu.S == 1);
	Console::UpdateStatus();
}

void TestLDP(BYTE opCode, BYTE value) {
	TMS1000::g_cpu.PB = ~value;
	TMS1000::g_cpu.S = false;

	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PB == value);
	assert(TMS1000::g_cpu.S == 1);
	Console::UpdateStatus();
}

void TestBR(BYTE opCode, BYTE addr) {
	TMS1000::g_cpu.PA = 0x01;
	TMS1000::g_cpu.PB = 0x02;
	TMS1000::g_cpu.PC = (~addr)&0x3F;
	TMS1000::g_cpu.CL = false;
	TMS1000::g_cpu.S = false;

	// S = 0 : No branch
	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PA == 0x01);
	assert(TMS1000::g_cpu.PB == 0x02);
	assert(TMS1000::g_cpu.PC == ((~addr) & 0x3F));
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.CL == false);
	Console::UpdateStatus();

	TMS1000::g_cpu.CL = true;
	// S = 1, CL = 0 : Branch
	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PA == 0x01);
	assert(TMS1000::g_cpu.PB == 0x02);
	assert(TMS1000::g_cpu.PC == addr);
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.CL == true);
	Console::UpdateStatus();

	TMS1000::g_cpu.CL = false;
	// S = 1, CL = 0 : Branch
	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PA == 0x02);
	assert(TMS1000::g_cpu.PB == 0x02);
	assert(TMS1000::g_cpu.PC == addr);
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.CL == false);
	Console::UpdateStatus();
}

void TestCALL(BYTE opCode, BYTE addr) {
	TMS1000::g_cpu.PA = 0x01;
	TMS1000::g_cpu.PB = 0x02;
	TMS1000::g_cpu.PC = (~addr) & 0x3F;
	TMS1000::g_cpu.SR = 0x03;
	TMS1000::g_cpu.CL = false;
	TMS1000::g_cpu.S = false;

	// S = 0 : No call
	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PA == 0x01);
	assert(TMS1000::g_cpu.PB == 0x02);
	assert(TMS1000::g_cpu.PC == ((~addr) & 0x3F));
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.SR == 0x03);
	assert(TMS1000::g_cpu.CL == false);
	Console::UpdateStatus();

	TMS1000::g_cpu.CL = true;
	// S = 1, CL = 0 : Call
	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PA == 0x01);
	assert(TMS1000::g_cpu.PB == 0x01);
	assert(TMS1000::g_cpu.PC == addr);
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.SR == 0x03);
	assert(TMS1000::g_cpu.CL == true);
	Console::UpdateStatus();

	TMS1000::g_cpu.PA = 0x01;
	TMS1000::g_cpu.PB = 0x02;
	TMS1000::g_cpu.PC = (~addr) & 0x3F;
	TMS1000::g_cpu.SR = 0x03;
	TMS1000::g_cpu.CL = false;
	BYTE retPC = (TMS1000::g_cpu.PC + 1) & 0x3F;

	// S = 1, CL = 0 : CALL
	TMS1000::Exec(opCode);
	assert(TMS1000::g_cpu.PA == 0x02);
	assert(TMS1000::g_cpu.PB == 0x01);
	assert(TMS1000::g_cpu.PC == addr);
	assert(TMS1000::g_cpu.SR == retPC);
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.CL == true);
	Console::UpdateStatus();
}

void TestRETN() {
	TMS1000::g_cpu.PA = 0x01;
	TMS1000::g_cpu.PB = 0x02;
	TMS1000::g_cpu.PC = 0x03;
	TMS1000::g_cpu.SR = 0x04;
	TMS1000::g_cpu.CL = false;
	TMS1000::g_cpu.S = false;

	// CL = 0
	TMS1000::Exec(0x0F); // RETN
	assert(TMS1000::g_cpu.PA == 0x02);
	assert(TMS1000::g_cpu.PB == 0x02);
	assert(TMS1000::g_cpu.PC == 0x03);
	assert(TMS1000::g_cpu.SR == 0x04);
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.CL == false);
	Console::UpdateStatus();

	TMS1000::g_cpu.PA = 0x01;
	TMS1000::g_cpu.PB = 0x02;
	TMS1000::g_cpu.PC = 0x03;
	TMS1000::g_cpu.SR = 0x04;
	TMS1000::g_cpu.CL = true;
	TMS1000::g_cpu.S = false;

	// CL = 1
	TMS1000::Exec(0x0F); // RETN
	assert(TMS1000::g_cpu.PA == 0x02);
	assert(TMS1000::g_cpu.PB == 0x02);
	assert(TMS1000::g_cpu.PC == 0x04);
	assert(TMS1000::g_cpu.SR == 0x04);
	assert(TMS1000::g_cpu.S == true);
	assert(TMS1000::g_cpu.CL == false);
	Console::UpdateStatus();
}

void TestCPU() {
	// Unit tests
	memset(TMS1000::g_memory.RAM, 0xAA, TMS1000::g_memory.ramSize);

	// Register <=> Register

	// TAY
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 5;
	TMS1000::g_cpu.A = 0xE;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x24); // TAY
	assert(TMS1000::g_cpu.X == 0);
	assert(TMS1000::g_cpu.Y == 0xE);
	assert(TMS1000::g_cpu.A == 0xE);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();

	// TYA
	TMS1000::g_cpu.X = 2;
	TMS1000::g_cpu.Y = 9;
	TMS1000::g_cpu.A = 6;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x23); // TYA
	assert(TMS1000::g_cpu.X == 2);
	assert(TMS1000::g_cpu.Y == 9);
	assert(TMS1000::g_cpu.A == 9);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();

	// CLA
	TMS1000::g_cpu.X = 1;
	TMS1000::g_cpu.Y = 4;
	TMS1000::g_cpu.A = 7;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x2F); // CLA
	assert(TMS1000::g_cpu.X == 1);
	assert(TMS1000::g_cpu.Y == 4);
	assert(TMS1000::g_cpu.A == 0);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();

	// Register <=> Memory

	// TAM
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 6;
	TMS1000::g_cpu.A = 4;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x03); // TAM
	assert(TMS1000::g_cpu.X == 0);
	assert(TMS1000::g_cpu.Y == 6);
	assert(TMS1000::g_cpu.A == 4);
	assert(TMS1000::g_cpu.S);
	assert(TMS1000::GetRAM(TMS1000::GetM(0, 6)) == 4);
	Console::UpdateStatus();

	// TAMIY
	TMS1000::g_cpu.X = 3;
	TMS1000::g_cpu.Y = 9;
	TMS1000::g_cpu.A = 0;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x20); // TAMIY
	assert(TMS1000::g_cpu.X == 3);
	assert(TMS1000::g_cpu.Y == 0xA);
	assert(TMS1000::g_cpu.A == 0);
	assert(TMS1000::g_cpu.S);
	assert(TMS1000::GetRAM(TMS1000::GetM(3, 9)) == 0);
	Console::UpdateStatus();

	// TAMZA
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 0xE;
	TMS1000::g_cpu.A = 0xB;
	TMS1000::g_cpu.S = false;
	TMS1000::Exec(0x04); // TAMZA
	assert(TMS1000::g_cpu.X == 0);
	assert(TMS1000::g_cpu.Y == 0xE);
	assert(TMS1000::g_cpu.A == 0);
	assert(TMS1000::g_cpu.S);
	assert(TMS1000::GetRAM(TMS1000::GetM(0, 0xE)) == 0xB);
	Console::UpdateStatus();

	// TMY
	TMS1000::g_cpu.X = 1;
	TMS1000::g_cpu.Y = 3;
	TMS1000::g_cpu.A = 9;
	TMS1000::g_cpu.S = false;
	assert(TMS1000::GetRAM(TMS1000::GetM(1, 3)) == 0xA);
	TMS1000::Exec(0x22); // TMY
	assert(TMS1000::g_cpu.X == 1);
	assert(TMS1000::g_cpu.Y == 0xA);
	assert(TMS1000::g_cpu.A == 9);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();

	// TMA
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 9;
	TMS1000::g_cpu.A = 3;
	TMS1000::g_cpu.S = false;
	assert(TMS1000::GetRAM(TMS1000::GetM(0, 9)) == 0xA);
	TMS1000::Exec(0x21); // TMA
	assert(TMS1000::g_cpu.X == 0);
	assert(TMS1000::g_cpu.Y == 9);
	assert(TMS1000::g_cpu.A == 0xA);
	assert(TMS1000::GetRAM(TMS1000::GetM(0, 9)) == 0xA);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();

	// XMA
	TMS1000::g_cpu.X = 1;
	TMS1000::g_cpu.Y = 9;
	TMS1000::g_cpu.A = 3;
	TMS1000::g_cpu.S = false;
	assert(TMS1000::GetRAM(TMS1000::GetM(1, 9)) == 0xA);
	TMS1000::Exec(0x2E); // XMA
	assert(TMS1000::g_cpu.X == 1);
	assert(TMS1000::g_cpu.Y == 9);
	assert(TMS1000::g_cpu.A == 0xA);
	assert(TMS1000::GetRAM(TMS1000::GetM(1, 9)) == 3);
	assert(TMS1000::g_cpu.S);
	Console::UpdateStatus();

	// Arithmetic instructions

	// AMAAC
	TMS1000::g_cpu.X = 2;
	TMS1000::g_cpu.Y = 8;
	TMS1000::g_cpu.A = 1;
	TMS1000::g_cpu.S = false;
	TMS1000::PutRAM(15);
	TMS1000::Exec(0x25); // AMAAC
	assert(TMS1000::g_cpu.X == 2);
	assert(TMS1000::g_cpu.Y == 8);
	assert(TMS1000::g_cpu.A == 0); // (15 + 1) MOD 16
	assert(TMS1000::GetRAM(TMS1000::GetM(2, 8)) == 15);
	assert(TMS1000::g_cpu.S); // Carry set
	Console::UpdateStatus();

	// AMAAC (2)
	TMS1000::g_cpu.X = 2;
	TMS1000::g_cpu.Y = 9;
	TMS1000::g_cpu.A = 0;
	TMS1000::g_cpu.S = false;
	TMS1000::PutRAM(15);
	TMS1000::Exec(0x25); // AMAAC
	assert(TMS1000::g_cpu.X == 2);
	assert(TMS1000::g_cpu.Y == 9);
	assert(TMS1000::g_cpu.A == 15); // (15 + 0) MOD 16
	assert(TMS1000::GetRAM(TMS1000::GetM(2, 9)) == 15);
	assert(!TMS1000::g_cpu.S); // Carry clear
	Console::UpdateStatus();

	// SAMAN
	TestSAMAN(15, 0, 15, true);
	TestSAMAN(15, 15, 0, true);
	TestSAMAN(15, 14, 1, true);
	TestSAMAN(5, 2, 3, true);
	TestSAMAN(5, 0, 5, true);
	TestSAMAN(1, 1, 0, true);
	TestSAMAN(0, 0, 0, true);
	
	TestSAMAN(0, 1, 0xf, false);
	TestSAMAN(0, 2, 0xe, false);
	TestSAMAN(1, 2, 0xf, false);
	TestSAMAN(14, 15, 0xf, false);
	TestSAMAN(10, 15, 0xB, false);

	// IMAC
	TestIMAC(0, 1, false);
	TestIMAC(1, 2, false);
	TestIMAC(14, 15, false);
	TestIMAC(15, 0, true);
	
	// DMAN
	TestDMAN(0, 15, false);
	TestDMAN(1, 0, true);
	TestDMAN(14, 13, true);
	TestDMAN(15, 14, true);

	// IA
	TestIA(0, 1);
	TestIA(1, 2);
	TestIA(14, 15);
	TestIA(15, 0);

	// IYC
	TestIYC(0, 1, false);
	TestIYC(14, 15, false);
	TestIYC(15, 0, true);

	// DAN
	TestDAN(0, 15, false);
	TestDAN(1, 0, true);
	TestDAN(15, 14, true);

	// DYN
	TestDYN(0, 15, false);
	TestDYN(1, 0, true);
	TestDYN(15, 14, true);

	// A6AAC
	TestA6AAC(0, 6, false);
	TestA6AAC(9, 15, false);
	TestA6AAC(10, 0, true);
	TestA6AAC(11, 1, true);

	// A8AAC
	TestA8AAC(0, 8, false);
	TestA8AAC(7, 15, false);
	TestA8AAC(8, 0, true);
	TestA8AAC(9, 1, true);

	// A6AAC
	TestA10AAC(0, 10, false);
	TestA10AAC(5, 15, false);
	TestA10AAC(6, 0, true);
	TestA10AAC(7, 1, true);
	
	// CPAIZ (Two's complement A)
	TestCPAIZ(0, 0, true);
	TestCPAIZ(1, 15, false);
	TestCPAIZ(2, 14, false);
	TestCPAIZ(7, 9, false);
	TestCPAIZ(8, 8, false);
	TestCPAIZ(9, 7, false);
	TestCPAIZ(15, 1, false);

	// Arithmetic Compare Instructions

	// ALEM (A <= M)
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			TestALEM(i, j, i <= j);
		}
	}

	// ALEC (A <= const)
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			TestALEC(0x70 + TMS1000::GetC(i), j, j <= i);
		}
	}

	// Logical Compare Instructions

	// MNEZ
	TestMNEZ(0, false);
	TestMNEZ(1, true);
	TestMNEZ(2, true);
	TestMNEZ(15, true);

	// YNEA
	TestYNEA(0, 0, false);
	TestYNEA(1, 1, false);
	TestYNEA(15, 15, false);
	TestYNEA(0, 1, true);
	TestYNEA(1, 2, true);
	TestYNEA(15, 14, true);

	// YNEC
	for (int i = 0; i < 16; ++i) {
		TestYNEC(0x50 + TMS1000::GetC(i), i, false);
		TestYNEC(0x50 + TMS1000::GetC(i), i + 1, true);
		TestYNEC(0x50 + TMS1000::GetC(i), i - 1, true);
	}

	// Bit Manipulation in Memory

	// SBIT
	const BYTE SBIT0 = 0x30;
	const BYTE SBIT2 = 0x31;
	const BYTE SBIT1 = 0x32;
	const BYTE SBIT3 = 0x33;

	TestSRBIT(SBIT0, 0, 1);
	TestSRBIT(SBIT1, 0, 2);
	TestSRBIT(SBIT2, 0, 4);
	TestSRBIT(SBIT3, 0, 8);

	// RBIT
	const BYTE RBIT0 = 0x34;
	const BYTE RBIT2 = 0x35;
	const BYTE RBIT1 = 0x36;
	const BYTE RBIT3 = 0x37;

	TestSRBIT(RBIT0, 0xF, 0xE);
	TestSRBIT(RBIT1, 0xF, 0xD);
	TestSRBIT(RBIT2, 0xF, 0xB);
	TestSRBIT(RBIT3, 0xF, 0x7);

	// TBIT
	const BYTE TBIT0 = 0x38;
	const BYTE TBIT2 = 0x39;
	const BYTE TBIT1 = 0x3A;
	const BYTE TBIT3 = 0x3B;

	TestTBIT(TBIT0, 0xF, true);
	TestTBIT(TBIT1, 0xF, true);
	TestTBIT(TBIT2, 0xF, true);
	TestTBIT(TBIT3, 0xF, true);

	TestTBIT(TBIT0, 0x0, false);
	TestTBIT(TBIT1, 0x0, false);
	TestTBIT(TBIT2, 0x0, false);
	TestTBIT(TBIT3, 0x0, false);

	TestTBIT(TBIT0, 0x1, true);
	TestTBIT(TBIT1, 0x2, true);
	TestTBIT(TBIT2, 0x4, true);
	TestTBIT(TBIT3, 0x8, true);
	
	TestTBIT(TBIT0, 0xE, false);
	TestTBIT(TBIT1, 0xD, false);
	TestTBIT(TBIT2, 0xB, false);
	TestTBIT(TBIT3, 0x7, false);

	// Constrant Transfer Instructions

	// TCY
	const BYTE TCY0  = 0x40;
	const BYTE TCY8  = 0x41;
	const BYTE TCY4	 = 0x42;
	const BYTE TCY12 = 0x43;
	const BYTE TCY2  = 0x44;
	const BYTE TCY10 = 0x45;
	const BYTE TCY6  = 0x46;
	const BYTE TCY14 = 0x47;
	const BYTE TCY1  = 0x48;
	const BYTE TCY9  = 0x49;
	const BYTE TCY5  = 0x4a;
	const BYTE TCY13 = 0x4b;
	const BYTE TCY3  = 0x4c;
	const BYTE TCY11 = 0x4d;
	const BYTE TCY7  = 0x4e;
	const BYTE TCY15 = 0x4f;

	TestTCY(TCY0,  0);
	TestTCY(TCY1,  1);
	TestTCY(TCY2,  2);
	TestTCY(TCY3,  3);
	TestTCY(TCY4,  4);
	TestTCY(TCY5,  5);
	TestTCY(TCY6,  6);
	TestTCY(TCY7,  7);
	TestTCY(TCY8,  8);
	TestTCY(TCY9,  9);
	TestTCY(TCY10, 10);
	TestTCY(TCY11, 11);
	TestTCY(TCY12, 12);
	TestTCY(TCY13, 13);
	TestTCY(TCY14, 14);
	TestTCY(TCY15, 15);

	// TCMIY
	TMS1000::g_cpu.X = 0;
	TMS1000::g_cpu.Y = 0;
	for (int i = 0; i < 16; ++i) {
		TestTCMIY(0x60 + TMS1000::GetC(i), i);
	}

	TMS1000::g_cpu.X = 1;
	TMS1000::g_cpu.Y = 0;
	for (int i = 0; i < 16; ++i) {
		TestTCMIY(0x60 + TMS1000::GetC(1), 1);
	}

	TMS1000::g_cpu.X = 2;
	TMS1000::g_cpu.Y = 0;
	for (int i = 0; i < 16; ++i) {
		TestTCMIY(0x60 + TMS1000::GetC(0), 0);
	}

	TMS1000::g_cpu.X = 3;
	TMS1000::g_cpu.Y = 0;
	for (int i = 0; i < 16; ++i) {
		TestTCMIY(0x60 + TMS1000::GetC(15), 15);
	}

	// Input Instructions

	// KNEZ
	TestKNEZ(0, false);
	TestKNEZ(1, true);
	TestKNEZ(2, true);
	TestKNEZ(4, true);
	TestKNEZ(8, true);
	TestKNEZ(15, true);

	// TKA
	TestTKA(0);
	TestTKA(1);
	TestTKA(2);
	TestTKA(4);
	TestTKA(8);
	TestTKA(15);

	// Output Instructions
	
	// SETR
	for (int i = 0; i <= 10; ++i) {
		TestSETR(i);
	}

	// RSTR
	for (int i = 0; i <= 10; ++i) {
		TestRSTR(i);
	}

	// TDO
	for (int i = 0; i < 16; ++i) {
		TestTDO(i, false);
		TestTDO(i, true);
	}

	// CLO
	TestCLO();

	// RAM X Addressing Instructions
	
	// LDX
	const BYTE LDX0 = 0x3C;
	const BYTE LDX2 = 0x3D;
	const BYTE LDX1 = 0x3E;
	const BYTE LDX3 = 0x3F;

	TestLDX(LDX0, 0);
	TestLDX(LDX1, 1);
	TestLDX(LDX2, 2);
	TestLDX(LDX3, 3);

	// COMX
	TextCOMX(0, 3);
	TextCOMX(1, 2);
	TextCOMX(2, 1);
	TextCOMX(3, 0);

	// ROM Addressing Instructions
	// LDP
	for (int i = 0; i < 16; ++i) {
		TestLDP(0x10 + TMS1000::GetC(i), i);
	}

	// BR
	for (int i = 0; i < 64; ++i) {
		TestBR(0x80 + i, i);
	}

	// CALL
	for (int i = 0; i < 64; ++i) {
		TestCALL(0xC0 + i, i);
	}

	// RETN
	TestRETN();
}

void ShowMonitor(CPUInfo &cpuInfo) {
	Console::Init(&cpuInfo);
	TMS1000::Reset();
	Console::UpdateStatus();
	long lastTicks = 0;
	bool loop = true;
	while (loop) {
		switch (Console::ReadInput()) {
		case 27: // ESC
			loop = false;
			break;
		case 0x3B: // F1
			TMS1000::Reset();
			Console::UpdateStatus();
			break;
		case 0x3F: // F5
			Console::SetRunMode(true);
			while (!_kbhit()) {
				TMS1000::Step();
				if ((TMS1000::GetTicks() - lastTicks) > 10000) {
					Console::UpdateStatus();
					lastTicks = TMS1000::GetTicks();
				}
			}
			Console::SetRunMode(false);
			break;
		case 0x40: // F6
			TMS1000::Step();
			Console::UpdateStatus();
			break;
		case 0x3C: // F2
		case 0x3D: // F3
		case 0x3E: // F4
		case 0x41: // F7
		case 0x42: // F8
		case 0x43: // F9
		case 0x44: // F10
		default:
			break;
		}
	}
}

// Select Game:  4= Game 1

// R0 (1) : SELECT GAME: GAME1 (K2) / GAME2 (K1) / GAME3 (K4)
// R1 (2) : COLOR SWITCHES: GREEN (K1) /RED (K2) /YELLOW (K4) /BLUE (K8)
// R2 (4) : START (K1) / LAST (K2) / LONGEST (K4)
//	  (8)
// R4 (16) : GREEN
// R5 (32) : RED
// R6 (64) : YELLOW 
// R7 (128): BLUE

// R8: (256): SPKR

// R9: (512): SKILL SWITCH: LEVEL1 (K2) / LEVEL2 (K4) / LEVEL3 (K8) / LEVEL4 (K1)

void onReadInput() {
	if (TMS1000::g_cpu.R & 1) {
		std::cout << "Check Select Game" << std::endl;
		TMS1000::g_cpu.K = 2; // Select game: K1: Game2 / K4: Game3 / K2: Game1
	} else if (TMS1000::g_cpu.R & 512) {
		std::cout << "Check Skill" << std::endl;
		TMS1000::g_cpu.K = 2; // Skill switch: K2 = L1 / K4 = L2 / K8 = L3 / K1 = L4
	}
	else if (TMS1000::g_cpu.R & 2) { // COLOR SWITCHES GREEN(K1) / RED(K2) / YELLOW(K4) / BLUE(K8)
		TMS1000::g_cpu.K = 
			((GetAsyncKeyState(0x31) & 0x8000) ? 1 : 0) |
			((GetAsyncKeyState(0x32) & 0x8000) ? 2 : 0)|
			((GetAsyncKeyState(0x33) & 0x8000) ? 4 : 0)|
			((GetAsyncKeyState(0x34) & 0x8000) ? 8 : 0);
	}
	else if (TMS1000::g_cpu.R & 4) { // START (K1) /LAST (K2) / LONGEST (K4)
		TMS1000::g_cpu.K =
			((GetAsyncKeyState(0x53) & 0x8000) ? 1 : 0) |
			((GetAsyncKeyState(0x4C) & 0x8000) ? 2 : 0) |
			((GetAsyncKeyState(0x4D) & 0x8000) ? 4 : 0);
	}
	else {
		TMS1000::g_cpu.K = 0;
	}
}

void onWriteOutput() {
	static WORD lastR;

	WORD outBits = TMS1000::g_cpu.R & 0xF0;//& 0x1F0;
	if (outBits != lastR) {
		if (outBits) {
			std::cout
				<< ((outBits & 16) ? "GREEN" : "    ")
				<< ((outBits & 32) ? "RED" : "  ")
				<< ((outBits & 64) ? "YELLOW" : "      ")
				<< ((outBits & 128) ? "BLUE" : "    ")
				//			<< " SPKR: " << ((outBits & 256) ? "1" : "0")
				<< std::endl;
		}
		else {
			std::cout << "LED OFF" << std::endl;
		}
		lastR = outBits;
	}
}

void uSleep(int waitTime) {
	__int64 time1 = 0, time2 = 0, freq = 0;

	QueryPerformanceCounter((LARGE_INTEGER *)&time1);
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

	do {
		QueryPerformanceCounter((LARGE_INTEGER *)&time2);
	} while ((time2 - time1) < waitTime);
}

int main() {
	Logger::RegisterLogCallback(LogCallback);

	CPUInfo cpuInfo = g_tms1000Info[CPU_TMS1000];

	try {
		cpuInfo.LoadConfig();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	TMS1000::Init(cpuInfo.GetROMWords(), cpuInfo.GetRAMWords());
	TMS1000::LoadROM("roms/TMS1000/Simon/simon.bin");
//	TMS1000::SaveROM("roms/TMS1000/Simon/simon2.h");

	ShowMonitor(cpuInfo);
	TestCPU();
//	return 1;

	TMS1000::SetInputCallback(onReadInput);
	TMS1000::SetOutputCallback(onWriteOutput);
	TMS1000::Reset();
	long lastTicks = 0;
	const int hbInterval = 1000000;
	uint64_t start = GetTickCount64();

	bool loop = true;
	std::cout << "Run" << std::endl;
	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
		TMS1000::Step();
		uSleep(200);
		long deltaCPU = TMS1000::GetTicks() - lastTicks;
		if (deltaCPU > hbInterval) {
			lastTicks = TMS1000::GetTicks();
			uint64_t end = GetTickCount64();
			std::cout << "Ticks:" << lastTicks
				<< " ("  << ((end-start) ? (deltaCPU * 1000 / (end - start)) : -1)
				<< " ticks/s)" << std::endl;
			start = end;
		}
	}

	return 0;
}
