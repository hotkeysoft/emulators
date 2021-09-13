#include "DeviceCGA.h"

namespace cga
{
	DeviceCGA::DeviceCGA(WORD baseAddress) :
		Logger("CGA"),
		m_baseAddress(baseAddress),
		m_hPos(0),
		m_vPos(0),
		m_screenB800("CGA", 0x4000, emul::MemoryType::RAM)
	{
		Reset();
	}

	void DeviceCGA::Reset()
	{
		m_hPos = 0;
		m_vPos = 0;
	}

	void DeviceCGA::Init()
	{
		// Register Select
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		// Register Value
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));


		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		Connect(m_baseAddress + 8, static_cast<PortConnector::INFunction>(&DeviceCGA::IN));

		// Color Control Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		Connect(m_baseAddress + 9, static_cast<PortConnector::INFunction>(&DeviceCGA::IN));


		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadStatus));
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

	BYTE DeviceCGA::ReadStatus()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatus, hSync=%d, vSync=%d", IsHSync(), IsVSync());
		return (IsHSync() ? 1 : 0) | (IsVSync() ? 8 : 0);
	}

	void DeviceCGA::Tick()
	{
		// Fake sync pulses
		m_hPos += 8;
		if (m_hPos > 448)
		{
			m_hPos = 0;
			++m_vPos;
		}
		if (m_vPos > 256)
		{
			m_vPos = 0;
		}
	}
}
