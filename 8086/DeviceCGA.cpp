#include "DeviceCGA.h"

namespace cga
{
	DeviceCGA::DeviceCGA(WORD baseAddress) :
		Logger("CGA"),
		m_baseAddress(baseAddress)
	{
		Reset();
	}

	void DeviceCGA::Reset()
	{

	}

	void DeviceCGA::Init()
	{
		for (WORD i = 0; i < 16; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::INFunction>(&DeviceCGA::IN));
			Connect(m_baseAddress + i, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		}
	}

	BYTE DeviceCGA::IN()
	{
		LogPrintf(Logger::LOG_DEBUG, "IN");
		return 0;
	}
	void DeviceCGA::OUT(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "OUT");
	}

	void DeviceCGA::Tick()
	{
	}
}
