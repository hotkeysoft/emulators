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

		// Reads an integer in decimal format only.
		// For hex/oct/dec parsing use GetValueBYTE/WORD/DWORD
		int32_t GetValueInt32(const char* section, const char* key, int32_t defaultValue = 0);

		// Parses different number formats: dddddddddd (decimal), 0xhhhhhhhh (hex), 0ooooooooooo (octal)
		DWORD GetValueDWORD(const char* section, const char* key, DWORD defaultValue = 0);

		// Parses different number formats: ddddd (decimal), 0xhhhh (hex), 0oooooo (octal)
		WORD GetValueWORD(const char* section, const char* key, WORD defaultValue = 0);

		// Parses different number formats: ddd (decimal), 0xhh (hex), 0ooo (octal)
		BYTE GetValueBYTE(const char* section, const char* key, BYTE defaultValue = 0);

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
