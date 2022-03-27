#include "stdafx.h"
#include "Device8042AT.h"
#include "CPU/CPU80286.h"

namespace ppi
{
	Device8042AT::Device8042AT(WORD baseAddress) :
		DevicePPI(baseAddress),
		Logger("8042")
	{
	}

	void Device8042AT::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8042AT::ReadBuffer));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8042AT::WriteBuffer));

		// 8255 PortB compatibility
		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8042AT::ReadPortB));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8042AT::WritePortB));

		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&Device8042AT::ReadStatus));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&Device8042AT::WriteCommand));
	}

	void Device8042AT::SetCPU(emul::CPU8086* cpu)
	{
		assert(cpu);
		// Need minimum 286 for A20 line control
		m_cpu = dynamic_cast<emul::CPU80286*>(cpu);
		if (m_cpu == nullptr)
		{
			LogPrintf(LOG_ERROR, "Fatal: Keyboard Controller needs a 286+ CPU");
			throw std::exception("Need a 286+ CPU");
		}
	}

	BYTE Device8042AT::ReadBuffer()
	{
		LogPrintf(LOG_DEBUG, "Read Buffer");
		return 0xFF;
	}

	void Device8042AT::WriteBuffer(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Buffer, value=%02x", value);
	}

	BYTE Device8042AT::ReadStatus()
	{
		LogPrintf(LOG_DEBUG, "Read Status");
		return 0xFF;
	}

	void Device8042AT::WriteCommand(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Command, value=%02x", value);
	}

	BYTE Device8042AT::ReadPortB()
	{
		LogPrintf(LOG_TRACE, "Read Port B");

		return (m_portB.refresh << 4);
	}
	void Device8042AT::WritePortB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Read Port B, value=%02x", value);
	}
}
