#include "stdafx.h"

#include "Device6522VIC_VIA1.h"
#include <IO/DeviceJoystickDigital.h>

namespace via
{
	void Device6522VIC_VIA1::Init(joy::DeviceJoystick* joy)
	{
		Device6522::Init();

		m_joystick = dynamic_cast<joy::DeviceJoystickDigital*>(joy);
		assert(m_joystick);
	}

	void Device6522VIC_VIA1::OnReadPort(VIAPort* source)
	{
		// Joystick input
		if (source == &m_portA)
		{
			m_portA.SetInputBit(2, !m_joystick->GetUp());
			m_portA.SetInputBit(3, !m_joystick->GetDown());
			m_portA.SetInputBit(4, !m_joystick->GetLeft());
			m_portA.SetInputBit(5, !m_joystick->GetFire());
		}
	}
}
