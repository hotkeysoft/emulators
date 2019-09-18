#pragma once

#include <map>

enum TMS1000Family { TMS1000, TMS1200, TMS1070, TMS1270, TMS1100, TMS1300 };

typedef std::map<std::string, std::tuple<int, int> > guiPositionMap;

struct CPUDef {
	const char* name;
	int ROMsize;
	int RAMwords;
	int Rlatches;
	int Olatches;

	DWORD guiResourceID;
};

typedef std::map<TMS1000Family, CPUDef> CPUDefMap;

extern CPUDefMap g_tms1000Def;
