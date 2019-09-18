#pragma once

#include <map>
#include "json.hpp"
using json = nlohmann::json;

enum TMS1000Family { TMS1000, TMS1200, TMS1070, TMS1270, TMS1100, TMS1300 };

class CPUInfo {
public:
	CPUInfo();
	CPUInfo(const char* name, const char* configFileName, DWORD guiResourceID);
	
	const char* GetName() { return m_name; }
	DWORD GetResourceID() { return m_guiResourceID; }
	void LoadConfig();

protected:

	const char* m_name;
	const char* m_configFileName;
	DWORD m_guiResourceID;

	json m_config;
};

typedef std::map<TMS1000Family, CPUInfo> CPUInfoMap;

extern CPUInfoMap g_tms1000Info;
