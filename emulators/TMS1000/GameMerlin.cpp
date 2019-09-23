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
	

	void onReadKInput() {
		switch (TMS1000::g_cpu.O & 15) {
		case 0: // O0: K1(R0), K2(R1), K8(R2), K4(R3)
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(VK_OEM_3) & 0x8000) ? 1 : 0) | // '~
				((GetAsyncKeyState(0x31) & 0x8000) ? 2 : 0) | // 1
				((GetAsyncKeyState(0x32) & 0x8000) ? 8 : 0) | // 2
				((GetAsyncKeyState(0x33) & 0x8000) ? 4 : 0);  // 3
			break;
		case 4: // O1: K1(R4), K2(R5), K8(R6), K4(R7)
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(0x34) & 0x8000) ? 1 : 0) | // 4
				((GetAsyncKeyState(0x35) & 0x8000) ? 2 : 0) | // 5
				((GetAsyncKeyState(0x36) & 0x8000) ? 8 : 0) | // 6
				((GetAsyncKeyState(0x37) & 0x8000) ? 4 : 0);  // 7
			break;
		case 8: // O2: K1(R8), K2(R9), K8(R10), K4(SG)
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(0x38) & 0x8000) ? 1 : 0) | // 8
				((GetAsyncKeyState(0x39) & 0x8000) ? 2 : 0) | // 9
				((GetAsyncKeyState(0x30) & 0x8000) ? 8 : 0) | // 0
				((GetAsyncKeyState(0x53) & 0x8000) ? 4 : 0);  // S
			break;
		case 12: // O3:         K2(CT), K8(NG), K4(HM)
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(0x43) & 0x8000) ? 2 : 0) | // C
				((GetAsyncKeyState(0x4E) & 0x8000) ? 8 : 0) | // S
				((GetAsyncKeyState(0x48) & 0x8000) ? 4 : 0);  // H
			break;
		}
	}

	void onWriteROutput(BYTE bit, bool state) {
		static uint8_t button[] = { 219, 219 };

		std::tuple<short, short>& pos = LEDPosition[bit];
		Console::GotoXY(
			std::get<0>(pos),
			std::get<1>(pos));
		Console::SetColor(state ? 12 : 4);
		Console::Write((const char *)button);
	}

	void onWriteOOutput(BYTE o) {
		char output[] = "BUZZ";
		static bool lastSound = false;
		bool sound = o & 1;

		if (sound && !lastSound) {
			Console::WriteAt(1, 2, output, 4, 15);
			lastSound = true;
		}
		else if (!sound && lastSound) {
			Console::WriteAt(1, 2, output, 4, 7);
			lastSound = false;
		}
	}

	void ResetButtons() {
		for (int i = 0; i < 10; ++i) {
			onWriteROutput(i, false);
		}
	}

	void Init() {
		Console::Init();
		ResetButtons();

		TMS1000::LoadROM("roms/TMS1100/Merlin/mp3404.bin");

		TMS1000::SetInputKCallback(onReadKInput);
		TMS1000::SetOutputRCallback(onWriteROutput);
		TMS1000::SetOutputOCallback(onWriteOOutput);
	}
}
