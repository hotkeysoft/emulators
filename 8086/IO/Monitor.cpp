#include "stdafx.h"

#include "Monitor.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;

namespace emul
{
	static const char hexDigits[] = "0123456789ABCDEF";

	Monitor::Monitor(Console& console) :
		m_console(console)
	{
	}

	void Monitor::Init(CPU8086& cpu, Memory& memory)
	{
		m_cpu = &cpu;
		m_memory = &memory;
	}

	MonitorState Monitor::ProcessKey()
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

	MonitorState Monitor::Run()
	{
		Update();
		if (m_runMode == RUNMode::STEP)
		{
			m_console.WaitForKey();
		}

		return ProcessKey();
	}

	void Monitor::Show()
	{
		std::string ansiFile = m_cpu->GetInfo().GetANSIFile();

		if (ansiFile.size()) 
		{
			m_console.WriteBuffer(ansiFile.c_str(), ansiFile.size());
		}

		Update();
		UpdateRunMode();
		UpdateRAMMode();
	}

	void Monitor::WriteValueHex(BYTE value, const Coord& coord, WORD attr)
	{
		static char hex[2];
		hex[0] = hexDigits[value >> 4];
		hex[1] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 2, attr);
	}

	void Monitor::WriteValueHex(WORD value, const Coord& coord, WORD attr)
	{
		static char hex[4];
		hex[0] = hexDigits[(value >> 12) & 0x0F];
		hex[1] = hexDigits[(value >> 8) & 0x0F];
		hex[2] = hexDigits[(value >> 4) & 0x0F];
		hex[3] = hexDigits[(value & 0x0F)];

		m_console.WriteAt(coord.x, coord.y, hex, 4, attr);
	}

	void Monitor::Update()
	{
		UpdateRegisters();
		UpdateTicks();
		UpdateFlags();
		UpdateRAM();
		UpdateCode();
	}

	void Monitor::ToggleRunMode()
	{
		if (m_runMode == RUNMode::RUN) m_runMode = RUNMode::STEP;
		else m_runMode = RUNMode::RUN;

		UpdateRunMode();
	}

	void Monitor::UpdateRunMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (1 << 4) | 14;
		static Coord run = m_cpu->GetInfo().GetCoord("status.RUN");
		static Coord stop = m_cpu->GetInfo().GetCoord("status.STOP");

		m_console.WriteAttrAt(run.x, run.y, (m_runMode == RUNMode::RUN) ? highlight : regular, run.w);
		m_console.WriteAttrAt(stop.x, stop.y, (m_runMode == RUNMode::STEP) ? highlight : regular, stop.w);
	}

	void Monitor::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::DSSI: m_ramMode = RAMMode::ESDI; break;
		case RAMMode::ESDI: m_ramMode = RAMMode::STACK; break;
		case RAMMode::STACK: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::DSSI; break;
		}

		UpdateRAMMode();
	}

	void Monitor::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramDSSI = m_cpu->GetInfo().GetCoord("ram.DSSI");
		static Coord ramESDI = m_cpu->GetInfo().GetCoord("ram.ESDI");
		static Coord ramSTACK = m_cpu->GetInfo().GetCoord("ram.STACK");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramDSSI.x, ramDSSI.y, (m_ramMode == RAMMode::DSSI) ? highlight : regular, ramDSSI.w);
		m_console.WriteAttrAt(ramESDI.x, ramESDI.y, (m_ramMode == RAMMode::ESDI) ? highlight : regular, ramESDI.w);
		m_console.WriteAttrAt(ramSTACK.x, ramSTACK.y, (m_ramMode == RAMMode::STACK) ? highlight : regular, ramSTACK.w);
		
		{
			static char buf[32];
			sprintf(buf, " CUSTOM:%s", m_customMemView.ToString());
			m_console.WriteAt(ramCustom.x, ramCustom.y, buf);
			m_console.WriteAttrAt(ramCustom.x, ramCustom.y, (m_ramMode == RAMMode::CUSTOM) ? highlight : regular, ramCustom.w);
		}

		UpdateRAM();
	}

	void Monitor::UpdateRegisters()
	{
		WriteValueHex(m_cpu->m_reg[REG8::AH], m_cpu->GetInfo().GetCoord("AH"));
		WriteValueHex(m_cpu->m_reg[REG8::AL], m_cpu->GetInfo().GetCoord("AL"));

		WriteValueHex(m_cpu->m_reg[REG8::BH], m_cpu->GetInfo().GetCoord("BH"));
		WriteValueHex(m_cpu->m_reg[REG8::BL], m_cpu->GetInfo().GetCoord("BL"));

		WriteValueHex(m_cpu->m_reg[REG8::CH], m_cpu->GetInfo().GetCoord("CH"));
		WriteValueHex(m_cpu->m_reg[REG8::CL], m_cpu->GetInfo().GetCoord("CL"));

		WriteValueHex(m_cpu->m_reg[REG8::DH], m_cpu->GetInfo().GetCoord("DH"));
		WriteValueHex(m_cpu->m_reg[REG8::DL], m_cpu->GetInfo().GetCoord("DL"));

		WriteValueHex(m_cpu->m_reg[REG16::DS], m_cpu->GetInfo().GetCoord("DS"));
		WriteValueHex(m_cpu->m_reg[REG16::SI], m_cpu->GetInfo().GetCoord("SI"));

		WriteValueHex(m_cpu->m_reg[REG16::ES], m_cpu->GetInfo().GetCoord("ES"));
		WriteValueHex(m_cpu->m_reg[REG16::DI], m_cpu->GetInfo().GetCoord("DI"));

		WriteValueHex(m_cpu->m_reg[REG16::BP], m_cpu->GetInfo().GetCoord("BP"));

		WriteValueHex(m_cpu->m_reg[REG16::CS], m_cpu->GetInfo().GetCoord("CS"));
		WriteValueHex(m_cpu->m_reg[REG16::IP], m_cpu->GetInfo().GetCoord("IP"));

		WriteValueHex(m_cpu->m_reg[REG16::SS], m_cpu->GetInfo().GetCoord("SS"));
		WriteValueHex(m_cpu->m_reg[REG16::SP], m_cpu->GetInfo().GetCoord("SP"));
	}

	void Monitor::UpdateTicks()
	{
		uint32_t ticks = m_cpu->GetInstructionTicks();
		static char buf[5];
		sprintf(buf, "%04d", (BYTE)ticks);

		static Coord coord = m_cpu->GetInfo().GetCoord("TICKS");

		m_console.WriteAt(coord.x, coord.y, buf, 4);
	}

	void Monitor::UpdateFlags()
	{
		static WORD attr[12];

		for (int i = 0; i < 12; ++i)
		{
			attr[11-i] = (m_cpu->m_reg[REG16::FLAGS] & (1 << i)) ? 15 : 8;
		}

		static Coord coord = m_cpu->GetInfo().GetCoord("FLAGS");

		m_console.WriteAttrAt(coord.x, coord.y, attr, 12);
	}

	void Monitor::UpdateRAM()
	{
		static Coord addrPos = m_cpu->GetInfo().GetCoord("ram.ADDR");
		static Coord hexPos = m_cpu->GetInfo().GetCoord("ram.HEX");
		static Coord charPos = m_cpu->GetInfo().GetCoord("ram.CHAR");
		static int bytesPerLine = charPos.w;
		static int bytesTotal = charPos.w * charPos.h;

		// Adjust position so the view doesn't move around too much
		WORD segment = 0;
		WORD offset = 0;

		switch (m_ramMode)
		{
		case RAMMode::DSSI:
			segment = m_cpu->m_reg[REG16::DS];
			offset = m_cpu->m_reg[REG16::SI];
			break;
		case RAMMode::ESDI:
			segment = m_cpu->m_reg[REG16::ES];
			offset = m_cpu->m_reg[REG16::DI];
			break;
		case RAMMode::STACK:
			segment = m_cpu->m_reg[REG16::SS];
			offset = m_cpu->m_reg[REG16::SP];
			break;
		case RAMMode::CUSTOM:
		default:
			segment = m_customMemView.segment;
			offset = m_customMemView.offset;
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
		
		ADDRESS curr = S2A(segment, offset);
		ADDRESS data = S2A(segment, adjustedOffset);
		for (int y = 0; y < hexPos.h; ++y)
		{
			Coord pos;
			pos.x = addrPos.x;
			pos.y = addrPos.y + y;
			WriteValueHex(segment, pos);
			pos.x += 5;
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

	void Monitor::PrintInstruction(short y, Instruction& instr)
	{
		static Coord segmentPos = m_cpu->GetInfo().GetCoord("CODE.segment");
		static Coord offsetPos = m_cpu->GetInfo().GetCoord("CODE.offset");
		static Coord rawPos = m_cpu->GetInfo().GetCoord("CODE.raw");
		static Coord textPos = m_cpu->GetInfo().GetCoord("CODE.text");
		static short baseY = segmentPos.y;

		// TODO: Horribly inefficient
		Coord pos;
		pos.y = baseY + y;

		pos.x = segmentPos.x;
		WriteValueHex(instr.address.segment, pos);
		pos.x = offsetPos.x;
		WriteValueHex(instr.address.offset, pos);
		pos.x = rawPos.x;
		m_console.WriteAt(pos.x, pos.y, (const char*)instr.raw, instr.len);
		for (int i = 0; i < rawPos.w - instr.len; ++i)
		{
			m_console.WriteAt(pos.x + instr.len + i, pos.y, 0xFAu, 8);
		}

		pos.x = textPos.x;
		m_console.WriteAt(pos.x, pos.y, instr.text, textPos.w);
	}

	void Monitor::UpdateCode()
	{
		static Coord codePos = m_cpu->GetInfo().GetCoord("CODE");

		SegmentOffset address{ m_cpu->m_reg[REG16::CS], m_cpu->m_reg[REG16::IP] };

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

	SegmentOffset Monitor::Disassemble(SegmentOffset address, Monitor::Instruction& decoded)
	{
		decoded.address = address;
		BYTE data = m_memory->Read8(address.GetAddress());

		decoded.AddRaw(data);

		Opcode instr = m_cpu->GetInfo().GetOpcode(data);
		std::string text = instr.text;

		if (instr.modRegRm != Opcode::MODREGRM::NONE ||
			instr.multi != Opcode::MULTI::NONE)
		{
			address.offset++;
			BYTE modRegRm = m_memory->Read8(address.GetAddress());
			decoded.AddRaw(modRegRm);

			BYTE op2 = (modRegRm >> 3) & 7;
			const std::string op2Str = m_cpu->GetInfo().GetSubOpcode(instr, op2);

			char grpLabel[16] = "";
			if (instr.multi != Opcode::MULTI::NONE)
			{
				sprintf(grpLabel, "{grp%d}", (int)instr.multi + 1);
			}

			switch (instr.multi)
			{
			case Opcode::MULTI::GRP1:
			case Opcode::MULTI::GRP2:
			case Opcode::MULTI::GRP3:
			case Opcode::MULTI::GRP4:
			case Opcode::MULTI::GRP5:
				replace(text, grpLabel, op2Str); break;
			default:
				break;
			}

			const char* regStr = nullptr;
			BYTE reg = modRegRm >> 3;
			switch (instr.modRegRm)
			{
			case Opcode::MODREGRM::SR:
				if (instr.sr) replace(text, "{sr}", CPU8086::GetReg16Str(reg, true));
				break;
			case Opcode::MODREGRM::W8:
				if (instr.r8) replace(text, "{r8}", CPU8086::GetReg8Str(reg));
				break;
			case Opcode::MODREGRM::W16:
				if (instr.r16) replace(text, "{r16}", CPU8086::GetReg16Str(reg, false));
				break;
			}

			BYTE disp = 0;
			if (instr.rm8)
			{
				replace(text, "{rm8}", CPU8086::GetModRMStr(modRegRm, false, disp));
			}
			if (instr.rm16)
			{
				replace(text, "{rm16}", CPU8086::GetModRMStr(modRegRm, true, disp));
			}

			// GetModRMStr can insert a {d8} or {d16} displacement, we have to fetch it
			char buf[32];
			if (disp == 8)
			{
				address.offset++;
				BYTE disp8 = m_memory->Read8(address.GetAddress());
				decoded.AddRaw(disp8);

				sprintf(buf, "0%Xh", disp8);
				replace(text, "{d8}", buf);
			}
			else if (disp == 16)
			{
				address.offset++;
				WORD disp16 = m_memory->Read16(address.GetAddress());
				address.offset++;
				decoded.AddRaw(disp16);

				sprintf(buf, "0%Xh", disp16);
				replace(text, "{d16}", buf);
			}
		}

		char buf[32];
		switch (instr.imm)
		{
		case Opcode::IMM::W8:
		{
			++address.offset;
			BYTE imm8 = m_memory->Read8(address.GetAddress());
			decoded.AddRaw(imm8);

			sprintf(buf, "0%Xh", imm8);
			replace(text, "{i8}", buf);
			break;
		}
		case Opcode::IMM::W16:
		{
			++address.offset;
			WORD imm16 = m_memory->Read16(address.GetAddress());
			address.offset++;
			decoded.AddRaw(imm16);

			sprintf(buf, "0%Xh", imm16);
			replace(text, "{i16}", buf);
			break;
		}
		case Opcode::IMM::W32:
		{
			++address.offset;
			WORD imm16Offset = m_memory->Read16(address.GetAddress());
			address.offset++;
			decoded.AddRaw(imm16Offset);
			address.offset++;
			WORD imm16Segment = m_memory->Read16(address.GetAddress());
			address.offset++;
			decoded.AddRaw(imm16Segment);
			sprintf(buf, "0%Xh:0%Xh", imm16Segment, imm16Offset);
			replace(text, "{i32}", buf);
			break;
		}
		default:
			break;
		}

		memset(decoded.text, ' ', 32);
		memcpy(decoded.text, text.c_str(), text.size());

		++address.offset;

		return address;
	}

	void Monitor::Instruction::AddRaw(BYTE b)
	{
		this->raw[this->len++] = hexDigits[b >> 4];
		this->raw[this->len++] = hexDigits[(b & 0x0F)];
	}
	void Monitor::Instruction::AddRaw(WORD w)
	{
		// LOW BYTE
		this->raw[this->len++] = hexDigits[(w >> 4) & 0x0F];
		this->raw[this->len++] = hexDigits[(w & 0x0F)];
		// HIGH BYTE
		this->raw[this->len++] = hexDigits[(w >> 12) & 0x0F];
		this->raw[this->len++] = hexDigits[(w >> 8) & 0x0F];
	}
}
