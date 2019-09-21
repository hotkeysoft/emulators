#pragma once

#include <map>
#include <string>
#include "json.hpp"
using json = nlohmann::json;

enum TMS1000Family { CPU_TMS1000, CPU_TMS1200, CPU_TMS1070, CPU_TMS1270, CPU_TMS1100, CPU_TMS1300 };

class CPUInfo {
public:
	struct Coord { 
		bool IsSet() const { return w != 0 && h != 0; }
		short x; short y; short w; short h;
	};

	CPUInfo();
	CPUInfo(const char* name, const char* configFileName);
	
	const char* GetName() { return m_name; }
	void LoadConfig();

	int GetRAMWords() const;
	int GetROMWords() const;
	int GetRLatches() const;
	int GetOLatches() const;

	std::string GetANSIFile() const;
	Coord GetCoord(const char* label) const;
	std::string Disassemble(BYTE opcode) const;

protected:

	const char* m_name;
	const char* m_configFileName;

	json m_config;
};

typedef std::map<TMS1000Family, CPUInfo> CPUInfoMap;

extern CPUInfoMap g_tms1000Info;
