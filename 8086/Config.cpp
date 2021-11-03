#include "Config.h"
#include <fstream>
#include <assert.h>

namespace cfg
{
	Config::Config() : Logger("ini")
	{
	}

	bool Config::LoadConfigFile(const char* path)
	{
		if (!path || !path[0])
		{
			LogPrintf(LOG_ERROR, "Empty path");
			return false;
		}

		std::ifstream is(path);
		if (!is)
		{
			LogPrintf(LOG_ERROR, "Error opening config file [%s]", path);
			return false;
		}

		m_config.clear();
		m_config.parse(is);

		for (auto it = m_config.errors.begin(); it != m_config.errors.end(); ++it)
		{
			LogPrintf(LOG_WARNING, "Parsing error: [%s]", it->c_str());
		}

		return true;
	}

	std::string Config::GetValueStr(const char* section, const char* key, const char* defaultValue)
	{
		assert(defaultValue);
		assert(section);
		assert(key);
		std::string ret;
		return inipp::get_value(m_config.sections[section], key, ret) ? ret : defaultValue;
	}

	int32_t Config::GetValueInt32(const char* section, const char* key, int32_t defaultValue)
	{
		assert(section);
		assert(key);
		int32_t ret;
		return inipp::get_value(m_config.sections[section], key, ret) ? ret : defaultValue;
	}

	Logger::SEVERITY Config::GetLogLevel(const char* key)
	{
		// in .ini file, 0=off;
		// No value should be considered LOG_INFO
		int32_t level;
		if (!inipp::get_value(m_config.sections["loglevel"], key, level))
		{
			LogPrintf(LOG_WARNING, "No key [loglevel].%s, using default level [LOG_INFO]", key);
			return Logger::SEVERITY::LOG_INFO;
		}

		switch (level)
		{
		case 0:
			return Logger::SEVERITY::LOG_OFF;
		case 1:
			return Logger::SEVERITY::LOG_ERROR;
		case 2:
			return Logger::SEVERITY::LOG_WARNING;
		case 3:
			return Logger::SEVERITY::LOG_INFO;
		case 4:
			return Logger::SEVERITY::LOG_DEBUG;
		default:
			LogPrintf(LOG_WARNING, "Unknown log level [%d], using default level [LOG_INFO]");
			return Logger::SEVERITY::LOG_INFO;
		}
	}

}
