#pragma once

#include "CPUInfo.h"

class Console
{
public:
	Console() {}
	~Console() {}

	static void Init(const CPUInfo &cpuInfo);

protected:
	typedef std::tuple<char*, DWORD> ResourceData;
	static ResourceData GetBinaryResource(DWORD resourceID);

	static HANDLE m_hConsole;
	static CPUInfo&& m_cpuInfo;
};

