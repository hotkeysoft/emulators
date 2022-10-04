#include "stdafx.h"

#include <IO/InputEventHandler.h>
#include "InputEvents.h"

#include <SDL.h>

namespace events
{
	static KeyMap s_keyMapZX80 =
	{
		// Row 0
		{ SDL_SCANCODE_1, Key(0, 0x01) }, // !
		{ SDL_SCANCODE_3, Key(0, 0x02) }, // #
		{ SDL_SCANCODE_5, Key(0, 0x04) }, // %
		{ SDL_SCANCODE_7, Key(0, 0x08) }, // &
		{ SDL_SCANCODE_9, Key(0, 0x10) }, // (
		{ SDL_SCANCODE_MINUS, Key(0, 0x20) }, // <-
		{ SDL_SCANCODE_HOME, Key(0, 0x40) }, // CLR HOME
		{ SDL_SCANCODE_RIGHT, Key(0, 0x80) }, // CRSR LEFT RIGHT

		{ SDL_SCANCODE_2, Key(1, 0x01) }, // "
		{ SDL_SCANCODE_4, Key(1, 0x02) }, // $
		{ SDL_SCANCODE_6, Key(1, 0x04) }, // '
		{ SDL_SCANCODE_8, Key(1, 0x08) }, // Backslash
		{ SDL_SCANCODE_0, Key(1, 0x10) }, // )
		//{ , Key(1, 0x20) }, // NC
		{ SDL_SCANCODE_DOWN, Key(1, 0x40) }, // CRSR UP DOWN
		{ SDL_SCANCODE_BACKSPACE, Key(1, 0x80) }, // DEL

		{ SDL_SCANCODE_Q, Key(2, 0x01) }, // q
		{ SDL_SCANCODE_E, Key(2, 0x02) }, // e
		{ SDL_SCANCODE_T, Key(2, 0x04) }, // t
		{ SDL_SCANCODE_U, Key(2, 0x08) }, // u
		{ SDL_SCANCODE_O, Key(2, 0x10) }, // o
		{ SDL_SCANCODE_BACKSLASH, Key(2, 0x20) }, // PI/EXP
		{ SDL_SCANCODE_KP_7, Key(2, 0x40) }, // Keypad 7
		{ SDL_SCANCODE_KP_9, Key(2, 0x80) }, // Keypad 9

		{ SDL_SCANCODE_W, Key(3, 0x01) }, // w
		{ SDL_SCANCODE_R, Key(3, 0x02) }, // r
		{ SDL_SCANCODE_Y, Key(3, 0x04) }, // y
		{ SDL_SCANCODE_I, Key(3, 0x08) }, // i
		{ SDL_SCANCODE_P, Key(3, 0x10) }, // p
		//{ , Key(3, 0x20) }, // NC
		{ SDL_SCANCODE_KP_8, Key(3, 0x40) }, // Keypad 8
		{ SDL_SCANCODE_KP_DIVIDE, Key(3, 0x80) }, // Keypad /

		{ SDL_SCANCODE_A, Key(4, 0x01) }, // a
		{ SDL_SCANCODE_D, Key(4, 0x02) }, // d
		{ SDL_SCANCODE_G, Key(4, 0x04) }, // g
		{ SDL_SCANCODE_J, Key(4, 0x08) }, // j
		{ SDL_SCANCODE_L, Key(4, 0x10) }, // l
		//{ , Key(4, 0x20) }, // NC
		{ SDL_SCANCODE_KP_4, Key(4, 0x40) }, // Keypad 4
		{ SDL_SCANCODE_KP_6, Key(4, 0x80) }, // Keypad 6

		{ SDL_SCANCODE_S, Key(5, 0x01) }, // s
		{ SDL_SCANCODE_F, Key(5, 0x02) }, // f
		{ SDL_SCANCODE_H, Key(5, 0x04) }, // h
		{ SDL_SCANCODE_K, Key(5, 0x08) }, // k
		{ SDL_SCANCODE_SEMICOLON, Key(5, 0x10) }, // :
		//{ , Key(5, 0x20) }, // NC
		{ SDL_SCANCODE_KP_5, Key(5, 0x40) }, // Keypad 5
		{ SDL_SCANCODE_KP_MULTIPLY, Key(5, 0x80) }, // Keypad *

		{ SDL_SCANCODE_Z, Key(6, 0x01) }, // z
		{ SDL_SCANCODE_C, Key(6, 0x02) }, // c
		{ SDL_SCANCODE_B, Key(6, 0x04) }, // b
		{ SDL_SCANCODE_M, Key(6, 0x08) }, // m
		{ SDL_SCANCODE_PERIOD, Key(6, 0x10) }, // ;
		{ SDL_SCANCODE_RETURN, Key(6, 0x20) }, // RETURN
		{ SDL_SCANCODE_KP_ENTER, Key(6, 0x20) }, // RETURN
		{ SDL_SCANCODE_KP_1, Key(6, 0x40) }, // Keypad 1
		{ SDL_SCANCODE_KP_3, Key(6, 0x80) }, // Keypad 3

		{ SDL_SCANCODE_X, Key(7, 0x01) }, // x
		{ SDL_SCANCODE_V, Key(7, 0x02) }, // v
		{ SDL_SCANCODE_N, Key(7, 0x04) }, // n
		{ SDL_SCANCODE_COMMA, Key(7, 0x08) }, // ,
		{ SDL_SCANCODE_SLASH, Key(7, 0x10) }, // ?
		//{ , Key(7, 0x20) }, // NC
		{ SDL_SCANCODE_KP_2, Key(7, 0x40) }, // Keypad 2
		{ SDL_SCANCODE_KP_PLUS, Key(7, 0x80) }, // Keypad +

		{ SDL_SCANCODE_LSHIFT, Key(8, 0x01) }, // LEFT SHIFT
		{ SDL_SCANCODE_GRAVE, Key(8, 0x02) }, // @
		{ SDL_SCANCODE_RIGHTBRACKET, Key(8, 0x04) }, // ]
		//{ , Key(8, 0x08) }, // NC
		{ SDL_SCANCODE_PAGEDOWN, Key(8, 0x10) }, // >
		{ SDL_SCANCODE_RSHIFT, Key(8, 0x20) }, // RIGHT SHIFT
		{ SDL_SCANCODE_KP_0, Key(8, 0x40) }, // Keypad 0
		{ SDL_SCANCODE_KP_MINUS, Key(8, 0x80) }, // Keypad -

		{ SDL_SCANCODE_LCTRL, Key(9, 0x01) }, // REVERSE OFF
		{ SDL_SCANCODE_LEFTBRACKET, Key(9, 0x02) }, // [
		{ SDL_SCANCODE_SPACE, Key(9, 0x04) }, // SPACE
		{ SDL_SCANCODE_PAGEUP, Key(9, 0x08) }, // <
		{ SDL_SCANCODE_RCTRL, Key(9, 0x10) }, // RUN STOP
		//{ , Key(9, 0x20) }, // NC
		{ SDL_SCANCODE_KP_PERIOD, Key(9, 0x40) }, // Keypad .
		{ SDL_SCANCODE_EQUALS, Key(9, 0x80) }, // Keypad =
	};

	// Process events at 60Hz
	InputEvents::InputEvents(size_t clockSpeedHz, size_t pollInterval) :
		Logger("INPUT"),
		m_clockSpeedHz(clockSpeedHz),
		m_pollInterval(pollInterval),
		m_cooldown(pollInterval),
		m_keyMap(&s_keyMapZX80)
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