#include "stdafx.h"
#include "Monitor68000.h"
#include "CPU/CPU68000.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;
using emul::GetBit;

namespace emul
{
	using namespace cpu68k;

	static const char hexDigits[] = "0123456789ABCDEF";

	void Monitor68000::EffectiveAddress::BuildImmediate()
	{
		switch (m_size)
		{
		case EASize::Byte:
		{
			WORD byteImm = m_memory.Read16be(m_currAddress);
			m_currAddress += 2;
			m_currInstruction.AddRaw(byteImm);
			sprintf(m_text, "#$%02X", GetLByte(byteImm));
			break;
		}
		case EASize::Word:
		{
			WORD wordImm = m_memory.Read16be(m_currAddress);
			m_currAddress += 2;
			m_currInstruction.AddRaw(wordImm);
			sprintf(m_text, "#$%04X", wordImm);
			break;
		}
		case EASize::Long:
		{
			DWORD longImm = m_memory.Read32be(m_currAddress);
			m_currAddress += 4;
			m_currInstruction.AddRaw(longImm);
			sprintf(m_text, "#$%08X", longImm);
			break;
		}
		default:
			strcpy(m_text, "[imm unknown]");
			break;
		}
	}

	const char* Monitor68000::EffectiveAddress::BuildText()
	{
		switch (m_mode)
		{
		case EAMode::DataRegDirect:
			sprintf(m_text, "D%u", m_regNumber);
			break;
		case EAMode::AddrRegDirect:
			sprintf(m_text, "A%u", m_regNumber);
			break;
		case EAMode::AddrRegIndirect:
			sprintf(m_text, "(A%u)", m_regNumber);
			break;
		case EAMode::AddrRegIndirectPostincrement:
			sprintf(m_text, "(A%u)+", m_regNumber);
			break;
		case EAMode::AddrRegIndirectPredecrement:
			sprintf(m_text, "-(A%u)", m_regNumber);
			break;
		case EAMode::AddrRegIndirectDisplacement:
			sprintf(m_text, "%d(A%u)", GetDisplacementWord(), m_regNumber);
			break;
		case EAMode::AddrRegIndirectIndex:
			strcpy(m_text, GetExtensionWord(m_regNumber));
			break;
		case EAMode::AbsoluteShort:
			sprintf(m_text, "($%04X).w", m_address);
			break;
		case EAMode::AbsoluteLong:
			sprintf(m_text, "($%08X).l", m_address);
			break;
		case EAMode::ProgramCounterDisplacement:
			sprintf(m_text, "%d(PC)", GetDisplacementWord());
			break;
		case EAMode::ProgramCounterIndex:
			strcpy(m_text, GetExtensionWord(PC));
			break;
		case EAMode::Immediate: // TODO: Need size
			BuildImmediate();
			break;
		case EAMode::Invalid:
			sprintf(m_text, "[eaERR]");
			break;
		}

		return m_text;
	}

	void Monitor68000::EffectiveAddress::ComputeEA(WORD opcode)
	{
		m_regNumber = opcode & 7;
		m_size = (EASize)((opcode >> 6) & 3);
		m_mode = CPU68000::GetEAMode(opcode);

		// Get data for absolute modes
		switch (m_mode)
		{
		case EAMode::AbsoluteShort:
		{
			WORD shortAddress = m_memory.Read16be(m_currAddress);
			m_currAddress += 2;
			m_address = emul::Widen(shortAddress);
			m_currInstruction.AddRaw(shortAddress);
			break;
		}
		case EAMode::AbsoluteLong:
		{
			m_address = m_memory.Read32be(m_currAddress);
			m_currAddress += 4;
			m_currInstruction.AddRaw(m_address);
			break;
		}
		}
	}

	SWORD Monitor68000::EffectiveAddress::GetDisplacementWord()
	{
		WORD disp = m_memory.Read16be(m_currAddress);
		m_currAddress += 2;
		m_currInstruction.AddRaw(disp);
		return SWORD(disp);
	}

	const char* Monitor68000::EffectiveAddress::GetExtensionWord(BYTE reg)
	{
		char addrRegName[3] = "PC";
		if (reg != PC)
		{
			addrRegName[0] = 'A';
			addrRegName[1] = reg + '0';
		}

		WORD extWord = m_memory.Read16be(m_currAddress);
		m_currAddress += 2;
		m_currInstruction.AddRaw(extWord);

		SBYTE displacement = (SBYTE)(emul::GetLByte(extWord));

		sprintf(m_extWord, "%d(%s,%c%u.%c)",
			displacement,
			addrRegName,
			GetMSB(extWord) ? 'A' : 'D',
			((extWord >> 12) & 7),
			GetBit(extWord, 11) ? 'l' : 'w');

		return m_extWord;
	}

	Monitor68000::Monitor68000(Console& console) :
		m_console(console)
	{
	}

	void Monitor68000::Init(CPU* cpu, Memory& memory)
	{
		m_cpu = dynamic_cast<CPU68000*>(cpu);
		m_memory = &memory;
	}

	MonitorState Monitor68000::ProcessKey()
	{
		if (!_kbhit())
			return MonitorState::RUN;

		int ch = _getch();
		if (ch == 27)
		{
			return MonitorState::EXIT;
		}
		else if (ch == 224)
		{
			switch (ch = _getch())
			{
			case 134: // F12
				return MonitorState::SWITCH_MODE;
			}
		}
		else if (ch == 0)
		{
			switch (ch = _getch())
			{
			case 61: // F3
				ToggleCodeMode();
				break;

			case 62: // F4
				ToggleRAMMode();
				break;

			case 63: // F5
				ToggleRunMode();
				break;

			case 66: // F8
				m_runMode = RUNMode::STEP;
				UpdateRunMode();
				return MonitorState::RUN;

			case 98: // CTRL-F5
				m_runMode = RUNMode::STEP;
				m_cpu->Reset();
				Update();
				break;

				// Not implemented, ignore
			case 59: // F1
			case 60: // F2
			case 64: // F6
			case 65: // F7
			case 67: // F9
			case 68: // F10
			default:
				break;
			}
		}
		return (m_runMode == RUNMode::STEP) ? MonitorState::WAIT : MonitorState::RUN;
	}

	MonitorState Monitor68000::Run()
	{
		Update();
		if (m_runMode == RUNMode::STEP)
		{
			m_console.WaitForKey();
		}

		return ProcessKey();
	}

	void Monitor68000::Show()
	{
		std::string ansiFile = m_cpu->GetInfo().GetANSIFile();

		if (ansiFile.size())
		{
			m_console.WriteBuffer(ansiFile.c_str(), ansiFile.size());
		}

		Update();
		UpdateRunMode();
		UpdateRAMMode();
		UpdateCPUType();
		ClearCode();
	}

	void Monitor68000::WriteValueNibble(BYTE value, const cpuInfo::Coord& coord, WORD attr)
	{
		static char hex[1];
		hex[0] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 1, attr);
	}

	void Monitor68000::WriteValueHex(BYTE value, const Coord& coord, WORD attr)
	{
		static char hex[2];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 2, attr);
	}

	void Monitor68000::WriteValueHex(WORD value, const Coord& coord, WORD attr)
	{
		static char hex[4];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
	}

	void Monitor68000::WriteValueHex(DWORD value, const Coord& coord, WORD attr)
	{
		static char hex[8];

		hex[0] = hexDigits[(value >> 28) & 0x0F];
		hex[1] = hexDigits[(value >> 24) & 0x0F];
		hex[2] = hexDigits[(value >> 20) & 0x0F];
		hex[3] = hexDigits[(value >> 16) & 0x0F];
		hex[4] = hexDigits[(value >> 12) & 0x0F];
		hex[5] = hexDigits[(value >> 8) & 0x0F];
		hex[6] = hexDigits[(value >> 4) & 0x0F];
		hex[7] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 8, attr);
	}

	void Monitor68000::WriteValueHex24(DWORD value, const Coord& coord, WORD attr)
	{
		static char hex[6];

		hex[0] = hexDigits[(value >> 20) & 0x0F];
		hex[1] = hexDigits[(value >> 16) & 0x0F];
		hex[2] = hexDigits[(value >> 12) & 0x0F];
		hex[3] = hexDigits[(value >> 8) & 0x0F];
		hex[4] = hexDigits[(value >> 4) & 0x0F];
		hex[5] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 6, attr);
	}

	void Monitor68000::Update()
	{
		UpdateRegisters();
		UpdateTicks();
		UpdateFlags();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor68000::UpdateCPUType()
	{
		static Coord cpuId = m_cpu->GetInfo().GetCoord("CPUID");
		std::ostringstream os;
		os << std::setw(cpuId.w) << m_cpu->GetInfo().GetId();
		m_console.WriteAt(cpuId.x, cpuId.y, os.str().c_str());
	}

	void Monitor68000::ToggleRunMode()
	{
		m_runMode = (m_runMode == RUNMode::RUN) ? RUNMode::STEP : RUNMode::RUN;
		UpdateRunMode();
	}

	void Monitor68000::ToggleCodeMode()
	{
		m_codeMode = (m_codeMode == CODEMode::RAW_AND_TEXT) ? CODEMode::TEXT_ONLY : CODEMode::RAW_AND_TEXT;
		ClearCode();
		UpdateCode();
	}

	void Monitor68000::UpdateRunMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (1 << 4) | 14;
		static Coord run = m_cpu->GetInfo().GetCoord("status.RUN");
		static Coord stop = m_cpu->GetInfo().GetCoord("status.STOP");

		m_console.WriteAttrAt(run.x, run.y, (m_runMode == RUNMode::RUN) ? highlight : regular, run.w);
		m_console.WriteAttrAt(stop.x, stop.y, (m_runMode == RUNMode::STEP) ? highlight : regular, stop.w);
	}

	void Monitor68000::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::A0: m_ramMode = RAMMode::A1; break;
		case RAMMode::A1: m_ramMode = RAMMode::A2; break;
		case RAMMode::A2: m_ramMode = RAMMode::A3; break;
		case RAMMode::A3: m_ramMode = RAMMode::A4; break;
		case RAMMode::A4: m_ramMode = RAMMode::A5; break;
		case RAMMode::A5: m_ramMode = RAMMode::A6; break;
		case RAMMode::A6: m_ramMode = RAMMode::USP; break;
		case RAMMode::USP: m_ramMode = RAMMode::SSP; break;
		case RAMMode::SSP: m_ramMode = RAMMode::PC; break;
		case RAMMode::PC: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::A0; break;
		}

		UpdateRAMMode();
	}

	void Monitor68000::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramA0 = m_cpu->GetInfo().GetCoord("ram.A0");
		static Coord ramA1 = m_cpu->GetInfo().GetCoord("ram.A1");
		static Coord ramA2 = m_cpu->GetInfo().GetCoord("ram.A2");
		static Coord ramA3 = m_cpu->GetInfo().GetCoord("ram.A3");
		static Coord ramA4 = m_cpu->GetInfo().GetCoord("ram.A4");
		static Coord ramA5 = m_cpu->GetInfo().GetCoord("ram.A5");
		static Coord ramA6 = m_cpu->GetInfo().GetCoord("ram.A6");
		static Coord ramUSP = m_cpu->GetInfo().GetCoord("ram.USP");
		static Coord ramSSP = m_cpu->GetInfo().GetCoord("ram.SSP");
		static Coord ramPC = m_cpu->GetInfo().GetCoord("ram.PC");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramA0.x, ramA0.y, (m_ramMode == RAMMode::A0) ? highlight : regular, ramA0.w);
		m_console.WriteAttrAt(ramA1.x, ramA1.y, (m_ramMode == RAMMode::A1) ? highlight : regular, ramA1.w);
		m_console.WriteAttrAt(ramA2.x, ramA2.y, (m_ramMode == RAMMode::A2) ? highlight : regular, ramA2.w);
		m_console.WriteAttrAt(ramA3.x, ramA3.y, (m_ramMode == RAMMode::A3) ? highlight : regular, ramA3.w);
		m_console.WriteAttrAt(ramA4.x, ramA4.y, (m_ramMode == RAMMode::A4) ? highlight : regular, ramA4.w);
		m_console.WriteAttrAt(ramA5.x, ramA5.y, (m_ramMode == RAMMode::A5) ? highlight : regular, ramA5.w);
		m_console.WriteAttrAt(ramA6.x, ramA6.y, (m_ramMode == RAMMode::A6) ? highlight : regular, ramA6.w);
		m_console.WriteAttrAt(ramUSP.x, ramUSP.y, (m_ramMode == RAMMode::USP) ? highlight : regular, ramUSP.w);
		m_console.WriteAttrAt(ramSSP.x, ramSSP.y, (m_ramMode == RAMMode::SSP) ? highlight : regular, ramSSP.w);
		m_console.WriteAttrAt(ramPC.x, ramPC.y, (m_ramMode == RAMMode::PC) ? highlight : regular, ramPC.w);

		{
			static char buf[32];
			sprintf(buf, " CUSTOM:%06X", m_customMemView);
			m_console.WriteAt(ramCustom.x, ramCustom.y, buf);
			m_console.WriteAttrAt(ramCustom.x, ramCustom.y, (m_ramMode == RAMMode::CUSTOM) ? highlight : regular, ramCustom.w);
		}

		UpdateRAM();
	}

	void Monitor68000::UpdateRegisters()
	{
		WriteValueHex(m_cpu->m_reg.ADDR[0], m_cpu->GetInfo().GetCoord("A0"));
		WriteValueHex(m_cpu->m_reg.ADDR[1], m_cpu->GetInfo().GetCoord("A1"));
		WriteValueHex(m_cpu->m_reg.ADDR[2], m_cpu->GetInfo().GetCoord("A2"));
		WriteValueHex(m_cpu->m_reg.ADDR[3], m_cpu->GetInfo().GetCoord("A3"));
		WriteValueHex(m_cpu->m_reg.ADDR[4], m_cpu->GetInfo().GetCoord("A4"));
		WriteValueHex(m_cpu->m_reg.ADDR[5], m_cpu->GetInfo().GetCoord("A5"));
		WriteValueHex(m_cpu->m_reg.ADDR[6], m_cpu->GetInfo().GetCoord("A6"));
		WriteValueHex(m_cpu->m_reg.ADDR[7], m_cpu->GetInfo().GetCoord("A7"), m_cpu->IsSupervisorMode() ? 12 : 15);

		WriteValueHex(m_cpu->m_reg.DATA[0], m_cpu->GetInfo().GetCoord("D0"));
		WriteValueHex(m_cpu->m_reg.DATA[1], m_cpu->GetInfo().GetCoord("D1"));
		WriteValueHex(m_cpu->m_reg.DATA[2], m_cpu->GetInfo().GetCoord("D2"));
		WriteValueHex(m_cpu->m_reg.DATA[3], m_cpu->GetInfo().GetCoord("D3"));
		WriteValueHex(m_cpu->m_reg.DATA[4], m_cpu->GetInfo().GetCoord("D4"));
		WriteValueHex(m_cpu->m_reg.DATA[5], m_cpu->GetInfo().GetCoord("D5"));
		WriteValueHex(m_cpu->m_reg.DATA[6], m_cpu->GetInfo().GetCoord("D6"));
		WriteValueHex(m_cpu->m_reg.DATA[7], m_cpu->GetInfo().GetCoord("D7"));

		DWORD a7Alt = m_cpu->IsSupervisorMode() ? m_cpu->m_reg.USP : m_cpu->m_reg.SSP;
		WriteValueHex(a7Alt, m_cpu->GetInfo().GetCoord("A7'"), 8);

		WriteValueHex24(m_cpu->GetCurrentAddress(), m_cpu->GetInfo().GetCoord("PC"), 11);
	}

	void Monitor68000::UpdateTicks()
	{
		uint32_t ticks = m_cpu->GetInstructionTicks();
		static char buf[5];
		sprintf(buf, "%4d", (BYTE)ticks);

		static Coord coord = m_cpu->GetInfo().GetCoord("TICKS");

		m_console.WriteAt(coord.x, coord.y, buf, 4);
	}

	void Monitor68000::UpdateFlags()
	{
		static Coord coord = m_cpu->GetInfo().GetCoord("FLAGS");
		int width = coord.w;

		static WORD attr[16];

		for (int i = 0; i < width; ++i)
		{
			BYTE highlight = ((i == 13) && m_cpu->IsSupervisorMode()) ? 12 : 15;
			attr[width-i-1] = GetBit(m_cpu->m_reg.flags, i) ? highlight : 8;
		}

		m_console.WriteAttrAt(coord.x, coord.y, attr, width);
	}

	ADDRESS Monitor68000::GetRAMBase() const
	{
		switch (m_ramMode)
		{
		case RAMMode::A0: return m_cpu->m_reg.ADDR[0];
		case RAMMode::A1: return m_cpu->m_reg.ADDR[1];
		case RAMMode::A2: return m_cpu->m_reg.ADDR[2];
		case RAMMode::A3: return m_cpu->m_reg.ADDR[3];
		case RAMMode::A4: return m_cpu->m_reg.ADDR[4];
		case RAMMode::A5: return m_cpu->m_reg.ADDR[5];
		case RAMMode::A6: return m_cpu->m_reg.ADDR[6];
		case RAMMode::USP: return m_cpu->GetUSP();
		case RAMMode::SSP: return m_cpu->GetSSP();
		case RAMMode::PC: return  m_cpu->GetCurrentAddress();
		case RAMMode::CUSTOM:
		default:
			return m_customMemView;
		}
	}

	void Monitor68000::UpdateRAM()
	{
		static Coord addrPos = m_cpu->GetInfo().GetCoord("ram.ADDR");
		static Coord hexPos = m_cpu->GetInfo().GetCoord("ram.HEX");
		static Coord charPos = m_cpu->GetInfo().GetCoord("ram.CHAR");
		static DWORD bytesPerLine = charPos.w;
		static DWORD bytesTotal = charPos.w * charPos.h;

		// Adjust position so the view doesn't move around too much
		ADDRESS offset = GetRAMBase();

		ADDRESS adjustedOffset = (offset / bytesPerLine) * bytesPerLine;
		if (adjustedOffset >= bytesPerLine)
		{
			adjustedOffset -= bytesPerLine; // Show one row before when possible
		}

		ADDRESS curr = offset;
		ADDRESS data = adjustedOffset;
		for (int y = 0; y < hexPos.h; ++y)
		{
			Coord pos;
			pos.x = addrPos.x;
			pos.y = addrPos.y + y;
			WriteValueHex24((adjustedOffset + (bytesPerLine * y)), pos, 8);

			for (DWORD x = 0; x < bytesPerLine; ++x)
			{
				ADDRESS a = data + (bytesPerLine * y) + x;
				BYTE ch = m_memory->Read8(a);
				pos.x = hexPos.x + (3 * x);
				WriteValueHex(ch, pos, (a==curr) ? 15 + (1<<4) : 7);

				m_console.WriteAt(charPos.x + x, pos.y, ch ? ch : 0xFA, ch ? 7 : 8);
			}
		}
	}

	void Monitor68000::PrintInstruction(short y, Instruction& instr)
	{
		static Coord addressPos = m_cpu->GetInfo().GetCoord("CODE.address");
		static Coord rawPos = m_cpu->GetInfo().GetCoord("CODE.raw");
		static Coord partialTextPos = m_cpu->GetInfo().GetCoord("CODE.text");
		static Coord fullTextPos = m_cpu->GetInfo().GetCoord("CODE.text.full");
		Coord* textPos = &partialTextPos;

		static short baseY = addressPos.y;

		// TODO: Horribly inefficient
		Coord pos;
		pos.y = baseY + y;

		// Write address
		pos.x = addressPos.x;
		WriteValueHex24(instr.address, pos, 8);

		// Write raw (if needed)
		if (m_codeMode == CODEMode::RAW_AND_TEXT)
		{
			pos.x = rawPos.x;
			m_console.WriteAt(pos.x, pos.y, (const char*)instr.raw, instr.rawLen);
			for (int i = 0; i < rawPos.w - instr.rawLen; ++i)
			{
				m_console.WriteAt(pos.x + instr.rawLen + i, pos.y, 0xFAu, 8);
			}
		}
		else // Text only
		{
			textPos = &fullTextPos;
		}

		// Write text (disassembly)
		pos.x = textPos->x;
		m_console.WriteAt(pos.x, pos.y, instr.text, textPos->w);
	}

	void Monitor68000::UpdateCode()
	{
		static Coord codePos = m_cpu->GetInfo().GetCoord("CODE");

		ADDRESS address = m_cpu->GetCurrentAddress();

		m_console.MoveBlockY(codePos.x, codePos.y, codePos.w, 4, codePos.y - 1);

		for (int i = 0; i < 8; ++i)
		{
			Instruction decoded;
			address = Disassemble(address, decoded);
			PrintInstruction(i+4, decoded);
		}
	}

	void Monitor68000::ClearCode()
	{
		static Coord codePos = m_cpu->GetInfo().GetCoord("CODE");

		m_console.Clear(codePos.x, codePos.y, codePos.w, codePos.h);
	}

	bool Monitor68000::Replace(std::string& str, const std::string& from, const std::string& to)
	{
		size_t start = str.find(from);
		if (start == std::string::npos)
		{
			return false;
		}
		str.replace(start, from.size(), to);
		return true;
	}

	ADDRESS Monitor68000::Disassemble(ADDRESS address, Monitor68000::Instruction& decoded)
	{
		decoded.address = address;
		WORD data = m_memory->Read16be(address);
		address += 2;
		decoded.AddRaw(data);

		// Get group from upper 4 bits
		BYTE group = (data >> 12) & 15;

		Opcode instr = m_cpu->GetInfo().GetOpcode(group);
		std::string text = instr.text;

		if (instr.multi != Opcode::MULTI::NONE)
		{
			// Sub opcode is next 4 or 6 bits
			BYTE op2;
			switch (instr.multi)
			{
			case Opcode::MULTI::GRP7: // 0b0110 Branch group
			case Opcode::MULTI::GRP8: // 0b0111 MOVEP group
				op2 = ((data >> 8) & 15);
				break;
			default:
				op2 = ((data >> 6) & 63);
				break;
			}

			const std::string op2Str = m_cpu->GetInfo().GetSubOpcodeStr(instr, op2);

			char grpLabel[16] = "";
			sprintf(grpLabel, "{grp%d}", (int)instr.multi + 1);
			Replace(text, grpLabel, op2Str);

			instr = m_cpu->GetInfo().GetSubOpcode(instr, op2);

			// Might be a last indirection
			if (instr.multi != Opcode::MULTI::NONE)
			{
				// Sub opcode is rightmost 6 bits
				BYTE op3 = data & 63;
				const std::string op3Str = m_cpu->GetInfo().GetSubOpcodeStr(instr, op3);

				sprintf(grpLabel, "{grp%d}", (int)instr.multi + 1);
				Replace(text, grpLabel, op3Str);

				instr = m_cpu->GetInfo().GetSubOpcode(instr, op3);
			}

			if (instr.alt && instr.altMask.IsMatch(data))
			{
				instr = *instr.alt;
				text = instr.text;
			}
		}
		else
		{
			// Invalid instruction, do nothing
			memset(decoded.text, ' ', Instruction::TEXT_LEN);

			char buf[16];
			sprintf(buf, "DB 0x%04X", data);
			memcpy(decoded.text, buf, 9);
			return address;
		}

		if (instr.overrideMask)
		{
			data = instr.overrideMask.Apply(data);
		}

		char buf[48];

		if (instr.dataReg || instr.addressReg)
		{
			const char* toReplace = instr.dataReg ? "{dr}" : "{ar}";
			buf[0] = instr.dataReg ? 'D' : 'A';
			buf[1] = '0' + (data & 7);
			buf[2] = '\0';
			Replace(text, toReplace, buf);
		}
		else if (instr.regs)
		{
			// EA not decoded yet but we need to know if we're in predecrement mode
			bool predecrement = (CPU68000::GetEAMode(data) == EAMode::AddrRegIndirectPredecrement);

			// Get register bitmask
			WORD regs = m_memory->Read16be(address);
			address += 2;
			decoded.AddRaw(regs);
			Replace(text, "{regs}", DecodeRegisterBitmask(regs, predecrement));
		}
		else if (instr.branch)
		{
			// If displacement fits in 8 bits, value is in lower byte (and op is .b)
			// If not, value is in next word. This is indicated with lower byte == 0.
			// This also means that encoding a zero displacement requires an extra word.

			SBYTE byteDisplacement = GetLByte(data);
			if (byteDisplacement)
			{
				sprintf(buf, ".b %d", byteDisplacement);
			}
			else
			{
				WORD wordDisplacement = m_memory->Read16be(address);
				address += 2;
				decoded.AddRaw(wordDisplacement);
				sprintf(buf, ".w %d", (SWORD)wordDisplacement);
			}
			Replace(text, "{branch}", buf);
		}
		else if (instr.regreg) // SBCD, ABCD, CMPM, EXG
		{
			constexpr const char* ddStr = "D%d,D%d";
			constexpr const char* aaStr = "A%d,A%d";
			constexpr const char* daStr = "D%d,A%d";
			constexpr const char* aIncStr = "(A%d)+,(A%d)+";
			constexpr const char* aDecStr = "-(A%d),-(A%d)";

			// reg/MEM = 0: Data register, 1: Address register
			bool regMEM = GetBit(data, 3);

			BYTE regX = (data >> 9) & 7;
			BYTE regY = data & 7;

			const char* regStr;
			switch (group)
			{
			case 8:  // SBCD: D,D or -(A),-(A)
			case 9:  // SUBX: "              "
			case 13: // ADDX: "              "
				regStr = regMEM ? aDecStr : ddStr;
				break;
			case 11: // CMPM: D,D or (A)+,(A)+
				regStr = aIncStr;
				break;
			case 12: // EXG, ABCD: D,D or A,A (or D,A for EXG)
				if (data & 0b11000000) // EXG
				{
					regStr = GetBit(data, 7) ? daStr : (regMEM ? aaStr : ddStr);
					// Rx, Ry are swapped for EXG for some reason
					emul::Swap(regX, regY);
				}
				else // ABCD
				{
					regStr = regMEM ? aDecStr : ddStr;
				}
				break;
			default:
				throw std::exception("Unknown {r,r} instruction");
			}

			sprintf(buf, regStr, regY, regX);
			Replace(text, "{r,r}", buf);
		}

		// Arithmetic/Logical Shift group
		if (group == 14)
		{
			// Need to decode:
			// 1) The shift type (in bits 4-3)
			// 2) The shift count (data register or immediate)
			//      This set by bit 5. Reg#/imm data is in bits 11-9
			const char* type;
			switch ((data >> 3) & 3)
			{
			case 0: type = "AS"; break;
			case 1: type = "LS"; break;
			case 2: type = "ROX"; break;
			case 3: type = "RO"; break;
			default:
				throw std::exception("not possible");
			}
			Replace(text, "{shift}", type);

			bool isReg = GetBit(data, 5);
			int count = (data >> 9) & 7;
			if (!isReg && (count == 0))
			{
				count = 8;
			}
			buf[0] = isReg ? 'D' : '#';
			buf[1] = '0' + count;
			buf[2] = '\0';

			Replace(text, "{#}", buf);
		}

		switch (instr.imm)
		{
		case Opcode::IMM::W8:
		{
			WORD imm8 = m_memory->Read16be(address);
			address += 2;
			decoded.AddRaw(imm8);
			sprintf(buf, "$%02X", imm8);
			Replace(text, "{i8}", buf);
			break;
		}
		case Opcode::IMM::W8OPCODE:
		{
			BYTE imm8 = GetLByte(data);
			sprintf(buf, "$%02X", imm8);
			Replace(text, "{i8opcode}", buf);
			break;
		}
		case Opcode::IMM::W16:
		{
			WORD imm16 = m_memory->Read16be(address);
			address += 2;
			decoded.AddRaw(imm16);
			sprintf(buf, "$%04X", imm16);
			Replace(text, "{i16}", buf);
			break;
		}
		case Opcode::IMM::W32:
		{
			DWORD imm32 = m_memory->Read32be(address);
			address += 4;
			decoded.AddRaw(imm32);
			sprintf(buf, "$%08X", imm32);
			Replace(text, "{i32}", buf);
			break;
		}
		case Opcode::IMM::S8: // Signed byte (show as decimal)
		{
			WORD imm16 = m_memory->Read16be(address);
			address += 2;
			decoded.AddRaw(imm16);
			sprintf(buf, "%d", (SBYTE)GetLByte(imm16));
			Replace(text, "{s8}", buf);
			break;
		}
		case Opcode::IMM::S16: // Signed word (show as decimal)
		{
			WORD imm16 = m_memory->Read16be(address);
			address += 2;
			decoded.AddRaw(imm16);
			sprintf(buf, "%d", (SWORD)imm16);
			Replace(text, "{s16}", buf);
			break;
		}
		default:
			break;
		}

		// Groups [1-3] have size encoded in group number
		EASize size = EASize::Undef;
		switch (group)
		{
		case 1: size = EASize::Byte; break;
		case 2: size = EASize::Long; break;
		case 3: size = EASize::Word; break;
		default:
			// Nothing to do
			break;
		}

		std::string idxText;
		bool isPredecrement = false;
		if (instr.idx || instr.idxidx) 	// Effective address
		{
			EffectiveAddress ea(*m_memory, decoded, address);
			ea.ComputeEA(data);
			if (size != EASize::Undef)
			{
				ea.SetSize(size);
			}

			// Bit instructions (BTST, etc.) have byte length
			// except for DataRegDirect addressing mode (where it's long)
			if (instr.bit)
			{
				// override size
				ea.SetSize((ea.GetMode() == EAMode::DataRegDirect) ? EASize::Long : EASize::Byte);

				std::string width = (ea.GetMode() == EAMode::DataRegDirect) ? "l" : "b";
				Replace(text, "{bit}", width);
			}

			isPredecrement = (ea.GetMode() == EAMode::AddrRegIndirectPredecrement);

			ea.BuildText();
			idxText = ea.GetText();
			Replace(text, "{idx}", idxText); // Won't replace anything for {idx,idx}
			address = ea.GetCurrAddress();
		}

		// Continue with the destination part
		if (instr.idxidx)
		{
			// Done with the source, replace this part
			std::string& srcIdx = idxText;
			Replace(text, "{idx", idxText);

			// Now for the destination
			EffectiveAddress ea(*m_memory, decoded, address);

			// Need to shuffle the bits
			// (opcode order is: 0|0| size | DestReg|DestMode | SrcMode|SrcReg)
			WORD destMode = (data >> 3) & 0b111000;
			WORD destReg = (data >> 9) & 0b000111;

			ea.ComputeEA(destMode | destReg);
			if (size != EASize::Undef)
			{
				ea.SetSize(size);
			}

			ea.BuildText();
			idxText = ea.GetText();
			Replace(text, "idx}", idxText); // Replace the destination part
			address = ea.GetCurrAddress();
		}

		memset(decoded.text, ' ', Instruction::TEXT_LEN);
		memcpy(decoded.text, text.c_str(), text.size());

		return address;
	}

	std::string Monitor68000::DecodeRegisterBitmask(WORD mask, bool isPredecrement)
	{
		// Register list bit mask :
		//
		//    All other modes: A7 A6 A5 A4 A3 A2 A1 A0 D7 D6 D5 D4 D3 D2 D1 D0
		//  Predecrement mode: D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6 A7
		if (isPredecrement)
		{
			mask = ReverseBits(mask);
		}

		std::ostringstream os;
		os << DecodeRegisters(GetLByte(mask), 'D');

		// Add separator only if there's something in the address block
		if (os.tellp() && (mask & 0xFF00))
		{
			os << '/';
		}
		os << DecodeRegisters(GetHByte(mask), 'A');

		if (!os.tellp())
		{
			os << "[]";
		}

		return os.str();
	}

	std::string Monitor68000::DecodeRegisters(BYTE mask, char prefix)
	{
		std::ostringstream os;
		bool first = true;
		bool inRange = false;
		int rangeCount = 0;
		int lastIndex = 0;
		for (int index = 0; mask; mask >>= 1, ++index)
		{
			if (GetLSB(mask))
			{
				if (!inRange)
				{
					if (!first)
					{
						os << '/';
					}
					else
					{
						first = false;
					}

					os << prefix << index;
					inRange = true;
					rangeCount = 1;
				}
				else
				{
					++rangeCount;
					lastIndex = index;
				}
			}
			else if (inRange)
			{
				inRange = false;
				if (rangeCount > 1)
				{
					os << '-' << prefix << lastIndex;
				}
				lastIndex = index;
			}
		}

		if (inRange)
		{
			if (rangeCount > 1)
			{
				os << '-' << prefix << lastIndex;
			}
		}

		return os.str();
	}


	void Monitor68000::Instruction::AddRaw(BYTE b)
	{
		raw[rawLen++] = hexDigits[b >> 4];
		raw[rawLen++] = hexDigits[(b & 0x0F)];
	}
	void Monitor68000::Instruction::AddRaw(WORD w)
	{
		// HIGH BYTE
		raw[rawLen++] = hexDigits[(w >> 12) & 0x0F];
		raw[rawLen++] = hexDigits[(w >> 8) & 0x0F];
		// LOW BYTE
		raw[rawLen++] = hexDigits[(w >> 4) & 0x0F];
		raw[rawLen++] = hexDigits[(w & 0x0F)];
	}
	void Monitor68000::Instruction::AddRaw(DWORD dw)
	{
		// HIGH WORD
		AddRaw(GetHWord(dw));
		// LOW WORD
		AddRaw(GetLWord(dw));
	}
}
