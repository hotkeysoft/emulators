#include "InputEvents.h"
#include "DeviceKeyboard.h"

#include <SDL.h>

#include <assert.h>

namespace events
{
	typedef std::map<SDL_Scancode, Key> KeyMap;

	static const KeyMap s_keyMap =
	{
		{ SDL_SCANCODE_ESCAPE, 0x01 },
		{ SDL_SCANCODE_1, 0x02 },
		{ SDL_SCANCODE_2, 0x03 },
		{ SDL_SCANCODE_3, 0x04 },
		{ SDL_SCANCODE_4, 0x05 },
		{ SDL_SCANCODE_5, 0x06 },
		{ SDL_SCANCODE_6, 0x07 },
		{ SDL_SCANCODE_7, 0x08 },
		{ SDL_SCANCODE_8, 0x09 },
		{ SDL_SCANCODE_9, 0x0A },
		{ SDL_SCANCODE_0, 0x0B },
		{ SDL_SCANCODE_MINUS, 0x0C },
		{ SDL_SCANCODE_EQUALS, 0x0D },
		{ SDL_SCANCODE_BACKSPACE, 0x0E },
		{ SDL_SCANCODE_TAB, 0x0F },

		{ SDL_SCANCODE_Q, 0x10 },
		{ SDL_SCANCODE_W, 0x11 },
		{ SDL_SCANCODE_E, 0x12 },
		{ SDL_SCANCODE_R, 0x13 },
		{ SDL_SCANCODE_T, 0x14 },
		{ SDL_SCANCODE_Y, 0x15 },
		{ SDL_SCANCODE_U, 0x16 },
		{ SDL_SCANCODE_I, 0x17 },
		{ SDL_SCANCODE_O, 0x18 },
		{ SDL_SCANCODE_P, 0x19 },
		{ SDL_SCANCODE_LEFTBRACKET, 0x1A },
		{ SDL_SCANCODE_RIGHTBRACKET, 0x1B },
		{ SDL_SCANCODE_RETURN, 0x1C },
		{ SDL_SCANCODE_LCTRL, NonRepeatingKey(0x1D) },
		{ SDL_SCANCODE_RCTRL, NonRepeatingKey(0x1D) },

		{ SDL_SCANCODE_A, 0x1E },
		{ SDL_SCANCODE_S, 0x1F },
		{ SDL_SCANCODE_D, 0x20 },
		{ SDL_SCANCODE_F, 0x21 },
		{ SDL_SCANCODE_G, 0x22 },
		{ SDL_SCANCODE_H, 0x23 },
		{ SDL_SCANCODE_J, 0x24 },
		{ SDL_SCANCODE_K, 0x25 },
		{ SDL_SCANCODE_L, 0x26 },
		{ SDL_SCANCODE_SEMICOLON, 0x27 },
		{ SDL_SCANCODE_APOSTROPHE, 0x28 },
		{ SDL_SCANCODE_GRAVE, 0x29 },

		{ SDL_SCANCODE_LSHIFT, NonRepeatingKey(0x2A) },
		{ SDL_SCANCODE_BACKSLASH, 0x2B },

		{ SDL_SCANCODE_Z, 0x2C },
		{ SDL_SCANCODE_X, 0x2D },
		{ SDL_SCANCODE_C, 0x2E },
		{ SDL_SCANCODE_V, 0x2F },
		{ SDL_SCANCODE_B, 0x30 },
		{ SDL_SCANCODE_N, 0x31 },
		{ SDL_SCANCODE_M, 0x32 },
		{ SDL_SCANCODE_COMMA, 0x33 },
		{ SDL_SCANCODE_PERIOD, 0x34 },
		{ SDL_SCANCODE_SLASH, 0x35 },
		{ SDL_SCANCODE_RSHIFT, NonRepeatingKey(0x36) },

		{ SDL_SCANCODE_KP_MULTIPLY, 0x37 },
		{ SDL_SCANCODE_LALT, NonRepeatingKey(0x38) },
		{ SDL_SCANCODE_SPACE, 0x39 },
		{ SDL_SCANCODE_CAPSLOCK, NonRepeatingKey(0x3A) },

		{ SDL_SCANCODE_F1, 0x3B },
		{ SDL_SCANCODE_F2, 0x3C },
		{ SDL_SCANCODE_F3, 0x3D },
		{ SDL_SCANCODE_F4, 0x3E },
		{ SDL_SCANCODE_F5, 0x3F },
		{ SDL_SCANCODE_F6, 0x40 },
		{ SDL_SCANCODE_F7, 0x41 },
		{ SDL_SCANCODE_F8, 0x42 },
		{ SDL_SCANCODE_F9, 0x43 },
		{ SDL_SCANCODE_F10, 0x44 },

		{ SDL_SCANCODE_NUMLOCKCLEAR, NonRepeatingKey(0x45) },
		{ SDL_SCANCODE_SCROLLLOCK, NonRepeatingKey(0x46) },

		{ SDL_SCANCODE_KP_7, 0x47 },
		{ SDL_SCANCODE_KP_8, 0x48 },
		{ SDL_SCANCODE_KP_9, 0x49 },
		{ SDL_SCANCODE_KP_MINUS, 0x4A },

		{ SDL_SCANCODE_KP_4, 0x4B },
		{ SDL_SCANCODE_KP_5, 0x4C },
		{ SDL_SCANCODE_KP_6, 0x4D },
		{ SDL_SCANCODE_KP_PLUS, 0x4E },

		{ SDL_SCANCODE_KP_1, 0x4F },
		{ SDL_SCANCODE_KP_2, 0x50 },
		{ SDL_SCANCODE_KP_3, 0x51 },

		{ SDL_SCANCODE_KP_0, 0x52 },
		{ SDL_SCANCODE_KP_PERIOD, 0x53 },

		{ SDL_SCANCODE_RALT, NonRepeatingKey(0x54) },

		{ SDL_SCANCODE_F11, 0x57 },
		{ SDL_SCANCODE_F12, 0x58 },

		{ SDL_SCANCODE_KP_ENTER, ExtendedKey(0xE0, 0x1C) },
		{ SDL_SCANCODE_KP_DIVIDE, ExtendedKey(0xE0, 0x35) },

		{ SDL_SCANCODE_HOME, ExtendedKey(0xE0, 0x47) },
		{ SDL_SCANCODE_UP, ExtendedKey(0xE0, 0x48) },
		{ SDL_SCANCODE_PAGEUP, ExtendedKey(0xE0, 0x49) },
		{ SDL_SCANCODE_LEFT, ExtendedKey(0xE0, 0x4B) },
		{ SDL_SCANCODE_RIGHT, ExtendedKey(0xE0, 0x4D) },
		{ SDL_SCANCODE_END, ExtendedKey(0xE0, 0x4F) },
		{ SDL_SCANCODE_DOWN, ExtendedKey(0xE0, 0x50) },
		{ SDL_SCANCODE_PAGEDOWN, ExtendedKey(0xE0, 0x51) },
		{ SDL_SCANCODE_INSERT, ExtendedKey(0xE0, 0x52) },
		{ SDL_SCANCODE_DELETE, ExtendedKey(0xE0, 0x53) },
	};

	// Process events at 60Hz
	InputEvents::InputEvents(size_t clockSpeedHz) :
		Logger("INPUT"),
		m_pollRate(clockSpeedHz / 60),
		m_cooldown(m_pollRate)
	{

	}
	void InputEvents::Init(kbd::DeviceKeyboard* kbd)
	{
		assert(kbd);
		m_keyboard = kbd;
		LogPrintf(LOG_INFO, "Polling rate: [%zu] ticks", m_pollRate);
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
			switch (e.type)
			{
			case SDL_QUIT:
				SDL_Quit();
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
		LogPrintf(LOG_DEBUG, "InputKey: [%s] key: %d, repeat: %d", evt.state == SDL_PRESSED ? "DOWN" : "UP", evt.keysym.scancode, evt.repeat);
		KeyMap::const_iterator it = s_keyMap.find(evt.keysym.scancode);
		if (it != s_keyMap.end())
		{
			const Key& key = it->second;

			if (evt.repeat && !key.IsRepeat())
			{
				return;
			}

			if (key.GetPrefix())
			{
				m_keyboard->InputKey(key.GetPrefix());
			}
			m_keyboard->InputKey(key.GetScancode() | (evt.state == SDL_PRESSED ? 0 : 0x80));
		}
		else
		{
			LogPrintf(LOG_WARNING, "Unmapped scancode %d", evt.keysym.scancode);
		}
	}
}