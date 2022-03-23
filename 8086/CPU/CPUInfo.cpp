#include "stdafx.h"

#include "CPUInfo.h"

namespace emul
{
	CPUInfo g_CPUInfo(CPUType::i8086, "8086", "8086.json");

	CPUInfo::CPUInfo() : CPUInfo(CPUType::i8086, nullptr, nullptr)
	{
	}

	CPUInfo::CPUInfo(CPUType model, const char* name, const char* configFileName) :
		m_model(model),
		m_name(name),
		m_configFileName(configFileName)
	{
	}

	void CPUInfo::LoadConfig()
	{
		std::ifstream configFile("config/" + std::string(m_configFileName));
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

		for (int i = 0; i < 5; ++i)
		{
			char grpName[32];
			sprintf(grpName, "opcodes.grp%d", i + 1);
			if (m_config["cpu"][grpName].size() != 8)
			{
				throw std::exception("opcode.grp# list incomplete");
			}
			BuildSubOpcodes(i, m_config["cpu"][grpName]);
		}
	}

	void CPUInfo::BuildOpcodes(const json& opcodes)
	{
		for (int i = 0; i < 256; ++i)
		{
			m_opcodes[i] = BuildOpcode(opcodes, i);
		}
	}

	void CPUInfo::BuildSubOpcodes(int index, const json& opcodes)
	{
		for (int i = 0; i < 8; ++i)
		{
			m_subOpcodes[index][i] = opcodes[i].get<std::string>();
		}
	}

	CPUInfo::Opcode CPUInfo::BuildOpcode(const json& opcodes, BYTE op)
	{
		Opcode ret;
		const std::string text = opcodes[op].get<std::string>();
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

		if (grp1) ret.multi = Opcode::MULTI::GRP1;
		else if (grp2) ret.multi = Opcode::MULTI::GRP2;
		else if (grp3) ret.multi = Opcode::MULTI::GRP3;
		else if (grp4) ret.multi = Opcode::MULTI::GRP4;
		else if (grp5) ret.multi = Opcode::MULTI::GRP5;
		else ret.multi = Opcode::MULTI::NONE;

		if (ret.i8) ret.imm = Opcode::IMM::W8;
		else if (ret.i16) ret.imm = Opcode::IMM::W16;
		else if (ret.i32) ret.imm = Opcode::IMM::W32;
		else ret.imm = Opcode::IMM::NONE;

		if (ret.sr) ret.modRegRm = Opcode::MODREGRM::SR;
		else if (ret.r8 || ret.rm8) ret.modRegRm = Opcode::MODREGRM::W8;
		else if (ret.r16 || ret.rm16) ret.modRegRm = Opcode::MODREGRM::W16;
		else ret.modRegRm = Opcode::MODREGRM::NONE;

		return ret;
	}

	const std::string CPUInfo::GetSubOpcode(const Opcode& parent, BYTE op2)
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

	CPUInfo::Coord CPUInfo::GetCoord(const char* label) const
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
