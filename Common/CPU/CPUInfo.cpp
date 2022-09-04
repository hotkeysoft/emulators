#include "stdafx.h"

#include <CPU/CPUInfo.h>
#include <regex>

namespace cpuInfo
{
	CPUInfo::CPUInfo(const char* cpuid) : Logger("CPUInfo")
	{
		m_id = cpuid ? cpuid : "";

		const std::regex validCPUName(R"(^[\w\-]+$)");

		if (!std::regex_match(m_id, validCPUName))
		{
			LogPrintf(LOG_ERROR, "Invalid CPUID: [%s]", m_id.c_str());
			m_configFile = "{INVALID_CPUID}";
		}
		else
		{
			m_configFile = m_id + ".json";
		}
	}

	void CPUInfo::LoadConfig()
	{
		std::ifstream configFile("config/" + std::string(m_configFile));
		if (configFile)
		{
			configFile >> m_config;
		}
		else
		{
			LogPrintf(LOG_ERROR, "Config File not found: [%s]", m_configFile.c_str());
			throw std::exception("Config File not found");
		}

		if (!m_config["cpu"].contains("opcodes"))
		{
			throw std::exception("missing [opcodes] array");
		}
		BuildOpcodes(m_config["cpu"]["opcodes"]);

		int groupCount = 0;
		if (m_config["cpu"].contains("opcodes.grp"))
		{
			groupCount = m_config["cpu"]["opcodes.grp"];
		}

		if (groupCount > (int)Opcode::MULTI::_COUNT)
		{
			throw std::exception("invalid opcode.grp count");
		}

		for (int i = 0; i < groupCount; ++i)
		{
			char grpName[32];
			sprintf(grpName, "opcodes.grp%d", i + 1);
			if (!m_config["cpu"].contains(grpName))
			{
				throw std::exception("opcode.grp# list missing");
			}
			BuildSubOpcodes(i, m_config["cpu"][grpName]);
		}

		if (m_config["cpu"].contains("misc"))
		{
			size_t miscCount = m_config["cpu"]["misc"].size();

			if (miscCount > (int)MiscTiming::_COUNT)
			{
				LogPrintf(LOG_ERROR, "MISC timing list too large: [%d>%d]", miscCount, (int)MiscTiming::_COUNT);
				throw std::exception("misc timing list too large");
			}
			for (int i = 0; i < miscCount; ++i)
			{
				m_miscTiming[i] = BuildTiming(m_config["cpu"]["misc"][i]);
			}
		}
	}

	void CPUInfo::BuildOpcodes(const json& opcodes)
	{
		// Trivial case is an array of 256 opcodes. However, to account for holes
		// and other weirdness, it is possible to specify a new index (int or hex string e.g. "0x12")
		// so that the count jumps to the new value and continue loading opcodes linearly from there.

		// First fill in the array so we don't have empty holes
		for (int i = 0; i <= MAX_OPCODE; ++i)
		{
			char buf[16];
			sprintf(buf, "DB 0x%02X", i);
			m_opcodes[i] = BuildOpcode(buf);
			m_timing[i] = OpcodeTiming {};
		}

		size_t opcode = 0;
		for (const json& curr : opcodes)
		{
			if (opcode > MAX_OPCODE)
			{
				throw std::exception("BuildOpcodes: index > MAX_OPCODE");
			}

			if (curr.is_number_integer()) // new index (integer)
			{
				int newIndex = curr;
				if (newIndex < 0 || newIndex > MAX_OPCODE)
				{
					throw std::exception("BuildOpcodes: Invalid index");
				}
				opcode = newIndex;
			}
			else if (curr.is_string()) // new index (string)
			{
				int newIndex = std::stoul((const std::string&)curr, nullptr, 0);
				if (newIndex < 0 || newIndex > MAX_OPCODE)
				{
					throw std::exception("BuildOpcodes: Invalid index");
				}
				opcode = newIndex;
			}
			else if (curr.is_object()) // opcode data
			{
				m_opcodes[opcode] = BuildOpcode(curr["name"]);
				m_timing[opcode] = BuildTiming(curr);
				++opcode;
			}
			else
			{
				throw std::exception("BuildOpcodes: Invalid value");
			}
		}
	}

	void CPUInfo::BuildSubOpcodes(int index, const json& opcodes)
	{
		// First fill in the array so we don't have empty holes
		for (int i = 0; i <= MAX_OPCODE; ++i)
		{
			char buf[16];
			sprintf(buf, "DB 0x%02X", i);
			m_subOpcodes[index][i] = buf;
			// All undefined sub opcodes are equivalent to 2xNOP
			m_subTiming[index][i] = OpcodeTiming{ 8, 0, 0, 0 };
		}

		// TODO: Code duplication w/BuildOpcodes
		BYTE opcode = 0;
		for (const json& curr : opcodes)
		{
			if (opcode > MAX_OPCODE)
			{
				throw std::exception("BuildOpcodes: index > MAX_OPCODE");
			}

			if (curr.is_number_integer()) // new index (integer)
			{
				int newIndex = curr;
				if (newIndex < 0 || newIndex > MAX_OPCODE)
				{
					throw std::exception("BuildSubOpcodes: Invalid index");
				}
				opcode = newIndex;
			}
			else if (curr.is_string()) // new index (string)
			{
				int newIndex = std::stoul((const std::string&)curr, nullptr, 16);
				if (newIndex < 0 || newIndex > MAX_OPCODE)
				{
					throw std::exception("BuildOpcodes: Invalid index");
				}
				opcode = newIndex;
			}
			else if (curr.is_object()) // opcode data
			{
				m_subOpcodes[index][opcode] = curr["name"];
				m_subTiming[index][opcode] = BuildTiming(curr);
				++opcode;
			}
			else
			{
				throw std::exception("BuildSubOpcodes: Invalid value");
			}
		}
	}

	OpcodeTiming CPUInfo::BuildTiming(const json& opcode) const
	{
		OpcodeTiming timing = { 4, 0, 0, 0 };
		if (!opcode.contains("timing"))
		{
			return timing;
		}

		const json& jsonTiming = opcode["timing"];
		if (!jsonTiming.is_array() || jsonTiming.size() < 1 || jsonTiming.size() > (int)OpcodeTimingType::_COUNT)
		{
			throw std::exception("invalid timing array");
		}

		for (int i = 0; i < jsonTiming.size(); ++i)
		{
			timing[i] = jsonTiming[i];
		}

		// Copy base to mem if not set
		if (timing[(int)OpcodeTimingType::MEM] == 0)
		{
			timing[(int)OpcodeTimingType::MEM] = timing[(int)OpcodeTimingType::BASE];
		}

		return timing;
	}

	Opcode CPUInfo::BuildOpcode(const std::string text) const
	{
		Opcode ret;
		ret.text = text;

		ret.rm8 = text.find("{rm8}") != std::string::npos;
		ret.rm16 = text.find("{rm16}") != std::string::npos;

		ret.r8 = text.find("{r8}") != std::string::npos;
		ret.r16 = text.find("{r16}") != std::string::npos;
		ret.sr = text.find("{sr}") != std::string::npos;

		ret.i8 = text.find("{i8}") != std::string::npos;
		ret.i16 = text.find("{i16}") != std::string::npos;
		ret.i32 = text.find("{i32}") != std::string::npos;

		bool grp1 = text.find("{grp1}") != std::string::npos;
		bool grp2 = text.find("{grp2}") != std::string::npos;
		bool grp3 = text.find("{grp3}") != std::string::npos;
		bool grp4 = text.find("{grp4}") != std::string::npos;
		bool grp5 = text.find("{grp5}") != std::string::npos;
		bool grp6 = text.find("{grp6}") != std::string::npos;
		bool grp7 = text.find("{grp7}") != std::string::npos;
		bool grp8 = text.find("{grp8}") != std::string::npos;

		if (grp1) ret.multi = Opcode::MULTI::GRP1;
		else if (grp2) ret.multi = Opcode::MULTI::GRP2;
		else if (grp3) ret.multi = Opcode::MULTI::GRP3;
		else if (grp4) ret.multi = Opcode::MULTI::GRP4;
		else if (grp5) ret.multi = Opcode::MULTI::GRP5;
		else if (grp6) ret.multi = Opcode::MULTI::GRP6;
		else if (grp7) ret.multi = Opcode::MULTI::GRP7;
		else if (grp8) ret.multi = Opcode::MULTI::GRP8;
		else ret.multi = Opcode::MULTI::NONE;

		if (ret.i8) ret.imm = Opcode::IMM::W8;
		else if (ret.i16) ret.imm = Opcode::IMM::W16;
		else if (ret.i32) ret.imm = Opcode::IMM::W32;
		else ret.imm = Opcode::IMM::NONE;

		// Special case: I16/I8 (ENTER)
		if (ret.i8 && ret.i16)
		{
			ret.imm = Opcode::IMM::W16W8;
		}
		// Check for double i8
		if (ret.i8)
		{
			// Skip over first one
			std::string::size_type pos = text.find("{i8}");
			pos = text.find("{i8}", pos + 1);
			if (pos != std::string::npos)
			{
				ret.imm = Opcode::IMM::W8W8;
			}
		}

		if (ret.sr) ret.modRegRm = Opcode::MODREGRM::SR;
		else if (ret.r8 || ret.rm8) ret.modRegRm = Opcode::MODREGRM::W8;
		else if (ret.r16 || ret.rm16) ret.modRegRm = Opcode::MODREGRM::W16;
		else ret.modRegRm = Opcode::MODREGRM::NONE;

		return ret;
	}

	const std::string CPUInfo::GetSubOpcodeStr(const Opcode& parent, BYTE op2) const
	{
		if (parent.multi == Opcode::MULTI::NONE)
		{
			return "{err}";
		}
		else
		{
			return m_subOpcodes[(int)parent.multi][op2];
		}
	}

	Opcode CPUInfo::GetSubOpcode(const Opcode& parent, BYTE op2) const
	{
		if (parent.multi == Opcode::MULTI::NONE)
		{
			return parent;
		}
		else
		{
			std::string subOpcodeText = m_subOpcodes[(int)parent.multi][op2];

			// There may be another level of indirection
			return BuildOpcode(subOpcodeText);
		}
	}

	std::string CPUInfo::GetANSIFile() const
	{
		std::string fileName = m_config["monitor"]["ANSIFile"].get<std::string>();

		std::string ansiFileData;
		std::string ansiFileName = "config/" + std::string(fileName);
		std::ifstream ansiFile(ansiFileName);
		if (ansiFile)
		{
			std::ostringstream ss;
			ss << ansiFile.rdbuf();
			return ss.str();
		}
		else
		{
			LogPrintf(LOG_ERROR, "GetANSIFile: File not found [%s]", ansiFileName.c_str());
			throw std::exception("file not found");
		}
	}

	Coord CPUInfo::GetCoord(const char* label) const
	{
		static json coords = m_config["monitor"]["coords"];

		if (coords.find(label) != coords.end())
		{
			auto coord = coords[label];
			Coord c;
			c.x = coord["x"].get<short>();
			c.y = coord["y"].get<short>();
			c.w = coord["w"].get<short>();
			if (!coord["h"].is_null())
			{
				c.h = coord["h"].get<short>();
			}
			else
			{
				c.h = 0;
			}
			return c;
		}

		return Coord();
	}
}
