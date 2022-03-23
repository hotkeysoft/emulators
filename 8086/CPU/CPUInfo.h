#pragma once

#include <map>
#include <string>

using json = nlohmann::json;

namespace cpuInfo
{
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
		enum class MULTI { NONE = -1, GRP1 = 0, GRP2, GRP3, GRP4, GRP5 } multi = MULTI::NONE;
	};

#pragma pack(push,1)                                   
	struct OpcodeTiming
	{
		BYTE base = 1; // Base number of ticks for instructions
		BYTE mem = 0;  // Ticks when operand is memory         
		BYTE t3 = 0;   // Extra timing info for some opcodes   
		BYTE t4 = 0;   // ""
	};
#pragma pack(pop) 

	struct Coord
	{
		bool IsSet() const { return w != 0 && h != 0; }
		short x; short y; short w; short h;
	};

	enum class CPUType { i8086 };

	class CPUInfo
	{
	public:
		CPUInfo();
		CPUInfo(CPUType model, const char* name, const char* configFileName);

		const char* GetName() { return m_name; }
		void LoadConfig();

		CPUType GetModel() const { return m_model; }

		std::string GetANSIFile() const;
		Coord GetCoord(const char* label) const;
		const Opcode& GetOpcode(BYTE opcode) const { return m_opcodes[opcode]; }
		const std::string GetSubOpcode(const Opcode& parent, BYTE op2);

		const OpcodeTiming& GetOpcodeTiming(BYTE opcode) const { return m_timing[opcode]; }
		const OpcodeTiming& GetSubOpcodeTiming(Opcode::MULTI sub, BYTE opcode) const { return m_subTiming[(int)sub][opcode]; }

	protected:
		CPUType m_model;
		const char* m_name;
		const char* m_configFileName;

		json m_config;

		void BuildOpcodes(const json& opcode);
		Opcode BuildOpcode(const json& opcode);
		void BuildSubOpcodes(int index, const json& opcodes);
		OpcodeTiming BuildTiming(const json& opcode);

		Opcode m_opcodes[256];
		std::string m_subOpcodes[5][8];

		OpcodeTiming m_timing[256];
		OpcodeTiming m_subTiming[5][8];
	};

	extern CPUInfo g_CPUInfo;
}