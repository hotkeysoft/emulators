#pragma once

namespace pia::thomson
{
	enum class ScreenRAM { PIXEL = 1, ATTR = 0, UNINITIALIZED = -1 };

	class EventHandler
	{
	public:
		virtual void OnScreenMapChange(ScreenRAM map) {}
		virtual void OnBorderChange(BYTE borderRGBP) {}
	};
}
