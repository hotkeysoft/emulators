#include "Device8255.h"

namespace emul
{
	Device8255::Device8255(WORD baseAddress) : Logger("8255"), m_baseAddress(baseAddress)
	{
		Reset();
	}

	void Device8255::Reset()
	{

	}

	void Device8255::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8255::PORTA_IN));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8255::PORTA_OUT));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8255::PORTB_IN));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8255::PORTB_OUT));

		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&Device8255::PORTC_IN));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&Device8255::PORTC_OUT));

		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&Device8255::CONTROL_IN));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device8255::CONTROL_OUT));

	}

	BYTE Device8255::PORTA_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTA IN");
		return 0;
	}
	void Device8255::PORTA_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTA OUT, value=%02X", value);
	}

	BYTE Device8255::PORTB_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTB IN");
		return 0;
	}
	void Device8255::PORTB_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTB OUT, value=%02X", value);
	}

	BYTE Device8255::PORTC_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTC IN");
		return 0;
	}
	void Device8255::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTC OUT, value=%02X", value);
	}

	BYTE Device8255::CONTROL_IN()
	{
		LogPrintf(LOG_DEBUG, "CONTROL IN");
		return 0;
	}
	void Device8255::CONTROL_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "CONTROL OUT, value=%02X", value);
	}

}
