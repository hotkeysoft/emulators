#include "stdafx.h"
#include "DeviceMouseMac.h"

#include "Hardware/Device6522Mac.h"
#include "Hardware/Device8530.h"

using scc::SCCChannel;

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
		LogPrintf(LOG_INFO, "Click(%d,%d)! %d, %d", button, down, x, y);

		if (button == 1)
		{
			m_via->SetMouseSwitchPressed(down);
		}
		else if (button == 3) // TODO:temp
		{
			SCCChannel& ch = m_scc->GetChannelA();
			static bool dcd = ch.GetDCD();
			dcd = !dcd;
			ch.SetDCD(dcd);

			m_via->SetMouseX2(dcd);
		}
	}

	void DeviceMouseMac::SetMouseMoveRel(int32_t x, int32_t y)
	{
		LogPrintf(LOG_INFO, "MouseMoveRel (%d,%d)", x, y);
		if (x)
		{
			SCCChannel& ch = m_scc->GetChannelA();
			static bool dcd = ch.GetDCD();
			dcd = !dcd;
			ch.SetDCD(dcd);

			m_via->SetMouseX2(x > 0 ? dcd : !dcd);
		}

		if (y)
		{
			SCCChannel& ch = m_scc->GetChannelB();
			static bool dcd = ch.GetDCD();
			dcd = !dcd;
			ch.SetDCD(dcd);

			m_via->SetMouseY2(y < 0 ? dcd : !dcd);
		}
	}
}
