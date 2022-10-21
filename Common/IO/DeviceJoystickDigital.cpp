#include "stdafx.h"
#include <IO/DeviceJoystickDigital.h>
#include <Config.h>
#include <algorithm>

using cfg::CONFIG;

namespace joy
{
	void DeviceJoystickDigital::Init()
	{
		DeviceJoystick::Init();

		uint32_t threshold = CONFIG().GetValueInt32("joystick", "threshold", 10000);
		m_threshold = std::clamp(threshold, 0U, 32767U);
		LogPrintf(LOG_INFO, "Threshold for digital inputs: %d", m_threshold);
	}
}