#include "stdafx.h"

#include "Device6520PET_PIA1.h"
#include "../IO/DeviceKeyboardPET2001.h"

namespace pia
{
	void Device6520PET_PIA1::Init(kbd::DeviceKeyboardPET2001* kbd)
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
			BYTE row = GetKeyboardRowSelect();
			BYTE rowData = m_keyboard->GetRowData(row);
			LogPrintf(LOG_DEBUG, "OnReadKeyboard, row = %d, data = %02X", row, rowData);

			SetKeyboardRow(rowData);
		}
	}
}
