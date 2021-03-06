
#include "stdafx.h"
#include "CPUInfo.h"
#include "Console.h"

#include <conio.h>
#include <iostream>
#include "TMS1000.h"
#include "TestTMS1000.h"
#include "TestTMS1100.h"

#include "GameSimon.h"
#include "GameMerlin.h"
#include "GameSplitSecond.h"
#include "GameRSPocketRepeat.h"

void LogCallback(const char *str) {
	fprintf(stderr, str);
}

void ShowMonitor(CPUInfo &cpuInfo) {
	Console::Init(&cpuInfo);
	TMS1000::Reset();
	Console::UpdateStatus();
}

bool IsBreakpoint() {
//	return (TMS1000::GetROMAddress() == 1085) && (TMS1000::g_cpu.PC = 61);
	return false;
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
			while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000) && !IsBreakpoint()) {
				TMS1000::Step();
				if ((TMS1000::GetTicks() - lastTicks) > 10000) {
					Console::UpdateStatus();
					lastTicks = TMS1000::GetTicks();
				}
			}
			Console::SetRunMode(false);
			Console::UpdateStatus();
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

void WriteASMFile(const CPUInfo &info, const char* path) {
	FILE* f = fopen(path, "w");
	if (f) {
		int pages = TMS1000::g_memory.romSize / 64;
		for (int p = 0; p < pages; ++p) {
			fprintf(f, "; Page %X\n", p);

			for (int b = 0; b < 64; ++b) {
				fprintf(f, "%X:%02X\t%s\n", p, b, info.Disassemble(TMS1000::g_memory.ROM[p * 64 + b]).c_str());
			}
		}
	}
	fclose(f);
}

int main() {
	Logger::RegisterLogCallback(LogCallback);

	if (0)
	{
		CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1000);
		TMS1000::LoadROM("roms/TMS1000/Simon/simon.bin");
		ShowMonitor(cpuInfo);
		TestTMS1000::Test();
	}

	if (0)
	{
		CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1100);
		TMS1000::LoadROM("roms/TMS1100/connect4.bin");
		ShowMonitor(cpuInfo);
		TestTMS1100::Test();
	}

	if (0) {
		CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1400);
		GameSplitSecond::Init();
		ShowMonitor(cpuInfo);
		while (1) {
			RunMonitor();
		}
		return 0;
	}

//	CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1100);
//	GameMerlin::Init(false);

//	CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1400);
//	GameMerlin::Init(true);

	CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1400);
	GameSplitSecond::Init();
	TMS1000::SaveROM("splitSecond.h");

//	CPUInfo &cpuInfo = InitCPU(TMS1000::CPU_TMS1700);
//	GameRSPocketRepeat::Init();


	TMS1000::Reset();
	long lastTicks = 0;
	const int hbInterval = 1000000;
	uint64_t start = GetTickCount64();

	bool loop = true;
	char statusLine[79];
	memset(statusLine, 0, 79);
	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
		TMS1000::Step();
		uSleep(200);
		long deltaCPU = TMS1000::GetTicks() - lastTicks;
		if (deltaCPU > hbInterval) {
			lastTicks = TMS1000::GetTicks();
			uint64_t end = GetTickCount64();
		sprintf(statusLine, "Ticks: %u (%I64u ticks/s)", 
			lastTicks, ((end - start) ? (deltaCPU * 1000 / (end - start)) : -1));
		Console::WriteAt(1, 1, statusLine, 79);
			start = end;
		}
	}

	return 0;
}

