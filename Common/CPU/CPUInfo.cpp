#include "stdafx.h"

#include <CPU/CPUInfo.h>
#include <regex>

namespace cpuInfo
{
	CPUInfo::CPUInfo(const char* cpuid) : Logger("CPUInfo")
	{
		EnableLog(LOG_WARNING);

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

		json& cpuData = m_config["cpu"];

		LoadDefaultTimings(cpuData);
		LoadMainOpcodeTable(cpuData);
		LoadOpcodeGroups(cpuData);
		LoadMiscTimings(cpuData);
	}

	void CPUInfo::LoadDefaultTimings(json& cpuData)
	{
		// Default timings
		if (cpuData.contains("opcodes.timing"))
		{
			m_defaultOpcodeTiming = BuildTimingDirect(m_config["cpu"]["opcodes.timing"]);
			LogPrintf(LOG_INFO, "Default Opcode timing: %s", GetTimingString(m_defaultOpcodeTiming).c_str());
			m_defaultSubOpcodeTiming = m_defaultOpcodeTiming;
		}

		if (cpuData.contains("opcodes.grp.timing"))
		{
			m_defaultSubOpcodeTiming = BuildTimingDirect(m_config["cpu"]["opcodes.grp.timing"]);
			LogPrintf(LOG_INFO, "Default Subopcode timing: %s", GetTimingString(m_defaultSubOpcodeTiming).c_str());
		}
	}

	void CPUInfo::LoadMainOpcodeTable(json& cpuData)
	{
		if (!cpuData.contains("opcodes"))
		{
			throw std::exception("missing [opcodes] array");
		}

		// Check for optional size
		size_t opcodeCount = MAX_OPCODE + 1;
		if (cpuData.contains("opcodes.size"))
		{
			opcodeCount = cpuData["opcodes.size"];
			if (opcodeCount == 0 || opcodeCount > MAX_OPCODE)
			{
				LogPrintf(LOG_ERROR, "Invalid opcode.size: %d", opcodeCount);
				throw std::exception("invalid opcode.size");
			}
		}
		LogPrintf(LOG_INFO, "Opcode table, size [%d]", opcodeCount);

		BuildOpcodes(cpuData["opcodes"], opcodeCount);
	}

	void CPUInfo::LoadOpcodeGroups(json& cpuData)
	{
		constexpr const char* OPCODES_GRP = "opcodes.grp";

		int groupCount = 0;

		if (cpuData.contains(OPCODES_GRP))
		{
			groupCount = cpuData[OPCODES_GRP];
		}

		if (groupCount > (int)Opcode::MULTI::_COUNT)
		{
			LogPrintf(LOG_ERROR, "Subopcode group count > %d", (int)Opcode::MULTI::_COUNT);
			throw std::exception("invalid opcode.grp count");
		}

		for (int i = 0; i < groupCount; ++i)
		{
			char groupName[32];
			sprintf(groupName, "%s%d", OPCODES_GRP, i + 1);
			LoadOpcodeGroup(cpuData, groupName, i);
		}
	}

	void CPUInfo::LoadOpcodeGroup(json& cpuData, const char* groupName, int groupIndex)
	{
		if (!cpuData.contains(groupName))
		{
			LogPrintf(LOG_ERROR, "Subopcode table [%s] not found", groupName);
			throw std::exception("opcode.grp# list missing");
		}

		// Check for optional size, otherwise use MAX_OPCODE
		std::string size(groupName);
		size.append(".size");
		size_t subOpcodeCount = MAX_OPCODE + 1;
		if (cpuData.contains(size))
		{
			subOpcodeCount = cpuData[size];
			if (subOpcodeCount == 0)
			{
				// Empty group, skip
				m_subOpcodes[groupIndex].clear();
				m_subTiming[groupIndex].clear();
				return;
			}
			else if (subOpcodeCount > MAX_OPCODE + 1)
			{
				LogPrintf(LOG_ERROR, "Subopcode table [%s] invalid size: %d", groupName, subOpcodeCount);
				throw std::exception("invalid grp size");
			}
		}

		LogPrintf(LOG_INFO, "Subopcode table [%s], size [%d]", groupName, subOpcodeCount);

		// Set opcode & timing table sizes
		m_subOpcodes[groupIndex].resize(subOpcodeCount);
		m_subTiming[groupIndex].resize(subOpcodeCount);

		// Check for fill opcode
		std::string fill(groupName);
		fill.append(".fill");
		Opcode fillOpcode;
		OpcodeTiming fillTiming;
		bool hasFillOpcode = false;
		if (cpuData.contains(fill) && cpuData[fill].contains("name"))
		{
			fillOpcode = BuildOpcode(cpuData[fill]["name"]);
			hasFillOpcode = fillOpcode.text.size();

			fillTiming = BuildTiming(cpuData[fill]);
			BuildSubOpcodes(groupIndex, cpuData[groupName], &fillOpcode, &fillTiming);
		}
		else
		{
			BuildSubOpcodes(groupIndex, cpuData[groupName]);
		}
	}

	void CPUInfo::LoadMiscTimings(json& cpuData)
	{
		// Misc timing table
		if (cpuData.contains("misc"))
		{
			size_t miscCount = cpuData["misc"].size();

			if (miscCount > (int)MiscTiming::_COUNT)
			{
				LogPrintf(LOG_ERROR, "MISC timing list too large: [%d>%d]", miscCount, (int)MiscTiming::_COUNT);
				throw std::exception("misc timing list too large");
			}
			for (size_t i = 0; i < miscCount; ++i)
			{
				m_miscTiming[i] = BuildTiming(m_config["cpu"]["misc"][i]);
				LogPrintf(LOG_DEBUG, "MISC Timing[%d]: %s", i, GetTimingString(m_miscTiming[i]).c_str());
			}
		}
	}


	void CPUInfo::BuildOpcodes(const json& opcodes, size_t tableSize)
	{
		LogPrintf(LOG_INFO, "BuildOpcodes");

		m_opcodes.resize(tableSize);
		m_timing.resize(tableSize);

		// First fill in the array so we don't have empty holes
		for (int i = 0; i < m_opcodes.size(); ++i)
		{
			char buf[16];
			sprintf(buf, "DB 0x%02X", i);
			m_opcodes[i] = buf;
			m_timing[i] = m_defaultOpcodeTiming;
		}

		AddOpcodes(opcodes, m_opcodes, m_timing);
	}

	void CPUInfo::BuildSubOpcodes(int index, const json& opcodesJson, Opcode* fillOpcode, OpcodeTiming* fillTiming)
	{
		LogPrintf(LOG_INFO, "BuildSubOpcodes[%d]", index);
		OpcodeTable& opcodes = m_subOpcodes[index];
		OpcodeTimingTable& timings = m_subTiming[index];

		// First fill in the array so we don't have empty holes
		for (int i = 0; i < opcodes.size(); ++i)
		{
			char buf[16];
			sprintf(buf, "DB 0x%02X", i);
			Opcode defaultOpcode(buf);
			opcodes[i] = fillOpcode ? (*fillOpcode) : defaultOpcode;
			timings[i] = fillTiming ? (*fillTiming) : m_defaultSubOpcodeTiming;
		}

		AddOpcodes(opcodesJson, opcodes, timings);
	}

	void CPUInfo::AddOpcodes(const json& opcodes, OpcodeTable& opcodeTable, OpcodeTimingTable& timingTable)
	{
		// Trivial case is an array of 256 opcodes. However, to account for holes
		// and other weirdness, it is possible to specify a new index (int or hex string e.g. "0x12")
		// so that the count jumps to the new value and continue loading opcodes linearly from there.
		size_t opcodeIndex = 0;
		for (const json& curr : opcodes)
		{
			if (opcodeIndex < 0 || opcodeIndex > (opcodeTable.size() - 1))
			{
				LogPrintf(LOG_ERROR, "AddOpcodes: invalid index [%d]", opcodeIndex);
				throw std::exception("AddOpcodes: invalid index");
			}

			if (curr.is_number_integer()) // new index (integer)
			{
				opcodeIndex = curr;
				LogPrintf(LOG_DEBUG, "Set New Index [%02X]", opcodeIndex);
			}
			else if (curr.is_string()) // new index (string)
			{
				opcodeIndex = std::stoul((const std::string&)curr, nullptr, 0);
				LogPrintf(LOG_DEBUG, "Set New Index [%02X]", opcodeIndex);
			}
			else if (curr.is_object()) // opcode data
			{
				Opcode opcode = BuildOpcode(curr["name"]);
				SetOverrideMask(curr, opcode, "overrideMask");

				// Look for alternate opcode
				if (curr.contains("alt"))
				{
					std::string mask = curr.contains("altMask") ? curr["altMask"] : "";

					if (mask.empty())
					{
						LogPrintf(LOG_ERROR, "Opcode[%02X]: Alternate opcode without mask", opcodeIndex);
					}
					else
					{
						std::shared_ptr<Opcode> altOpcode = std::make_shared<Opcode>();
						*altOpcode = BuildOpcode(curr["alt"]);
						SetOverrideMask(curr, *altOpcode, "altOverrideMask");

						opcode.altMask.Set(mask.c_str());
						opcode.alt = altOpcode;

						// TODO: Alt Timing

						LogPrintf(LOG_INFO, "\tAltOpcode[%02X]: [%-32s], Mask: [%s]",
							opcodeIndex,
							altOpcode->text.c_str(),
							opcode.altMask.ToString().c_str()
							/*GetTimingString(timing).c_str()*/);
					}
				}

				opcodeTable[opcodeIndex] = opcode;

				OpcodeTiming timing = BuildTiming(curr, "timing");
				timingTable[opcodeIndex] = timing;

				LogPrintf(LOG_DEBUG, "\tOpcode[%02X]: [%-32s], Timing: %s",
					opcodeIndex,
					opcode.text.c_str(),
					GetTimingString(timing).c_str());

				++opcodeIndex;
			}
			else
			{
				throw std::exception("AddOpcodes: Invalid value");
			}
		}
	}

	std::string CPUInfo::GetTimingString(const OpcodeTiming& timing) const
	{
		std::ostringstream os;
		os << "[ ";

		for (auto t : timing)
		{
			os << std::setw(3) << (int)t << ' ';
		}
		os << ']';
		return os.str();
	}

	OpcodeTiming CPUInfo::BuildTiming(const json& opcode, const char* timingKey) const
	{
		OpcodeTiming timing = m_defaultOpcodeTiming;
		if (!opcode.contains(timingKey))
		{
			return timing;
		}

		const json& jsonTiming = opcode[timingKey];
		return BuildTimingDirect(jsonTiming);
	}

	OpcodeTiming CPUInfo::BuildTimingDirect(const json& timingArray) const
	{
		OpcodeTiming timing = m_defaultOpcodeTiming;
		if (!timingArray.is_array() || timingArray.size() < 1 || timingArray.size() > (int)OpcodeTimingType::_COUNT)
		{
			throw std::exception("invalid timing array");
		}

		for (size_t i = 0; i < timingArray.size(); ++i)
		{
			timing[i] = timingArray[i];
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

		ret.regreg = text.find("{r,r}") != std::string::npos;
		ret.regs = text.find("{regs}") != std::string::npos;
		ret.idx = text.find("{idx}") != std::string::npos;
		ret.idxidx = text.find("{idx,idx}") != std::string::npos;
		ret.bit = text.find("{bit}") != std::string::npos;

		ret.rm8 = text.find("{rm8}") != std::string::npos;
		ret.rm16 = text.find("{rm16}") != std::string::npos;

		ret.r8 = text.find("{r8}") != std::string::npos;
		ret.r16 = text.find("{r16}") != std::string::npos;
		ret.sr = text.find("{sr}") != std::string::npos;

		ret.i8 = text.find("{i8}") != std::string::npos;
		ret.i16 = text.find("{i16}") != std::string::npos;
		ret.i32 = text.find("{i32}") != std::string::npos;

		ret.s8 = text.find("{s8}") != std::string::npos;
		ret.s16 = text.find("{s16}") != std::string::npos;

		ret.dataReg = text.find("{dr}") != std::string::npos;
		ret.addressReg = text.find("{ar}") != std::string::npos;

		bool grp1 = text.find("{grp1}") != std::string::npos;
		bool grp2 = text.find("{grp2}") != std::string::npos;
		bool grp3 = text.find("{grp3}") != std::string::npos;
		bool grp4 = text.find("{grp4}") != std::string::npos;
		bool grp5 = text.find("{grp5}") != std::string::npos;
		bool grp6 = text.find("{grp6}") != std::string::npos;
		bool grp7 = text.find("{grp7}") != std::string::npos;
		bool grp8 = text.find("{grp8}") != std::string::npos;
		bool grp9 = text.find("{grp9}") != std::string::npos;
		bool grp10 = text.find("{grp10}") != std::string::npos;
		bool grp11 = text.find("{grp11}") != std::string::npos;
		bool grp12 = text.find("{grp12}") != std::string::npos;
		bool grp13 = text.find("{grp13}") != std::string::npos;
		bool grp14 = text.find("{grp14}") != std::string::npos;
		bool grp15 = text.find("{grp15}") != std::string::npos;
		bool grp16 = text.find("{grp16}") != std::string::npos;

		if (grp1) ret.multi = Opcode::MULTI::GRP1;
		else if (grp2) ret.multi = Opcode::MULTI::GRP2;
		else if (grp3) ret.multi = Opcode::MULTI::GRP3;
		else if (grp4) ret.multi = Opcode::MULTI::GRP4;
		else if (grp5) ret.multi = Opcode::MULTI::GRP5;
		else if (grp6) ret.multi = Opcode::MULTI::GRP6;
		else if (grp7) ret.multi = Opcode::MULTI::GRP7;
		else if (grp8) ret.multi = Opcode::MULTI::GRP8;
		else if (grp9) ret.multi = Opcode::MULTI::GRP9;
		else if (grp10) ret.multi = Opcode::MULTI::GRP10;
		else if (grp11) ret.multi = Opcode::MULTI::GRP11;
		else if (grp12) ret.multi = Opcode::MULTI::GRP12;
		else if (grp13) ret.multi = Opcode::MULTI::GRP13;
		else if (grp14) ret.multi = Opcode::MULTI::GRP14;
		else if (grp15) ret.multi = Opcode::MULTI::GRP15;
		else if (grp16) ret.multi = Opcode::MULTI::GRP16;
		else ret.multi = Opcode::MULTI::NONE;

		if (ret.regreg) ret.imm = Opcode::IMM::REGREG;
		else if (ret.i8) ret.imm = Opcode::IMM::W8;
		else if (ret.i16) ret.imm = Opcode::IMM::W16;
		else if (ret.i32) ret.imm = Opcode::IMM::W32;
		else if (ret.s8) ret.imm = Opcode::IMM::S8;
		else if (ret.s16) ret.imm = Opcode::IMM::S16;
		else ret.imm = Opcode::IMM::NONE;

		// Special case: I16/I8 (8088 ENTER)
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
			return m_subOpcodes[(int)parent.multi][op2].text;
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
			const Opcode& subOpcode = m_subOpcodes[(int)parent.multi][op2];
			std::string subOpcodeText = m_subOpcodes[(int)parent.multi][op2].text;

			if (subOpcode.alt || subOpcode.overrideMask)
			{
				return subOpcode;
			}

			// There may be another level of indirection
			return BuildOpcode(subOpcodeText);
		}
	}

	void CPUInfo::SetOverrideMask(const json& jsonOpcode, Opcode& opcode, const char* key)
	{
		std::string overrideMask = jsonOpcode.contains(key) ? jsonOpcode[key] : "";
		if (overrideMask.size())
		{
			opcode.overrideMask = emul::BitMaskW(overrideMask.c_str());
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
