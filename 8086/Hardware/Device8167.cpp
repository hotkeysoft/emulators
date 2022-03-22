#include "stdafx.h"

#include "Device8167.h"

namespace rtc
{
	BYTE Dec2BCD(BYTE dec, BYTE clamp = 59)
	{
		dec = std::min(clamp, dec);
		return ((dec / 10) << 4) | (dec % 10);
	}

	Device8167::Device8167(WORD baseAddress) : 
		Logger("rtc"), 
		m_baseAddress(baseAddress)
	{
	}

	void Device8167::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterMilliSeconds));
		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterDeciCentiSeconds));
		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterSeconds));
		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterMinutes));
		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterHours));
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterDayOfWeek));
		Connect(m_baseAddress + 6, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterDayOfMonth));
		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&Device8167::ReadCounterMonth));

		Connect(m_baseAddress + 8, static_cast<PortConnector::INFunction>(&Device8167::ReadRAM0));
		Connect(m_baseAddress + 9, static_cast<PortConnector::INFunction>(&Device8167::ReadRAM1));

		for (int i = 10; i < 16; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::INFunction>(&Device8167::DummyRead));
		}

		for (int i = 0; i < 16; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::OUTFunction>(&Device8167::DummyWrite));
		}
	}

	void Device8167::UpdateCurrentTime()
	{
		time_t now = time(nullptr);
		m_now = *localtime(&now);
	}

	BYTE Device8167::DummyRead()
	{
		LogPrintf(LOG_WARNING, "Read unconnected port");
		return 0xFF;
	}

	void Device8167::DummyWrite(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write unconnected port, value=%02x (ignored)", value);
	}

	BYTE Device8167::ReadCounterMilliSeconds()
	{
		LogPrintf(LOG_WARNING, "ReadCounterMilliSeconds");
		return 0;
	}
	BYTE Device8167::ReadCounterDeciCentiSeconds()
	{
		LogPrintf(LOG_WARNING, "ReadCounterDeciCentiSeconds");
		UpdateCurrentTime();
		return 0;
	}
	BYTE Device8167::ReadCounterSeconds()
	{
		LogPrintf(LOG_WARNING, "ReadCounterSeconds");
		return Dec2BCD(m_now.tm_sec);
	}
	BYTE Device8167::ReadCounterMinutes()
	{
		LogPrintf(LOG_WARNING, "ReadCounterMinutes");
		return Dec2BCD(m_now.tm_min);
	}
	BYTE Device8167::ReadCounterHours()
	{
		LogPrintf(LOG_WARNING, "ReadCounterHours");
		return Dec2BCD(m_now.tm_hour);
	}
	BYTE Device8167::ReadCounterDayOfWeek()
	{
		LogPrintf(LOG_WARNING, "ReadCounterDayOfWeek");
		return m_now.tm_wday + 1;
	}
	BYTE Device8167::ReadCounterDayOfMonth()
	{
		LogPrintf(LOG_WARNING, "ReadCounterDayOfMonth");
		return Dec2BCD(m_now.tm_mday);
	}
	BYTE Device8167::ReadCounterMonth()
	{
		LogPrintf(LOG_WARNING, "ReadCounterMonth");
		return Dec2BCD(m_now.tm_mon + 1);
	}
	BYTE Device8167::ReadRAM0()
	{
		LogPrintf(LOG_WARNING, "ReadRAM0");
		return 0;
	}
	BYTE Device8167::ReadRAM1()
	{
		LogPrintf(LOG_WARNING, "ReadRAM1");

		// Time machine users, please don't run ths before the year 2000.
		// It's not 19xx compliant
		return Dec2BCD(m_now.tm_year - 100);
	}
}
