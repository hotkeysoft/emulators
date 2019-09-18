#include "stdafx.h"
#include "CPUInfo.h"
#include "resource.h"
#include <iostream>
#include <fstream>

CPUInfoMap g_tms1000Info = {
	{ TMS1000, CPUInfo("TMS1000", "TMS1000.json", IDR_TMS1000)},
	{ TMS1200, CPUInfo("TMS1200", "TMS1000.json", IDR_TMS1000)},
	{ TMS1070, CPUInfo("TMS1070", "TMS1000.json", IDR_TMS1000)},
	{ TMS1270, CPUInfo("TMS1270", "TMS1000.json", IDR_TMS1000)},
	{ TMS1100, CPUInfo("TMS1100", "TMS1000.json", IDR_TMS1000)},
	{ TMS1300, CPUInfo("TMS1300", "TMS1000.json", IDR_TMS1000)}
};

CPUInfo::CPUInfo() : CPUInfo(nullptr, nullptr, 0) 
{
}

CPUInfo::CPUInfo(const char * name, const char * configFileName, DWORD guiResourceID) :
	m_name(name), 
	m_configFileName(configFileName), 
	m_guiResourceID(guiResourceID) 
{
}

void CPUInfo::LoadConfig()
{
	std::ifstream configFile(m_configFileName);
	if (configFile) {
		configFile >> m_config;
	}
	else {
		throw std::exception("file not found");
	}
}
