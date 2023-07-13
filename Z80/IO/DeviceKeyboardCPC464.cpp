#include "stdafx.h"

#include "DeviceKeyboardCPC464.h"
#include <IO/InputEvents.h>

using events::KeyMap;
using events::Key;

namespace kbd
{
	static KeyMap s_keyMapNormal =
	{
		// Line 0
		{ SDL_SCANCODE_UP, Key(0x01, 0) }, // Cursor UP
		{ SDL_SCANCODE_RIGHT, Key(0x02, 0) }, // Cursor RIGHT
		{ SDL_SCANCODE_DOWN, Key(0x04, 0) }, // Cursor DOWN
		{ SDL_SCANCODE_KP_9, Key(0x08, 0) }, // F9
		{ SDL_SCANCODE_KP_6, Key(0x10, 0) }, // F6
		{ SDL_SCANCODE_KP_3, Key(0x20, 0) }, // F3
		{ SDL_SCANCODE_KP_ENTER, Key(0x40, 0) }, // ENTER
		{ SDL_SCANCODE_KP_PERIOD, Key(0x80, 0) }, // PERIOD (KEYPAD)

		// Line 1
		{ SDL_SCANCODE_LEFT, Key(0x01, 1) }, // Cursor LEFT
		{ SDL_SCANCODE_KP_PLUS, Key(0x02, 1) }, // COPY
		{ SDL_SCANCODE_KP_7, Key(0x04, 1) }, // F7
		{ SDL_SCANCODE_KP_8, Key(0x08, 1) }, // F8
		{ SDL_SCANCODE_KP_5, Key(0x10, 1) }, // F5
		{ SDL_SCANCODE_KP_1, Key(0x20, 1) }, // F1
		{ SDL_SCANCODE_KP_2, Key(0x40, 1) }, // F2
		{ SDL_SCANCODE_KP_0, Key(0x80, 1) }, // F0

		// Line 2
		{ SDL_SCANCODE_DELETE, Key(0x01, 2) }, // CLR
		{ SDL_SCANCODE_LEFTBRACKET, Key(0x02, 2) }, // [
		{ SDL_SCANCODE_RETURN, Key(0x04, 2) }, // RETURN
		{ SDL_SCANCODE_RIGHTBRACKET, Key(0x08, 2) }, // ]
		{ SDL_SCANCODE_KP_4, Key(0x10, 2) }, // F4
		{ SDL_SCANCODE_LSHIFT, Key(0x20, 2) }, // SHIFT
		{ SDL_SCANCODE_RSHIFT, Key(0x20, 2) }, // SHIFT
		{ SDL_SCANCODE_GRAVE, Key(0x40, 2) }, // \`
		{ SDL_SCANCODE_LCTRL, Key(0x80, 2) }, // CONTROL
		{ SDL_SCANCODE_RCTRL, Key(0x80, 2) }, // CONTROL

		// Line 3
		{ SDL_SCANCODE_EQUALS, Key(0x01, 3) }, // ^
		{ SDL_SCANCODE_MINUS, Key(0x02, 3) }, // -
		{ SDL_SCANCODE_BACKSLASH, Key(0x04, 3) }, // @
		{ SDL_SCANCODE_P, Key(0x08, 3) }, // P
		{ SDL_SCANCODE_SEMICOLON, Key(0x10, 3) }, // ;
		{ SDL_SCANCODE_APOSTROPHE, Key(0x20, 3)}, // :
		{ SDL_SCANCODE_SLASH, Key(0x40, 3) }, // /
		{ SDL_SCANCODE_PERIOD, Key(0x80, 3) }, // .

		// Line 4
		{ SDL_SCANCODE_0, Key(0x01, 4) }, // 0
		{ SDL_SCANCODE_9, Key(0x02, 4) }, // 9
		{ SDL_SCANCODE_O, Key(0x04, 4) }, // O
		{ SDL_SCANCODE_I, Key(0x08, 4) }, // I
		{ SDL_SCANCODE_L, Key(0x10, 4) }, // L
		{ SDL_SCANCODE_K, Key(0x20, 4) }, // K
		{ SDL_SCANCODE_M, Key(0x40, 4) }, // M
		{ SDL_SCANCODE_COMMA, Key(0x80, 4) }, // ,

		// Line 5
		{ SDL_SCANCODE_8, Key(0x01, 5) }, // 8
		{ SDL_SCANCODE_7, Key(0x02, 5) }, // 7
		{ SDL_SCANCODE_U, Key(0x04, 5) }, // U
		{ SDL_SCANCODE_Y, Key(0x08, 5) }, // Y
		{ SDL_SCANCODE_H, Key(0x10, 5) }, // H
		{ SDL_SCANCODE_J, Key(0x20, 5) }, // J
		{ SDL_SCANCODE_N, Key(0x40, 5) }, // N
		{ SDL_SCANCODE_SPACE, Key(0x80, 5) }, // SPACE

		// Line 6
		{ SDL_SCANCODE_6, Key(0x01, 6) }, // 6 (Joy 2 up)
		{ SDL_SCANCODE_5, Key(0x02, 6) }, // 5 (Joy 2 down)
		{ SDL_SCANCODE_R, Key(0x04, 6) }, // R (Joy 2 left)
		{ SDL_SCANCODE_T, Key(0x08, 6) }, // T (Joy 2 right)
		{ SDL_SCANCODE_G, Key(0x10, 6) }, // G (Joy 2 fire)
		{ SDL_SCANCODE_F, Key(0x20, 6) }, // F
		{ SDL_SCANCODE_B, Key(0x40, 6) }, // B
		{ SDL_SCANCODE_V, Key(0x80, 6) }, // V

		// Line 7
		{ SDL_SCANCODE_4, Key(0x01, 7) }, // 4
		{ SDL_SCANCODE_3, Key(0x02, 7) }, // 3
		{ SDL_SCANCODE_E, Key(0x04, 7) }, // E
		{ SDL_SCANCODE_W, Key(0x08, 7) }, // W
		{ SDL_SCANCODE_S, Key(0x10, 7) }, // S
		{ SDL_SCANCODE_D, Key(0x20, 7) }, // D
		{ SDL_SCANCODE_C, Key(0x40, 7) }, // C
		{ SDL_SCANCODE_X, Key(0x80, 7) }, // X

		// Line 8
		{ SDL_SCANCODE_1, Key(0x01, 8) }, // 1
		{ SDL_SCANCODE_2, Key(0x02, 8) }, // 2
		{ SDL_SCANCODE_ESCAPE, Key(0x04, 8) }, // ESC
		{ SDL_SCANCODE_Q, Key(0x08, 8) }, // Q
		{ SDL_SCANCODE_TAB, Key(0x10, 8) }, // TAB
		{ SDL_SCANCODE_A, Key(0x20, 8) }, // A
		{ SDL_SCANCODE_CAPSLOCK, Key(0x40, 8) }, // CAPSLOCK
		{ SDL_SCANCODE_Z, Key(0x80, 8) }, // Z

		// Line 9
		//{ , Key(0x01, 9) }, // Joy 1 up
		//{ , Key(0x02, 9) }, // Joy 1 down
		//{ , Key(0x04, 9) }, // Joy 1 left
		//{ , Key(0x08, 9) }, // Joy 1 right
		//{ , Key(0x10, 9) }, // Joy 1 Fire 1
		//{ , Key(0x20, 9) }, // Joy 1 Fire 2
		//{ , Key(0x40, 9) }, // Joy 1 Fire 3
		{ SDL_SCANCODE_BACKSPACE, Key(0x80, 9) }, // DEL
	};

	DeviceKeyboardCPC464::DeviceKeyboardCPC464() : Logger("kbd"), m_currKeymap(&s_keyMapNormal)
	{
		Reset();
	}
}
