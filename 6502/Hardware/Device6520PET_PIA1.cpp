#include "stdafx.h"

#include "Device6520PET_PIA1.h"
#include <IO/DeviceKeyboard.h>

namespace pia
{
	void Device6520PET_PIA1::Init(kbd::DeviceKeyboard* kbd)
	{
		Device6520::Init();

		assert(kbd);
		m_keyboard = kbd;
	}

	void Device6520PET_PIA1::OnReadPort(PIAPort* source)
	{
		// Keyboard data
		if (source == &m_portB)
		{
			BYTE column = GetKeyboardColumnSelect();
			BYTE rowData = m_keyboard->GetRowData(column);
			LogPrintf(LOG_DEBUG, "OnReadKeyboard, column = %d, data = %02X", column, rowData);

			SetKeyboardRow(rowData);
		}
	}
}
