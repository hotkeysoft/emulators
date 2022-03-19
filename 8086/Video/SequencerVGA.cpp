#include "stdafx.h"

#include "SequencerVGA.h"

using emul::GetBit;
using emul::SetBit;

namespace seq_vga
{
	Sequencer::Sequencer(WORD baseAddress) :
		Logger("seqVGA"),
		m_baseAddress(baseAddress)
	{
	}

	void Sequencer::Reset()
	{
	}

	void Sequencer::Init()
	{
		Reset();
	}

	void Sequencer::ConnectPorts()
	{
		Connect(m_baseAddress + 0x4, static_cast<PortConnector::INFunction>(&Sequencer::ReadAddress));
		Connect(m_baseAddress + 0x4, static_cast<PortConnector::OUTFunction>(&Sequencer::WriteAddress));

		Connect(m_baseAddress + 0x5, static_cast<PortConnector::INFunction>(&Sequencer::ReadValue));
		Connect(m_baseAddress + 0x5, static_cast<PortConnector::OUTFunction>(&Sequencer::WriteValue));
	}
	void Sequencer::DisconnectPorts()
	{
		DisconnectInput(m_baseAddress + 0x4);
		DisconnectOutput(m_baseAddress + 0x4);

		DisconnectOutput(m_baseAddress + 0x5);
		DisconnectInput(m_baseAddress + 0x5);
	}

	BYTE Sequencer::ReadAddress()
	{
		return (BYTE)m_currAddress;
	}

	void Sequencer::WriteAddress(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteAddress, value=%02Xh", value);

		switch (value & 0b1111)
		{
		case 0: m_currAddress = SequencerAddress::SEQ_RESET; break;
		case 1: m_currAddress = SequencerAddress::SEQ_CLOCKING_MODE; break;
		case 2: m_currAddress = SequencerAddress::SEQ_MAP_MASK; break;
		case 3: m_currAddress = SequencerAddress::SEQ_CHARMAP_SELECT; break;
		case 4: m_currAddress = SequencerAddress::SEQ_MEMORY_MODE; break;
		default:
			m_currAddress = SequencerAddress::SEQ_INVALID;
			LogPrintf(LOG_WARNING, "Invalid sequencer address %d", value);
			break;
		}
	}

	BYTE Sequencer::ReadValue()
	{
		switch (m_currAddress)
		{
		case SequencerAddress::SEQ_RESET:          return ReadReset();
		case SequencerAddress::SEQ_CLOCKING_MODE:  return ReadClockingMode();
		case SequencerAddress::SEQ_MAP_MASK:       return ReadMapMask();
		case SequencerAddress::SEQ_CHARMAP_SELECT: return ReadCharMapSelect();
		case SequencerAddress::SEQ_MEMORY_MODE:    return ReadMemoryMode();
		default:
			LogPrintf(LOG_WARNING, "Invalid sequencer address %d", m_currAddress);
			return 0xFF;
		}
	}


	// Dispatches value to functions below
	void Sequencer::WriteValue(BYTE value)
	{
		switch (m_currAddress)
		{
		case SequencerAddress::SEQ_RESET:          WriteReset(value);         break;
		case SequencerAddress::SEQ_CLOCKING_MODE:  WriteClockingMode(value);  break;
		case SequencerAddress::SEQ_MAP_MASK:       WriteMapMask(value);       break;
		case SequencerAddress::SEQ_CHARMAP_SELECT: WriteCharMapSelect(value); break;
		case SequencerAddress::SEQ_MEMORY_MODE:    WriteMemoryMode(value);    break;
		}
	}

	//
	BYTE Sequencer::ReadReset()
	{
		BYTE value = 
		((!m_data.reset.async) << 0) |
		((!m_data.reset.sync) << 1);

		return value;
	}

	void Sequencer::WriteReset(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteReset, value=%02Xh", value);

		m_data.reset.async = !GetBit(value, 0);
		m_data.reset.sync = !GetBit(value, 1);

		if (m_data.reset.async)
		{
			LogPrintf(Logger::LOG_INFO, "WriteReset: ASYNC RESET");
		}
		if (m_data.reset.sync)
		{
			LogPrintf(Logger::LOG_INFO, "WriteReset: SYNC RESET");
		}
	}

	BYTE Sequencer::ReadClockingMode()
	{
		BYTE value =
			(m_data.clockingMode.charWidth << 0) |
			(m_data.clockingMode.lowBandwidth << 1) |
			(m_data.clockingMode.load16 << 2) |
			(m_data.clockingMode.halfDotClock << 3) |
			(m_data.clockingMode.load32 << 4) |
			(m_data.clockingMode.screenOff << 5);
		return value;
	}

	void Sequencer::WriteClockingMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteClockingMode, value=%02Xh", value);

		m_data.clockingMode.charWidth = GetBit(value, 0) ? 8 : 9;
		m_data.clockingMode.lowBandwidth = GetBit(value, 1);
		m_data.clockingMode.load16 = GetBit(value, 2);
		m_data.clockingMode.halfDotClock = GetBit(value, 3);
		m_data.clockingMode.load32 = GetBit(value, 4);
		m_data.clockingMode.screenOff = GetBit(value, 5);

		LogPrintf(Logger::LOG_INFO, "WriteClockingMode [%cSCREENOFF %cLOAD32 %cHALFDOT %cLOAD16 %cLOWBW CHARWIDTH[%d]]",
			m_data.clockingMode.screenOff ? ' ' : '/',
			m_data.clockingMode.load32 ? ' ' : '/',
			m_data.clockingMode.halfDotClock ? ' ' : '/',
			m_data.clockingMode.load16 ? ' ' : '/',
			m_data.clockingMode.lowBandwidth ? ' ' : '/',	
			m_data.clockingMode.charWidth);

		if (m_data.clockingMode.load16)
		{
			LogPrintf(LOG_WARNING, "WriteClockingMode: load16 not implemented");
		}
		if (m_data.clockingMode.load32)
		{
			LogPrintf(LOG_WARNING, "WriteClockingMode: load32 not implemented");
		}
		if (m_data.clockingMode.screenOff)
		{
			LogPrintf(LOG_WARNING, "WriteClockingMode: screenOff not implemented");
		}

		FireChangeMode();
	}

	BYTE Sequencer::ReadMapMask()
	{
		return m_data.planeMask;
	}

	void Sequencer::WriteMapMask(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteMapMask, value=%02Xh", value);
		m_data.planeMask = value & 0x0F;

		LogPrintf(Logger::LOG_DEBUG, "WriteMapMask, Enable planes: [%c0][%c1][%c2][%c3]",
			GetBit(m_data.planeMask, 0) ? ' ' : '/',
			GetBit(m_data.planeMask, 1) ? ' ' : '/',
			GetBit(m_data.planeMask, 2) ? ' ' : '/',
			GetBit(m_data.planeMask, 3) ? ' ' : '/');
	}

	BYTE Sequencer::ReadCharMapSelect()
	{
		BYTE value =
			(GetBit(m_data.charMapSelectB, 0) << 0) |
			(GetBit(m_data.charMapSelectB, 1) << 1) |
			(GetBit(m_data.charMapSelectA, 0) << 2) |
			(GetBit(m_data.charMapSelectA, 1) << 3) |
			(GetBit(m_data.charMapSelectB, 2) << 4) |
			(GetBit(m_data.charMapSelectA, 2) << 5);
		return value;
	}

	void Sequencer::WriteCharMapSelect(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteCharMapSelect, value=%02Xh", value);

		// Char map, lower 2 bits
		m_data.charMapSelectB = value & 3;
		m_data.charMapSelectA = (value >> 2) & 3;

		// Char map Upper bit
		SetBit(m_data.charMapSelectB, 2, GetBit(value, 4));
		SetBit(m_data.charMapSelectA, 2, GetBit(value, 5));

		LogPrintf(Logger::LOG_INFO, "WriteCharMapSelect, charMapA[%d] charMapB[%d]", m_data.charMapSelectA, m_data.charMapSelectB);

		if (m_data.charMapSelectA != m_data.charMapSelectB)
		{
			LogPrintf(Logger::LOG_WARNING, "WriteCharMapSelect, Not supported: charMapA[%d] != charMapB[%d]", m_data.charMapSelectA, m_data.charMapSelectB);
		}
	}

	BYTE Sequencer::ReadMemoryMode()
	{
		BYTE value =
			(m_data.memoryMode.extMemory << 1) |
			(!m_data.memoryMode.oddEven << 2) |
			(m_data.memoryMode.chain4 << 3);
		return value;
	}

	void Sequencer::WriteMemoryMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteMemoryMode, value=%02Xh", value);

		m_data.memoryMode.extMemory = GetBit(value, 1);
		m_data.memoryMode.oddEven = !GetBit(value, 2);
		m_data.memoryMode.chain4 = GetBit(value, 3);

		LogPrintf(Logger::LOG_INFO, "WriteMemoryMode [%cCHAIN4 %cODDEVEN %cEXTRAM]",
			m_data.memoryMode.chain4 ? ' ' : '/',
			m_data.memoryMode.oddEven ? ' ' : '/',
			m_data.memoryMode.extMemory ? ' ' : '/');
	}

	void Sequencer::Serialize(json& to)
	{
		to["currAddress"] = m_currAddress;

		to["clk.charWidth"] = m_data.clockingMode.charWidth;
		to["clk.lowBandwidth"] = m_data.clockingMode.lowBandwidth;
		to["clk.load16"] = m_data.clockingMode.load16;
		to["clk.halfDotClock"] = m_data.clockingMode.halfDotClock;
		to["clk.load32"] = m_data.clockingMode.load32;
		to["clk.screenOff"] = m_data.clockingMode.screenOff;
		to["planeMask"] = m_data.planeMask;
		to["charMapSelectA"] = m_data.charMapSelectA;
		to["charMapSelectB"] = m_data.charMapSelectB;
		to["mem.extMemory"] = m_data.memoryMode.extMemory;
		to["mem.oddEven"] = m_data.memoryMode.oddEven;
		to["mem.chain4"] = m_data.memoryMode.chain4;
	}

	void Sequencer::Deserialize(const json& from)
	{
		m_currAddress = from["currAddress"];

		m_data.clockingMode.charWidth = from["clk.charWidth"];
		m_data.clockingMode.lowBandwidth = from["clk.lowBandwidth"];
		m_data.clockingMode.load16 = from["clk.load16"];
		m_data.clockingMode.halfDotClock = from["clk.halfDotClock"];
		m_data.clockingMode.load32 = from["clk.load32"];
		m_data.clockingMode.screenOff = from["clk.screenOff"];
		m_data.planeMask = from["planeMask"];
		m_data.charMapSelectA = from["charMapSelectA"];
		m_data.charMapSelectB = from["charMapSelectB"];
		m_data.memoryMode.extMemory = from["mem.extMemory"];
		m_data.memoryMode.oddEven = from["mem.oddEven"];
		m_data.memoryMode.chain4 = from["mem.chain4"];
	}
}