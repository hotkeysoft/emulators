#include "stdafx.h"

#include <IO/InputEvents.h>
#include <IO/InputEventHandler.h>
#include <IO/DeviceKeyboard.h>
#include <IO/DeviceJoystick.h>
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
		if (m_gameController)
		{
			SDL_GameControllerClose(m_gameController);
			m_gameController = nullptr;
		}
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

	void InputEvents::InitKeyboard(kbd::DeviceKeyboard* kbd)
	{
		assert(kbd);
		m_keyboard = kbd;
		m_keyMap = &m_keyboard->GetKeymap();
	}

	void InputEvents::InitJoystick(joy::DeviceJoystick* joy)
	{
		m_joystick = joy;

		if (!m_joystick)
		{
			return;
		}

		if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == 0)
		{
			LogPrintf(LOG_WARNING, "SDL Init Subsystem [Game Controller]");
			if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0)
			{
				LogPrintf(LOG_ERROR, "Error initializing game controller subsystem: %s", SDL_GetError());
			}
		}

		int nbJoystick = SDL_NumJoysticks();
		int ctrlIndex = -1;
		for (int i = 0; i < nbJoystick; ++i)
		{
			if (SDL_IsGameController(i))
			{
				ctrlIndex = i;
				break;
			}
		}

		if (ctrlIndex == -1)
		{
			LogPrintf(LOG_WARNING, "No Game Controller found");
		}
		else
		{
			LogPrintf(LOG_DEBUG, "First Game Controller found: %d", ctrlIndex);

			m_gameController = SDL_GameControllerOpen(ctrlIndex);

			if (SDL_GameControllerGetAttached(m_gameController))
			{
				m_controllerID = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(m_gameController));

				LogPrintf(LOG_INFO, "Opened Game Controller %d", ctrlIndex);
				LogPrintf(LOG_INFO, "  Name: %s", SDL_GameControllerNameForIndex(ctrlIndex));
				LogPrintf(LOG_INFO, "  ID:   %d", m_controllerID);

				m_joystick->SetConnected(0, true);

				// Receive events
				SDL_GameControllerEventState(SDL_ENABLE);
			}
			else
			{
				LogPrintf(LOG_ERROR, "Unable to open Game Controller %d", ctrlIndex);
				m_gameController = nullptr;
			}
		}
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
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				//TODO: Only 2 buttons for now
				if ((e.cbutton.which == m_controllerID) && (e.cbutton.button <= 1))
				{
					InputControllerButton(e.cbutton.button, e.cbutton.state);
				}
				break;
			case SDL_CONTROLLERAXISMOTION:
				// TODO: Only X-Y for now
				if ((e.cbutton.which == m_controllerID) && (e.caxis.axis <= 1))
				{
					InputControllerAxis(e.caxis.axis, e.caxis.value);
				}
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

	void InputEvents::InputControllerButton(uint8_t button, uint8_t state)
	{
		LogPrintf(LOG_DEBUG, "InputControllerButton: button[%d]=[%s]", button, (state == SDL_PRESSED) ? "PRESSED" : "RELEASED");
		if (m_joystick)
		{
			// TODO: Only 1 joystick for now
			m_joystick->SetButtonState(0, button, (state == SDL_PRESSED) ? true : false);
		}
	}
	void InputEvents::InputControllerAxis(uint8_t axis, int16_t value)
	{
		LogPrintf(LOG_DEBUG, "InputControllerAxis: axis[%d]=[%d]", axis, value);
		if (m_joystick)
		{
			// TODO: Only 1 joystick for now
			m_joystick->SetAxisState(0, axis, value);
		}
	}

}