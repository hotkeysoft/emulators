#include "stdafx.h"

#include <IO/InputEventHandler.h>
#include "InputEvents.h"
#include "DeviceKeyboardPET2001.h"
#include <SDL.h>

namespace events
{
	// Process events at 60Hz
	InputEvents::InputEvents(size_t clockSpeedHz, size_t pollInterval) :
		Logger("INPUT"),
		m_clockSpeedHz(clockSpeedHz),
		m_pollInterval(pollInterval),
		m_cooldown(pollInterval)
	{
		assert(clockSpeedHz);
		assert(pollInterval);
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

		LogPrintf(LOG_INFO, "Clock Frequency: %zi Hz", m_clockSpeedHz);
		LogPrintf(LOG_INFO, "Poll Interval:   %zi", m_pollInterval);
		LogPrintf(LOG_INFO, "Poll Frequency:  %.2f Hz", (float)m_clockSpeedHz/(float)m_pollInterval);
	}

	void InputEvents::InitKeyboard(kbd::DeviceKeyboardPET2001* kbd)
	{
		assert(kbd);
		m_keyboard = kbd;
		m_keyMap = &m_keyboard->GetKeymap();
	}

	void InputEvents::Tick()
	{
		if (--m_cooldown)
		{
			return;
		}

		m_cooldown = m_pollInterval;

		SDL_Event e;
		while (!m_quit && SDL_PollEvent(&e))
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

			m_keyboard->InputKey(key.GetRow(), key.GetCol(), evt.state == SDL_PRESSED);
		}
		else
		{
			LogPrintf(LOG_WARNING, "Unmapped scancode %d", evt.keysym.scancode);
		}
	}
}