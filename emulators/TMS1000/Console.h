#pragma once

#include "CPUInfo.h"

class Console
{
public:
	Console() {}
	~Console() {}

	static void Init(const CPUDef &cpuDef);

protected:
	typedef std::tuple<char*, DWORD> ResourceData;
	static ResourceData GetBinaryResource(DWORD resourceID);

	static HANDLE m_hConsole;
	static CPUDef m_cpuDef;
};

