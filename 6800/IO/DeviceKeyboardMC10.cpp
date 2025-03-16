#include "stdafx.h"

#include "DeviceKeyboardMC10.h"
#include <IO/InputEvents.h>

using events::KeyMap;
using events::Key;

namespace kbd
{
	static KeyMap s_keyMapMC10 =
	{
		// Row 0
		{ SDL_SCANCODE_LEFTBRACKET, Key(0, 0x01) }, // @
		{ SDL_SCANCODE_RIGHTBRACKET, Key(0, 0x01) }, // @
		{ SDL_SCANCODE_H, Key(0, 0x02) }, // H
		{ SDL_SCANCODE_P, Key(0, 0x04) }, // P
		{ SDL_SCANCODE_X, Key(0, 0x08) }, // X
		{ SDL_SCANCODE_0, Key(0, 0x10) }, // 0
		{ SDL_SCANCODE_KP_0, Key(0, 0x10) }, // 0
		{ SDL_SCANCODE_8, Key(0, 0x20) }, // 8
		{ SDL_SCANCODE_KP_8, Key(0, 0x20) }, // 8
		{ SDL_SCANCODE_LCTRL, Key(0, 0x40) }, // CONTROL
		{ SDL_SCANCODE_RCTRL, Key(0, 0x40) }, // CONTROL

		// Row 1
		{ SDL_SCANCODE_A, Key(1, 0x01) }, // A
		{ SDL_SCANCODE_I, Key(1, 0x02) }, // I
		{ SDL_SCANCODE_Q, Key(1, 0x04) }, // Q
		{ SDL_SCANCODE_Y, Key(1, 0x08) }, // Y
		{ SDL_SCANCODE_1, Key(1, 0x10) }, // 1
		{ SDL_SCANCODE_KP_1, Key(1, 0x10) }, // 1
		{ SDL_SCANCODE_9, Key(1, 0x20) }, // 9
		{ SDL_SCANCODE_KP_9, Key(1, 0x20) }, // 9

		// Row 2
		{ SDL_SCANCODE_B, Key(2, 0x01) }, // B
		{ SDL_SCANCODE_J, Key(2, 0x02) }, // J
		{ SDL_SCANCODE_R, Key(2, 0x04) }, // R
		{ SDL_SCANCODE_Z, Key(2, 0x08) }, // Z
		{ SDL_SCANCODE_2, Key(2, 0x10) }, // 2
		{ SDL_SCANCODE_KP_2, Key(2, 0x10) }, // 2
		{ SDL_SCANCODE_MINUS, Key(2, 0x20) }, // :*
		{ SDL_SCANCODE_KP_MULTIPLY, Key(2, 0x20) }, // :*
		{ SDL_SCANCODE_BACKSLASH, Key(2, 0x40) }, // BREAK
		{ SDL_SCANCODE_ESCAPE, Key(2, 0x40) }, // BREAK

		// Row 3
		{ SDL_SCANCODE_C, Key(3, 0x01) }, // C
		{ SDL_SCANCODE_K, Key(3, 0x02) }, // K
		{ SDL_SCANCODE_S, Key(3, 0x04) }, // S
		// { , Key(3, 0x08) }, // NC
		{ SDL_SCANCODE_3, Key(3, 0x10) }, // 3
		{ SDL_SCANCODE_KP_3, Key(3, 0x10) }, // 3
		{ SDL_SCANCODE_SEMICOLON, Key(3, 0x20) }, // ;+
		{ SDL_SCANCODE_KP_PLUS, Key(3, 0x20) }, // ;+

		// Row 4
		{ SDL_SCANCODE_D, Key(4, 0x01) }, // D
		{ SDL_SCANCODE_L, Key(4, 0x02) }, // L
		{ SDL_SCANCODE_T, Key(4, 0x04) }, // T
		// { , Key(4, 0x08) }, // NC
		{ SDL_SCANCODE_4, Key(4, 0x10) }, // 4
		{ SDL_SCANCODE_KP_4, Key(4, 0x10) }, // 4
		{ SDL_SCANCODE_COMMA, Key(4, 0x20) }, // ,<

		// Row 5
		{ SDL_SCANCODE_E, Key(5, 0x01) }, // E
		{ SDL_SCANCODE_M, Key(5, 0x02) }, // M
		{ SDL_SCANCODE_U, Key(5, 0x04) }, // U
		// { , Key(5, 0x08) }, // NC
		{ SDL_SCANCODE_5, Key(5, 0x10) }, // 5
		{ SDL_SCANCODE_KP_5, Key(5, 0x10) }, // 5
		{ SDL_SCANCODE_EQUALS, Key(5, 0x20) }, // -=
		{ SDL_SCANCODE_KP_MINUS, Key(5, 0x20) }, // -=

		// Row 6
		{ SDL_SCANCODE_F, Key(6, 0x01) }, // F
		{ SDL_SCANCODE_N, Key(6, 0x02) }, // N
		{ SDL_SCANCODE_V, Key(6, 0x04) }, // V
		{ SDL_SCANCODE_RETURN, Key(6, 0x08) }, // ENTER
		{ SDL_SCANCODE_KP_ENTER, Key(6, 0x08) }, // ENTER
		{ SDL_SCANCODE_6, Key(6, 0x10) }, // 6
		{ SDL_SCANCODE_KP_6, Key(6, 0x10) }, // 6
		{ SDL_SCANCODE_PERIOD, Key(6, 0x20) }, // .>
		{ SDL_SCANCODE_KP_PERIOD, Key(6, 0x20) }, // .>

		// Row 7
		{ SDL_SCANCODE_G, Key(7, 0x01) }, // G
		{ SDL_SCANCODE_O, Key(7, 0x02) }, // O
		{ SDL_SCANCODE_W, Key(7, 0x04) }, // W
		{ SDL_SCANCODE_SPACE, Key(7, 0x08) }, // SPACE
		{ SDL_SCANCODE_7, Key(7, 0x10) }, // 7
		{ SDL_SCANCODE_KP_7, Key(7, 0x10) }, // 7
		{ SDL_SCANCODE_SLASH, Key(7, 0x20) }, // /?
		{ SDL_SCANCODE_KP_DIVIDE, Key(7, 0x20) }, // /?
		{ SDL_SCANCODE_LSHIFT, Key(7, 0x40) }, // SHIFT
		{ SDL_SCANCODE_RSHIFT, Key(7, 0x40) }, // SHIFT
	};

	DeviceKeyboardMC10::DeviceKeyboardMC10() : Logger("kbd"), m_currKeymap(&s_keyMapMC10)
	{
		Reset();
	}
}
