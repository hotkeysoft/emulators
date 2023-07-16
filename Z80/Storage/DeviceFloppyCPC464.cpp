#include "stdafx.h"

#include "DeviceFloppyCPC464.h"

namespace fdc
{
	const emul::BitMaskW StatusRegisterMask("0xxxxxx0");
	const emul::BitMaskW DataRegisterMask("0xxxxxx1");

	DeviceFloppyCPC464::DeviceFloppyCPC464(WORD baseAddress, size_t clockSpeedHz) :
		DeviceFloppy(baseAddress, clockSpeedHz),
		Logger("floppyCPC464")
	{
	}

	void DeviceFloppyCPC464::Reset()
	{
		DeviceFloppy::Reset();
	}

	void DeviceFloppyCPC464::Init()
	{
		//MAIN_STATUS_REGISTER = 0x3F4, // read-only
		Connect("xxxxx0x1", static_cast<PortConnector::INFunction>(&DeviceFloppyCPC464::Read));
		Connect("xxxxx0x1", static_cast<PortConnector::OUTFunction>(&DeviceFloppyCPC464::Write));

		//Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadMainStatusReg));

		////DATA_FIFO = 0x3F5,
		//Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadDataFIFO));
		//Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataFIFO));

		SetDiskChanged();
	}

	BYTE DeviceFloppyCPC464::Read()
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "READ, port=%04X", port);

		if (StatusRegisterMask.IsMatch(port))
			return ReadMainStatusReg();
		else if (DataRegisterMask.IsMatch(port))
			return ReadDataFIFO();
		else
			return 0xFF;
	}
	void DeviceFloppyCPC464::Write(BYTE value)
	{
		const WORD port = GetCurrentPort();
		LogPrintf(LOG_TRACE, "WRITE, port = %04X, value=%02X", GetCurrentPort(), value);

		if (DataRegisterMask.IsMatch(port))
			return WriteDataFIFO(value);
	}

}
