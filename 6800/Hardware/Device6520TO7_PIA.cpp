#include "stdafx.h"

#include "Device6520TO7_PIA.h"
#include <IO/DeviceKeyboard.h>

namespace pia::thomson
{
	void Device6520TO7_PIA::Init(kbd::DeviceKeyboard* kbd)
	{
		Device6520::Init(true);
		SetKeyboard(kbd);
	}

	void Device6520TO7_PIA::OnReadPort(PIAPort* source)
	{
		// Keyboard data
		if (source == &m_portA)
		{
			BYTE colSel = GetKeyboardColumnSelect();

			BYTE rowData = m_keyboard->GetRowData(colSel);
			LogPrintf(LOG_DEBUG, "OnReadKeyboard, column = %d, data = %02X", colSel, rowData);

			SetKeyboardRowData(rowData);
		}
	}
}
