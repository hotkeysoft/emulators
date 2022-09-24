#include "stdafx.h"

#include "Device6522.h"

namespace via
{
	Device6522::Device6522(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x0F) // Addresses 0-F are decoded by device
	{
	}

	void Device6522::Init()
	{
		Reset();

		Connect(0x0, static_cast<IOConnector::READFunction>(&Device6522::ReadIRB));
		Connect(0x1, static_cast<IOConnector::READFunction>(&Device6522::ReadIRA));
		Connect(0x2, static_cast<IOConnector::READFunction>(&Device6522::ReadDDRB));
		Connect(0x3, static_cast<IOConnector::READFunction>(&Device6522::ReadDDRA));
		Connect(0x4, static_cast<IOConnector::READFunction>(&Device6522::ReadT1CounterL));
		Connect(0x5, static_cast<IOConnector::READFunction>(&Device6522::ReadT1CounterH));
		Connect(0x6, static_cast<IOConnector::READFunction>(&Device6522::ReadT1LatchesL));
		Connect(0x7, static_cast<IOConnector::READFunction>(&Device6522::ReadT1LatchesH));
		Connect(0x8, static_cast<IOConnector::READFunction>(&Device6522::ReadT2CounterL));
		Connect(0x9, static_cast<IOConnector::READFunction>(&Device6522::ReadT2CounterH));
		Connect(0xA, static_cast<IOConnector::READFunction>(&Device6522::ReadSR));
		Connect(0xB, static_cast<IOConnector::READFunction>(&Device6522::ReadACR));
		Connect(0xC, static_cast<IOConnector::READFunction>(&Device6522::ReadPCR));
		Connect(0xD, static_cast<IOConnector::READFunction>(&Device6522::ReadIFR));
		Connect(0xE, static_cast<IOConnector::READFunction>(&Device6522::ReadIER));
		Connect(0xF, static_cast<IOConnector::READFunction>(&Device6522::ReadIRANoHandshake));

		Connect(0x0, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteORB));
		Connect(0x1, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteORA));
		Connect(0x2, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteDDRB));
		Connect(0x3, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteDDRA));
		Connect(0x4, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchesL));
		Connect(0x5, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1CounterH));
		Connect(0x6, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchesL));
		Connect(0x7, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchesH));
		Connect(0x8, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT2LatchesL));
		Connect(0x9, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT2CounterH));
		Connect(0xA, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteSR));
		Connect(0xB, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteACR));
		Connect(0xC, static_cast<IOConnector::WRITEFunction>(&Device6522::WritePCR));
		Connect(0xD, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteIFR));
		Connect(0xE, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteIER));
		Connect(0xF, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteORANoHandshake));
	}

	void Device6522::Reset()
	{

	}

	// 0 - ORB/IRB: Output/Input Register B
	BYTE Device6522::ReadIRB()
	{
		LogPrintf(LOG_DEBUG, "ReadIRB");
		return 0xFF;
	}
	void Device6522::WriteORB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteORB, value=%02X", value);
	}

	// 1 - ORA/IRA: Output/Input Register A
	BYTE Device6522::ReadIRA()
	{
		LogPrintf(LOG_DEBUG, "ReadIRA");
		return 0xFF;
	}
	void Device6522::WriteORA(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteORA, value=%02X", value);
	}

	// 2 - DDRB: Data Direction Register B
	BYTE Device6522::ReadDDRB()
	{
		LogPrintf(LOG_DEBUG, "ReadDDRB");
		return 0xFF;
	}
	void Device6522::WriteDDRB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDDRB, value=%02X", value);
	}

	// 3 - DDRA: Data Direction Register A
	BYTE Device6522::ReadDDRA()
	{
		LogPrintf(LOG_DEBUG, "ReadDDRA");
		return 0xFF;
	}
	void Device6522::WriteDDRA(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDDRA, value=%02X", value);
	}

	// 4 - T1C-L: T1 Low-Order Counter
	BYTE Device6522::ReadT1CounterL()
	{
		LogPrintf(LOG_DEBUG, "ReadT1CounterL");
		return 0xFF;
	}

	// 5 - T1C-H: T1 High-Order Counter
	BYTE Device6522::ReadT1CounterH()
	{
		LogPrintf(LOG_DEBUG, "ReadT1CounterH");
		return 0xFF;
	}
	void Device6522::WriteT1CounterH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1CounterH, value=%02X", value);
	}

	// 6 - T1L-L: T1 Low-Order Latches
	BYTE Device6522::ReadT1LatchesL()
	{
		LogPrintf(LOG_DEBUG, "ReadT1LatchesL");
		return 0xFF;
	}
	void Device6522::WriteT1LatchesL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1LatchesL, value=%02X", value);
	}

	// 7 - T1L-H: T1 High-Order Latches
	BYTE Device6522::ReadT1LatchesH()
	{
		LogPrintf(LOG_DEBUG, "ReadT1LatchesH");
		return 0xFF;
	}
	void Device6522::WriteT1LatchesH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1LatchesH, value=%02X", value);
	}

	// 8 - T2C-L: T2 Low-Order Counter
	BYTE Device6522::ReadT2CounterL()
	{
		LogPrintf(LOG_DEBUG, "ReadT2CounterL");
		return 0xFF;
	}
	void Device6522::WriteT2LatchesL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2LatchesL, value=%02X", value);
	}

	// 9 - T2C-H: T2 High-Order Counter
	BYTE Device6522::ReadT2CounterH()
	{
		LogPrintf(LOG_DEBUG, "ReadT2CounterH");
		return 0xFF;
	}
	void Device6522::WriteT2CounterH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2CounterH, value=%02X", value);
	}

	// A - SR: Shift Register
	BYTE Device6522::ReadSR()
	{
		LogPrintf(LOG_DEBUG, "ReadSR");
		return 0xFF;
	}
	void Device6522::WriteSR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteSR, value=%02X", value);
	}

	// B - ACR: Auxiliary Control Register
	BYTE Device6522::ReadACR()
	{
		LogPrintf(LOG_DEBUG, "ReadACR");
		return 0xFF;
	}
	void Device6522::WriteACR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteACR, value=%02X", value);
	}

	// C - PCR: Peripheral Control Register
	BYTE Device6522::ReadPCR()
	{
		LogPrintf(LOG_DEBUG, "ReadPCR");
		return 0xFF;
	}
	void Device6522::WritePCR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WritePCR, value=%02X", value);
	}

	// D - IFR: Interrupt Flag Register
	BYTE Device6522::ReadIFR()
	{
		LogPrintf(LOG_DEBUG, "ReadIFR");
		return 0xFF;
	}
	void Device6522::WriteIFR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIFR, value=%02X", value);
}

	// E - IER: Interrupt Enable Register
	BYTE Device6522::ReadIER()
	{
		LogPrintf(LOG_DEBUG, "ReadIER");
		return 0xFF;
	}
	void Device6522::WriteIER(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIER, value=%02X", value);
	}

	// F - ORA/IRA: Same as reg 1 with no handshake
	BYTE Device6522::ReadIRANoHandshake()
	{
		LogPrintf(LOG_DEBUG, "ReadIRANoHandshake");
		return 0xFF;
	}
	void Device6522::WriteORANoHandshake(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteORANoHandshake, value=%02X", value);
	}
}
