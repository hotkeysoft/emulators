#include "stdafx.h"

#include "DeviceKeyboardThomson.h"
#include <IO/InputEvents.h>

using events::KeyMap;
using events::Key;

namespace kbd
{
	static KeyMap s_keyMapNormal =
	{
		// Line 0
		{ SDL_SCANCODE_N, Key(0x01, 0) }, // N
		{ SDL_SCANCODE_COMMA, Key(0x02, 0) }, // ,
		{ SDL_SCANCODE_PERIOD, Key(0x04, 0) }, // .
		{ SDL_SCANCODE_SLASH, Key(0x08, 0) }, // @
		{ SDL_SCANCODE_SPACE, Key(0x10, 0) }, // SP
		{ SDL_SCANCODE_X, Key(0x20, 0) }, // X
		{ SDL_SCANCODE_W, Key(0x40, 0) }, // W
		{ SDL_SCANCODE_LSHIFT, Key(0x80, 0) }, // Shift

		// Line 1
		{ SDL_SCANCODE_DELETE, Key(0x01, 1) }, // EFF
		{ SDL_SCANCODE_INSERT, Key(0x02, 1) }, // INS
		{ SDL_SCANCODE_HOME, Key(0x04, 1) }, // backspace?
		{ SDL_SCANCODE_RIGHT, Key(0x08, 1) }, // Right arrow
		{ SDL_SCANCODE_DOWN, Key(0x10, 1) }, // Down arrow
		{ SDL_SCANCODE_LEFT, Key(0x20, 1) }, // Left arrow
		{ SDL_SCANCODE_UP, Key(0x40, 1) }, // Up arrow
		{ SDL_SCANCODE_RSHIFT, Key(0x80, 1) }, // BASIC

		// Line 2
		{ SDL_SCANCODE_J, Key(0x01, 2) }, // J
		{ SDL_SCANCODE_K, Key(0x02, 2) }, // Z
		{ SDL_SCANCODE_L, Key(0x04, 2) }, // L
		{ SDL_SCANCODE_M, Key(0x08, 2) }, // M
		{ SDL_SCANCODE_B, Key(0x10, 2) }, // B
		{ SDL_SCANCODE_V, Key(0x20, 2) }, // V
		{ SDL_SCANCODE_C, Key(0x40, 2) }, // C

		// Line 3
		{ SDL_SCANCODE_H, Key(0x01, 3) }, // H
		{ SDL_SCANCODE_G, Key(0x02, 3) }, // G
		{ SDL_SCANCODE_F, Key(0x04, 3) }, // F
		{ SDL_SCANCODE_D, Key(0x08, 3) }, // D
		{ SDL_SCANCODE_S, Key(0x10, 3) }, // S
		{ SDL_SCANCODE_Q, Key(0x20, 3)}, // Q
		{ SDL_SCANCODE_TAB, Key(0x40, 3) }, // RAZ

		// Line 4
		{ SDL_SCANCODE_U, Key(0x01, 4) }, // U
		{ SDL_SCANCODE_I, Key(0x02, 4) }, // I
		{ SDL_SCANCODE_O, Key(0x04, 4) }, // O
		{ SDL_SCANCODE_P, Key(0x08, 4) }, // P
		{ SDL_SCANCODE_LEFTBRACKET, Key(0x10, 4) }, // /
		{ SDL_SCANCODE_RIGHTBRACKET, Key(0x20, 4) }, // *
		{ SDL_SCANCODE_RETURN, Key(0x40, 4) }, // ENTER
		{ SDL_SCANCODE_KP_ENTER, Key(0x40, 4) }, // ENTER

		// Line 5
		{ SDL_SCANCODE_Y, Key(0x01, 5) }, // Y
		{ SDL_SCANCODE_T, Key(0x02, 5) }, // T
		{ SDL_SCANCODE_R, Key(0x04, 5) }, // R
		{ SDL_SCANCODE_E, Key(0x08, 5) }, // E
		{ SDL_SCANCODE_Z, Key(0x10, 5) }, // Z
		{ SDL_SCANCODE_A, Key(0x20, 5) }, // A
		{ SDL_SCANCODE_GRAVE, Key(0x40, 5) }, // CNT

		// Line 6
		{ SDL_SCANCODE_7, Key(0x01, 6) }, // 7
		{ SDL_SCANCODE_8, Key(0x02, 6) }, // 8
		{ SDL_SCANCODE_9, Key(0x04, 6) }, // 9
		{ SDL_SCANCODE_0, Key(0x08, 6) }, // 0
		{ SDL_SCANCODE_MINUS, Key(0x10, 6) }, // -
		{ SDL_SCANCODE_EQUALS, Key(0x20, 6) }, // +
		{ SDL_SCANCODE_BACKSLASH, Key(0x40, 6) }, // ACC

		// Column 7
		{ SDL_SCANCODE_6, Key(0x01, 7) }, // 6
		{ SDL_SCANCODE_5, Key(0x02, 7) }, // 5
		{ SDL_SCANCODE_4, Key(0x04, 7) }, // 4
		{ SDL_SCANCODE_3, Key(0x08, 7) }, // 3
		{ SDL_SCANCODE_2, Key(0x10, 7) }, // 2
		{ SDL_SCANCODE_1, Key(0x20, 7) }, // 1
		{ SDL_SCANCODE_ESCAPE, Key(0x40, 7) }, // STOP
	};

	DeviceKeyboardThomson::DeviceKeyboardThomson() : Logger("kbd"), m_currKeymap(&s_keyMapNormal)
	{
		Reset();
	}
}
