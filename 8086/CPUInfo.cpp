#include "stdafx.h"
#include "CPUInfo.h"
#include <iostream>
#include <fstream>
#include <sstream>

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

		if (m_config["cpu"]["OpCodes"].size() != 256) 
		{
			throw std::exception("opcode list incomplete");
		}

		BuildOpcodes();
	}

	void CPUInfo::BuildOpcodes()
	{
		for (int i = 0; i < 256; ++i)
		{
			m_opcodes[i] = BuildOpcode(i);
		}
	}

	CPUInfo::Opcode CPUInfo::BuildOpcode(BYTE op)
	{
		Opcode ret;
		const std::string text = m_config["cpu"]["OpCodes"][op].get<std::string>();
		ret.text = text;

		ret.rm8 = text.find("{rm8}") != std::string::npos;
		ret.rm16 = text.find("{rm16}") != std::string::npos;

		ret.r8 = text.find("{r8}") != std::string::npos;
		ret.r16 = text.find("{r16}") != std::string::npos;
		ret.sr = text.find("{sr}") != std::string::npos;

		ret.i8 = text.find("{i8}") != std::string::npos;
		ret.i16 = text.find("{i16}") != std::string::npos;
		ret.i32 = text.find("{i32}") != std::string::npos;

		bool alu = text.find("{alu}") != std::string::npos;
		bool alu2 = text.find("{alu2}") != std::string::npos;
		bool shift = text.find("{shift}") != std::string::npos;
		bool incdec = text.find("{incdec}") != std::string::npos;
		bool multi = text.find("{multi}") != std::string::npos;

		if (alu) ret.multi = Opcode::MULTI::ALU;
		else if (alu2) ret.multi = Opcode::MULTI::ALU2;
		else if (shift) ret.multi = Opcode::MULTI::SHIFT;
		else if (incdec) ret.multi = Opcode::MULTI::INCDEC;
		else if (multi) ret.multi = Opcode::MULTI::MULTI;
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
