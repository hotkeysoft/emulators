#include "Logger.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <exception>

void(*Logger::m_logCallbackFunc)(const char *str);
char Logger::m_logBuffer[1024];
Logger::ModuleList Logger::m_moduleList;

Logger::Logger(const char* moduleID) : 
	m_moduleID(moduleID), 
	m_enabled(true), 
	m_minSeverity(SEVERITY::LOG_DEBUG)
{
	RegisterModuleID(moduleID);
}

Logger::~Logger()
{
}

void Logger::RegisterModuleID(const char* moduleID)
{
	if (moduleID == nullptr) 
	{
		throw std::exception("Logger: ModuleID is null");
	}
	
	m_moduleList.insert(std::pair<std::string, Logger*>(moduleID, this));
}

void Logger::EnableLog(bool enable, SEVERITY minSev)
{
	m_enabled = enable;
	m_minSeverity = minSev;
}

void Logger::RegisterLogCallback(void(*logCallbackFunc)(const char *))
{
	m_logCallbackFunc = logCallbackFunc;
}

void Logger::LogPrintf(SEVERITY sev, const char *msg, ...)
{
	if (!m_enabled || sev < m_minSeverity)
		return;

	va_list args;
	va_start(args, msg);

	m_logBuffer[0] = 0;
	char* pos = m_logBuffer + sprintf(m_logBuffer, "[%s]", m_moduleID.c_str());

	switch (sev)
	{
	case LOG_INFO:
		pos = strcat(pos, "[INFO]") + 6;
		break;
	case LOG_WARNING:
		pos = strcat(pos, "[WARN]") + 6;
		break;
	case LOG_ERROR:
		pos = strcat(pos, "[ERR]") + 5;
		break;
	}

	vsprintf(pos, msg, args);

	va_end(args);

	strcat(m_logBuffer, "\n");

	if (m_logCallbackFunc)
	{
		m_logCallbackFunc(m_logBuffer);
	}
}