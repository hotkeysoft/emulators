#pragma once

namespace emul::Thomson
{
	enum class Model { UNKNOWN, MO5, TO7 };

	static const std::map<std::string, Model> s_modelMap = {
		{"mo5", Model::MO5},
		{"to7", Model::TO7},
	};

	static Model StringToModel(const char* str)
	{
		auto m = s_modelMap.find(str);
		if (m != s_modelMap.end())
		{
			return m->second;
		}
		return Model::UNKNOWN;
	}

	static std::string ModelToString(Model model)
	{
		for (auto curr : s_modelMap)
		{
			if (curr.second == model)
			{
				return curr.first.c_str();
			}
		}
		return "unknown";
	}

}
