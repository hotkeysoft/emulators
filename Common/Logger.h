#pragma once
#include <string>
#include <map>

#define LogPrintf(sev, fmt, ...) \
	do { if (IsLog(sev)) _LogPrintf(sev, fmt, __VA_ARGS__); } while (0)

class Logger
{
public:
	Logger(const char* moduleID);
	virtual ~Logger();

	enum SEVERITY { LOG_DEBUG = 0, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_OFF = 999 };

	virtual void EnableLog(SEVERITY minSev = LOG_INFO);

	static void RegisterLogCallback(void(*)(const char *));

protected:
	inline bool IsLog(SEVERITY sev) const { return sev >= m_minSeverity; }
	void _LogPrintf(SEVERITY, const char *, ...) const;

private:
	Logger();

	typedef std::multimap<std::string, Logger*> ModuleList;

	void RegisterModuleID(const char* moduleID);

	SEVERITY m_minSeverity;
	std::string m_moduleID;

	static ModuleList m_moduleList;

	static char m_logBuffer[1024];

	static void(*m_logCallbackFunc)(const char *str);
};

