#pragma once

#include <map>
#include <string>

using json = nlohmann::json;
#include "TMS1000.h"

class CPUInfo {
public:
	struct Coord { 
		bool IsSet() const { return w != 0 && h != 0; }
		short x; short y; short w; short h;
	};

	CPUInfo();
	CPUInfo(TMS1000::TMS1000Family model, const char* name, const char* configFileName);
	
	const char* GetName() { return m_name; }
	void LoadConfig();

	TMS1000::TMS1000Family GetModel() const { return m_model; }

	int GetRAMWords() const;
	int GetROMWords() const;
	int GetRLatches() const;
	int GetOLatches() const;

	std::string GetANSIFile() const;
	Coord GetCoord(const char* label) const;
	std::string Disassemble(BYTE opcode) const;

protected:
	TMS1000::TMS1000Family m_model;
	const char* m_name;
	const char* m_configFileName;

	json m_config;
};

typedef std::map<TMS1000::TMS1000Family, CPUInfo> CPUInfoMap;

extern CPUInfoMap g_tms1000Info;
