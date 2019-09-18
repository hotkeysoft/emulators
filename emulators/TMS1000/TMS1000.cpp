// TMS1000.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CPUInfo.h"
#include "Console.h"

void LogCallback(const char *str) {
	fprintf(stderr, str);
}


int main() {
	Logger::RegisterLogCallback(LogCallback);

	Console::Init(g_tms1000Def[TMS1000]);


    return 0;
}
