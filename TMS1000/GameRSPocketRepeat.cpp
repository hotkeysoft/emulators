#include "stdafx.h"
#include "GameSimon.h"
#include "CPUInfo.h"
#include "Console.h"
#include "TMS1000.h"
#include <iostream>

namespace GameRSPocketRepeat 
{
	//	1  K1 - yellow button(beginner / game 1)
	//	2  K2 - blue button(amateur / game 2)
	//	3  K4 - orange button(expert / last)
	//	4  K8 - red button(champ / again)

	void onReadKInput() {
		TMS1000::g_cpu.K =
			((GetAsyncKeyState(0x31) & 0x8000) ? 1 : 0) |
			((GetAsyncKeyState(0x32) & 0x8000) ? 2 : 0) |
			((GetAsyncKeyState(0x33) & 0x8000) ? 4 : 0) |
			((GetAsyncKeyState(0x34) & 0x8000) ? 8 : 0);
	}

	void onWriteROutput(BYTE bit, bool set) {
		static const uint8_t offBlock[] = { 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2, 0xb2 };
		static const uint8_t onBlock[] = { 219, 219, 219, 219, 219, 219, 219 };

		char output[] = "BUZZ";

		//	R0 - yellow LED
		//	R1 - blue LED
		//	R2 - orange LED
		//	R3 - red LED

		short x;
		bool color = true;
		switch (bit) {
		case 0:
			x = 10;
			Console::SetColor(set ? 14 : 7);
			break;
		case 1:
			x = 20;
			Console::SetColor(set ? 12 : 4);
			break;
		case 2:
			x = 30;
			Console::SetColor(set ? 14 : 6);
			break;
		case 3:
			x = 40;
			Console::SetColor(set? 9 : 1);
			break;
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

		//	1  K1 - yellow button(beginner / game 1)
		//	2  K2 - blue button(amateur / game 2)
		//	3  K4 - orange button(expert / last)
		//	4  K8 - red button(champ / again)

		Console::SetColor(0, 7);
		Console::GotoXY(7, 3);
		Console::GotoXY(7, 9);
		Console::Write("      1         2         3         4    ");
		Console::GotoXY(7, 10);
		Console::Write("   Beginner  Amateur    Expert    Champ  ");
		Console::GotoXY(7, 11);
		Console::Write("    GAME1     GAME2      LAST     AGAIN  ");

		TMS1000::LoadROM("roms/TMS1700/RSPocketRepeat/mp1801.bin");

		TMS1000::SetInputKCallback(onReadKInput);
		TMS1000::SetOutputRCallback(onWriteROutput);
	}
}
