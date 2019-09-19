#include "stdafx.h"
#include "CPUInfo.h"
#include "resource.h"
#include <iostream>
#include <fstream>

CPUInfoMap g_tms1000Info = {
	{ CPU_TMS1000, CPUInfo("TMS1000", "TMS1000.json", IDR_TMS1000)},
	{ CPU_TMS1200, CPUInfo("TMS1200", "TMS1000.json", IDR_TMS1000)},
	{ CPU_TMS1070, CPUInfo("TMS1070", "TMS1000.json", IDR_TMS1000)},
	{ CPU_TMS1270, CPUInfo("TMS1270", "TMS1000.json", IDR_TMS1000)},
	{ CPU_TMS1100, CPUInfo("TMS1100", "TMS1000.json", IDR_TMS1000)},
	{ CPU_TMS1300, CPUInfo("TMS1300", "TMS1000.json", IDR_TMS1000)}
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

int CPUInfo::GetRAMWords() const {
	return m_config["cpu"]["RAMWords"].get<int>();
}
int CPUInfo::GetROMWords() const {
	return m_config["cpu"]["ROMWords"].get<int>();
}
int CPUInfo::GetRLatches() const {
	return m_config["cpu"]["RLatches"].get<int>();
}
int CPUInfo::GetOLatches() const {
	return m_config["cpu"]["OLatches"].get<int>();
}

std::string CPUInfo::GetANSIFile() const {
	return m_config["monitor"]["ANSIFile"].get<std::string>();
}

CPUInfo::Coord CPUInfo::GetCoord(const char* label) const {
	static json coords = m_config["monitor"]["coords"];

	if (coords.find(label) != coords.end())	{
		auto coord = coords[label];
		Coord c;
		c.x = coord["x"].get<short>();
		c.y = coord["y"].get<short>();
		c.w = coord["w"].get<short>();
		return c;
	}

	return Coord();
}
