#include "stdafx.h"

#include "Device6522VIC_VIA2.h"
#include "../IO/DeviceKeyboard.h"

namespace via
{
	void Device6522VIC_VIA2::Init(kbd::DeviceKeyboard* kbd)
	{
		Device6522::Init();

		assert(kbd);
		m_keyboard = kbd;
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
	}
}
