#include "stdafx.h"
#include "GameSimon.h"
#include "CPUInfo.h"
#include "Console.h"
#include "TMS1000.h"
#include <iostream>

namespace GameSplitSecond
{
	//  LED numbering
	//  
	//     3     2     1
	// 10  9  8  7  6  5  4  
	//    13    12    11
	// 20 19 18 17 16 15 14
	//    23    22    21
	// 30 29 28 27 26 25 24
	//    33    32    31    
	// 40 39 38 37 36 35 34
	//    43    42    41
	// 50 49 48 47 46 45 44
	//    53    52    51
	//     
	// 
	// LED matrix
	// 
	//     R7  R6  R5  R4  R3  R2  R1  R0
	// O6   0   0  44  34  24  14   4   0
	// O5  41 (35) 31 (25) 21 (15) 11  (5)
	// O4  51 (45) 46  36  26  16   6   1
	// O3  42 (37) 32 (27) 22 (17) 12  (7)
	// O2  52 (47) 48  38  28  18   8   2
	// O1  43 (39) 33 (29) 23 (19) 13  (9)
	// O0  53 (49) 50  40  30  20  10   3  


	const int LEDcount = 53;

	std::tuple<short, short> LEDPosition[LEDcount] = {
	{  7,  1 },{ 4,  1 },{ 1,  1 },
	{ 10,  2 },{ 8,  2 },{ 7,  2 },{ 5,  2 },{ 4,  2 },{ 2,  2 },{ 1,  2 },
	{  7,  3 },{ 4,  3 },{ 1,  3 },
	{ 10,  4 },{ 8,  4 },{ 7,  4 },{ 5,  4 },{ 4,  4 },{ 2,  4 },{ 1,  4 },
	{  7,  5 },{ 4,  5 },{ 1,  5 },
	{ 10,  6 },{ 8,  6 },{ 7,  6 },{ 5,  6 },{ 4,  6 },{ 2,  6 },{ 1,  6 },
	{  7,  7 },{ 4,  7 },{ 1,  7 },
	{ 10,  8 },{ 8,  8 },{ 7,  8 },{ 5,  8 },{ 4,  8 },{ 2,  8 },{ 1,  8 },
	{  7,  9 },{ 4,  9 },{ 1,  9 },
	{ 10, 10 },{ 8, 10 },{ 7, 10 },{ 5, 10 },{ 4, 10 },{ 2, 10 },{ 1, 10 },
	{  7, 11 },{ 4, 11 },{ 1, 11 },
	};


	const char* LED[LEDcount] = {
		" \xcd\xcd ", 				" \xcd\xcd", 				" \xcd\xcd",
		"\xba",		"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba",
		" \xcd\xcd ", 				" \xcd\xcd", 				" \xcd\xcd ",
		"\xba",		"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba",
		" \xcd\xcd ", 				" \xcd\xcd", 				" \xcd\xcd ",
		"\xba",		"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba",
		" \xcd\xcd ", 				" \xcd\xcd", 				" \xcd\xcd ",
		"\xba",		"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba",
		" \xcd\xcd ", 				" \xcd\xcd", 				" \xcd\xcd ",
		"\xba",		"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba", 	"\xde\xdd", 	"\xba",
		" \xcd\xcd ", 				" \xcd\xcd", 				" \xcd\xcd ",
	};

	// keypad matrix
	//      K1     K2     K4
	// R9  Right  Up     Left
	// R10 Down   Select Start

	void onReadKInput() {
		if (TMS1000::g_cpu.R[9]) {
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(VK_RIGHT) & 0x8000) ? 1 : 0) |
				((GetAsyncKeyState(VK_UP) & 0x8000) ? 2 : 0) |
				((GetAsyncKeyState(VK_LEFT) & 0x8000) ? 4 : 0);
		}
		else if (TMS1000::g_cpu.R[10]) {
			TMS1000::g_cpu.K =
				((GetAsyncKeyState(VK_DOWN) & 0x8000) ? 1 : 0) |
				((GetAsyncKeyState(0x54) & 0x8000) ? 2 : 0) | // Selec[t]
				((GetAsyncKeyState(0x53) & 0x8000) ? 4 : 0); // [S]tart
		}
	}

	void writeLED(int id, bool state) {
		if (id < 0) { 
			return; 
		}
		std::tuple<short, short>& pos = LEDPosition[id];
		Console::GotoXY(
			std::get<0>(pos)+10,
			std::get<1>(pos)+5);
		Console::SetColor(state ? (1*16)+14 : (1*16)+0);
		Console::Write((const char *)LED[id]);
	}

	void onWriteROutput(BYTE bit, bool state) {
		const char output[] = "BUZZ";
		static bool lastSound = false;
		bool sound = (bit == 8) && state;

		if (sound && !lastSound) {
			Console::WriteAt(1, 2, output, 4, 15);
			lastSound = true;
		}
		else if (!sound && lastSound) {
			Console::WriteAt(1, 2, output, 4, 0);
			lastSound = false;
		}

	}
	
	//     R7  R6  R5  R4  R3  R2  R1  R0
	const int ROMap[7][8] = {
	{ 53,  49,  50,  40,  30,  20,  10,   3 }, //O0
	{ 43,  39,  33,  29,  23,  19,  13,   9 }, //O1
	{ 52,  47,  48,  38,  28,  18,   8,   2 }, //O2
	{ 42,  37,  32,  27,  22,  17,  12,   7 }, //O3
	{ 51,  45,  46,  36,  26,  16,   6,   1 }, //O4
	{ 41,  35,  31,  25,  21,  15,  11,   5 }, //O5
	{  0,   0,  44,  34,  24,  14,   4,   0 }, //O6
	};

	void onWriteOOutput(BYTE o) {
		if (o > 0) {
			for (int i = 0; i < 8; ++i) {
				writeLED(ROMap[(o & 7)-1][7-i] - 1, TMS1000::g_cpu.R[i]);
			}
		}
	}

	void ResetButtons() {
		for (int i = 0; i < LEDcount; ++i) {
			writeLED(i, true);
		}
		for (int i = 0; i < LEDcount; ++i) {
			writeLED(i, false);
		}
	}

	void Init() {
		Console::Init();
		ResetButtons();
		
		TMS1000::LoadROM("roms/TMS1400/SplitSecond/splitsecond.bin");

		TMS1000::SetInputKCallback(onReadKInput);
		TMS1000::SetOutputRCallback(onWriteROutput);
		TMS1000::SetOutputOCallback(onWriteOOutput);
	}
}
