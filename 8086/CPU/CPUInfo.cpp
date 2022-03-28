#include "stdafx.h"

#include "CPUInfo.h"

namespace cpuInfo
{
	CPUInfo::CPUInfo(CPUType model) :
		m_model(model)
	{
		const auto& cpu = m_cpus.find(model);
		if (cpu == m_cpus.end())
		{
			throw std::exception("cpu model not found");
		}

		m_name = cpu->second.name;
		m_configFile = cpu->second.configFile;
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
			throw std::exception("file not found");
		}

		if (m_config["cpu"]["opcodes"].size() != 256) 
		{
			throw std::exception("opcode list incomplete");
		}	
		BuildOpcodes(m_config["cpu"]["opcodes"]);

		int groupCount = m_config["cpu"]["opcodes.grp"];
		if ((groupCount < 1) || (groupCount > (int)Opcode::MULTI::_COUNT))
		{
			throw std::exception("invalid opcode.grp count");
		}

		for (int i = 0; i < groupCount; ++i)
		{
			char grpName[32];
			sprintf(grpName, "opcodes.grp%d", i + 1);
			if (m_config["cpu"][grpName].size() != 8)
			{
				throw std::exception("opcode.grp# list incomplete");
			}
			BuildSubOpcodes(i, m_config["cpu"][grpName]);
		}

		if (m_config["cpu"]["misc"].size() != (int)MiscTiming::_COUNT)
		{
			throw std::exception("misc timing list incomplete");
		}
		for (int i = 0; i < (int)MiscTiming::_COUNT; ++i)
		{
			m_miscTiming[i] = BuildTiming(m_config["cpu"]["misc"][i]);
		}
	}

	void CPUInfo::BuildOpcodes(const json& opcodes)
	{
		for (int i = 0; i < 256; ++i)
		{
			m_opcodes[i] = BuildOpcode(opcodes[i]["name"]);
			m_timing[i] = BuildTiming(opcodes[i]);
		}
	}

	void CPUInfo::BuildSubOpcodes(int index, const json& opcodes)
	{
		for (int i = 0; i < 8; ++i)
		{
			m_subOpcodes[index][i] = opcodes[i]["name"];
			m_subTiming[index][i] = BuildTiming(opcodes[i]);
		}
	}

	OpcodeTiming CPUInfo::BuildTiming(const json& opcode) const
	{
		OpcodeTiming timing = { 1, 0, 0, 0 };
		if (!opcode.contains("timing"))
		{
			return timing;
		}

		const json& jsonTiming = opcode["timing"];
		if (!jsonTiming.is_array() || jsonTiming.size() < 1 || jsonTiming.size() > 4)
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

		if (ret.sr) ret.modRegRm = Opcode::MODREGRM::SR;
		else if (ret.r8 || ret.rm8) ret.modRegRm = Opcode::MODREGRM::W8;
		else if (ret.r16 || ret.rm16) ret.modRegRm = Opcode::MODREGRM::W16;
		else ret.modRegRm = Opcode::MODREGRM::NONE;

		return ret;
	}

	const std::string CPUInfo::GetSubOpcodeStr(const Opcode& parent, BYTE op2) const
	{
		if (parent.multi == Opcode::MULTI::NONE || op2 > 7)
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
		std::ifstream ansiFile("config/" + std::string(fileName));
		if (ansiFile)
		{
			std::ostringstream ss;
			ss << ansiFile.rdbuf();
			return ss.str();
		}
		else
		{
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
