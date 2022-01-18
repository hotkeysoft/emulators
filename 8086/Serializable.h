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
	};
}

