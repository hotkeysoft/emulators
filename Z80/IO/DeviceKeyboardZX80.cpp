#include "stdafx.h"

#include "DeviceKeyboardZX80.h"
#include <IO/InputEvents.h>

using events::KeyMap;
using events::Key;

namespace kbd
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

	DeviceKeyboardZX80::DeviceKeyboardZX80() : m_currKeymap(&s_keyMapZX80)
	{
		Reset();
	}

	void DeviceKeyboardZX80::Reset()
	{
		m_keyGrid[0xFE] = 0;
		m_keyGrid[0xFD] = 0;
		m_keyGrid[0xFB] = 0;
		m_keyGrid[0xF7] = 0;
		m_keyGrid[0xEF] = 0;
		m_keyGrid[0xDF] = 0;
		m_keyGrid[0xBF] = 0;
		m_keyGrid[0x7F] = 0;
	}
}
