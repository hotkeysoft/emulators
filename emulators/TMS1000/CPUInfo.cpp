#include "stdafx.h"
#include "CPUInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>

CPUInfoMap g_tms1000Info = {
	{ TMS1000::CPU_TMS1000, CPUInfo(TMS1000::CPU_TMS1000, "TMS1000", "TMS1000.json")},
	{ TMS1000::CPU_TMS1200, CPUInfo(TMS1000::CPU_TMS1200, "TMS1200", "TMS1000.json")},
	{ TMS1000::CPU_TMS1070, CPUInfo(TMS1000::CPU_TMS1070, "TMS1070", "TMS1000.json")},
	{ TMS1000::CPU_TMS1270, CPUInfo(TMS1000::CPU_TMS1270, "TMS1270", "TMS1000.json")},
	{ TMS1000::CPU_TMS1100, CPUInfo(TMS1000::CPU_TMS1100, "TMS1100", "TMS1100.json")},
	{ TMS1000::CPU_TMS1300, CPUInfo(TMS1000::CPU_TMS1300, "TMS1300", "TMS1000.json")},
	{ TMS1000::CPU_TMS1400, CPUInfo(TMS1000::CPU_TMS1400, "TMS1400", "TMS1400.json")}
};

CPUInfo::CPUInfo() : CPUInfo(TMS1000::CPU_TMS1000, nullptr, nullptr)
{
}

CPUInfo::CPUInfo(TMS1000::TMS1000Family model, const char * name, const char * configFileName) :
	m_model(model),
	m_name(name), 
	m_configFileName(configFileName)
{
}

void CPUInfo::LoadConfig()
{
	std::ifstream configFile("config/" + std::string(m_configFileName));
	if (configFile) {
		configFile >> m_config;
	}
	else {
		throw std::exception("file not found");
	}

	if (m_config["cpu"]["OpCodes"].size() != 256) {
		throw std::exception("opcode list incomplete");
	}
}

std::string CPUInfo::Disassemble(BYTE opcode) const {
	return m_config["cpu"]["OpCodes"][opcode].get<std::string>();
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
	std::string fileName = m_config["monitor"]["ANSIFile"].get<std::string>();

	std::string ansiFileData;
	std::ifstream ansiFile("config/" + std::string(fileName));
	if (ansiFile) {
		std::ostringstream ss;
		ss << ansiFile.rdbuf();
		return ss.str();
	}
	else {
		throw std::exception("file not found");
	}
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
