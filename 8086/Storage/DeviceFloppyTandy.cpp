#include "DeviceFloppyTandy.h"
#include <assert.h>

namespace fdc
{
	DeviceFloppyTandy::DeviceFloppyTandy(WORD baseAddress, size_t clockSpeedHz) :
		DeviceFloppy(baseAddress, clockSpeedHz),
		Logger("floppyTandy")
	{
	}

	void DeviceFloppyTandy::Reset()
	{
		DeviceFloppy::Reset();

		m_dor.motor[0] = false;
		m_dor.motor[1] = false;

		m_dor.driveSel = 0;
		m_dor.terminalCount = false;
	}

	void DeviceFloppyTandy::Init()
	{
		DeviceFloppy::Init();

		// DIGITAL_OUTPUT_REGISTER = 0x3F0-0x3F3,
		for (int i = 0; i < 4; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::OUTFunction>(&DeviceFloppyTandy::WriteDigitalOutputReg));
		}
	}

	void DeviceFloppyTandy::WriteDigitalOutputReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDigitalOutputReg, value=%02X", value);

		if (!(value & DOR::RESET))
		{
			LogPrintf(LOG_DEBUG, "RESET");
			Reset();
			return;
		}

		m_dor.motor[0] = (value & DOR::MOTA);
		m_dor.motor[1] = (value & DOR::MOTB);

		for (size_t drive = 0; drive < 2; ++drive)
		{
			LogPrintf(LOG_DEBUG, "Drive %d Motor: %s", drive, m_dor.motor[drive] ? "ON" : "OFF");
		}

		m_dor.irqDMAEnabled = (value & DOR::IRQ);
		LogPrintf(LOG_DEBUG, "IRQ/DMA: %s", m_dor.irqDMAEnabled ? "Enabled" : "Disabled");

		switch (value & (DOR::DSEL1 | DOR::DSEL0))
		{
		case 0: m_dor.driveSel = 0; break;
		case DOR::DSEL0: m_dor.driveSel = 1; break;
		case DOR::DSEL1: m_dor.driveSel = 2; break;
		case DOR::DSEL1 | DOR::DSEL0: m_dor.driveSel = 3; break;
		default: throw std::exception("not possible");
		}

		LogPrintf(LOG_DEBUG, "Drive Select: %d", m_dor.driveSel);

		bool lastTerminalCount = m_dor.terminalCount;
		m_dor.terminalCount = (value & DOR::TC);
		LogPrintf(LOG_DEBUG, "Terminal Count: %d", m_dor.terminalCount);
		if (!lastTerminalCount && m_dor.terminalCount)
		{
			DMATerminalCount();
		}
	}
}
