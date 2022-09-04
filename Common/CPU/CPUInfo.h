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
		enum class IMM { NONE, W8, W8W8, W16, W16W8, W32 } imm = IMM::NONE;
		enum class MULTI { NONE = -1, GRP1 = 0, GRP2, GRP3, GRP4, GRP5, GRP6, GRP7, GRP8, _COUNT } multi = MULTI::NONE;
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

	class CPUInfo : public Logger
	{
	public:
		// CPU Model e.g. "8080", "z80", "80286"
		// The config will be retrieved from file "{cpuid}.json"
		CPUInfo(const char* cpuid);

		CPUInfo() = delete;
		CPUInfo(const CPUInfo&) = delete;
		CPUInfo& operator=(const CPUInfo&) = delete;
		CPUInfo(CPUInfo&&) = delete;
		CPUInfo& operator=(CPUInfo&&) = delete;

		const char* GetId() const { return m_id.c_str(); }
		void LoadConfig();

		std::string GetANSIFile() const;
		Coord GetCoord(const char* label) const;
		const Opcode& GetOpcode(BYTE opcode) const { return m_opcodes[opcode]; }
		const std::string GetSubOpcodeStr(const Opcode& parent, BYTE op2) const;
		Opcode GetSubOpcode(const Opcode& parent, BYTE op2) const;

		const OpcodeTiming& GetOpcodeTiming(BYTE opcode) const { return m_timing[opcode]; }
		const OpcodeTiming& GetSubOpcodeTiming(Opcode::MULTI sub, BYTE opcode) const { return m_subTiming[(int)sub][opcode]; }
		const OpcodeTiming& GetMiscTiming(MiscTiming timing) const { return m_miscTiming[(int)timing]; }

	protected:
		std::string m_id;
		std::string m_configFile;

		json m_config;

		void BuildOpcodes(const json& opcode);
		Opcode BuildOpcode(std::string opcocode) const;
		void BuildSubOpcodes(int index, const json& opcodes);
		OpcodeTiming BuildTiming(const json& opcode) const;

		static const size_t MAX_OPCODE = 0xFF;

		Opcode m_opcodes[MAX_OPCODE + 1];
		std::string m_subOpcodes[(int)Opcode::MULTI::_COUNT][MAX_OPCODE + 1];

		OpcodeTiming m_timing[MAX_OPCODE + 1];
		OpcodeTiming m_subTiming[(int)Opcode::MULTI::_COUNT][MAX_OPCODE + 1];
		OpcodeTiming m_miscTiming[(int)MiscTiming::_COUNT];
	};
}