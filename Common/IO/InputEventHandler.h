#pragma once

#include <SDL.h>

namespace events
{
	class EventHandler
	{
	public:
		virtual bool HandleEvent(SDL_Event& e) = 0;
	};
}
