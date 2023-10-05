#include "stdafx.h"
#include "Monitor6800.h"
#include "CPU/CPU6800.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;
using emul::GetBit;

namespace emul
{
	static const char hexDigits[] = "0123456789ABCDEF";

	Monitor6800::Monitor6800(Console& console) :
		m_console(console)
	{
	}

	void Monitor6800::Init(CPU* cpu, Memory& memory)
	{
		m_cpu = dynamic_cast<CPU6800*>(cpu);
		m_memory = &memory;
	}

	MonitorState Monitor6800::ProcessKey()
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

			case 91: // SHIFT-F8
			{
				Instruction decoded;
				// Set breakpoint at next instruction
				ADDRESS next = Disassemble(m_cpu->GetCurrentAddress(), decoded);
				m_breakpointEnabled = true;
				m_breakpoint = next;
				m_runMode = RUNMode::RUN;
				break;
			}
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
				break;
			case 67: // F9
				if (IsBreakpoint())
				{
					m_breakpointEnabled = false;
				}
				else
				{
					m_breakpointEnabled = true;
					m_breakpoint = m_cpu->GetCurrentAddress();
				}
				UpdateCode();
				break;
			case 68: // F10
			default:
				break;
			}
		}
		return (m_runMode == RUNMode::STEP) ? MonitorState::WAIT : MonitorState::RUN;
	}

	MonitorState Monitor6800::Run()
	{
		Update();
		if (m_runMode == RUNMode::STEP)
		{
			m_console.WaitForKey();
		}

		return ProcessKey();
	}

	void Monitor6800::Show()
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

	void Monitor6800::WriteValueNibble(BYTE value, const cpuInfo::Coord& coord, WORD attr)
	{
		static char hex[1];
		hex[0] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 1, attr);
	}

	void Monitor6800::WriteValueHex(BYTE value, const Coord& coord, WORD attr)
	{
		static char hex[2];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 2, attr);
	}

	void Monitor6800::WriteValueHex(WORD value, const Coord& coord, WORD attr)
	{
		static char hex[4];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
	}

	void Monitor6800::Update()
	{
		UpdateRegisters();
		UpdateTicks();
		UpdateFlags();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor6800::UpdateCPUType()
	{
		static Coord cpuId = m_cpu->GetInfo().GetCoord("CPUID");
		std::ostringstream os;
		os << std::setw(cpuId.w) << m_cpu->GetInfo().GetId();
		m_console.WriteAt(cpuId.x, cpuId.y, os.str().c_str());
	}

	void Monitor6800::ToggleRunMode()
	{
		m_runMode = (m_runMode == RUNMode::RUN) ? RUNMode::STEP : RUNMode::RUN;
		UpdateRunMode();

	}

	void Monitor6800::UpdateRunMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (1 << 4) | 14;
		static Coord run = m_cpu->GetInfo().GetCoord("status.RUN");
		static Coord stop = m_cpu->GetInfo().GetCoord("status.STOP");

		m_console.WriteAttrAt(run.x, run.y, (m_runMode == RUNMode::RUN) ? highlight : regular, run.w);
		m_console.WriteAttrAt(stop.x, stop.y, (m_runMode == RUNMode::STEP) ? highlight : regular, stop.w);
	}

	void Monitor6800::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::ZP: m_ramMode = RAMMode::IX; break;
		case RAMMode::IX: m_ramMode = RAMMode::SP; break;
		case RAMMode::SP: m_ramMode = RAMMode::PC; break;
		case RAMMode::PC: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::ZP; break;
		}

		UpdateRAMMode();
	}

	void Monitor6800::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramZP = m_cpu->GetInfo().GetCoord("ram.ZP");
		static Coord ramIX = m_cpu->GetInfo().GetCoord("ram.IX");
		static Coord ramPC = m_cpu->GetInfo().GetCoord("ram.PC");
		static Coord ramSTACK = m_cpu->GetInfo().GetCoord("ram.SP");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramZP.x, ramZP.y, (m_ramMode == RAMMode::ZP) ? highlight : regular, ramZP.w);
		m_console.WriteAttrAt(ramIX.x, ramIX.y, (m_ramMode == RAMMode::IX) ? highlight : regular, ramIX.w);
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

	void Monitor6800::UpdateRegisters()
	{
		static Coord regA = m_cpu->GetInfo().GetCoord("A");
		static Coord regB = m_cpu->GetInfo().GetCoord("B");
		static Coord regIX = m_cpu->GetInfo().GetCoord("IX");
		static Coord regPC = m_cpu->GetInfo().GetCoord("PC");
		static Coord regSP = m_cpu->GetInfo().GetCoord("SP");

		WriteValueHex(m_cpu->m_reg.A, regA);
		WriteValueHex(m_cpu->m_reg.B, regB);
		WriteValueHex(m_cpu->m_reg.IX, regIX);

		WriteValueHex((WORD)m_cpu->m_reg.SP, regSP);
		WriteValueHex((WORD)m_cpu->GetCurrentAddress(), regPC);
	}

	void Monitor6800::UpdateTicks()
	{
		uint32_t ticks = m_cpu->GetInstructionTicks();
		static char buf[5];
		sprintf(buf, "%4d", (BYTE)ticks);

		static Coord coord = m_cpu->GetInfo().GetCoord("TICKS");

		m_console.WriteAt(coord.x, coord.y, buf, 4);
	}

	void Monitor6800::UpdateFlags()
	{
		static Coord coord = m_cpu->GetInfo().GetCoord("FLAGS");
		int width = coord.w;

		static WORD attr[8];

		for (int i = 0; i < width; ++i)
		{
			attr[width-i-1] = GetBit(m_cpu->m_reg.flags, i) ? 15 : 8;
		}

		m_console.WriteAttrAt(coord.x, coord.y, attr, width);
	}

	ADDRESS Monitor6800::GetRAMBase() const
	{
		switch (m_ramMode)
		{
		case RAMMode::ZP:
			return 0;
		case RAMMode::SP:
			return  m_cpu->m_reg.SP;
		case RAMMode::PC:
			return  m_cpu->GetCurrentAddress();
		case RAMMode::CUSTOM:
		default:
			return m_customMemView;
		}
	}

	void Monitor6800::UpdateRAM()
	{
		static Coord addrPos = m_cpu->GetInfo().GetCoord("ram.ADDR");
		static Coord hexPos = m_cpu->GetInfo().GetCoord("ram.HEX");
		static Coord charPos = m_cpu->GetInfo().GetCoord("ram.CHAR");
		static unsigned bytesPerLine = charPos.w;
		static unsigned bytesTotal = charPos.w * charPos.h;

		// Adjust position so the view doesn't move around too much
		ADDRESS addr = GetRAMBase();

		ADDRESS adjustedAddress = (addr / bytesPerLine) * bytesPerLine;
		if (adjustedAddress >= bytesPerLine)
		{
			adjustedAddress -= bytesPerLine; // Show one row before when possible
		}

		ADDRESS curr = addr;
		ADDRESS data = adjustedAddress;
		for (int y = 0; y < hexPos.h; ++y)
		{
			Coord pos;
			pos.x = addrPos.x;
			pos.y = addrPos.y + y;
			WriteValueHex((WORD)(adjustedAddress + (bytesPerLine * y)), pos, 8);

			ADDRESS a = (data + (bytesPerLine * y)) & m_memory->GetAddressMask();
			auto& block = m_memory->FindBlock(a);
			enum class MemBlockType { UNALLOCATED, IO, MEM } type = MemBlockType::UNALLOCATED;

			if (block.blockR)
			{
				type = (block.blockR->GetType() == emul::MemoryType::IO) ? MemBlockType::IO : MemBlockType::MEM;
			}

			for (unsigned x = 0; x < bytesPerLine; ++x)
			{
				pos.x = hexPos.x + (3 * x);
				BYTE ch = 0;

				switch (type)
				{
				case MemBlockType::UNALLOCATED:
					m_console.WriteAt(pos.x, pos.y, "..", 2, 8);
					break;
				case MemBlockType::IO:
					m_console.WriteAt(pos.x, pos.y, "IO", 2, 8);
					break;
				case MemBlockType::MEM:
				{
					ch = m_memory->Read8(a);
					WriteValueHex(ch, pos, (a == curr) ? 15 + (1 << 4) : 7);
					break;
				}
				default:
					NODEFAULT;
				}

				m_console.WriteAt(charPos.x + x, pos.y, ch ? ch : 0xFA, ch ? 7 : 8);
				++a;
			}
		}
	}

	void Monitor6800::PrintInstruction(short y, Instruction& instr)
	{
		static Coord addressPos = m_cpu->GetInfo().GetCoord("CODE.address");
		static Coord rawPos = m_cpu->GetInfo().GetCoord("CODE.raw");
		static Coord textPos = m_cpu->GetInfo().GetCoord("CODE.text");
		static short baseY = addressPos.y;

		// TODO: Horribly inefficient
		Coord pos;
		pos.y = baseY + y;

		// Write address
		pos.x = addressPos.x;
		bool isBreakpoint = m_breakpointEnabled && (instr.address == m_breakpoint);
		WriteValueHex((WORD)instr.address, pos, isBreakpoint ? (4 << 4) | 15 : 8 );
		pos.x = rawPos.x;
		m_console.WriteAt(pos.x, pos.y, (const char*)instr.raw, instr.len);
		for (int i = 0; i < rawPos.w - instr.len; ++i)
		{
			m_console.WriteAt(pos.x + instr.len + i, pos.y, 0xFAu, 8);
		}

		// Write text (disassembly)
		pos.x = textPos.x;
		m_console.WriteAt(pos.x, pos.y, instr.text, textPos.w);
	}

	void Monitor6800::UpdateCode()
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

	void Monitor6800::ClearCode()
	{
		static Coord codePos = m_cpu->GetInfo().GetCoord("CODE");

		m_console.Clear(codePos.x, codePos.y, codePos.w, codePos.h);

		// Show last instruction
		Instruction decoded;
		Disassemble(m_cpu->GetLastAddress(), decoded);
		PrintInstruction(4, decoded);
	}

	bool Monitor6800::Replace(std::string& str, const std::string& from, const std::string& to)
	{
		size_t start = str.find(from);
		if (start == std::string::npos)
		{
			return false;
		}
		str.replace(start, from.size(), to);
		return true;
	}

	ADDRESS Monitor6800::Disassemble(ADDRESS address, Monitor6800::Instruction& decoded)
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

			sprintf(buf, "$%02X", imm8);
			Replace(text, "{i8}", buf);
			break;
		}
		case Opcode::IMM::W16:
		{
			WORD imm16 = m_memory->Read16(++address);
			++address;
			decoded.AddRaw(imm16);

			sprintf(buf, "$%04X", imm16);
			Replace(text, "{i16}", buf);
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

	void Monitor6800::Instruction::AddRaw(BYTE b)
	{
		this->raw[this->len++] = hexDigits[b >> 4];
		this->raw[this->len++] = hexDigits[(b & 0x0F)];
	}
	void Monitor6800::Instruction::AddRaw(WORD w)
	{
		// LOW BYTE
		this->raw[this->len++] = hexDigits[(w >> 4) & 0x0F];
		this->raw[this->len++] = hexDigits[(w & 0x0F)];
		// HIGH BYTE
		this->raw[this->len++] = hexDigits[(w >> 12) & 0x0F];
		this->raw[this->len++] = hexDigits[(w >> 8) & 0x0F];
	}
}
