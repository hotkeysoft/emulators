#include "stdafx.h"
#include "TMS9918.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	// TODO: Different sizes
	TMS9918::TMS9918() : Logger("tms9918"), m_vram("vram", 0x4000)
	{
	}

	void TMS9918::Reset()
	{
		m_dataFlipFlop = false;
		m_tempData = 0;

		m_currWriteAddress = 0;
		m_currReadAddress = 0;
	}

	void TMS9918::Init(video::Video* video)
	{
		assert(video);
		m_video = video;
	}

	void TMS9918::Tick()
	{
	}

	BYTE TMS9918::ReadStatus()
	{
		// Reset data flip flop
		m_dataFlipFlop = false;

		LogPrintf(LOG_INFO, "ReadStatus");
		return 0xFF;
	}

	void TMS9918::Write(BYTE value)
	{
		if (!m_dataFlipFlop)
		{
			LogPrintf(LOG_DEBUG, "Write temp data, value = %02X", value);
			m_tempData = value;
		}
		else
		{
			LogPrintf(LOG_INFO, "Write, value = %02X", value);
		}

		m_dataFlipFlop = !m_dataFlipFlop;
	}

	BYTE TMS9918::ReadVRAMData()
	{
		LogPrintf(LOG_INFO, "ReadVRAMData, address=%04X", m_currReadAddress);

		BYTE value = m_vram.read(m_currReadAddress++);
		m_currReadAddress &= m_addressMask;
		return value;
	}

	void TMS9918::WriteVRAMData(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteVRAMData, address=%04X, value = %02X", m_currWriteAddress, value);
		m_vram.write(m_currWriteAddress++, value);
		m_currWriteAddress &= m_addressMask;
	}

	void TMS9918::Serialize(json& to)
	{
		to["dataFlipFlop"]= m_dataFlipFlop;
		to["tempData"] = m_tempData;

		to["currReadAddress"] = m_currReadAddress;
		to["currWriteAddress"] = m_currWriteAddress;
	}
	void TMS9918::Deserialize(const json& from)
	{
		m_dataFlipFlop = from["dataFlipFlop"];
		m_tempData = from["tempData"];

		m_currReadAddress = from["currReadAddress"];
		m_currWriteAddress = from["currWriteAddress"];
	}
}
