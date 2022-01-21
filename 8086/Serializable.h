#pragma once

#include <filesystem>

#include <json.hpp>
using json = nlohmann::json;

namespace emul
{
	class SerializableException : public std::exception
	{
	public:
		SerializableException(const char* const what) : std::exception(what) {}
	};

	class Serializable
	{
	public:
		virtual void Serialize(json& to) = 0;
		virtual void Deserialize(json& from) = 0;

		static std::filesystem::path GetSerializationDir() { return m_serializationDir; }
		static void SetSerializationDir(std::filesystem::path dir) { m_serializationDir = dir; }
		
	protected:
		static std::filesystem::path m_serializationDir;
	};
}

