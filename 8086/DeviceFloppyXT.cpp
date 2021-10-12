#include "DeviceFloppyXT.h"
#include <assert.h>

namespace fdc
{
	DeviceFloppyXT::DeviceFloppyXT(WORD baseAddress, size_t clockSpeedHz) :
		DeviceFloppy(baseAddress, clockSpeedHz),
		Logger("floppyXT")
	{
	}

	void DeviceFloppyXT::Reset()
	{
		DeviceFloppy::Reset();

		m_dor.motor[0] = false;
		m_dor.motor[1] = false;
		m_dor.motor[2] = false;
		m_dor.motor[3] = false;

		m_dor.driveSel = 0;
	}

	void DeviceFloppyXT::Init()
	{
		DeviceFloppy::Init();

		// DIGITAL_OUTPUT_REGISTER = 0x3F2,
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceFloppyXT::WriteDigitalOutputReg));
	}

	void DeviceFloppyXT::WriteDigitalOutputReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDigitalOutputReg, value=%02X", value);

		if (!(value & DOR::RESET))
		{
			LogPrintf(LOG_INFO, "RESET");
			Reset();
			return;
		}

		m_dor.motor[0] = (value & DOR::MOTA);
		m_dor.motor[1] = (value & DOR::MOTB);
		m_dor.motor[2] = (value & DOR::MOTC);
		m_dor.motor[3] = (value & DOR::MOTD);

		for (size_t drive = 0; drive < 4; ++drive)
		{
			LogPrintf(LOG_INFO, "Drive %d Motor: %s", drive, m_dor.motor[drive] ? "ON" : "OFF");
		}

		m_enableIRQDMA = (value & DOR::IRQ);
		LogPrintf(LOG_INFO, "IRQ/DMA: %s", m_enableIRQDMA ? "Enabled" : "Disabled");

		switch (value & (DOR::DSEL1 | DOR::DSEL0))
		{
		case 0: m_dor.driveSel = 0; break;
		case DOR::DSEL0: m_dor.driveSel = 1; break;
		case DOR::DSEL1: m_dor.driveSel = 2; break;
		case DOR::DSEL1 | DOR::DSEL0: m_dor.driveSel = 3; break;
		default: throw std::exception("not possible");
		}

		LogPrintf(LOG_INFO, "Drive Select: %d", m_dor.driveSel);
	}
}
