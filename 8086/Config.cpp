#include "stdafx.h"

#include "Config.h"

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

		for (auto error : m_config.errors)
		{
			LogPrintf(LOG_WARNING, "Parsing error: [%s]", error.c_str());
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

	float Config::GetValueFloat(const char* section, const char* key, float defaultValue)
	{
		assert(section);
		assert(key);
		float ret;
		return inipp::get_value(m_config.sections[section], key, ret) ? ret : defaultValue;
	}

	bool Config::GetValueBool(const char* section, const char* key)
	{
		assert(section);
		assert(key);
		std::string str;
		if (!inipp::get_value(m_config.sections[section], key, str))
		{
			return false;
		}

		// TODO: Case insensitive
		return (str == "1") || (str == "true");
	}

	Logger::SEVERITY Config::GetLogLevel(const char* key)
	{
		// in .ini file, 0=off;
		// No value should be considered LOG_INFO
		int32_t level;
		if (!inipp::get_value(m_config.sections["loglevels"], key, level))
		{
			LogPrintf(LOG_WARNING, "No key [loglevels].%s, using default level [LOG_INFO]", key);
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
		case 5:
			return Logger::SEVERITY::LOG_TRACE;
		default:
			LogPrintf(LOG_WARNING, "Unknown log level [%d] for key [loglevels].%s, using default level [LOG_INFO]", level, key);
			return Logger::SEVERITY::LOG_INFO;
		}
	}

}
