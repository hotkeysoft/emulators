#include "stdafx.h"
#include "CPU6803.h"

using cpuInfo::Opcode;

namespace emul
{
	static CPU6803::IOEventHandler s_defaultHandler;

	CPU6803::CPU6803(Memory& memory, bool internalRAM) :
		CPU6800(CPUID_6803, memory),
		m_ioEventHandler(&s_defaultHandler),
		Logger(CPUID_6803)
	{
		if (internalRAM)
		{
			m_internalRAM = new MemoryBlock("INT_RAM", RAM_SIZE);
		}
		m_sub16SetCarry = true;
	}

	CPU6803::~CPU6803()
	{
		delete m_internalRAM;
	};

	void CPU6803::SetIOEventHandler(IOEventHandler* handler)
	{
		m_ioEventHandler = handler ? handler : &s_defaultHandler;
	}

	void CPU6803::Init()
	{
		CPU6800::Init();

		m_opcodes[0x04] = [=]() { UnknownOpcode(); }; // LSRD
		m_opcodes[0x05] = [=]() { ASL(m_reg.D); }; // ASLD

		m_opcodes[0x21] = [=]() { BRA(false); }; // BRN

		m_opcodes[0x38] = [=]() { m_reg.X = popW(); }; // PULX
		m_opcodes[0x3A] = [=]() { m_reg.X += m_reg.ab.B; }; // ABX
		m_opcodes[0x3C] = [=]() { pushW(m_reg.X); }; // PSHX
		m_opcodes[0x3D] = [=]() { MUL(); }; // MUL

		m_opcodes[0x9D] = [=]() { JSR(GetDirect()); }; // JSR direct

		m_opcodes[0xC3] = [=]() { ADD16(m_reg.D, FetchWord()); }; // ADDD imm
		m_opcodes[0xD3] = [=]() { ADD16(m_reg.D, GetMemDirectWord()); }; // ADDD direct
		m_opcodes[0xE3] = [=]() { ADD16(m_reg.D, GetMemIndexedWord()); }; // ADDD indexed
		m_opcodes[0xF3] = [=]() { ADD16(m_reg.D, GetMemExtendedWord()); }; // ADDD extended

		m_opcodes[0xCC] = [=]() { LD16(m_reg.D, FetchWord()); }; // LDD imm
		m_opcodes[0xDC] = [=]() { LD16(m_reg.D, GetMemDirectWord()); }; // LDD direct
		m_opcodes[0xEC] = [=]() { LD16(m_reg.D, GetMemIndexedWord()); }; // LDD indexed
		m_opcodes[0xFC] = [=]() { LD16(m_reg.D, GetMemExtendedWord()); }; // LDD extended

		m_opcodes[0xDD] = [=]() { ST16(GetDirect(), m_reg.D); }; // STD direct
		m_opcodes[0xED] = [=]() { ST16(GetIndexed(), m_reg.D); }; // STD indexed
		m_opcodes[0xFD] = [=]() { ST16(GetExtended(), m_reg.D); }; // STD extended

		m_opcodes[0x83] = [=]() { SUB16(m_reg.D, FetchWord()); }; // SUBD imm
		m_opcodes[0x93] = [=]() { SUB16(m_reg.D, GetMemDirectWord()); }; // SUBD direct
		m_opcodes[0xA3] = [=]() { SUB16(m_reg.D, GetMemIndexedWord()); }; // SUBD indexed
		m_opcodes[0xB3] = [=]() { SUB16(m_reg.D, GetMemExtendedWord()); }; // SUBD extended
	}

	void CPU6803::Reset()
	{
		CPU6800::Reset();
		if (m_internalRAM)
		{
			LogPrintf(LOG_INFO, "Enable 128 bytes RAM block at 0x0080");
			if (m_internalRAM->GetBlockGranularity() > RAM_SIZE)
			{
				LogPrintf(LOG_ERROR, "Internal RAM: Block granularity > 128");
				throw std::exception("Internal RAM: Block granularity > 128");
			}
			m_memory.Allocate(m_internalRAM, RAM_BASE);
		}
	}

	void CPU6803::Dump()
	{
		LogPrintf(LOG_ERROR, "AB = %04X", m_reg.D);
		LogPrintf(LOG_ERROR, "X  = %04X", m_reg.X);
		LogPrintf(LOG_ERROR, "Flags --HINZVC");
		LogPrintf(LOG_ERROR, "      "PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(m_flags));
		LogPrintf(LOG_ERROR, "S  = %04X", m_reg.SP);
		LogPrintf(LOG_ERROR, "PC = %04X", m_programCounter);
		LogPrintf(LOG_ERROR, "");
	}

	BYTE CPU6803::MemRead8(ADDRESS address)
	{
		if (address < ADDRESS(IORegister::_REG_COUNT))
		{
			return IORead(IORegister(address));
		}
		else
		{
			return m_memory.Read8(address);
		}
	}
	void CPU6803::MemWrite8(ADDRESS address, BYTE value)
	{
		if (address < ADDRESS(IORegister::_REG_COUNT))
		{
			return IOWrite(IORegister(address), value);
		}
		else
		{
			return m_memory.Write8(address, value);
		}
	}

	void CPU6803::MUL()
	{
		WORD res = m_reg.ab.A * m_reg.ab.B;

		SetFlag(FLAG_C, GetBit(res, 7));

		m_reg.D = res;
	}

	void CPU6803::ASL(WORD& dest)
	{
		bool carry = GetMSB(dest);
		SetFlag(FLAG_V, GetBit(dest, 6 + 8) ^ carry);
		SetFlag(FLAG_C, carry);

		dest <<= 1;

		AdjustNZ(dest);
	}

	BYTE CPU6803::IORead(IORegister reg)
	{
		switch (reg)
		{
		case IORegister::Port1DataDirection:
			LogPrintf(LOG_WARNING, "IORead(Port1DataDirection): Not implemented"); return 0xFF;
		case IORegister::Port2DataDirection:
			LogPrintf(LOG_WARNING, "IORead(Port2DataDirection): Not implemented"); return 0xFF;
		case IORegister::Port1Data:
			LogPrintf(LOG_DEBUG, "IORead(Port1Data)");
			return m_ioEventHandler->OnReadPort1();
		case IORegister::Port2Data:
			LogPrintf(LOG_DEBUG, "IORead(Port2Data)");
			return m_ioEventHandler->OnReadPort2();
		case IORegister::TimerControlStatus:
			LogPrintf(LOG_ERROR, "IORead(TimerControlStatus): Not implemented"); return 0xFF;
		case IORegister::CounterHigh:
			LogPrintf(LOG_ERROR, "IORead(CounterHigh): Not implemented"); return 0xFF;
		case IORegister::CounterLow:
			LogPrintf(LOG_ERROR, "IORead(CounterLow): Not implemented"); return 0xFF;
		case IORegister::OutputCompareHigh:
			LogPrintf(LOG_ERROR, "IORead(OutputCompareHigh): Not implemented"); return 0xFF;
		case IORegister::OutputCompareLow:
			LogPrintf(LOG_ERROR, "IORead(OutputCompareLow): Not implemented"); return 0xFF;
		case IORegister::InputCaptureHigh:
			LogPrintf(LOG_ERROR, "IORead(InputCaptureHigh): Not implemented"); return 0xFF;
		case IORegister::InputCaptureLow:
			LogPrintf(LOG_ERROR, "IORead(InputCaptureLow): Not implemented"); return 0xFF;
		case IORegister::RateModeControl:
			LogPrintf(LOG_ERROR, "IORead(RateModeControl): Not implemented"); return 0xFF;
		case IORegister::TxRxControlStatus:
			LogPrintf(LOG_ERROR, "IORead(TxRxControlStatus): Not implemented"); return 0xFF;
		case IORegister::RxData:
			LogPrintf(LOG_ERROR, "IORead(RxData): Not implemented"); return 0xFF;
		case IORegister::TxData:
			LogPrintf(LOG_ERROR, "IORead(TxData): Not implemented"); return 0xFF;
		case IORegister::RAMControl:
			LogPrintf(LOG_ERROR, "IORead(RAMControl): Not implemented"); return 0xFF;

		case IORegister::_Reserved15:
		case IORegister::_Reserved16:
		case IORegister::_Reserved17:
		case IORegister::_Reserved18:
		case IORegister::_Reserved19:
		case IORegister::_Reserved1A:
		case IORegister::_Reserved1B:
		case IORegister::_Reserved1C:
		case IORegister::_Reserved1D:
		case IORegister::_Reserved1E:
		case IORegister::_Reserved1F:
			LogPrintf(LOG_WARNING, "IORead(%02X): Reserved", reg);
			return 0xFF;
		default:
			// Passthrough for 04-07, 0F
			return m_memory.Read8(ADDRESS(reg));
		}

		return 0xFF;
	}
	void CPU6803::IOWrite(IORegister reg, BYTE value)
	{
		switch (reg)
		{
		case IORegister::Port1DataDirection:
			LogPrintf(LOG_WARNING, "IOWrite(Port1DataDirection, %02X): Not implemented", value); break;
		case IORegister::Port2DataDirection:
			LogPrintf(LOG_WARNING, "IOWrite(Port2DataDirection, %02X): Not implemented", value); break;
		case IORegister::Port1Data:
			LogPrintf(LOG_DEBUG, "IOWrite(Port1Data, %02X)", value);
			m_ioEventHandler->OnWritePort1(value);
			break;
		case IORegister::Port2Data:
			LogPrintf(LOG_DEBUG, "IOWrite(Port2Data, %02X)", value);
			m_ioEventHandler->OnWritePort2(value);
			break;
		case IORegister::TimerControlStatus:
			LogPrintf(LOG_ERROR, "IOWrite(TimerControlStatus, %02X): Not implemented", value); break;
		case IORegister::CounterHigh:
			LogPrintf(LOG_ERROR, "IOWrite(CounterHigh, %02X): Not implemented", value); break;
		case IORegister::CounterLow:
			LogPrintf(LOG_ERROR, "IOWrite(CounterLow, %02X): Not implemented", value); break;
		case IORegister::OutputCompareHigh:
			LogPrintf(LOG_ERROR, "IOWrite(OutputCompareHigh, %02X): Not implemented", value); break;
		case IORegister::OutputCompareLow:
			LogPrintf(LOG_ERROR, "IOWrite(OutputCompareLow, %02X): Not implemented", value); break;
		case IORegister::InputCaptureHigh:
			LogPrintf(LOG_ERROR, "IOWrite(InputCaptureHigh, %02X): Not implemented", value); break;
		case IORegister::InputCaptureLow:
			LogPrintf(LOG_ERROR, "IOWrite(InputCaptureLow, %02X): Not implemented", value); break;
		case IORegister::RateModeControl:
			LogPrintf(LOG_ERROR, "IOWrite(RateModeControl, %02X): Not implemented", value); break;
		case IORegister::TxRxControlStatus:
			LogPrintf(LOG_ERROR, "IOWrite(TxRxControlStatus, %02X): Not implemented", value); break;
		case IORegister::RxData:
			LogPrintf(LOG_ERROR, "IOWrite(RxData, %02X): Not implemented", value); break;
		case IORegister::TxData:
			LogPrintf(LOG_ERROR, "IOWrite(TxData, %02X): Not implemented", value); break;
		case IORegister::RAMControl:
			LogPrintf(LOG_ERROR, "IOWrite(RAMControl, %02X): Not implemented", value); break;

		case IORegister::_Reserved15:
		case IORegister::_Reserved16:
		case IORegister::_Reserved17:
		case IORegister::_Reserved18:
		case IORegister::_Reserved19:
		case IORegister::_Reserved1A:
		case IORegister::_Reserved1B:
		case IORegister::_Reserved1C:
		case IORegister::_Reserved1D:
		case IORegister::_Reserved1E:
		case IORegister::_Reserved1F:
			LogPrintf(LOG_WARNING, "IOWrite(%02X, %02X): Reserved", reg, value); break;
		default:
			// Passthrough for 04-07, 0F
			return m_memory.Write8(ADDRESS(reg), value);
		}
	}
}
