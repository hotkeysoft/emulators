#include "stdafx.h"
#include "CPUInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace emul
{
	CPUInfo g_CPUInfo(CPUType::i8086, "8086", "8086.json");

	CPUInfo::CPUInfo() : CPUInfo(CPUType::i8086, nullptr, nullptr)
	{
	}

	CPUInfo::CPUInfo(CPUType model, const char* name, const char* configFileName) :
		m_model(model),
		m_name(name),
		m_configFileName(configFileName)
	{
	}

	void CPUInfo::LoadConfig()
	{
		std::ifstream configFile("config/" + std::string(m_configFileName));
		if (configFile) 
		{
			configFile >> m_config;
		}
		else 
		{
			throw std::exception("file not found");
		}

		if (m_config["cpu"]["OpCodes"].size() != 256) 
		{
			throw std::exception("opcode list incomplete");
		}
	}

	std::string CPUInfo::Disassemble(BYTE opcode) const
	{
		return m_config["cpu"]["OpCodes"][opcode].get<std::string>();
	}

	std::string CPUInfo::GetANSIFile() const
	{
		std::string fileName = m_config["monitor"]["ANSIFile"].get<std::string>();

		std::string ansiFileData;
		std::ifstream ansiFile("config/" + std::string(fileName));
		if (ansiFile)
		{
			std::ostringstream ss;
			ss << ansiFile.rdbuf();
			return ss.str();
		}
		else
		{
			throw std::exception("file not found");
		}
	}

	CPUInfo::Coord CPUInfo::GetCoord(const char* label) const
	{
		static json coords = m_config["monitor"]["coords"];

		if (coords.find(label) != coords.end())
		{
			auto coord = coords[label];
			Coord c;
			c.x = coord["x"].get<short>();
			c.y = coord["y"].get<short>();
			c.w = coord["w"].get<short>();
			return c;
		}

		return Coord();
	}
}
