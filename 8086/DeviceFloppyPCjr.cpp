#include "DeviceFloppyPCjr.h"
#include <assert.h>

namespace fdc
{
	DeviceFloppyPCjr::DeviceFloppyPCjr(WORD baseAddress, size_t clockSpeedHz) :
		DeviceFloppy(baseAddress, clockSpeedHz),
		Logger("floppyPCjr")
	{
	}

	void DeviceFloppyPCjr::Reset()
	{
		DeviceFloppy::Reset();

		m_dor.driveEnable = false;
		m_dor.wdEnable = false;
		m_dor.wdTrigger = false;

		m_enableIRQDMA = false;
	}

	void DeviceFloppyPCjr::Init()
	{
		DeviceFloppy::Init();

		// DIGITAL_OUTPUT_REGISTER = 0x3F2,
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceFloppyPCjr::WriteDigitalOutputReg));
	}

	void DeviceFloppyPCjr::WriteDigitalOutputReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDigitalOutputReg, value=%02X", value);

		if (!(value & DOR::FDC_RESET))
		{
			LogPrintf(LOG_INFO, "RESET");
			Reset();
			return;
		}

		m_dor.driveEnable = (value & DOR::DRIVE_ENABLE);
		LogPrintf(LOG_INFO, "Drive Motor: %s", m_dor.driveEnable ? "ON" : "OFF");

		bool lastEnable = m_dor.wdEnable;
		m_dor.wdEnable = (value & DOR::WD_ENABLE);
		LogPrintf(LOG_INFO, "Watchdog Timer: %s", m_dor.wdEnable ? "ENABLED" : "DISABLED");
		if (!lastEnable && m_dor.wdEnable)
		{
			ResetWatchdog();
		}

		bool lastTrigger = m_dor.wdTrigger;
		m_dor.wdTrigger = (value & DOR::WD_TRIGGER);
		LogPrintf(LOG_DEBUG, "Watchdog Trigger: %s", m_dor.wdTrigger ? "ON" : "OFF");

		if (lastTrigger && !m_dor.wdTrigger)
		{
			LaunchWatchdog();
		}
	}

	void DeviceFloppyPCjr::Tick()
	{
		DeviceFloppy::Tick();

		if (m_wd.active)
		{
			--m_wd.counter;

			if (m_wd.counter % 100000 == 0)	LogPrintf(LOG_DEBUG, "Watchdog [%zu]", m_wd.counter);


			if (m_wd.counter == 0)
			{
				m_wd.active = false;
				LogPrintf(LOG_INFO, "Watchdog timer expired");

				SetInterruptPending();
			}
		}
	}

	void DeviceFloppyPCjr::ResetWatchdog()
	{
		LogPrintf(LOG_INFO, "Reset Watchdog timer");

		m_wd.active = false;
		m_wd.counter = 3 * m_clockSpeed; // 3 seconds
		ClearInterrupt();
	}

	void DeviceFloppyPCjr::LaunchWatchdog()
	{
		LogPrintf(LOG_INFO, "Launch Watchdog timer");
		m_wd.active = true;
	}

}
