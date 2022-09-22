#include "stdafx.h"

#include "InputEvents.h"

#include <SDL.h>

namespace events
{

	// Process events at 60Hz
	InputEvents::InputEvents(size_t clockSpeedHz, size_t pollingHz) :
		Logger("INPUT"),
		m_pollRate(clockSpeedHz / pollingHz),
		m_cooldown(m_pollRate)
	{
	}

	InputEvents::~InputEvents()
	{
	}

	void InputEvents::Init()
	{
		if (SDL_WasInit(SDL_INIT_EVENTS) == 0)
		{
			LogPrintf(LOG_WARNING, "SDL Init Subsystem [Events]");
			if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0)
			{
				LogPrintf(LOG_ERROR, "Error initializing events subsystem: %s", SDL_GetError());
			}
		}
	}

	void InputEvents::Tick()
	{
		if (--m_cooldown)
		{
			return;
		}

		m_cooldown = m_pollRate;

		SDL_Event e;
		while (!m_quit && SDL_PollEvent(&e))
		{
			if (e.type < SDL_MOUSEMOTION && e.type >= SDL_JOYAXISMOTION)
			{
				bool handled = false;
				for (auto handler : m_handlers)
				{
					if (handler->HandleEvent(e))
					{
						handled = true;
						break;
					}
				}
				if (handled)
				{
					continue;
				}
			}

			switch (e.type)
			{
			case SDL_QUIT:
				m_quit = true;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				InputKey(e.key);
				break;
			default:
				break;
			}
		}
	}
	void InputEvents::InputKey(SDL_KeyboardEvent& evt)
	{
		if (evt.repeat)
		{
			return;
		}

		LogPrintf(LOG_DEBUG, "InputKey: [%s] key: %d, repeat: %d", evt.state == SDL_PRESSED ? "DOWN" : "UP", evt.keysym.scancode, evt.repeat);
		KeyMap::iterator it = m_keyMap->find(evt.keysym.scancode);
		if (it != m_keyMap->end())
		{
			Key& key = it->second;
		}
		else
		{
			LogPrintf(LOG_WARNING, "Unmapped scancode %d", evt.keysym.scancode);
		}
	}
}