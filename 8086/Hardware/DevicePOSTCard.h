#pragma once

#include "../CPU/PortConnector.h"
#include "../Config.h"

namespace post
{
	class DevicePOSTCard : public PortConnector
	{
	public:
		DevicePOSTCard(WORD baseAddress) : 
			Logger("POST"), 
			m_baseAddress(baseAddress)
		{
		}

		void Init()
		{
			EnableLog(cfg::CONFIG().GetLogLevel("post"));
			Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&DevicePOSTCard::WritePOST), true);
		}

		void WritePOST(BYTE value)
		{
			LogPrintf(LOG_INFO, "[%02x]", value);
		}

	protected:
		WORD m_baseAddress = 0;
	};
}

