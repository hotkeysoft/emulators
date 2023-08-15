#include "stdafx.h"

#include "Device6520MO5_PIA.h"
#include <IO/DeviceKeyboard.h>

namespace pia::thomson
{
	void Device6520MO5_PIA::Init(kbd::DeviceKeyboard* kbd)
	{
		Device6520::Init(true);
		SetKeyboard(kbd);
	}

	void Device6520MO5_PIA::OnReadPort(PIAPort* source)
	{
		// Keyboard data
		if (source == &m_portB)
		{
			BYTE colSel = GetKeyboardColumnSelect();
			BYTE rowSel = GetKeyboardRowSelect();

			BYTE rowData = m_keyboard->GetRowData(colSel);
			LogPrintf(LOG_DEBUG, "OnReadKeyboard, column = %d, data = %02X", colSel, rowData);

			SetSelectedKeyInput(emul::GetBit(rowData, rowSel));
		}
	}

	void Device6520MO5_PIA::OnWritePort(PIAPort* source)
	{
		if (source == &m_portA)
		{
			const ScreenRAM newMapping = GetScreenMapping();
			if (newMapping != m_screenRAM)
			{
				LogPrintf(LOG_INFO, "Screen Mapping: %s", newMapping == ScreenRAM::PIXEL ? "PIXEL" : "ATTR");
				m_screenRAM = newMapping;
				m_piaEventHandler->OnScreenMapChange(newMapping);
			}

			const BYTE newBorderRGBP = GetBorderRGBP();
			if (newBorderRGBP != m_borderRGBP)
			{
				LogPrintf(LOG_INFO, "New Border Color: %02X", newBorderRGBP);
				m_borderRGBP = newBorderRGBP;
				m_piaEventHandler->OnBorderChange(newBorderRGBP);
			}
		}
	}
}
