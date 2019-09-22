#include "stdafx.h"
#include "GameSimon.h"
#include "CPUInfo.h"
#include "Console.h"
#include "TMS1000.h"
#include <iostream>

namespace GameMerlin
{
	std::tuple<short, short> LEDPosition[] = {
		       { 20,  4 },
	{ 10,  8 },{ 20,  8 },{ 30,  8 },
	{ 10, 12 },{ 20, 12 },{ 30, 12 },
	{ 10, 16 },{ 20, 16 },{ 30, 16 },
	           { 20, 20 }
	};

	//       R0
	//	R1   R2   R3
	//	R4   R5   R6
	//	R7   R8   R9
	//	     R10

//       +----+  +----+  +----+  +----+
//O0 o---| R0 |--| R1 |--| R2 |--| R3 |
//       +----+  +----+  +----+  +----+
//          |       |       |       |
//       +----+  +----+  +----+  +----+
//O1 o---| R4 |--| R5 |--| R6 |--| R7 |
//       +----+  +----+  +----+  +----+
//          |       |       |       |
//       +----+  +----+  +----+  +----+
//O2 o---| R8 |--| R9 |--|R10 |--| SG |
//       +----+  +----+  +----+  +----+
//          |       |       |       |
//          |    +----+  +----+  +----+
//O3 o------+----| CT |--| NG |--| HM |
//          |    +----+  +----+  +----+
//          |       |       |       |
//          o       o       o       o
//         K1      K2      K8      K4
	

	void onReadInput() {
//		char output[16] = "trest";
///		sprintf(output, "output = %02x\n", TMS1000::g_cpu.O);
//		Console::SetColor(15);
//		fprintf(stderr, output);
//		Console::GotoXY(1, 2);
//		Console::Write(output);
		//if (TMS1000::g_cpu.R & 1) {
		//	//std::cout << "Check Select Game" << std::endl;
		//	TMS1000::g_cpu.K = 2; // Select game: K1: Game2 / K4: Game3 / K2: Game1
		//}
		//else if (TMS1000::g_cpu.R & 512) {
		//	//std::cout << "Check Skill" << std::endl;
		//	TMS1000::g_cpu.K = 2; // Skill switch: K2 = L1 / K4 = L2 / K8 = L3 / K1 = L4
		//}
		//else if (TMS1000::g_cpu.R & 2) { // COLOR SWITCHES GREEN(K1) / RED(K2) / YELLOW(K4) / BLUE(K8)
		//	TMS1000::g_cpu.K =
		//		((GetAsyncKeyState(0x31) & 0x8000) ? 1 : 0) |
		//		((GetAsyncKeyState(0x32) & 0x8000) ? 2 : 0) |
		//		((GetAsyncKeyState(0x33) & 0x8000) ? 4 : 0) |
		//		((GetAsyncKeyState(0x34) & 0x8000) ? 8 : 0);
		//}
		//else if (TMS1000::g_cpu.R & 4) { // START (K1) /LAST (K2) / LONGEST (K4)
		//	TMS1000::g_cpu.K =
		//		((GetAsyncKeyState(0x53) & 0x8000) ? 1 : 0) | //S
		//		((GetAsyncKeyState(0x4C) & 0x8000) ? 2 : 0) | //T
		//		((GetAsyncKeyState(0x54) & 0x8000) ? 4 : 0);  //L
		//}
		//else {
		//	TMS1000::g_cpu.K = 0;
		//}
	}

	void ShowButtons(WORD r) {
		static uint8_t button[] = { 219, 219 };
		for (int i = 0; i < 11; ++i)
		{
			std::tuple<short, short>& pos = LEDPosition[i];
			Console::GotoXY(
				std::get<0>(pos),
				std::get<1>(pos));
			Console::SetColor((r & (1 << i)) ? 12 : 4);
			Console::Write((const char *)button);
		}
	}

	void onWriteOutput() {
		static WORD lastR;

		WORD outBits = TMS1000::g_cpu.R & 0x7FF;;
		if (outBits != lastR) {
			ShowButtons(outBits);
			lastR = outBits;
		}

		char output[16] = "trest";
		sprintf(output, "output = %02x\n", TMS1000::g_cpu.O);
		Console::SetColor(15);
		fprintf(stderr, output);
	}

	void Init() {
		Console::Init();
		ShowButtons(0);
		//Console::SetColor(0, 7);
		//Console::GotoXY(7, 3);
		//Console::Write("  S: Start      L: Last      T: Longest  ");

		//Console::GotoXY(7, 9);
		//Console::Write("      1         2         3         4    ");
		TMS1000::LoadROM("roms/TMS1100/Merlin/mp3404.bin");

		TMS1000::SetInputCallback(onReadInput);
		TMS1000::SetOutputCallback(onWriteOutput);
	}
}
