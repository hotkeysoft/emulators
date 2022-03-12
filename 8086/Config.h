#pragma once

namespace cfg
{
	class Config : public Logger
	{
	public:
		static Config& Get()
		{
			static Config instance;
			return instance;
		}

		Config(const Config&) = delete;
		Config& operator=(const Config&) = delete;
		Config(Config&&) = delete;
		Config& operator=(Config&&) = delete;

		bool LoadConfigFile(const char* path);
		bool SaveConfigFile(const char* path);

		std::string GetValueStr(const char* section, const char* key, const char* defaultValue = "");
		int32_t GetValueInt32(const char* section, const char* key, int32_t defaultValue = 0);
		float GetValueFloat(const char* section, const char* key, float defaultValue = 0.0f);
		bool GetValueBool(const char* section, const char* key);

		// Reads from section [loglevels]
		Logger::SEVERITY GetLogLevel(const char* key);

	protected:
		inipp::Ini<char> m_config;

	private:
		Config();
	};

	constexpr auto CONFIG = &Config::Get;
}
