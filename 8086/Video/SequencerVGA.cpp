#include "stdafx.h"

#include "SequencerVGA.h"

using emul::GetBit;

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
		Connect(m_baseAddress + 0x4, static_cast<PortConnector::OUTFunction>(&Sequencer::WriteAddress));
		Connect(m_baseAddress + 0x5, static_cast<PortConnector::OUTFunction>(&Sequencer::WriteValue));
	}
	void Sequencer::DisconnectPorts()
	{
		DisconnectOutput(m_baseAddress + 0x4);
		DisconnectOutput(m_baseAddress + 0x5);
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
	void Sequencer::WriteReset(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteReset, value=%02Xh", value);

		if (!GetBit(value, 0))
		{
			LogPrintf(Logger::LOG_INFO, "WriteReset: ASYNC RESET");
		}
		if (!GetBit(value, 1))
		{
			LogPrintf(Logger::LOG_INFO, "WriteReset: SYNC RESET");
		}
	}

	void Sequencer::WriteClockingMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteClockingMode, value=%02Xh", value);

		m_data.clockingMode.charWidth = GetBit(value, 0) ? 8 : 9;
		m_data.clockingMode.lowBandwidth = GetBit(value, 1);
		m_data.clockingMode.load16 = GetBit(value, 2);
		m_data.clockingMode.halfDotClock = GetBit(value, 3);

		LogPrintf(Logger::LOG_INFO, "WriteClockingMode [%cLOWBW %cLOAD16 %cHALFDOT CHARWIDTH[%d]]",
			m_data.clockingMode.lowBandwidth ? ' ' : '/',
			m_data.clockingMode.load16 ? ' ' : '/',
			m_data.clockingMode.halfDotClock ? ' ' : '/',
			m_data.clockingMode.charWidth);
	}

	void Sequencer::WriteMapMask(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteMapMask, value=%02Xh", value);
		m_data.planeMask = value & 0x0F;
		
		//TODO
		//egaRAM.SetPlaneMask(m_planeMask);

		LogPrintf(Logger::LOG_DEBUG, "WriteMapMask, Enable planes: [%c0][%c1][%c2][%c3]",
			GetBit(m_data.planeMask, 0) ? ' ' : '/',
			GetBit(m_data.planeMask, 1) ? ' ' : '/',
			GetBit(m_data.planeMask, 2) ? ' ' : '/',
			GetBit(m_data.planeMask, 3) ? ' ' : '/');
	}

	void Sequencer::WriteCharMapSelect(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteCharMapSelect, value=%02Xh", value);

		m_data.charMapSelectB = value & 3;
		m_data.charMapSelectA = (value >> 2) & 3;

		LogPrintf(Logger::LOG_INFO, "WriteCharMapSelect, charMapA[%d] charMapB[%d]", m_data.charMapSelectA, m_data.charMapSelectB);

		if (m_data.charMapSelectA != m_data.charMapSelectB)
		{
			LogPrintf(Logger::LOG_WARNING, "WriteCharMapSelect, Not supported: charMapA[%d] != charMapB[%d]", m_data.charMapSelectA, m_data.charMapSelectB);
		}
	}

	void Sequencer::WriteMemoryMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteMemoryMode, value=%02Xh", value);

		m_data.memoryMode.alpha = GetBit(value, 0);
		m_data.memoryMode.extMemory = GetBit(value, 1);
		m_data.memoryMode.oddEven = !GetBit(value, 2);

		LogPrintf(Logger::LOG_INFO, "WriteMemoryMode [%cALPHA %cEXTRAM %cODDEVEN]",
			m_data.memoryMode.alpha ? ' ' : '/',
			m_data.memoryMode.extMemory ? ' ' : '/',
			m_data.memoryMode.oddEven ? ' ' : '/');
	}

	void Sequencer::Serialize(json& to)
	{
		to["currAddress"] = m_currAddress;

		to["clk.charWidth"] = m_data.clockingMode.charWidth;
		to["clk.lowBandwidth"] = m_data.clockingMode.lowBandwidth;
		to["clk.load16"] = m_data.clockingMode.load16;
		to["clk.halfDotClock"] = m_data.clockingMode.halfDotClock;
		to["planeMask"] = m_data.planeMask;
		to["charMapSelectA"] = m_data.charMapSelectA;
		to["charMapSelectB"] = m_data.charMapSelectB;
		to["mem.alpha"] = m_data.memoryMode.alpha;
		to["mem.extMemory"] = m_data.memoryMode.extMemory;
		to["mem.oddEven"] = m_data.memoryMode.oddEven;
	}

	void Sequencer::Deserialize(const json& from)
	{
		m_currAddress = from["currAddress"];

		m_data.clockingMode.charWidth = from["clk.charWidth"];
		m_data.clockingMode.lowBandwidth = from["clk.lowBandwidth"];
		m_data.clockingMode.load16 = from["clk.load16"];
		m_data.clockingMode.halfDotClock = from["clk.halfDotClock"];
		m_data.planeMask = from["planeMask"];
		m_data.charMapSelectA = from["charMapSelectA"];
		m_data.charMapSelectB = from["charMapSelectB"];
		m_data.memoryMode.alpha = from["mem.alpha"];
		m_data.memoryMode.extMemory = from["mem.extMemory"];
		m_data.memoryMode.oddEven = from["mem.oddEven"];
	}
}