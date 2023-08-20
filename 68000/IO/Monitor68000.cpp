#include "stdafx.h"
#include "Monitor68000.h"
#include "CPU/CPU68000.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;
using emul::GetBit;

namespace emul
{
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
		case EAMode::AddrRegIndirectPostIncrement:
			sprintf(m_text, "(A%u)+", m_regNumber);
			break;
		case EAMode::AddrRegIndirectPreDecrement:
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

		if ((opcode & 0b111000) != 0b111000)
		{
			// Normal modes (Mode != 0b111)
			m_mode = EAMode(opcode & 0b111000);
		}
		else switch (m_regNumber) // Special modes
		{
		case 0b000:
		{
			m_mode = EAMode::AbsoluteShort;
			WORD shortAddress = m_memory.Read16be(m_currAddress);
			m_currAddress += 2;
			m_address = emul::Widen(shortAddress);
			m_currInstruction.AddRaw(shortAddress);
			break;
		}
		case 0b001:
		{
			m_mode = EAMode::AbsoluteLong;
			m_address = m_memory.Read32be(m_currAddress);
			m_currAddress += 4;
			m_currInstruction.AddRaw(m_address);
			break;
		}
		case 0b010: m_mode = EAMode::ProgramCounterDisplacement; break;
		case 0b011: m_mode = EAMode::ProgramCounterIndex; break;
		case 0b100: m_mode = EAMode::Immediate; break;
		default: m_mode = EAMode::Invalid;
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
			case 63: // F5
				ToggleRunMode();
				break;

			case 62: // F4
				ToggleRAMMode();
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
			case 61: // F3
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

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
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
		if (m_runMode == RUNMode::RUN) m_runMode = RUNMode::STEP;
		else m_runMode = RUNMode::RUN;

		UpdateRunMode();
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
		case RAMMode::ZP: m_ramMode = RAMMode::SP; break;
		case RAMMode::SP: m_ramMode = RAMMode::PC; break;
		case RAMMode::PC: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::ZP; break;
		}

		UpdateRAMMode();
	}

	void Monitor68000::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramZP = m_cpu->GetInfo().GetCoord("ram.ZP");
		static Coord ramPC = m_cpu->GetInfo().GetCoord("ram.PC");
		static Coord ramSTACK = m_cpu->GetInfo().GetCoord("ram.SP");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramZP.x, ramZP.y, (m_ramMode == RAMMode::ZP) ? highlight : regular, ramZP.w);
		m_console.WriteAttrAt(ramPC.x, ramPC.y, (m_ramMode == RAMMode::PC) ? highlight : regular, ramPC.w);
		m_console.WriteAttrAt(ramSTACK.x, ramSTACK.y, (m_ramMode == RAMMode::SP) ? highlight : regular, ramSTACK.w);

		{
			static char buf[32];
			sprintf(buf, " CUSTOM:%04X", m_customMemView);
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
		WriteValueHex(m_cpu->m_reg.ADDR[7], m_cpu->GetInfo().GetCoord("A7"));

		WriteValueHex(m_cpu->m_reg.DATA[0], m_cpu->GetInfo().GetCoord("D0"));
		WriteValueHex(m_cpu->m_reg.DATA[1], m_cpu->GetInfo().GetCoord("D1"));
		WriteValueHex(m_cpu->m_reg.DATA[2], m_cpu->GetInfo().GetCoord("D2"));
		WriteValueHex(m_cpu->m_reg.DATA[3], m_cpu->GetInfo().GetCoord("D3"));
		WriteValueHex(m_cpu->m_reg.DATA[4], m_cpu->GetInfo().GetCoord("D4"));
		WriteValueHex(m_cpu->m_reg.DATA[5], m_cpu->GetInfo().GetCoord("D5"));
		WriteValueHex(m_cpu->m_reg.DATA[6], m_cpu->GetInfo().GetCoord("D6"));
		WriteValueHex(m_cpu->m_reg.DATA[7], m_cpu->GetInfo().GetCoord("D7"));


		WriteValueHex(m_cpu->m_reg.USP, m_cpu->GetInfo().GetCoord("USP"));
		WriteValueHex(m_cpu->m_reg.SSP, m_cpu->GetInfo().GetCoord("SSP"));

		WriteValueHex((WORD)m_cpu->GetCurrentAddress(), m_cpu->GetInfo().GetCoord("PC"));
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
			attr[width-i-1] = GetBit(m_cpu->m_reg.flags, i) ? 15 : 8;
		}

		m_console.WriteAttrAt(coord.x, coord.y, attr, width);
	}

	ADDRESS Monitor68000::GetRAMBase() const
	{
		switch (m_ramMode)
		{
		case RAMMode::ZP:
			return 0;
		case RAMMode::SP:
			return  m_cpu->GetSP();
		case RAMMode::PC:
			return  m_cpu->GetCurrentAddress();
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
		static int bytesPerLine = charPos.w;
		static int bytesTotal = charPos.w * charPos.h;

		// Adjust position so the view doesn't move around too much
		WORD offset = GetRAMBase();

		WORD adjustedOffset = (offset / bytesPerLine) * bytesPerLine;

		// Check for end of segment
		if (((DWORD)adjustedOffset + bytesTotal) >= 0x10000)
		{
			adjustedOffset = 0x10000 - bytesTotal;
		}
		else if (adjustedOffset >= bytesPerLine)
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
			WriteValueHex((WORD)(adjustedOffset + (bytesPerLine * y)), pos);

			for (int x = 0; x < bytesPerLine; ++x)
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
		static Coord offsetPos = m_cpu->GetInfo().GetCoord("CODE.offset");
		static Coord rawPos = m_cpu->GetInfo().GetCoord("CODE.raw");
		static Coord textPos = m_cpu->GetInfo().GetCoord("CODE.text");
		static short baseY = offsetPos.y;

		// TODO: Horribly inefficient
		Coord pos;
		pos.y = baseY + y;

		pos.x = offsetPos.x;
		WriteValueHex((WORD)instr.address, pos);
		pos.x = rawPos.x;
		m_console.WriteAt(pos.x, pos.y, (const char*)instr.raw, instr.rawLen);
		for (int i = 0; i < rawPos.w - instr.rawLen; ++i)
		{
			m_console.WriteAt(pos.x + instr.rawLen + i, pos.y, 0xFAu, 8);
		}

		pos.x = textPos.x;
		m_console.WriteAt(pos.x, pos.y, instr.text, textPos.w);
	}

	void Monitor68000::UpdateCode()
	{
		static Coord codePos = m_cpu->GetInfo().GetCoord("CODE");

		ADDRESS address = m_cpu->GetCurrentAddress();

		m_console.MoveBlockY(codePos.x, codePos.y, codePos.w - 1, 4, codePos.y - 1);

		for (int i = 0; i < 8; ++i)
		{
			Instruction decoded;
			address = Disassemble(address, decoded);
			PrintInstruction(i+4, decoded);
		}
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
		BYTE group = (data >> 12) & 3;

		Opcode instr = m_cpu->GetInfo().GetOpcode(group);
		std::string text = instr.text;

		if (instr.multi != Opcode::MULTI::NONE)
		{
			// Sub opcode is next 6 bits
			// TODO depends on group
			BYTE op2 = (data >> 6) & 63;
			const std::string op2Str = m_cpu->GetInfo().GetSubOpcodeStr(instr, op2);

			char grpLabel[16] = "";
			sprintf(grpLabel, "{grp%d}", (int)instr.multi + 1);
			Replace(text, grpLabel, op2Str);

			instr = m_cpu->GetInfo().GetSubOpcode(instr, op2);

			if (instr.alt && instr.altMask.IsMatch(data))
			{
				instr = *instr.alt;
				text = instr.text;
			}
		}
		else
		{
			// Should not happen
			throw std::exception("Top level instruction should be multi");
		}

		if (instr.overrideMask)
		{
			data = instr.overrideMask.Apply(data);
		}

		char buf[48];
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
