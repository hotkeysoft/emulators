#pragma once
#include <string>
#include <map>

class Logger
{
public:
	Logger(const char* moduleID);
	virtual ~Logger();

	enum SEVERITY { LOG_DEBUG = 0, LOG_INFO, LOG_WARNING, LOG_ERROR };

	virtual void EnableLog(bool enable, SEVERITY minSev = LOG_INFO);

	static void RegisterLogCallback(void(*)(const char *));

protected:
	void LogPrintf(SEVERITY, const char *, ...);

private:
	Logger();

	typedef std::multimap<std::string, Logger*> ModuleList;

	void RegisterModuleID(const char* moduleID);

	bool m_enabled;
	SEVERITY m_minSeverity;
	std::string m_moduleID;

	static ModuleList m_moduleList;

	static char m_logBuffer[1024];

	static void(*m_logCallbackFunc)(const char *str);
};

