#include "stdafx.h"
#include "DeviceLightpenThomson.h"

#include "Hardware/DevicePIAThomson.h"
#include "../Video/VideoThomson.h"

namespace mouse
{
	DeviceLightpenThomson::DeviceLightpenThomson() : Logger("lightpen")
	{

	}

	void DeviceLightpenThomson::Reset()
	{
		m_pia->SetLightPenButtonInput(false);
	}

	void DeviceLightpenThomson::SetButtonClick(int32_t x, int32_t y, int button, bool down)
	{
		LogPrintf(LOG_DEBUG, "Click(%d,%d)! %d, %d", button, down, x, y);

		if (button == 1)
		{
			m_pia->SetLightPenButtonInput(down);
		}
	}

	void DeviceLightpenThomson::SetMouseMoveAbs(int32_t x, int32_t y)
	{
		LogPrintf(LOG_TRACE, "MouseMove (%d,%d)", x, y);

		SDL_Point displayCoords = m_video->ClientToDisplayRect({ x, y });
		LogPrintf(LOG_DEBUG, "[%zu] Translated(%d,%d)", emul::g_ticks, displayCoords.x, displayCoords.y);

		m_video->SetLightPenPos(displayCoords.x, displayCoords.y);
	}
}