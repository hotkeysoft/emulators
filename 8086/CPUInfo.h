#pragma once

#include <map>
#include <string>
#include "Common.h"
#include "json.hpp"
using json = nlohmann::json;

namespace emul
{
	enum class CPUType { i8086 };

	class CPUInfo
	{
	public:
		struct Coord
		{
			bool IsSet() const { return w != 0 && h != 0; }
			short x; short y; short w; short h;
		};

		CPUInfo();
		CPUInfo(CPUType model, const char* name, const char* configFileName);

		const char* GetName() { return m_name; }
		void LoadConfig();

		CPUType GetModel() const { return m_model; }

		std::string GetANSIFile() const;
		Coord GetCoord(const char* label) const;
		std::string Disassemble(BYTE opcode) const;

	protected:
		CPUType m_model;
		const char* m_name;
		const char* m_configFileName;

		json m_config;
	};

	extern CPUInfo g_CPUInfo;
}