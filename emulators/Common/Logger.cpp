#include "Logger.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <exception>

void(*Logger::m_logCallbackFunc)(const char *str);
char Logger::m_logBuffer[1024];
Logger::ModuleList Logger::m_moduleList;

Logger::Logger(const char* moduleID) : m_moduleID(moduleID), m_enabled(true)
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

	if (m_moduleList.find(moduleID) != m_moduleList.end())
	{
		throw std::exception("ModuleID already defined");
	}
	
	m_moduleList[moduleID] = this;
}

void Logger::EnableLog(bool enable)
{
	m_enabled = enable;
}

void Logger::RegisterLogCallback(void(*logCallbackFunc)(const char *))
{
	m_logCallbackFunc = logCallbackFunc;
}

void Logger::LogPrintf(SEVERITY sev, const char *msg, ...)
{
	if (!m_enabled)
		return;

	va_list args;
	va_start(args, msg);

	m_logBuffer[0] = 0;
	char* pos = m_logBuffer + sprintf(m_logBuffer, "[%s]", m_moduleID.c_str());

	switch (sev)
	{
	case LOG_INFO:
		pos = strcat(m_logBuffer, "[INFO]") + 6;
		break;
	case LOG_WARNING:
		pos = strcat(m_logBuffer, "[WARN]") + 6;
		break;
	case LOG_ERROR:
		pos = strcat(m_logBuffer, "[ERR]") + 5;
		break;
	}

	vsprintf(pos, msg, args);

	va_end(args);

	strcat(m_logBuffer, "\n");

	if (m_logCallbackFunc)
	{
		m_logCallbackFunc(m_logBuffer);
	}

	if (sev == LOG_ERROR)
	{
		throw std::exception(m_logBuffer);
	}
}