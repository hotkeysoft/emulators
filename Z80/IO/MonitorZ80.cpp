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
				Replace(text, grpLabel, op2Str); break;
			default:
				break;
			}
		}

		char buf[32];
		switch (instr.imm)
		{
		case Opcode::IMM::W8:
		{
			BYTE imm8 = m_memory->Read8(++address);
			decoded.AddRaw(imm8);

			sprintf(buf, "0%Xh", imm8);
			Replace(text, "{i8}", buf);
			break;
		}
		case Opcode::IMM::W16:
		{
			WORD imm16 = m_memory->Read16(++address);
			++address;
			decoded.AddRaw(imm16);

			sprintf(buf, "0%Xh", imm16);
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
}
