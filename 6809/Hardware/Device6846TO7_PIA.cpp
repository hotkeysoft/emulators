#include "stdafx.h"

#include "Device6846TO7_PIA.h"
#include <IO/DeviceKeyboard.h>

namespace pia::thomson
{
	void Device6846TO7_PIA::Init(kbd::DeviceKeyboard* kbd)
	{
		Device6846::Init();
		SetKeyboard(kbd);
	}

	void Device6846TO7_PIA::OnWritePort()
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
