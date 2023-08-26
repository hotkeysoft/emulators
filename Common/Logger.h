#pragma once
#include <string>
#include <map>
#include <algorithm>

#define LogPrintf(sev, fmt, ...) \
	do { if (IsLog(sev)) _LogPrintf(sev, fmt, __VA_ARGS__); } while (0)

#define LogPrintHex(sev, buf, size) \
	do { if (IsLog(sev)) _LogPrintHex(sev, buf, size); } while (0)

class Logger
{
public:
	Logger(const char* moduleID);
	virtual ~Logger();

	enum SEVERITY { LOG_TRACE = 0, LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_OFF = 999 };

	virtual void EnableLog(SEVERITY minSev = LOG_INFO);

	static void EnableColors(bool enable = true) { m_enableColors = enable; }

	static void RegisterLogCallback(void(*)(const char *));

protected:
	const char* GetModuleID() const { return m_moduleID.c_str(); }

	inline bool IsLog(SEVERITY sev) const { return sev >= m_minSeverity; }
	void _LogPrintf(SEVERITY, const char *, ...) const;
	void _LogPrintHex(SEVERITY, const uint8_t*, size_t) const;

private:
	Logger();

	typedef std::multimap<std::string, Logger*> ModuleList;

	void RegisterModuleID(const char* moduleID);

	SEVERITY m_minSeverity;
	std::string m_moduleID;

	static ModuleList m_moduleList;
	static bool m_enableColors;
	static char m_logBuffer[1024];

	static void(*m_logCallbackFunc)(const char *str);
};

