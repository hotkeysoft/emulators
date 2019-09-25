#include "stdafx.h"
#include "TestTMS1100.h"
#include "Console.h"
#include "TMS1000.h"
#include <cassert>

namespace TestTMS1100
{
	using TMS1000::BYTE;
	using TMS1000::WORD;

	void TestTAY()
	{
		TMS1000::g_cpu.X = 0;
		TMS1000::g_cpu.Y = 5;
		TMS1000::g_cpu.A = 0xE;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x20); // TAY
		assert(TMS1000::g_cpu.X == 0);
		assert(TMS1000::g_cpu.Y == 0xE);
		assert(TMS1000::g_cpu.A == 0xE);
		assert(TMS1000::g_cpu.S);
		Console::UpdateStatus();
	}

	void TestTYA()
	{
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
	}

	void TestCLA()
	{
		TMS1000::g_cpu.X = 1;
		TMS1000::g_cpu.Y = 4;
		TMS1000::g_cpu.A = 7;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x7F); // CLA
		assert(TMS1000::g_cpu.X == 1);
		assert(TMS1000::g_cpu.Y == 4);
		assert(TMS1000::g_cpu.A == 0);
		assert(TMS1000::g_cpu.S);
		Console::UpdateStatus();
	}

	void TestTAM()
	{
		TMS1000::g_cpu.X = 0;
		TMS1000::g_cpu.Y = 6;
		TMS1000::g_cpu.A = 4;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x27); // TAM
		assert(TMS1000::g_cpu.X == 0);
		assert(TMS1000::g_cpu.Y == 6);
		assert(TMS1000::g_cpu.A == 4);
		assert(TMS1000::g_cpu.S);
		assert(TMS1000::GetRAM(TMS1000::GetM(0, 6)) == 4);
		Console::UpdateStatus();
	}

	void TestTAMIYC()
	{
		TMS1000::g_cpu.X = 3;
		TMS1000::g_cpu.Y = 9;
		TMS1000::g_cpu.A = 0;
		TMS1000::g_cpu.S = true;
		TMS1000::Exec(0x25); // TAMIYC
		assert(TMS1000::g_cpu.X == 3);
		assert(TMS1000::g_cpu.Y == 0xA);
		assert(TMS1000::g_cpu.A == 0);
		assert(TMS1000::g_cpu.S == false);
		assert(TMS1000::GetRAM(TMS1000::GetM(3, 9)) == 0);
		Console::UpdateStatus();

		TMS1000::g_cpu.Y = 15;
		TMS1000::g_cpu.A = 1;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x25); // TAMIYC
		assert(TMS1000::g_cpu.Y == 0);
		assert(TMS1000::g_cpu.A == 1);
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::GetRAM(TMS1000::GetM(3, 15)) == 1);
		Console::UpdateStatus();
	}

	void TestTAMDYN()
	{
		TMS1000::g_cpu.X = 3;
		TMS1000::g_cpu.Y = 9;
		TMS1000::g_cpu.A = 0;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x24); // TADYN
		assert(TMS1000::g_cpu.X == 3);
		assert(TMS1000::g_cpu.Y == 8);
		assert(TMS1000::g_cpu.A == 0);
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::GetRAM(TMS1000::GetM(3, 9)) == 0);
		Console::UpdateStatus();

		TMS1000::g_cpu.X = 3;
		TMS1000::g_cpu.Y = 0;
		TMS1000::g_cpu.A = 1;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x24); // TADYN
		assert(TMS1000::g_cpu.X == 3);
		assert(TMS1000::g_cpu.Y == 15);
		assert(TMS1000::g_cpu.A == 1);
		assert(TMS1000::g_cpu.S == false);
		assert(TMS1000::GetRAM(TMS1000::GetM(3, 0)) == 1);
		Console::UpdateStatus();

	}

	void TestTAMZA()
	{
		TMS1000::g_cpu.X = 0;
		TMS1000::g_cpu.Y = 0xE;
		TMS1000::g_cpu.A = 0xB;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x26); // TAMZA
		assert(TMS1000::g_cpu.X == 0);
		assert(TMS1000::g_cpu.Y == 0xE);
		assert(TMS1000::g_cpu.A == 0);
		assert(TMS1000::g_cpu.S);
		assert(TMS1000::GetRAM(TMS1000::GetM(0, 0xE)) == 0xB);
		Console::UpdateStatus();
	}

	void TestTMY()
	{
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
	}

	void TestTMA()
	{
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
	}

	void TestXMA()
	{
		TMS1000::g_cpu.X = 1;
		TMS1000::g_cpu.Y = 9;
		TMS1000::g_cpu.A = 3;
		TMS1000::g_cpu.S = false;
		assert(TMS1000::GetRAM(TMS1000::GetM(1, 9)) == 0xA);
		TMS1000::Exec(0x03); // XMA
		assert(TMS1000::g_cpu.X == 1);
		assert(TMS1000::g_cpu.Y == 9);
		assert(TMS1000::g_cpu.A == 0xA);
		assert(TMS1000::GetRAM(TMS1000::GetM(1, 9)) == 3);
		assert(TMS1000::g_cpu.S);
		Console::UpdateStatus();
	}

	void TestAMAAC()
	{
		TMS1000::g_cpu.X = 2;
		TMS1000::g_cpu.Y = 8;
		TMS1000::g_cpu.A = 1;
		TMS1000::g_cpu.S = false;
		TMS1000::PutRAM(15);
		TMS1000::Exec(0x06); // AMAAC
		assert(TMS1000::g_cpu.X == 2);
		assert(TMS1000::g_cpu.Y == 8);
		assert(TMS1000::g_cpu.A == 0); // (15 + 1) MOD 16
		assert(TMS1000::GetRAM(TMS1000::GetM(2, 8)) == 15);
		assert(TMS1000::g_cpu.S); // Carry set
		Console::UpdateStatus();

		TMS1000::g_cpu.X = 2;
		TMS1000::g_cpu.Y = 9;
		TMS1000::g_cpu.A = 0;
		TMS1000::g_cpu.S = false;
		TMS1000::PutRAM(15);
		TMS1000::Exec(0x06); // AMAAC
		assert(TMS1000::g_cpu.X == 2);
		assert(TMS1000::g_cpu.Y == 9);
		assert(TMS1000::g_cpu.A == 15); // (15 + 0) MOD 16
		assert(TMS1000::GetRAM(TMS1000::GetM(2, 9)) == 15);
		assert(!TMS1000::g_cpu.S); // Carry clear
		Console::UpdateStatus();
	}

	void TestSAMAN(BYTE m, BYTE a, BYTE res, bool carry) {
		TMS1000::g_cpu.X = 2;
		TMS1000::g_cpu.Y = 15;
		TMS1000::PutRAM(m);
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;

		TMS1000::Exec(0x3C); // SAMAN
		assert(TMS1000::g_cpu.X == 2);
		assert(TMS1000::g_cpu.Y == 15);
		assert(TMS1000::g_cpu.A == res);
		assert(TMS1000::GetRAM(TMS1000::GetM(2, 15)) == m);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestIMAC(BYTE m, BYTE expect, bool carry) {
		TMS1000::g_cpu.X = 0;
		TMS1000::g_cpu.Y = 1;
		TMS1000::g_cpu.A = 0;
		TMS1000::g_cpu.S = false;
		TMS1000::PutRAM(m);
		TMS1000::Exec(0x3E); // IMAC
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
		TMS1000::Exec(0x07); // IMAC
		assert(TMS1000::g_cpu.X == 0);
		assert(TMS1000::g_cpu.Y == 2);
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::GetRAM(TMS1000::GetM(0, 2)) == m);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestIAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x70); // IAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestDAN(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x77); // DAN
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA2AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x78); // A2AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA3AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x74); // A3AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA4AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x7C); // A4AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA5AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x72); // A5AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}
		
	void TestA6AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x7A); // A6AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA7AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x76); // A7AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA8AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x7E); // A8AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA9AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x71); // A9AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA10AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x79); // A10AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA11AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x75); // A11AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA12AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x7D); // A12AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA13AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x73); // A13AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestA14AAC(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x7B); // A14AAC
		assert(TMS1000::g_cpu.A == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestIYC(BYTE y, BYTE expect, bool carry) {
		TMS1000::g_cpu.Y = y;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x05); // IA
		assert(TMS1000::g_cpu.Y == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestDYN(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.Y = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x04); // DYN
		assert(TMS1000::g_cpu.Y == expect);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestCPAIZ(BYTE a, BYTE expect, bool carry) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.S = false;
		TMS1000::Exec(0x3D); // CPAIZ
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

		TMS1000::Exec(0x01); // ALEM
		assert(TMS1000::g_cpu.X == 2);
		assert(TMS1000::g_cpu.Y == 10);
		assert(TMS1000::g_cpu.A == a);
		assert(TMS1000::GetRAM(TMS1000::GetM(2, 10)) == m);
		assert(TMS1000::g_cpu.S == carry);
		Console::UpdateStatus();
	}

	void TestMNEA(BYTE a, BYTE m, bool status) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.X = 4;
		TMS1000::g_cpu.Y = m;
		TMS1000::PutRAM(m);
		TMS1000::g_cpu.S = false;

		TMS1000::Exec(0x00); // MNEA
		assert(TMS1000::g_cpu.A == a);
		assert(TMS1000::g_cpu.X == 4);
		assert(TMS1000::g_cpu.Y == m);
		assert(TMS1000::GetRAM(TMS1000::GetM(4, m)) == m);
		assert(TMS1000::g_cpu.S == status);
		Console::UpdateStatus();
	}

	void TestMNEZ(BYTE m, bool status) {
		TMS1000::g_cpu.X = 3;
		TMS1000::g_cpu.Y = 3;
		TMS1000::PutRAM(m);
		TMS1000::g_cpu.S = false;

		TMS1000::Exec(0x3F); // MNEZ
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

		TMS1000::Exec(0x0E); // KNEZ
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

	void TestSETR(BYTE r, bool set) {
		TMS1000::g_cpu.R[r] = false;
		TMS1000::g_cpu.Y = r;
		TMS1000::g_cpu.S = false;

		TMS1000::Exec(0x0D); // SETR
		assert(set == TMS1000::g_cpu.R[r]);
		assert(TMS1000::g_cpu.S == true);
		Console::UpdateStatus();
	}

	void TestRSTR(BYTE r, bool reset) {
		TMS1000::g_cpu.R[r] = true;
		TMS1000::g_cpu.Y = r;
		TMS1000::g_cpu.S = false;

		TMS1000::Exec(0x0C); // RSTR
		assert(TMS1000::g_cpu.R[r] != reset);
		assert(TMS1000::g_cpu.S == true);
		Console::UpdateStatus();
	}

	void TestTDO(BYTE a, bool SL) {
		TMS1000::g_cpu.A = a;
		TMS1000::g_cpu.SL = SL;
		TMS1000::g_cpu.S = false;

		TMS1000::Exec(0x0A); // TDO
		assert(TMS1000::g_cpu.O == (a | (SL ? 0x10 : 0)));
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

		TMS1000::Exec(0x09);
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

	void TestCOMC() {
		TMS1000::g_cpu.CB = 0;
		
		TMS1000::Exec(0x0B);
		assert(TMS1000::g_cpu.CB == 1);
		Console::UpdateStatus();

		TMS1000::Exec(0x0B);
		assert(TMS1000::g_cpu.CB == 0);
		Console::UpdateStatus();

		TMS1000::Exec(0x0B);
		assert(TMS1000::g_cpu.CB == 1);
		Console::UpdateStatus();
	}

	void TestBR(BYTE opCode, BYTE addr) {
		TMS1000::g_cpu.PA = 0x01;
		TMS1000::g_cpu.PB = 0x02;
		TMS1000::g_cpu.PC = (~addr) & 0x3F;
		TMS1000::g_cpu.CA = 0;
		TMS1000::g_cpu.CB = 1;
		TMS1000::g_cpu.CL = false;
		TMS1000::g_cpu.S = false;

		// S = 0 : No branch
		TMS1000::Exec(opCode);
		assert(TMS1000::g_cpu.PA == 0x01);
		assert(TMS1000::g_cpu.PB == 0x02);
		assert(TMS1000::g_cpu.PC == ((~addr) & 0x3F));
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::g_cpu.CA == 0);
		assert(TMS1000::g_cpu.CB == 1);
		assert(TMS1000::g_cpu.CL == false);
		Console::UpdateStatus();

		TMS1000::g_cpu.CL = true;
		// S = 1, CL = 0 : Branch
		TMS1000::Exec(opCode);
		assert(TMS1000::g_cpu.PA == 0x01);
		assert(TMS1000::g_cpu.PB == 0x02);
		assert(TMS1000::g_cpu.PC == addr);
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::g_cpu.CA == 1);
		assert(TMS1000::g_cpu.CB == 1);
		assert(TMS1000::g_cpu.CL == true);
		Console::UpdateStatus();

		TMS1000::g_cpu.CL = false;
		TMS1000::g_cpu.CA = false;
		TMS1000::g_cpu.CB = 1;
		// S = 1, CL = 0 : Branch
		TMS1000::Exec(opCode);
		assert(TMS1000::g_cpu.PA == 0x02);
		assert(TMS1000::g_cpu.PB == 0x02);
		assert(TMS1000::g_cpu.PC == addr);
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::g_cpu.CA == 1);
		assert(TMS1000::g_cpu.CB == 1);
		assert(TMS1000::g_cpu.CL == false);
		Console::UpdateStatus();
	}

	void TestCALL(BYTE opCode, BYTE addr) {
		TMS1000::g_cpu.PA = 0x01;
		TMS1000::g_cpu.PB = 0x02;
		TMS1000::g_cpu.PC = (~addr) & 0x3F;
		TMS1000::g_cpu.SR = 0x03;
		TMS1000::g_cpu.CA = 0;
		TMS1000::g_cpu.CB = 1;
		TMS1000::g_cpu.CS = 1;
		TMS1000::g_cpu.CL = false;
		TMS1000::g_cpu.S = false;

		// S = 0 : No call
		TMS1000::Exec(opCode);
		assert(TMS1000::g_cpu.PA == 0x01);
		assert(TMS1000::g_cpu.PB == 0x02);
		assert(TMS1000::g_cpu.PC == ((~addr) & 0x3F));
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::g_cpu.SR == 0x03);
		assert(TMS1000::g_cpu.CA == 0);
		assert(TMS1000::g_cpu.CB == 1);
		assert(TMS1000::g_cpu.CS == 1);
		assert(TMS1000::g_cpu.CL == false);
		Console::UpdateStatus();

		TMS1000::g_cpu.CL = true;
		// S = 1, CL = 1 : Call
		TMS1000::Exec(opCode);
		assert(TMS1000::g_cpu.PA == 0x01);
		assert(TMS1000::g_cpu.PB == 0x01);
		assert(TMS1000::g_cpu.PC == addr);
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::g_cpu.SR == 0x03);
		assert(TMS1000::g_cpu.CA == 1);
		assert(TMS1000::g_cpu.CB == 1);
		assert(TMS1000::g_cpu.CS == 1);
		assert(TMS1000::g_cpu.CL == true);
		Console::UpdateStatus();

		TMS1000::g_cpu.PA = 0x01;
		TMS1000::g_cpu.PB = 0x02;
		TMS1000::g_cpu.PC = (~addr) & 0x3F;
		TMS1000::g_cpu.SR = 0x03;
		TMS1000::g_cpu.CA = 0;
		TMS1000::g_cpu.CB = 1;
		TMS1000::g_cpu.CS = 1;
		TMS1000::g_cpu.CL = false;
		// PC incremented in Step, so SR will be = PC if exec is call directly
		BYTE retPC = TMS1000::g_cpu.PC;

		// S = 1, CL = 0 : CALL
		TMS1000::Exec(opCode);
		assert(TMS1000::g_cpu.PA == 0x02);
		assert(TMS1000::g_cpu.PB == 0x01);
		assert(TMS1000::g_cpu.PC == addr);
		assert(TMS1000::g_cpu.SR == retPC);
		assert(TMS1000::g_cpu.S == true);
		assert(TMS1000::g_cpu.CA == 1);
		assert(TMS1000::g_cpu.CB == 1);
		assert(TMS1000::g_cpu.CS == 0);
		assert(TMS1000::g_cpu.CL == true);
		Console::UpdateStatus();
	}

	void TestRETN() {
		TMS1000::g_cpu.PA = 0x01;
		TMS1000::g_cpu.PB = 0x02;
		TMS1000::g_cpu.PC = 0x03;
		TMS1000::g_cpu.SR = 0x04;
		TMS1000::g_cpu.CL = false;
		TMS1000::g_cpu.CA = 0;
		TMS1000::g_cpu.CS = 1;
		TMS1000::g_cpu.S = false;

		// CL = 0
		TMS1000::Exec(0x0F); // RETN
		assert(TMS1000::g_cpu.PA == 0x02);
		assert(TMS1000::g_cpu.PB == 0x02);
		assert(TMS1000::g_cpu.PC == 0x03);
		assert(TMS1000::g_cpu.SR == 0x04);
		assert(TMS1000::g_cpu.CL == false);
		assert(TMS1000::g_cpu.CA == 0);
		assert(TMS1000::g_cpu.CS == 1);
		assert(TMS1000::g_cpu.S == true);
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
		assert(TMS1000::g_cpu.CL == false);
		assert(TMS1000::g_cpu.CA == 1);
		assert(TMS1000::g_cpu.CS == 1);
		assert(TMS1000::g_cpu.S == true);
		Console::UpdateStatus();
	}

	void Test() {
		// Unit tests
		memset(TMS1000::g_memory.RAM, 0xA, TMS1000::g_memory.ramSize);
		assert(TMS1000::g_memory.ramSize == 128);
		assert(TMS1000::g_memory.romSize == 2048);

		// Register <=> Register

		// TAY
		TestTAY();

		// TYA
		TestTYA();

		// CLA
		TestCLA();

		// Register <=> Memory

		// TAM
		TestTAM();

		// TAMIY
		TestTAMIYC();

		// TAMDYN
		TestTAMDYN();

		// TAMZA
		TestTAMZA();

		// TMY
		TestTMY();

		// TMA
		TestTMA();

		// XMA
		TestXMA();

		// Arithmetic instructions

		// AMAAC
		TestAMAAC();

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

		// IAC
		TestIAC(0, 1, false);
		TestIAC(1, 2, false);
		TestIAC(14, 15, false);
		TestIAC(15, 0, true);

		// DAN
		TestDAN(0, 15, false);
		TestDAN(1, 0, true);
		TestDAN(15, 14, true);

		// A2AAC
		TestA2AAC(0, 2, false);
		TestA2AAC(13, 15, false);
		TestA2AAC(14, 0, true);
		TestA2AAC(15, 1, true);

		// A3AAC
		TestA3AAC(0, 3, false);
		TestA3AAC(12, 15, false);
		TestA3AAC(13, 0, true);
		TestA3AAC(14, 1, true);

		// A4AAC
		TestA4AAC(0, 4, false);
		TestA4AAC(11, 15, false);
		TestA4AAC(12, 0, true);
		TestA4AAC(13, 1, true);

		TestA5AAC(0, 5, false);
		TestA5AAC(10, 15, false);
		TestA5AAC(11, 0, true);
		TestA5AAC(12, 1, true);

		// A6AAC
		TestA6AAC(0, 6, false);
		TestA6AAC(9, 15, false);
		TestA6AAC(10, 0, true);
		TestA6AAC(11, 1, true);

		// A7AAC
		TestA7AAC(0, 7, false);
		TestA7AAC(8, 15, false);
		TestA7AAC(9, 0, true);
		TestA7AAC(10, 1, true);

		// A8AAC
		TestA8AAC(0, 8, false);
		TestA8AAC(7, 15, false);
		TestA8AAC(8, 0, true);
		TestA8AAC(9, 1, true);

		// A9AAC
		TestA9AAC(0, 9, false);
		TestA9AAC(6, 15, false);
		TestA9AAC(7, 0, true);
		TestA9AAC(8, 1, true);

		// A10AAC
		TestA10AAC(0, 10, false);
		TestA10AAC(5, 15, false);
		TestA10AAC(6, 0, true);
		TestA10AAC(7, 1, true);

		// A11AAC
		TestA11AAC(0, 11, false);
		TestA11AAC(4, 15, false);
		TestA11AAC(5, 0, true);
		TestA11AAC(6, 1, true);

		// A12AAC
		TestA12AAC(0, 12, false);
		TestA12AAC(3, 15, false);
		TestA12AAC(4, 0, true);
		TestA12AAC(5, 1, true);

		// A13AAC
		TestA13AAC(0, 13, false);
		TestA13AAC(2, 15, false);
		TestA13AAC(3, 0, true);
		TestA13AAC(4, 1, true);

		// A14AAC
		TestA14AAC(0, 14, false);
		TestA14AAC(1, 15, false);
		TestA14AAC(2, 0, true);
		TestA14AAC(3, 1, true);

		// IYC
		TestIYC(0, 1, false);
		TestIYC(14, 15, false);
		TestIYC(15, 0, true);

		// DYN
		TestDYN(0, 15, false);
		TestDYN(1, 0, true);
		TestDYN(15, 14, true);

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

		// Logical Compare Instructions

		// MNEA
		for (int i = 0; i < 16; ++i) {
			for (int j = 0; j < 16; ++j) {
				TestMNEA(i, j, i != j);
			}
		}

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

		// Constant Transfer Instructions

		// TCY
		const BYTE TCY0 = 0x40;
		const BYTE TCY8 = 0x41;
		const BYTE TCY4 = 0x42;
		const BYTE TCY12 = 0x43;
		const BYTE TCY2 = 0x44;
		const BYTE TCY10 = 0x45;
		const BYTE TCY6 = 0x46;
		const BYTE TCY14 = 0x47;
		const BYTE TCY1 = 0x48;
		const BYTE TCY9 = 0x49;
		const BYTE TCY5 = 0x4a;
		const BYTE TCY13 = 0x4b;
		const BYTE TCY3 = 0x4c;
		const BYTE TCY11 = 0x4d;
		const BYTE TCY7 = 0x4e;
		const BYTE TCY15 = 0x4f;

		TestTCY(TCY0, 0);
		TestTCY(TCY1, 1);
		TestTCY(TCY2, 2);
		TestTCY(TCY3, 3);
		TestTCY(TCY4, 4);
		TestTCY(TCY5, 5);
		TestTCY(TCY6, 6);
		TestTCY(TCY7, 7);
		TestTCY(TCY8, 8);
		TestTCY(TCY9, 9);
		TestTCY(TCY10, 10);
		TestTCY(TCY11, 11);
		TestTCY(TCY12, 12);
		TestTCY(TCY13, 13);
		TestTCY(TCY14, 14);
		TestTCY(TCY15, 15);

		// TCMIY
		for (int x = 0; x < 8; ++x) {
			TMS1000::g_cpu.X = x;
			TMS1000::g_cpu.Y = 0;
			for (int i = 0; i < 16; ++i) {
				TestTCMIY(0x60 + TMS1000::GetC(x), x);
			}
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
			for (int x = 0; x < 8; ++x) {
				TMS1000::g_cpu.X = x;
				TestSETR(i, x<=3);
			}
		}

		// RSTR
		for (int i = 0; i <= 10; ++i) {
			for (int x = 0; x < 8; ++x) {
				TMS1000::g_cpu.X = x;
				TestRSTR(i, x<=3);
			}
		}

		// TDO
		for (int i = 0; i < 16; ++i) {
			TestTDO(i, false);
			TestTDO(i, true);
		}

		// RAM X Addressing Instructions

		// LDX
		const BYTE LDX0 = 0x28;
		const BYTE LDX4 = 0x29;
		const BYTE LDX2 = 0x2A;
		const BYTE LDX6 = 0x2B;
		const BYTE LDX1 = 0x2C;
		const BYTE LDX5 = 0x2D;
		const BYTE LDX3 = 0x2E;
		const BYTE LDX7 = 0x2F;

		TestLDX(LDX0, 0);
		TestLDX(LDX1, 1);
		TestLDX(LDX2, 2);
		TestLDX(LDX3, 3);
		TestLDX(LDX4, 4);
		TestLDX(LDX5, 5);
		TestLDX(LDX6, 6);
		TestLDX(LDX7, 7);

		// COMX
		TextCOMX(0, 4);
		TextCOMX(1, 5);
		TextCOMX(2, 6);
		TextCOMX(3, 7);
		TextCOMX(4, 0);
		TextCOMX(5, 1);
		TextCOMX(6, 2);
		TextCOMX(7, 3);

		// ROM Addressing Instructions

		// COMC
		TestCOMC();

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
}