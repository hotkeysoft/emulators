#include "stdafx.h"
#include "CPU80186.h"

using cpuInfo::CPUType;

namespace emul
{
	CPU80186::CPU80186(Memory& memory) : 
		CPU8086(CPUType::i80186, memory),
		Logger("CPU80186")
	{
	}

	void CPU80186::Init()
	{
		CPU8086::Init();

		// PUSH SP
		// Fixes the "Bug" on 8086/80186 where push sp pushed an already-decremented value
		m_opcodes[0x54] = [=]() { PUSH(m_reg[REG16::SP]); };

		// PUSHA
		m_opcodes[0x60] = [=]() { PUSHA(); };

		// POPA
		m_opcodes[0x61] = [=]() { POPA(); };

		m_opcodes[0x62] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x63] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x64] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x65] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x66] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x67] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x68] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x69] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6A] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6B] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6C] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6D] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6E] = [=]() { NotImplemented(m_opcode); };
		m_opcodes[0x6F] = [=]() { NotImplemented(m_opcode); };
	}

	void CPU80186::PUSHA()
	{
		WORD originalSP = m_reg[REG16::SP];

		PUSH(REG16::AX);
		PUSH(REG16::CX);
		PUSH(REG16::DX);
		PUSH(REG16::BX);
		PUSH(originalSP);
		PUSH(REG16::BP);
		PUSH(REG16::SI);
		PUSH(REG16::DI);
	}

	void CPU80186::POPA()
	{
		POP(REG16::DI);
		POP(REG16::SI);
		POP(REG16::BP);
		POP(REG16::_T0); // Dummy read, don't put in SP
		POP(REG16::BX);
		POP(REG16::DX);
		POP(REG16::CX);
		POP(REG16::AX);
	}

}
