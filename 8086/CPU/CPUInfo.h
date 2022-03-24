#pragma once

#include <map>
#include <string>

using json = nlohmann::json;

namespace cpuInfo
{
	// Make sure these are in the same order in the json file
	enum class MiscTiming
	{
		TRAP = 0,
		IRQ,
		EA_BASE,
		EA_DIRECT,
		EA_DISP,
		EA_INDEX_LO,
		EA_INDEX_HI,

		_COUNT
	};

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
		enum class MULTI { NONE = -1, GRP1 = 0, GRP2, GRP3, GRP4, GRP5, _COUNT } multi = MULTI::NONE;
	};

	enum class OpcodeTimingType {
		BASE = 0, // Base number of ticks for instructions
		MEM,      // Ticks when operand is memory         
		T3,       // Extra timing info for some opcodes (conditionals, jumps, 16 bits overhead, etc.)
		T4,       // ""

		_COUNT
	};
	using OpcodeTiming = std::array<BYTE, (int)OpcodeTimingType::_COUNT>;

	struct Coord
	{
		bool IsSet() const { return w != 0 && h != 0; }
		short x; short y; short w; short h;
	};

	enum class CPUType { i8086, i80186, i80286 };

	class CPUInfo
	{
	public:
		CPUInfo(CPUType model);

		CPUInfo() = delete;
		CPUInfo(const CPUInfo&) = delete;
		CPUInfo& operator=(const CPUInfo&) = delete;
		CPUInfo(CPUInfo&&) = delete;
		CPUInfo& operator=(CPUInfo&&) = delete;

		std::string GetName() { return m_name; }
		void LoadConfig();

		CPUType GetModel() const { return m_model; }

		std::string GetANSIFile() const;
		Coord GetCoord(const char* label) const;
		const Opcode& GetOpcode(BYTE opcode) const { return m_opcodes[opcode]; }
		const std::string GetSubOpcode(const Opcode& parent, BYTE op2) const;

		const OpcodeTiming& GetOpcodeTiming(BYTE opcode) const { return m_timing[opcode]; }
		const OpcodeTiming& GetSubOpcodeTiming(Opcode::MULTI sub, BYTE opcode) const { return m_subTiming[(int)sub][opcode]; }
		const OpcodeTiming& GetMiscTiming(MiscTiming timing) const { return m_miscTiming[(int)timing]; }

	protected:
		struct CPUData
		{
			const char* name;
			const char* configFile;
		};

		using CPUMap = std::map<CPUType, CPUData>;

		const CPUMap m_cpus = {
			{ CPUType::i8086, { "8086", "8086.json" } },
			{ CPUType::i80186, { "80186", "80186.json" } },
			{ CPUType::i80286, { "80286", "80286.json" } }
		};

		CPUType m_model;
		std::string m_name;
		std::string m_configFile;

		json m_config;

		void BuildOpcodes(const json& opcode);
		Opcode BuildOpcode(const json& opcode);
		void BuildSubOpcodes(int index, const json& opcodes);
		OpcodeTiming BuildTiming(const json& opcode);

		Opcode m_opcodes[256];
		std::string m_subOpcodes[(int)Opcode::MULTI::_COUNT][8];

		OpcodeTiming m_timing[256];
		OpcodeTiming m_subTiming[(int)Opcode::MULTI::_COUNT][8];
		OpcodeTiming m_miscTiming[(int)MiscTiming::_COUNT];
	};
}