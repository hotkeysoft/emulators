#include "stdafx.h"
#include "Logger.h"

void(*Logger::m_logCallbackFunc)(const char *str);
char Logger::m_logBuffer[1024];
Logger::ModuleList Logger::m_moduleList;

Logger::Logger(const char* moduleID) :
	m_moduleID(moduleID),
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

void Logger::EnableLog(SEVERITY minSev)
{
	m_minSeverity = minSev;
}

void Logger::RegisterLogCallback(void(*logCallbackFunc)(const char *))
{
	m_logCallbackFunc = logCallbackFunc;
}

void Logger::_LogPrintf(SEVERITY sev, const char *msg, ...) const
{
	va_list args;
	va_start(args, msg);

	m_logBuffer[0] = 0;
	char* pos = m_logBuffer + sprintf(m_logBuffer, "[%s]", m_moduleID.c_str());

	switch (sev)
	{
	case LOG_TRACE:
		pos = strcat(pos, "[TRC]") + 5;
		break;
	case LOG_DEBUG:
		pos = strcat(pos, "[DBG]") + 5;
		break;
	case LOG_INFO:
		pos = strcat(pos, "[INF]") + 5;
		break;
	case LOG_WARNING:
		pos = strcat(pos, "[WRN]") + 5;
		break;
	case LOG_ERROR:
		pos = strcat(pos, "[ERR]") + 5;
		break;
	case LOG_OFF: // Should not really be used...
		pos = strcat(pos, "[OFF]") + 5;
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

void Logger::_LogPrintHex(SEVERITY sev, const uint8_t* buf, size_t size) const
{
	size = std::min(size, (size_t)65536);

	const int bytesPerLine = 16;

	static std::vector<char> hex;
	static std::vector<char> raw;
	hex.resize((bytesPerLine * 3) + 1);
	raw.resize((bytesPerLine) + 1);

	size_t hStart = 0;
	for (size_t i = 0; i < size; ++i)
	{
		const int hpos = i % bytesPerLine;
		const uint8_t ch = buf[i];

		if (hpos == 0)
		{
			hStart = i;
			std::fill(hex.begin(), hex.end(), 'x');
			std::fill(raw.begin(), raw.end(), 'x');
			*hex.rbegin() = '\0';
			*raw.rbegin() = '\0';
		}

		sprintf(&hex[hpos * 3], "%02X ", ch);

		raw[hpos] = (ch >= 32) ? ch : '.';

		if ((hpos == bytesPerLine - 1) || (i == (size - 1)))
		{
			_LogPrintf(sev, " %04X: %s %s", hStart, &hex[0], &raw[0]);
		}
	}
}
