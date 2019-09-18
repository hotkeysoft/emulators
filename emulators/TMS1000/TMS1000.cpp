// TMS1000.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CPUInfo.h"
#include "Console.h"

#include <iostream>

void LogCallback(const char *str) {
	fprintf(stderr, str);
}


int main() {
	Logger::RegisterLogCallback(LogCallback);

	Console::Init(g_tms1000Info[TMS1000]);

	try {
		g_tms1000Info[TMS1000].LoadConfig();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}


    return 0;
}
