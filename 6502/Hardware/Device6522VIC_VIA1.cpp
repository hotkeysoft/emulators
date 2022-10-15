#include "stdafx.h"

#include "Device6522VIC_VIA2.h"
#include "../IO/DeviceKeyboard.h"
#include "../IO/DeviceJoystickDigital.h"

namespace via
{
	void Device6522VIC_VIA2::Init(kbd::DeviceKeyboard* kbd, joy::DeviceJoystick* joy)
	{
		Device6522::Init();

		assert(kbd);
		m_keyboard = kbd;

		m_joystick = dynamic_cast<joy::DeviceJoystickDigital*>(joy);
		assert(joy);
	}

	void Device6522VIC_VIA2::OnReadPort(VIAPort* source)
	{
		// Keyboard data
		if (source == &m_portA)
		{
			BYTE column = GetKeyboardColumnSelect();
			BYTE rowData = m_keyboard->GetRowData(column);
			LogPrintf(LOG_DEBUG, "OnReadKeyboard, column = %d, data = %02X", column, rowData);

			SetKeyboardRow(rowData);
		}
		else // Normally keyboard column output, PB7 doubles as JOY3 (right) input
		{
			m_portB.SetInputBit(7, !m_joystick->GetRight());
		}
	}
}
