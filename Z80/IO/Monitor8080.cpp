#include "stdafx.h"
#include "Monitor8080.h"
#include "CPU/CPU8080.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;
using emul::GetBit;

namespace emul
{
	static const char hexDigits[] = "0123456789ABCDEF";

	Monitor8080::Monitor8080(Console& console) :
		m_console(console)
	{
	}

	void Monitor8080::Init(CPU8080& cpu, Memory& memory)
	{
		m_cpu = &cpu;
		m_memory = &memory;
	}

	MonitorState Monitor8080::ProcessKey()
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

	MonitorState Monitor8080::Run()
	{
		Update();
		if (m_runMode == RUNMode::STEP)
		{
			m_console.WaitForKey();
		}

		return ProcessKey();
	}

	void Monitor8080::Show()
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

	void Monitor8080::WriteValueHex(BYTE value, const Coord& coord, WORD attr)
	{
		static char hex[2];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 2, attr);
	}

	void Monitor8080::WriteValueHex(WORD value, const Coord& coord, WORD attr)
	{
		static char hex[4];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
	}

	void Monitor8080::Update()
	{
		UpdateRegisters();
		UpdateTicks();
		UpdateFlags();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor8080::UpdateCPUType()
	{
		static Coord cpuId = m_cpu->GetInfo().GetCoord("CPUID");
		std::ostringstream os;
		os << std::setw(cpuId.w) << m_cpu->GetInfo().GetName();
		m_console.WriteAt(cpuId.x, cpuId.y, os.str().c_str());
	}

	void Monitor8080::ToggleRunMode()
	{
		if (m_runMode == RUNMode::RUN) m_runMode = RUNMode::STEP;
		else m_runMode = RUNMode::RUN;

		UpdateRunMode();
	}

	void Monitor8080::UpdateRunMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (1 << 4) | 14;
		static Coord run = m_cpu->GetInfo().GetCoord("status.RUN");
		static Coord stop = m_cpu->GetInfo().GetCoord("status.STOP");

		m_console.WriteAttrAt(run.x, run.y, (m_runMode == RUNMode::RUN) ? highlight : regular, run.w);
		m_console.WriteAttrAt(stop.x, stop.y, (m_runMode == RUNMode::STEP) ? highlight : regular, stop.w);
	}

	void Monitor8080::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::HL: m_ramMode = RAMMode::SP; break;
		case RAMMode::SP: m_ramMode = RAMMode::PC; break;
		case RAMMode::PC: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::HL; break;
		}

		UpdateRAMMode();
	}

	void Monitor8080::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramHL = m_cpu->GetInfo().GetCoord("ram.HL");
		static Coord ramPC = m_cpu->GetInfo().GetCoord("ram.PC");
		static Coord ramSTACK = m_cpu->GetInfo().GetCoord("ram.SP");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramHL.x, ramHL.y, (m_ramMode == RAMMode::HL) ? highlight : regular, ramHL.w);
		m_console.WriteAttrAt(ramPC.x, ramPC.y, (m_ramMode == RAMMode::PC) ? highlight : regular, ramHL.w);
		m_console.WriteAttrAt(ramSTACK.x, ramSTACK.y, (m_ramMode == RAMMode::SP) ? highlight : regular, ramSTACK.w);
		
		{
			static char buf[32];
			sprintf(buf, " CUSTOM:%04X", m_customMemView);
			m_console.WriteAt(ramCustom.x, ramCustom.y, buf);
			m_console.WriteAttrAt(ramCustom.x, ramCustom.y, (m_ramMode == RAMMode::CUSTOM) ? highlight : regular, ramCustom.w);
		}

		UpdateRAM();
	}

	void Monitor8080::UpdateRegisters()
	{
		WriteValueHex(m_cpu->regA, m_cpu->GetInfo().GetCoord("A"));
		WriteValueHex(m_cpu->regB, m_cpu->GetInfo().GetCoord("B"));
		WriteValueHex(m_cpu->regC, m_cpu->GetInfo().GetCoord("C"));
		WriteValueHex(m_cpu->regD, m_cpu->GetInfo().GetCoord("D"));
		WriteValueHex(m_cpu->regE, m_cpu->GetInfo().GetCoord("E"));

		WriteValueHex(m_cpu->regH, m_cpu->GetInfo().GetCoord("H"));
		WriteValueHex(m_cpu->regL, m_cpu->GetInfo().GetCoord("L"));

		WriteValueHex(m_cpu->flags, m_cpu->GetInfo().GetCoord("F"));
			
		WriteValueHex((WORD)m_cpu->regSP, m_cpu->GetInfo().GetCoord("SP"));
		WriteValueHex((WORD)m_cpu->GetCurrentAddress(), m_cpu->GetInfo().GetCoord("PC"));
	}

	void Monitor8080::UpdateTicks()
	{
		uint32_t ticks = m_cpu->GetInstructionTicks();
		static char buf[5];
		sprintf(buf, "%4d", (BYTE)ticks);

		static Coord coord = m_cpu->GetInfo().GetCoord("TICKS");

		m_console.WriteAt(coord.x, coord.y, buf, 4);
	}

	void Monitor8080::UpdateFlags()
	{
		static Coord coord = m_cpu->GetInfo().GetCoord("FLAGS");
		int width = coord.w;

		static WORD attr[16];

		for (int i = 0; i < width; ++i)
		{
			attr[width-i-1] = GetBit(m_cpu->flags, i) ? 15 : 8;
		}

		m_console.WriteAttrAt(coord.x, coord.y, attr, width);
	}

	void Monitor8080::UpdateRAM()
	{
		static Coord addrPos = m_cpu->GetInfo().GetCoord("ram.ADDR");
		static Coord hexPos = m_cpu->GetInfo().GetCoord("ram.HEX");
		static Coord charPos = m_cpu->GetInfo().GetCoord("ram.CHAR");
		static int bytesPerLine = charPos.w;
		static int bytesTotal = charPos.w * charPos.h;

		// Adjust position so the view doesn't move around too much
		WORD offset = 0;

		switch (m_ramMode)
		{
		case RAMMode::HL:
			offset = m_cpu->getHL();
			break;
		case RAMMode::SP:
			offset = m_cpu->regSP;
			break;
		case RAMMode::PC:
			offset = m_cpu->GetCurrentAddress();
			break;
		case RAMMode::CUSTOM:
		default:
			offset = m_customMemView;
			break;
		}

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

	void Monitor8080::PrintInstruction(short y, Instruction& instr)
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

	void Monitor8080::UpdateCode()
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

	bool replace(std::string& str, const std::string& from, const std::string& to) 
	{
		size_t start = str.find(from);
		if (start == std::string::npos)
		{
			return false;
		}
		str.replace(start, from.size(), to);
		return true;
	}

	ADDRESS Monitor8080::Disassemble(ADDRESS address, Monitor8080::Instruction& decoded)
	{
		decoded.address = address;
		BYTE data = m_memory->Read8(address);

		decoded.AddRaw(data);

		Opcode instr = m_cpu->GetInfo().GetOpcode(data);
		std::string text = instr.text;

		char buf[32];
		switch (instr.imm)
		{
		case Opcode::IMM::W8:
		{
			BYTE imm8 = m_memory->Read8(++address);
			decoded.AddRaw(imm8);

			sprintf(buf, "0%Xh", imm8);
			replace(text, "{i8}", buf);
			break;
		}
		case Opcode::IMM::W16:
		{
			WORD imm16 = m_memory->Read16(++address);
			++address;
			decoded.AddRaw(imm16);

			sprintf(buf, "0%Xh", imm16);
			replace(text, "{i16}", buf);
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

	void Monitor8080::Instruction::AddRaw(BYTE b)
	{
		this->raw[this->len++] = hexDigits[b >> 4];
		this->raw[this->len++] = hexDigits[(b & 0x0F)];
	}
	void Monitor8080::Instruction::AddRaw(WORD w)
	{
		// LOW BYTE
		this->raw[this->len++] = hexDigits[(w >> 4) & 0x0F];
		this->raw[this->len++] = hexDigits[(w & 0x0F)];
		// HIGH BYTE
		this->raw[this->len++] = hexDigits[(w >> 12) & 0x0F];
		this->raw[this->len++] = hexDigits[(w >> 8) & 0x0F];
	}
}
