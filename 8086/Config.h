#pragma once

#include <Logger.h>
#include <inipp.h>

namespace cfg
{
	class Config : Logger
	{
	public:

		static Config& Instance()
		{
			static Config instance;
			return instance;
		}

		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;
		Config(Config&&) = delete;
		Config& operator=(Config&&) = delete;

		bool LoadConfigFile(const char* path);

		std::string GetValueStr(const char* section, const char* key);
		int32_t GetValueInt32(const char* section, const char* key);

	protected:
		inipp::Ini<char> m_config;

	private:
		Config();
	};
}
