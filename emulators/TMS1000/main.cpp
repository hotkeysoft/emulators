
#include "stdafx.h"
#include "CPUInfo.h"
#include "Console.h"

#include <iostream>
#include "TMS1000.h"

void LogCallback(const char *str) {
	fprintf(stderr, str);
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

	Console::Init(&cpuInfo);
	TMS1000::Init(CPU_TMS1000, &cpuInfo);
	TMS1000::LoadROM("mp3300.bin");
	TMS1000::Reset();
	Console::UpdateStatus();
	return 0;
}

