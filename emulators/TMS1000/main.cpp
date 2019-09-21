
#include "stdafx.h"
#include "CPUInfo.h"
#include "Console.h"

#include <conio.h>
#include <iostream>
#include "TMS1000.h"
#include "TestTMS1000.h"

void LogCallback(const char *str) {
	fprintf(stderr, str);
}

void ShowMonitor(CPUInfo &cpuInfo) {
	Console::Init(&cpuInfo);
	TMS1000::Reset();
	Console::UpdateStatus();
}

void RunMonitor() {
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

CPUInfo& InitCPU(TMS1000::TMS1000Family model) {
	CPUInfo& cpuInfo = g_tms1000Info[model];

	try {
		cpuInfo.LoadConfig();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		throw;
	}

	TMS1000::Init(model, cpuInfo.GetROMWords(), cpuInfo.GetRAMWords());

	return cpuInfo;
}

int main() {
	Logger::RegisterLogCallback(LogCallback);

	if (1)
	{
		CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1000);
		TMS1000::LoadROM("roms/TMS1000/Simon/simon.bin");
		ShowMonitor(cpuInfo);
		TestTMS1000::Test();
	}

	if (0)
	{
		CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1100);
		//TMS1000::LoadROM("roms/TMS1100/test.bin");
		TMS1000::LoadROM("roms/TMS1100/connect4.bin");
		ShowMonitor(cpuInfo);
		//TestTMS1100();
	}

	RunMonitor();
	return 0;

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

