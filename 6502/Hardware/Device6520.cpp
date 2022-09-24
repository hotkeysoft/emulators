#include "stdafx.h"

#include "Device6520.h"

namespace pia
{
	Device6520::Device6520(const char* id) :
		Logger(id),
		IOConnector(0x03) // Addresses 0-3 are decoded by device
	{
	}

	void Device6520::Init()
	{
		Reset();

		Connect(0x0, static_cast<IOConnector::READFunction>(&Device6520::Read0));
		Connect(0x1, static_cast<IOConnector::READFunction>(&Device6520::ReadCRA));
		Connect(0x2, static_cast<IOConnector::READFunction>(&Device6520::Read2));
		Connect(0x3, static_cast<IOConnector::READFunction>(&Device6520::ReadCRB));

		Connect(0x0, static_cast<IOConnector::WRITEFunction>(&Device6520::Write0));
		Connect(0x1, static_cast<IOConnector::WRITEFunction>(&Device6520::WriteCRA));
		Connect(0x2, static_cast<IOConnector::WRITEFunction>(&Device6520::Write2));
		Connect(0x3, static_cast<IOConnector::WRITEFunction>(&Device6520::WriteCRB));
	}

	void Device6520::Reset()
	{

	}

	// 0 - Read PIBA/DDRA
	BYTE Device6520::Read0()
	{
		LogPrintf(LOG_DEBUG, "Read0");
		return 0xFF;
	}
	BYTE Device6520::ReadPIBA()
	{
		LogPrintf(LOG_DEBUG, "ReadPIBA");
		return 0xFF;
	}
	BYTE Device6520::ReadDDRA()
	{
		LogPrintf(LOG_DEBUG, "ReadDDRA");
		return 0xFF;
	}

	// 0 - Write ORA/DDRA
	void Device6520::Write0(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write0, value=%02x", value);
	}
	void Device6520::WriteORA(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteORA, value=%02x", value);
	}
	void Device6520::WriteDDRA(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDDRA, value=%02x", value);
	}

	// 1 - Read/Write CRA
	BYTE Device6520::ReadCRA()
	{
		LogPrintf(LOG_DEBUG, "ReadCRA");
		return 0xFF;
	}
	void Device6520::WriteCRA(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteCRA, value=%02x", value);
	}

	// 2 - Read PIBB/DDRB
	BYTE Device6520::Read2()
	{
		LogPrintf(LOG_DEBUG, "Read2");
		return 0xFF;
	}
	BYTE Device6520::ReadPIBB()
	{
		LogPrintf(LOG_DEBUG, "ReadPIBB");
		return 0xFF;
	}
	BYTE Device6520::ReadDDRB()
	{
		LogPrintf(LOG_DEBUG, "ReadDDRB");
		return 0xFF;
	}


	// 2 - Write ORB/DDRB
	void Device6520::Write2(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write2, value=%02x", value);
	}
	void Device6520::WriteORB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteORB, value=%02x", value);
	}
	void Device6520::WriteDDRB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDDRB, value=%02x", value);
	}

	// 3 - Read/Write CRB
	BYTE Device6520::ReadCRB()
	{
		LogPrintf(LOG_DEBUG, "ReadCRB");
		return 0xFF;
	}
	void Device6520::WriteCRB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteCRB, value=%02x", value);
	}
}
