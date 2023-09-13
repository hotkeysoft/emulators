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

	void DeviceMouseMac::SetMouseMoveRel(int32_t dx, int32_t dy)
	{
		LogPrintf(LOG_INFO, "MouseMoveRel (%d,%d)", dx, dy);

		PlotLine(0, 0, dx, dy);
	}

	void DeviceMouseMac::PlotLine(int x0, int y0, int x1, int y1)
	{
		int dx = abs(x1 - x0);
		int sx = (x0 < x1) ? 1 : -1;

		int dy = -abs(y1 - y0);
		int sy = (y0 < y1) ? 1 : -1;

		int error = dx + dy;

		while (true)
		{
			MouseData data;

			LogPrintf(LOG_DEBUG, "MouseMove: (%d, %d)", x0, y0);

			if ((x0 == x1) && (y0 == y1))
			{
				break;
			}
			int e2 = 2 * error;
			if (e2 >= dy)
			{
				if (x0 == x1)
				{
					break;
				}
				error += dy;
				x0 += sx;
				data.dx = sx;
			}
			if (e2 <= dx)
			{
				if (y0 == y1)
				{
					break;
				}
				error += dx;
				y0 += sy;
				data.dy = sy;
			}
			m_mouseQueue.push_back(data);
		}
	}

	void DeviceMouseMac::Tick()
	{
		constexpr static int COOLDOWN = 10000;
		static int cooldown = COOLDOWN;

		if (--cooldown == 0)
		{
			cooldown = COOLDOWN;

			if (!m_mouseQueue.empty())
			{
				const MouseData& data = m_mouseQueue.front();

				m_mouseQueue.pop_front();

				if (data.dx)
				{
					SCCChannel& ch = m_scc->GetChannelA();
					bool dcd = ch.GetDCD();
					dcd = !dcd;
					ch.SetDCD(dcd);

					m_via->SetMouseX2(data.dx > 0 ? dcd : !dcd);
				}

				if (data.dy)
				{
					SCCChannel& ch = m_scc->GetChannelB();
					bool dcd = ch.GetDCD();
					dcd = !dcd;
					ch.SetDCD(dcd);

					m_via->SetMouseY2(data.dy < 0 ? dcd : !dcd);
				}
			}
		}
	}
}
