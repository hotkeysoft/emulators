// TMS1000.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

enum TMS1000Family { TMS1000, TMS1200, TMS1070, TMS1270, TMS1100, TMS1300 };

struct CPUDef {
	TMS1000Family model;
	const char* name;
	int ROMsize;
	int RAMwords;
	int Rlatches;
	int Olatches;
};


CPUDef tms1000[] = {
	{ TMS1000, "TMS1000", 1024,  64, 11, 8},
	{ TMS1200, "TMS1200", 1024,  64, 13, 8},
	{ TMS1070, "TMS1070", 1024,  64, 11, 8},
	{ TMS1270, "TMS1270", 1024,  64, 13, 8},
	{ TMS1100, "TMS1100", 2048, 128, 11, 8},
	{ TMS1300, "TMS1300", 2048, 128, 16, 8}
};

void LogCallback(const char *str)
{
	fprintf(stderr, str);
}

int main() {
	Logger::RegisterLogCallback();
    return 0;
}
