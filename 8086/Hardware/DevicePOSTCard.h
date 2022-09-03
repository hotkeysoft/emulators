#pragma once

#include <CPU/PortConnector.h>
#include "../CPU/CPU8086.h"
#include <Config.h>

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

		void Init(emul::CPU8086* cpu)
		{
			assert(cpu);
			m_cpu = cpu;

			EnableLog(cfg::CONFIG().GetLogLevel("post"));
			Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&DevicePOSTCard::WritePOST), true);
		}

		void WritePOST(BYTE value)
		{
			LogPrintf(LOG_INFO, "[%02x] @ [%04x:%04x]", value,
				m_cpu->GetRegValue(emul::REG16::CS),
				m_cpu->GetRegValue(emul::REG16::IP));
		}

	protected:
		WORD m_baseAddress = 0;

		emul::CPU8086* m_cpu;
	};
}

