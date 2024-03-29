#include "stdafx.h"
#include "Monitor6809.h"
#include "CPU/CPU6809.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;
using emul::GetBit;

namespace emul
{
	static const char hexDigits[] = "0123456789ABCDEF";

	const char* GetRegister(BYTE r)
	{
		switch (r)
		{
		case 0b0000: return "D";
		case 0b0001: return "X";
		case 0b0010: return "Y";
		case 0b0011: return "U";
		case 0b0100: return "S";
		case 0b0101: return "PC";

		case 0b1000: return "A";
		case 0b1001: return "B";
		case 0b1010: return "CC";
		case 0b1011: return "DP";

		default: return "invalid";
		}
	}

	Monitor6809::Monitor6809(Console& console) :
		m_console(console)
	{
	}

	void Monitor6809::Init(CPU* cpu, Memory& memory)
	{
		m_cpu = dynamic_cast<CPU6809*>(cpu);
		m_memory = &memory;
	}

	MonitorState Monitor6809::ProcessKey()
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

	MonitorState Monitor6809::Run()
	{
		Update();
		if (m_runMode == RUNMode::STEP)
		{
			m_console.WaitForKey();
		}

		return ProcessKey();
	}

	void Monitor6809::Show()
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

	void Monitor6809::WriteValueNibble(BYTE value, const cpuInfo::Coord& coord, WORD attr)
	{
		static char hex[1];
		hex[0] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 1, attr);
	}

	void Monitor6809::WriteValueHex(BYTE value, const Coord& coord, WORD attr)
	{
		static char hex[2];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 2, attr);
	}

	void Monitor6809::WriteValueHex(WORD value, const Coord& coord, WORD attr)
	{
		static char hex[4];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
	}

	void Monitor6809::Update()
	{
		UpdateRegisters();
		UpdateTicks();
		UpdateFlags();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor6809::UpdateCPUType()
	{
		static Coord cpuId = m_cpu->GetInfo().GetCoord("CPUID");
		std::ostringstream os;
		os << std::setw(cpuId.w) << m_cpu->GetInfo().GetId();
		m_console.WriteAt(cpuId.x, cpuId.y, os.str().c_str());
	}

	void Monitor6809::ToggleRunMode()
	{
		if (m_runMode == RUNMode::RUN) m_runMode = RUNMode::STEP;
		else m_runMode = RUNMode::RUN;

		UpdateRunMode();
	}

	void Monitor6809::UpdateRunMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (1 << 4) | 14;
		static Coord run = m_cpu->GetInfo().GetCoord("status.RUN");
		static Coord stop = m_cpu->GetInfo().GetCoord("status.STOP");

		m_console.WriteAttrAt(run.x, run.y, (m_runMode == RUNMode::RUN) ? highlight : regular, run.w);
		m_console.WriteAttrAt(stop.x, stop.y, (m_runMode == RUNMode::STEP) ? highlight : regular, stop.w);
	}

	void Monitor6809::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::DP: m_ramMode = RAMMode::X; break;
		case RAMMode::X: m_ramMode = RAMMode::Y; break;
		case RAMMode::Y: m_ramMode = RAMMode::SP; break;
		case RAMMode::SP: m_ramMode = RAMMode::USP; break;
		case RAMMode::USP: m_ramMode = RAMMode::PC; break;
		case RAMMode::PC: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::DP; break;
		}

		UpdateRAMMode();
	}

	void Monitor6809::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramDP = m_cpu->GetInfo().GetCoord("ram.DP");
		static Coord ramX = m_cpu->GetInfo().GetCoord("ram.X");
		static Coord ramY = m_cpu->GetInfo().GetCoord("ram.Y");
		static Coord ramSP = m_cpu->GetInfo().GetCoord("ram.SP");
		static Coord ramUSP = m_cpu->GetInfo().GetCoord("ram.USP");
		static Coord ramPC = m_cpu->GetInfo().GetCoord("ram.PC");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramDP.x, ramDP.y, (m_ramMode == RAMMode::DP) ? highlight : regular, ramDP.w);
		m_console.WriteAttrAt(ramX.x, ramX.y, (m_ramMode == RAMMode::X) ? highlight : regular, ramX.w);
		m_console.WriteAttrAt(ramY.x, ramY.y, (m_ramMode == RAMMode::Y) ? highlight : regular, ramY.w);
		m_console.WriteAttrAt(ramSP.x, ramSP.y, (m_ramMode == RAMMode::SP) ? highlight : regular, ramSP.w);
		m_console.WriteAttrAt(ramUSP.x, ramUSP.y, (m_ramMode == RAMMode::USP) ? highlight : regular, ramUSP.w);
		m_console.WriteAttrAt(ramPC.x, ramPC.y, (m_ramMode == RAMMode::PC) ? highlight : regular, ramPC.w);

		{
			static char buf[32];
			sprintf(buf, " CUSTOM:%04X", m_customMemView);
			m_console.WriteAt(ramCustom.x, ramCustom.y, buf);
			m_console.WriteAttrAt(ramCustom.x, ramCustom.y, (m_ramMode == RAMMode::CUSTOM) ? highlight : regular, ramCustom.w);
		}

		UpdateRAM();
	}

	void Monitor6809::UpdateRegisters()
	{
		WriteValueHex(m_cpu->m_reg.D, m_cpu->GetInfo().GetCoord("D"));
		WriteValueHex(m_cpu->m_reg.X, m_cpu->GetInfo().GetCoord("X"));
		WriteValueHex(m_cpu->m_reg.Y, m_cpu->GetInfo().GetCoord("Y"));
		WriteValueHex(m_cpu->m_reg.S, m_cpu->GetInfo().GetCoord("S"));
		WriteValueHex(m_cpu->m_reg.U, m_cpu->GetInfo().GetCoord("U"));
		WriteValueHex(m_cpu->m_reg.DP, m_cpu->GetInfo().GetCoord("DP"));

		WriteValueHex((WORD)m_cpu->GetCurrentAddress(), m_cpu->GetInfo().GetCoord("PC"));
	}

	void Monitor6809::UpdateTicks()
	{
		uint32_t ticks = m_cpu->GetInstructionTicks();
		static char buf[5];
		sprintf(buf, "%4d", (BYTE)ticks);

		static Coord coord = m_cpu->GetInfo().GetCoord("TICKS");

		m_console.WriteAt(coord.x, coord.y, buf, 4);
	}

	void Monitor6809::UpdateFlags()
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

	ADDRESS Monitor6809::GetRAMBase() const
	{
		switch (m_ramMode)
		{
		case RAMMode::DP:
			return (m_cpu->m_reg.DP << 8);
		case RAMMode::X:
			return  m_cpu->m_reg.X;
		case RAMMode::Y:
			return  m_cpu->m_reg.Y;
		case RAMMode::SP:
			return  m_cpu->m_reg.S;
		case RAMMode::USP:
			return  m_cpu->m_reg.U;
		case RAMMode::PC:
			return  m_cpu->GetCurrentAddress();
		case RAMMode::CUSTOM:
		default:
			return m_customMemView;
		}
	}

	void Monitor6809::UpdateRAM()
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

	void Monitor6809::PrintInstruction(short y, Instruction& instr)
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
		m_console.WriteAt(pos.x, pos.y, (const char*)instr.raw, instr.len);
		for (int i = 0; i < rawPos.w - instr.len; ++i)
		{
			m_console.WriteAt(pos.x + instr.len + i, pos.y, 0xFAu, 8);
		}

		pos.x = textPos.x;
		m_console.WriteAt(pos.x, pos.y, instr.text, textPos.w);
	}

	void Monitor6809::UpdateCode()
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

	bool Monitor6809::Replace(std::string& str, const std::string& from, const std::string& to)
	{
		size_t start = str.find(from);
		if (start == std::string::npos)
		{
			return false;
		}
		str.replace(start, from.size(), to);
		return true;
	}

	char GetIndexedRegister(BYTE idx)
	{
		switch ((idx >> 5) & 3)
		{
		case 0: return 'X';
		case 1: return 'Y';
		case 2: return 'U';
		case 3: return 'S';
		default:
			throw std::exception("not possible");
		}
	}

	void Monitor6809::DecodeIndexedInstruction(Opcode& opcode, BYTE idx)
	{
		char reg = GetIndexedRegister(idx);

		bool indirect = GetBit(idx, 7) && GetBit(idx, 4);

		std::ostringstream os;
		if (indirect)
		{
			os << '[';
		}

		if (!GetMSB(idx)) //,R + 5 bit offset
		{
			bool signBit = GetBit(idx, 4);
			BYTE offset = idx;
			SetBitMask(offset, 0b11100000, signBit);

			os << static_cast<int16_t>((SBYTE)offset) << ',' << reg;
		}
		else switch (idx & 0b1111)
		{
		// Constant Offset from R (twos complement offset)
		case 0b0100: os << ',' << reg; break; // ,R + 0 Offset
		case 0b1000: os << "{s8}," << reg; opcode.imm = Opcode::IMM::S8; break; // ,R + 8 bit offset
		case 0b1001: os << "{s16}," << reg; opcode.imm = Opcode::IMM::S16; break; // ,R + 16 bit offset

		// Accumulator Offset from R (twos complement offset)
		case 0b0101: os << "B," << reg; break; // ,R + B Offset
		case 0b0110: os << "A," << reg; break; // ,R + A Offset
		case 0b1011: os << "D," << reg; break; // ,R + D offset

		// Auto Increment/Decrement of R
		case 0b0000: os << ',' << reg << '+'; break; // ,R+
		case 0b0001: os << ',' << reg << "++"; break; // ,R++
		case 0b0010: os << ",-" << reg; break; // ,-R
		case 0b0011: os << ",--" << reg; break; // ,--R

		// Constant Offset from PC (twos complement offset)
		case 0b1100: os << "{s8},PCR"; opcode.imm = Opcode::IMM::S8; break; // ,PC + 8 bit offset
		case 0b1101: os << "{s16},PCR"; opcode.imm = Opcode::IMM::S16; break; // ,PC + 16 bit offset

		// Extended Indirect
		case 0b1111: os << ",{i16}"; opcode.imm = Opcode::IMM::W16; break; // [,Address]

		// 0b0111: n/a
		// 0b1010: n/a
		// 0b1110: n/a
		default:
			throw std::exception("Invalid addressing mode");
		}

		if (indirect)
		{
			os << ']';
		}

		Replace(opcode.text, "{idx}", os.str());
	}

	void Monitor6809::DecodeStackRegs(char* buf, BYTE regs, bool isU)
	{
		buf[0] = '\0';

		if (GetBit(regs, 7)) strcat(buf, "PC,");
		if (GetBit(regs, 6)) strcat(buf, isU ? "S," : "U,");
		if (GetBit(regs, 5)) strcat(buf, "Y,");
		if (GetBit(regs, 4)) strcat(buf, "X,");
		if (GetBit(regs, 3)) strcat(buf, "SP,");
		if (GetBit(regs, 2)) strcat(buf, "B,");
		if (GetBit(regs, 1)) strcat(buf, "A,");
		if (GetBit(regs, 0)) strcat(buf, "CC,");

		size_t len = strlen(buf);
		if (!len)
		{
			strcat(buf, "0");
		}
		else
		{
			buf[strlen(buf) - 1] = '\0';
		}
	}

	ADDRESS Monitor6809::Disassemble(ADDRESS address, Monitor6809::Instruction& decoded)
	{
		decoded.address = address;
		BYTE data = m_memory->Read8(address);

		decoded.AddRaw(data);

		Opcode instr = m_cpu->GetInfo().GetOpcode(data);
		std::string text = instr.text;

		if (instr.multi != Opcode::MULTI::NONE)
		{
			data = m_memory->Read8(++address);
			decoded.AddRaw(data);

			std::string grpText = instr.text;
			instr = m_cpu->GetInfo().GetSubOpcode(instr, data);
			Replace(text, grpText, instr.text);
		}

		if (instr.idx)
		{
			BYTE idx = m_memory->Read8(++address);
			decoded.AddRaw(idx);

			DecodeIndexedInstruction(instr, idx);
			text = instr.text;
		}

		char buf[32];
		switch (instr.imm)
		{
		case Opcode::IMM::W8:
		{
			BYTE imm8 = m_memory->Read8(++address);
			decoded.AddRaw(imm8);

			// look for push/pull opcodes and pretty them up
			if ((data & 0xFC) == 0x34)
			{
				DecodeStackRegs(buf, imm8, GetBit(data, 1));
			}
			else
			{
				sprintf(buf, "$%02X", imm8);
			}
			Replace(text, "{i8}", buf);

			break;
		}
		case Opcode::IMM::W16:
		{
			WORD imm16 = m_memory->Read16be(++address);
			++address;
			decoded.AddRaw(imm16);

			sprintf(buf, "$%04X", imm16);
			Replace(text, "{i16}", buf);
			break;
		}
		case Opcode::IMM::S8: // Signed byte (show as decimal)
		{
			BYTE imm8 = m_memory->Read8(++address);
			decoded.AddRaw(imm8);
			sprintf(buf, "%d", (SBYTE)imm8);
			Replace(text, "{s8}", buf);
			break;
		}
		case Opcode::IMM::S16: // Signed word (show as decimal)
		{
			WORD imm16 = m_memory->Read16be(++address);
			++address;
			decoded.AddRaw(imm16);
			sprintf(buf, "%d", (SWORD)imm16);
			Replace(text, "{s16}", buf);
			break;
		}
		case Opcode::IMM::REGREG:
		{
			BYTE imm8 = m_memory->Read8(++address);
			decoded.AddRaw(imm8);

			const char* r0 = GetRegister(GetLNibble(imm8));
			const char* r1 = GetRegister(GetHNibble(imm8));

			sprintf(buf, "%s,%s", r1, r0);
			Replace(text, "{r,r}", buf);
			break;
		}
		default:
			break;
		}

		memset(decoded.text, ' ', 32);
		memcpy(decoded.text, text.c_str(), text.size());

		++address;

		return address;
	}

	void Monitor6809::Instruction::AddRaw(BYTE b)
	{
		this->raw[this->len++] = hexDigits[b >> 4];
		this->raw[this->len++] = hexDigits[(b & 0x0F)];
	}
	void Monitor6809::Instruction::AddRaw(WORD w)
	{
		// HIGH BYTE
		this->raw[this->len++] = hexDigits[(w >> 12) & 0x0F];
		this->raw[this->len++] = hexDigits[(w >> 8) & 0x0F];
		// LOW BYTE
		this->raw[this->len++] = hexDigits[(w >> 4) & 0x0F];
		this->raw[this->len++] = hexDigits[(w & 0x0F)];
	}
}
