#include "stdafx.h"
#include "Console.h"

HANDLE Console::m_hConsole;
CPUDef Console::m_cpuDef;

Console::ResourceData Console::GetBinaryResource(DWORD resourceID)
{
	HRSRC		res;
	HGLOBAL		res_handle = NULL;
	char *      res_data;
	DWORD       res_size;

	res = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(resourceID), RT_RCDATA);
	if (!res)
		return std::make_tuple(nullptr, 0);
	res_handle = LoadResource(NULL, res);
	if (!res_handle)
		return std::make_tuple(nullptr, 0);

	res_data = (char*)LockResource(res_handle);
	res_size = SizeofResource(NULL, res);

	return std::make_tuple(res_data, res_size);
}

void Console::Init(const CPUDef &cpuDef)
{
	m_cpuDef = cpuDef;
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD dwMode = 0;
	GetConsoleMode(m_hConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(m_hConsole, dwMode);

	ResourceData data = GetBinaryResource(m_cpuDef.guiResourceID);

	if (std::get<0>(data)) {
		DWORD written;
		WriteConsole(m_hConsole,
			std::get<0>(data),
			std::get<1>(data),
			&written,
			NULL);
	}

}
