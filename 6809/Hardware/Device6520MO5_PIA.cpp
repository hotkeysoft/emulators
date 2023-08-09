#include "stdafx.h"

#include "Device6520MO5_PIA.h"
#include <IO/DeviceKeyboard.h>

namespace pia
{
	void Device6520MO5_PIA::Init(kbd::DeviceKeyboard* kbd)
	{
		Device6520::Init(true);

		assert(kbd);
		m_keyboard = kbd;
	}

	void Device6520MO5_PIA::OnReadPort(PIAPort* source)
	{
		// Keyboard data
		if (source == &m_portB)
		{
			BYTE colSel = GetKeyboardColumnSelect();
			BYTE rowSel = GetKeyboardRowSelect();

			BYTE rowData = m_keyboard->GetRowData(colSel);
			LogPrintf(LOG_INFO, "OnReadKeyboard, column = %d, data = %02X", colSel, rowData);

			SetSelectedKeyInput(emul::GetBit(rowData, rowSel));
		}
	}
}
