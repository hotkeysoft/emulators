#include "stdafx.h"

#include "Device6520.h"

namespace pia
{
	PIAPort::PIAPort(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x03) // Addresses 0-3 are decoded by device
	{
	}

	void PIAPort::Init(Device6520* parent, bool isPortB)
	{
		Reset();

		WORD base = isPortB ? 2 : 0;
		Connect(base + 0, static_cast<IOConnector::READFunction>(&PIAPort::Read0));
		Connect(base + 1, static_cast<IOConnector::READFunction>(&PIAPort::ReadCR));

		Connect(base + 0, static_cast<IOConnector::WRITEFunction>(&PIAPort::Write0));
		Connect(base + 1, static_cast<IOConnector::WRITEFunction>(&PIAPort::WriteCR));
	}

	void PIAPort::Reset()
	{
		DDR = 0;
		OR = 0;
		CR = 0;
		ISC = 0;
		C1 = false;
		C2 = false;
		IRQ = false;
	}

	// 0 - Read PIBA/DDRA
	BYTE PIAPort::Read0()
	{
		LogPrintf(LOG_DEBUG, "Read0");
		return 0xFF;
	}
	BYTE PIAPort::ReadPIB()
	{
		LogPrintf(LOG_DEBUG, "ReadPIB");
		return 0xFF;
	}
	BYTE PIAPort::ReadDDR()
	{
		LogPrintf(LOG_DEBUG, "ReadDDR");
		return 0xFF;
	}

	// 0 - Write OR/DDR
	void PIAPort::Write0(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write0, value=%02x", value);
	}
	void PIAPort::WriteOR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteOR, value=%02x", value);
	}
	void PIAPort::WriteDDR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDDR, value=%02x", value);
	}

	// 1 - Read/Write CR
	BYTE PIAPort::ReadCR()
	{
		LogPrintf(LOG_DEBUG, "ReadCR");
		return 0xFF;
	}
	void PIAPort::WriteCR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteCR, value=%02x", value);
	}

	Device6520::Device6520(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x03), // Addresses 0-3 are decoded by device
		m_portA(id + ".A"),
		m_portB(id + ".B")
	{
	}

	void Device6520::Init()
	{
		Reset();

		m_portA.Init(this, false);
		m_portB.Init(this, true);

		// Attach connections made in children objects
		Attach(m_portA);
		Attach(m_portB);
	}

	void Device6520::Reset()
	{
		m_portA.Reset();
		m_portB.Reset();
	}
}
