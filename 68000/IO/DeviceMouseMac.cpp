#include "stdafx.h"
#include "DeviceMouseMac.h"

#include "Hardware/Device6522Mac.h"

namespace mouse::mac
{
	DeviceMouseMac::DeviceMouseMac() : Logger("mouse")
	{
	}

	void DeviceMouseMac::Reset()
	{
		if (m_via)
		{
			m_via->SetMouseSwitchPressed(false);
		}
	}

	void DeviceMouseMac::SetButtonClick(int32_t x, int32_t y, int button, bool down)
	{
		LogPrintf(LOG_DEBUG, "Click(%d,%d)! %d, %d", button, down, x, y);

		if (button == 1)
		{
			m_via->SetMouseSwitchPressed(down);
		}
	}

	void DeviceMouseMac::SetMouseMoveRel(int32_t x, int32_t y)
	{
		LogPrintf(LOG_INFO, "MouseMoveRel (%d,%d)", x, y);
	}
}
