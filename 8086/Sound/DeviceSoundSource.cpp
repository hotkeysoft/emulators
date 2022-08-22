#include "stdafx.h"

#include "DeviceSoundSource.h"

namespace dss
{
	DeviceSoundSource::DeviceSoundSource(WORD baseAddress, size_t baseFrequency) :
		Logger("dss"),
		m_baseAddress(baseAddress),
		m_tickDivider(baseFrequency / FIFO_FREQUENCY)
	{
		Reset();
	}

	DeviceSoundSource::~DeviceSoundSource()
	{
	}

	void DeviceSoundSource::Reset()
	{
		m_output = 0;
		m_currData = 0;
		while (m_fifo.size())
		{
			m_fifo.pop();
		}
		m_fifoFull = false;
	}

	void DeviceSoundSource::Init()
	{
		Reset();
		LogPrintf(LOG_DEBUG, "Init(), tickDivider = %zu", m_tickDivider);

		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&DeviceSoundSource::WriteData));
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&DeviceSoundSource::ReadData));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&DeviceSoundSource::ReadStatus));

		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceSoundSource::WriteControl));
	}

	void DeviceSoundSource::Tick()
	{
		static size_t cooldown = m_tickDivider;

		if (--cooldown)
		{
			return;
		}

		cooldown = m_tickDivider;
		m_output = Pop();
	}

	WORD DeviceSoundSource::GetOutput() const
	{	
		return m_output;
	}

	void DeviceSoundSource::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteData, value=%02x", value);
		m_currData = value;
	}

	BYTE DeviceSoundSource::ReadData()
	{
		LogPrintf(LOG_DEBUG, "ReadData, currData=%02x", m_currData);
		return m_currData;
	}

	void DeviceSoundSource::WriteControl(BYTE value)
	{
		bool select = !(value & 8);
		LogPrintf(LOG_DEBUG, "WriteControl, Select=%d", select);

		if (!m_select && select)
		{
			Push(m_currData);
		}

		m_select = select;
	}

	BYTE DeviceSoundSource::ReadStatus()
	{
		BYTE value = m_fifoFull ? 0xFF : ~0x40;
		LogPrintf(LOG_DEBUG, "ReadStatus, value=%02x", value);
		return value;
	}

	void DeviceSoundSource::Push(BYTE value) 
	{ 
		m_fifoFull = m_fifo.size() == 16;
		if (m_fifoFull)
		{
			m_fifo.pop();
		}

		LogPrintf(LOG_DEBUG, "PUSH(%02x)", value);
		m_fifo.push(value);
	}

	BYTE DeviceSoundSource::Pop() 
	{
		m_fifoFull = false;
		if (!m_fifo.size()) 
		{
			return 0;
		}

		const BYTE &value = m_fifo.front();
		m_fifo.pop();
		LogPrintf(LOG_DEBUG, "POP, %02x", value);
		return value;
	}

	void DeviceSoundSource::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
	}

	void DeviceSoundSource::Deserialize(const json& from)
	{
		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("DeviceSoundSource: Incompatible baseAddress");
		}
	}
}
