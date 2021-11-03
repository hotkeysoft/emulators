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

}
