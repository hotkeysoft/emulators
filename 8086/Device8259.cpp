#include "Device8259.h"

namespace pic
{
	Device8259::Device8259(WORD baseAddress) :
		Logger("pic"),
		m_baseAddress(baseAddress)
	{
		Reset();
	}

	void Device8259::Reset()
	{

	}

	void Device8259::Init()
	{
		Connect(m_baseAddress, static_cast<PortConnector::INFunction>(&Device8259::IN));
		Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&Device8259::OUT));

		Connect(m_baseAddress+1, static_cast<PortConnector::INFunction>(&Device8259::Mask_IN));
		Connect(m_baseAddress+1, static_cast<PortConnector::OUTFunction>(&Device8259::Mask_OUT));
	}

	BYTE Device8259::IN()
	{
		LogPrintf(LOG_DEBUG, "IN");
		return 0;
	}
	void Device8259::OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "OUT, value=%02X", value);
	}

	BYTE Device8259::Mask_IN()
	{
		LogPrintf(LOG_INFO, "GET mask=%02X", m_mask);
		return m_mask;
	}
	void Device8259::Mask_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "PUT mask=%02X", value);
		m_mask = value;
	}

	void Device8259::Tick()
	{
	}
}
