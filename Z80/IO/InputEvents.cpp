#include "stdafx.h"

#include "InputEvents.h"

#include <SDL.h>

namespace events
{
	static KeyMap s_keyMapZX80 =
	{
		{ SDL_SCANCODE_LSHIFT, Key(0xFE, 0x01) },
		{ SDL_SCANCODE_RSHIFT, Key(0xFE, 0x01) },
		{ SDL_SCANCODE_Z, Key(0xFE, 0x02) },
		{ SDL_SCANCODE_X, Key(0xFE, 0x04) },
		{ SDL_SCANCODE_C, Key(0xFE, 0x08) },
		{ SDL_SCANCODE_V, Key(0xFE, 0x10) },

		{ SDL_SCANCODE_A, Key(0xFD, 0x01) },
		{ SDL_SCANCODE_S, Key(0xFD, 0x02) },
		{ SDL_SCANCODE_D, Key(0xFD, 0x04) },
		{ SDL_SCANCODE_F, Key(0xFD, 0x08) },
		{ SDL_SCANCODE_G, Key(0xFD, 0x10) },

		{ SDL_SCANCODE_Q, Key(0xFB, 0x01) },
		{ SDL_SCANCODE_W, Key(0xFB, 0x02) },
		{ SDL_SCANCODE_E, Key(0xFB, 0x04) },
		{ SDL_SCANCODE_R, Key(0xFB, 0x08) },
		{ SDL_SCANCODE_T, Key(0xFB, 0x10) },

		{ SDL_SCANCODE_1, Key(0xF7, 0x01) },
		{ SDL_SCANCODE_2, Key(0xF7, 0x02) },
		{ SDL_SCANCODE_3, Key(0xF7, 0x04) },
		{ SDL_SCANCODE_4, Key(0xF7, 0x08) },
		{ SDL_SCANCODE_5, Key(0xF7, 0x10) },

		{ SDL_SCANCODE_0, Key(0xEF, 0x01) },
		{ SDL_SCANCODE_9, Key(0xEF, 0x02) },
		{ SDL_SCANCODE_8, Key(0xEF, 0x04) },
		{ SDL_SCANCODE_7, Key(0xEF, 0x08) },
		{ SDL_SCANCODE_6, Key(0xEF, 0x10) },

		{ SDL_SCANCODE_P, Key(0xDF, 0x01) },
		{ SDL_SCANCODE_O, Key(0xDF, 0x02) },
		{ SDL_SCANCODE_I, Key(0xDF, 0x04) },
		{ SDL_SCANCODE_U, Key(0xDF, 0x08) },
		{ SDL_SCANCODE_Y, Key(0xDF, 0x10) },

		{ SDL_SCANCODE_RETURN, Key(0xBF, 0x01) },
		{ SDL_SCANCODE_L, Key(0xBF, 0x02) },
		{ SDL_SCANCODE_K, Key(0xBF, 0x04) },
		{ SDL_SCANCODE_J, Key(0xBF, 0x08) },
		{ SDL_SCANCODE_H, Key(0xBF, 0x10) },

		{ SDL_SCANCODE_SPACE, Key(0x7F, 0x01) },
		{ SDL_SCANCODE_PERIOD, Key(0x7F, 0x02) },
		{ SDL_SCANCODE_M, Key(0x7F, 0x04) },
		{ SDL_SCANCODE_N, Key(0x7F, 0x08) },
		{ SDL_SCANCODE_B, Key(0x7F, 0x10) },
	};

	// Process events at 60Hz
	InputEvents::InputEvents(size_t clockSpeedHz) :
		Logger("INPUT"),
		m_pollRate(clockSpeedHz / 60),
		m_cooldown(m_pollRate),
		m_keyMap(&s_keyMapZX80)
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

	void InputEvents::InitKeyboard(kbd::DeviceKeyboardZX80* kbd)
	{
		assert(kbd);
		m_keyboard = kbd;
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
			// No repeat on ZX80
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