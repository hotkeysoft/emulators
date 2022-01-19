#pragma once

#include <json.hpp>
using json = nlohmann::json;

namespace emul
{
	class Serializable
	{
	public:
		virtual void Serialize(json& to) = 0;
		virtual void Deserialize(json& from) = 0;

		static std::string GetSerializationDir() { return m_serializationDir; }
		static void SetSerializationDir(const char* dir) { m_serializationDir = dir; }
		
	protected:
		static std::string m_serializationDir;
	};
}

