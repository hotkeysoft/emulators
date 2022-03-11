#pragma once

#include <filesystem>

using json = nlohmann::json;

namespace emul
{
	enum class SerializationError
	{
		FATAL, // Something we can't recover from
		COMPAT, // Data is ok but not compatible with current architecture
	};
	class SerializableException : public std::runtime_error
	{
	public:
		SerializableException(const char* const what, SerializationError error = SerializationError::FATAL) : 
			std::runtime_error(what),
			m_error(error)
		{
		}

		SerializationError GetError() const { return m_error; }

	protected:
		SerializationError m_error = SerializationError::FATAL;
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

