#pragma once

#include <map>
#include <string>
#include "../Common.h"
#include <json.hpp>
using json = nlohmann::json;

namespace emul
{
	enum class CPUType { i8086 };

	class CPUInfo
	{
	public:
		struct Opcode
		{
			std::string text;

			bool rm8 = false;
			bool rm16 = false;
			bool r8 = false;
			bool r16 = false;
			bool sr = false;
			bool i8 = false;
			bool i16 = false;
			bool i32 = false;

			enum class MODREGRM { NONE, W8, W16, SR } modRegRm = MODREGRM::NONE;
			enum class IMM { NONE, W8, W16, W32 } imm = IMM::NONE;
			enum class MULTI { NONE, ALU, ALU2, SHIFT, INCDEC, MULTI } multi = MULTI::NONE;
		};

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
		const Opcode& GetOpcode(BYTE opcode) const { return m_opcodes[opcode]; }

	protected:
		CPUType m_model;
		const char* m_name;
		const char* m_configFileName;

		json m_config;

		void BuildOpcodes();
		Opcode BuildOpcode(BYTE op);

		Opcode m_opcodes[256];
	};

	extern CPUInfo g_CPUInfo;
}