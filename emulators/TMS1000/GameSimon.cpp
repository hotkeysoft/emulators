#include "stdafx.h"
#include "GameSimon.h"
#include "CPUInfo.h"
#include "Console.h"
#include "TMS1000.h"
#include <iostream>

namespace GameSimon 
{
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

	void onReadKInput() {
		if (TMS1000::g_cpu.R & 1) {
			//std::cout << "Check Select Game" << std::endl;
			TMS1000::g_cpu.K = 2; // Select game: K1: Game2 / K4: Game3 / K2: Game1
		}
		else if (TMS1000::g_cpu.R & 512) {
			//std::cout << "Check Skill" << std::endl;
			TMS1000::g_cpu.K = 2; // Skill switch: K2 = L1 / K4 = L2 / K8 = L3 / K1 = L4
		}
		else if (TMS1000::g_cpu.R & 2) { // COLOR SWITCHES GREEN(K1) / RED(K2) / YELLOW(K4) / BLUE(K8)
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(0x31) & 0x8000) ? 1 : 0) |
				((GetAsyncKeyState(0x32) & 0x8000) ? 2 : 0) |
				((GetAsyncKeyState(0x33) & 0x8000) ? 4 : 0) |
				((GetAsyncKeyState(0x34) & 0x8000) ? 8 : 0);
		}
		else if (TMS1000::g_cpu.R & 4) { // START (K1) /LAST (K2) / LONGEST (K4)
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(0x53) & 0x8000) ? 1 : 0) | //S
				((GetAsyncKeyState(0x4C) & 0x8000) ? 2 : 0) | //T
				((GetAsyncKeyState(0x54) & 0x8000) ? 4 : 0);  //L
		}
		else {
			TMS1000::g_cpu.K = 0;
		}
	}

	void onWriteROutput(BYTE bit, bool set) {
		static const uint8_t offBlock[] = { 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2 };
		static const uint8_t onBlock[] = { 219, 219, 219, 219, 219, 219, 219 };

		char output[] = "BUZZ";

		short x;
		bool color = true;
		switch (bit) {
		case 4:
			x = 10;
			Console::SetColor(set ? 10 : 2);
			break;
		case 5:
			x = 20;
			Console::SetColor(set ? 12 : 4);
			break;
		case 6:
			x = 30;
			Console::SetColor(set ? 14 : 6);
			break;
		case 7:
			x = 40;
			Console::SetColor(set? 9 : 1);
			break;
		case 8: Console::WriteAt(1, 2, output, 4, set ? 15 : 7);
			// fallthrough
		default: 
			color = false;
		}


		if (color) {
			for (int i = 0; i < 3; ++i)
			{
				Console::GotoXY(x, 5 + i);
				Console::Write((const char*)(set ? onBlock : offBlock));
			}
		}
	}

	void ResetColors(){		
		onWriteROutput(4, false);
		onWriteROutput(5, false);
		onWriteROutput(6, false);
		onWriteROutput(7, false);
	}

	void Init() {
		Console::Init();
		ResetColors();

		Console::SetColor(0, 7);
		Console::GotoXY(7, 3);
		Console::Write("  S: Start      L: Last      T: Longest  ");

		Console::GotoXY(7, 9);
		Console::Write("      1         2         3         4    ");
		TMS1000::LoadROM("roms/TMS1000/Simon/simon.bin");

		TMS1000::SetInputKCallback(onReadKInput);
		TMS1000::SetOutputRCallback(onWriteROutput);
	}
}
