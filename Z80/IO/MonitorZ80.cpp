#include "stdafx.h"
#include "MonitorZ80.h"
#include "CPU/CPUZ80.h"

using cpuInfo::Opcode;
using cpuInfo::Coord;
using emul::GetBit;

namespace emul
{
	MonitorZ80::MonitorZ80(Console& console) : Monitor8080(console)		
	{
	}

	void MonitorZ80::Init(CPU8080& cpu, Memory& memory)
	{
		Monitor8080::Init(cpu, memory);

		m_cpuZ80 = dynamic_cast<CPUZ80*>(&cpu);
		if (m_cpuZ80 == nullptr)
		{
			throw std::exception("MonitorZ80::Init, Expected Z80 CPU");
		}
	}

	void MonitorZ80::ToggleRAMMode()
	{
		switch (m_ramMode)
		{
		case RAMMode::HL: m_ramMode = (RAMMode)RAMModeZ80::IX; break;
		case (RAMMode)RAMModeZ80::IX: m_ramMode = (RAMMode)RAMModeZ80::IY; break;
		case (RAMMode)RAMModeZ80::IY: m_ramMode = RAMMode::SP; break;
		case RAMMode::SP: m_ramMode = RAMMode::PC; break;
		case RAMMode::PC: m_ramMode = RAMMode::CUSTOM; break;
		case RAMMode::CUSTOM: m_ramMode = RAMMode::HL; break;
		}

		UpdateRAMMode();
	}

	void MonitorZ80::UpdateRAMMode()
	{
		const WORD highlight = (3 << 4) | 14;
		const WORD regular = (0 << 4) | 8;
		static Coord ramHL = m_cpu->GetInfo().GetCoord("ram.HL");
		static Coord ramPC = m_cpu->GetInfo().GetCoord("ram.PC");
		static Coord ramIX = m_cpu->GetInfo().GetCoord("ram.IX");
		static Coord ramIY = m_cpu->GetInfo().GetCoord("ram.IY");
		static Coord ramSP = m_cpu->GetInfo().GetCoord("ram.SP");
		static Coord ramCustom = m_cpu->GetInfo().GetCoord("ram.CUSTOM");

		m_console.WriteAttrAt(ramHL.x, ramHL.y, (m_ramMode == RAMMode::HL) ? highlight : regular, ramHL.w);
		m_console.WriteAttrAt(ramPC.x, ramPC.y, (m_ramMode == RAMMode::PC) ? highlight : regular, ramHL.w);
		m_console.WriteAttrAt(ramIX.x, ramIX.y, (m_ramMode == (RAMMode)RAMModeZ80::IX) ? highlight : regular, ramHL.w);
		m_console.WriteAttrAt(ramIY.x, ramIY.y, (m_ramMode == (RAMMode)RAMModeZ80::IY) ? highlight : regular, ramHL.w);
		m_console.WriteAttrAt(ramSP.x, ramSP.y, (m_ramMode == RAMMode::SP) ? highlight : regular, ramSP.w);

		{
			static char buf[32];
			sprintf(buf, " CUSTOM:%04X", m_customMemView);
			m_console.WriteAt(ramCustom.x, ramCustom.y, buf);
			m_console.WriteAttrAt(ramCustom.x, ramCustom.y, (m_ramMode == RAMMode::CUSTOM) ? highlight : regular, ramCustom.w);
		}

		UpdateRAM();
	}

	ADDRESS MonitorZ80::GetRAMBase() const
	{
		switch ((RAMModeZ80)m_ramMode)
		{
		case RAMModeZ80::IX:
			return m_cpuZ80->m_regIX;
		case RAMModeZ80::IY:
			return m_cpuZ80->m_regIY;

		default:
			return Monitor8080::GetRAMBase();
		}
	}

	void MonitorZ80::UpdateRegisters()
	{
		Monitor8080::UpdateRegisters();

		WriteValueHex(m_cpuZ80->m_regAlt.A, m_cpu->GetInfo().GetCoord("A'"));
		WriteValueHex(m_cpuZ80->m_regAlt.B, m_cpu->GetInfo().GetCoord("B'"));
		WriteValueHex(m_cpuZ80->m_regAlt.C, m_cpu->GetInfo().GetCoord("C'"));
		WriteValueHex(m_cpuZ80->m_regAlt.D, m_cpu->GetInfo().GetCoord("D'"));
		WriteValueHex(m_cpuZ80->m_regAlt.E, m_cpu->GetInfo().GetCoord("E'"));

		WriteValueHex(m_cpuZ80->m_regAlt.H, m_cpu->GetInfo().GetCoord("H'"));
		WriteValueHex(m_cpuZ80->m_regAlt.L, m_cpu->GetInfo().GetCoord("L'"));

		WriteValueHex(m_cpuZ80->m_regAlt.flags, m_cpu->GetInfo().GetCoord("F'"));

		WriteValueHex(m_cpuZ80->m_regIX, m_cpu->GetInfo().GetCoord("IX"));
		WriteValueHex(m_cpuZ80->m_regIY, m_cpu->GetInfo().GetCoord("IY"));

		WriteValueHex(m_cpuZ80->m_regI, m_cpu->GetInfo().GetCoord("I"));
		WriteValueHex(m_cpuZ80->m_regR, m_cpu->GetInfo().GetCoord("R"));

		WriteValueNibble(m_cpuZ80->m_interruptMode, m_cpu->GetInfo().GetCoord("IM"));
		WriteValueNibble(m_cpuZ80->m_iff1, m_cpu->GetInfo().GetCoord("IFF1"));
		WriteValueNibble(m_cpuZ80->m_iff1, m_cpu->GetInfo().GetCoord("IFF2"));
	}

	std::string ToHex(DWORD val)
	{
		static char buf[32];
		sprintf(buf, "0%Xh", val);
		return (isalpha(buf[1])) ? buf : (buf + 1);
	}

	ADDRESS MonitorZ80::Disassemble(ADDRESS address, MonitorZ80::Instruction& decoded)
	{
		decoded.address = address;
		BYTE data = m_memory->Read8(address);

		decoded.AddRaw(data);

		Opcode instr = m_cpu->GetInfo().GetOpcode(data);
		std::string text = instr.text;

		if (instr.multi != Opcode::MULTI::NONE)
		{
			BYTE op2 = m_memory->Read8(++address);
			decoded.AddRaw(op2);
			const std::string op2Str = m_cpu->GetInfo().GetSubOpcodeStr(instr, op2);

			char grpLabel[16] = "";
			sprintf(grpLabel, "{grp%d}", (int)instr.multi + 1);

			switch (instr.multi)
			{
			case Opcode::MULTI::GRP1:
			case Opcode::MULTI::GRP2:
			case Opcode::MULTI::GRP3:
			case Opcode::MULTI::GRP4:
				instr = m_cpu->GetInfo().GetSubOpcode(instr, op2);
				Replace(text, grpLabel, op2Str); break;
			default:
				break;
			}
		}

		switch (instr.imm)
		{
		case Opcode::IMM::W8:
		case Opcode::IMM::W8W8:
		{
			int count = (instr.imm == Opcode::IMM::W8W8) ? 2 : 1;
			for (int i = 0; i < count; ++i)
			{
				BYTE imm8 = m_memory->Read8(++address);
				decoded.AddRaw(imm8);		
				Replace(text, "{i8}", ToHex(imm8));
			}
			break;
		}
		case Opcode::IMM::W16:
		{
			WORD imm16 = m_memory->Read16(++address);
			++address;
			decoded.AddRaw(imm16);
			Replace(text, "{i16}", ToHex(imm16));
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
}
