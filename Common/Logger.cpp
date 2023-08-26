#include "stdafx.h"
#include "Logger.h"

void(*Logger::m_logCallbackFunc)(const char *str);
bool Logger::m_enableColors = true;
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

constexpr const char* BLK = "\x1B[0;30m";
constexpr const char* RED = "\x1B[0;31m";
constexpr const char* GRN = "\x1B[0;32m";
constexpr const char* YEL = "\x1B[0;33m";
constexpr const char* BLU = "\x1B[0;34m";
constexpr const char* MAG = "\x1B[0;35m";
constexpr const char* CYN = "\x1B[0;36m";
constexpr const char* WHT = "\x1B[0;37m";

constexpr const char* HBLK = "\x1B[0;90m";
constexpr const char* HRED = "\x1B[0;91m";
constexpr const char* HGRN = "\x1B[0;92m";
constexpr const char* HYEL = "\x1B[0;93m";
constexpr const char* HBLU = "\x1B[0;94m";
constexpr const char* HMAG = "\x1B[0;95m";
constexpr const char* HCYN = "\x1B[0;96m";
constexpr const char* HWHT = "\x1B[0;97m";
constexpr int COLOR_SIZE = 7;

constexpr const char* COLOR_RESET = "\x1B[0m";
constexpr int COLOR_RESET_SIZE = 4;

constexpr const char* LOG_LEVEL_STRINGS[] = { "TRC", "DBG", "INF", "WRN", "ERR" };
constexpr const char* LOG_LEVEL_COLORS[] = { HBLK, CYN, GRN, YEL, RED };

static void AddStr(char*& dest, const std::string& s, const char* color = nullptr)
{
	if (color)
	{
		memcpy(dest, color, COLOR_SIZE);
		dest += COLOR_SIZE;
	}
	const auto size = s.size();
	memcpy(dest, s.c_str(), size);
	dest+= size;
	if (color)
	{
		memcpy(dest, COLOR_RESET, COLOR_RESET_SIZE);
		dest += COLOR_RESET_SIZE;
	}
}

static void AddChar(char*& dest, char ch)
{
	*(dest++) = ch;
}

void Logger::_LogPrintf(SEVERITY sev, const char *msg, ...) const
{
	switch (sev)
	{
	case LOG_TRACE:
	case LOG_DEBUG:
	case LOG_INFO:
	case LOG_WARNING:
	case LOG_ERROR:
		break;
	default:
		throw std::exception("Invalid log level");
	}

	va_list args;
	va_start(args, msg);

	char* pos = m_logBuffer;
	AddChar(pos, '[');
	AddStr(pos, m_moduleID, m_enableColors ? HBLK : nullptr);
	AddChar(pos, ']');
	AddChar(pos, '[');
	AddStr(pos, LOG_LEVEL_STRINGS[int(sev)], m_enableColors ? LOG_LEVEL_COLORS[int(sev)] : nullptr);
	AddChar(pos, ']');

	pos += vsprintf(pos, msg, args);

	va_end(args);

	AddChar(pos, '\n');
	AddChar(pos, '\0');

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
