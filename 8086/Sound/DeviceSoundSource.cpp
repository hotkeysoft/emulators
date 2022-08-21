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
	}

	void DeviceSoundSource::Init()
	{
		LogPrintf(LOG_DEBUG, "Init(), tickDivider = %zu", m_tickDivider);
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&DeviceSoundSource::WriteData));
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


	}

	WORD DeviceSoundSource::GetOutput() const
	{	
		return 0;
	}

	void DeviceSoundSource::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteData, value=%02x", value);
	}

	void DeviceSoundSource::WriteControl(BYTE value)
	{
		bool select = !(value & 8);
		LogPrintf(LOG_DEBUG, "WriteData, Select=%d", select);

		m_select = select;
	}

	BYTE DeviceSoundSource::ReadStatus()
	{
		LogPrintf(LOG_INFO, "ReadStatus");
		return 0xFF;
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
