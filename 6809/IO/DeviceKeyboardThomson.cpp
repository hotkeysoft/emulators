#include "stdafx.h"

#include "DeviceKeyboardThomson.h"
#include <IO/InputEvents.h>

using events::KeyMap;
using events::Key;
using Model = emul::Thomson::Model;

namespace kbd
{
	static KeyMap s_keyMapMO5 =
	{
		// Column 0
		{ SDL_SCANCODE_N, Key(0x01, 0) }, // N
		{ SDL_SCANCODE_COMMA, Key(0x02, 0) }, // ,
		{ SDL_SCANCODE_PERIOD, Key(0x04, 0) }, // .
		{ SDL_SCANCODE_KP_PERIOD, Key(0x04, 0) }, // .
		{ SDL_SCANCODE_LEFTBRACKET, Key(0x08, 0) }, // @
		{ SDL_SCANCODE_SPACE, Key(0x10, 0) }, // SP
		{ SDL_SCANCODE_X, Key(0x20, 0) }, // X
		{ SDL_SCANCODE_W, Key(0x40, 0) }, // W
		{ SDL_SCANCODE_LSHIFT, Key(0x80, 0) }, // Shift
		{ SDL_SCANCODE_RSHIFT, Key(0x80, 0) }, // Shift

		// Column 1
		{ SDL_SCANCODE_DELETE, Key(0x01, 1) }, // EFF
		{ SDL_SCANCODE_BACKSPACE, Key(0x01, 1) }, // EFF
		{ SDL_SCANCODE_INSERT, Key(0x02, 1) }, // INS
		{ SDL_SCANCODE_HOME, Key(0x04, 1) }, // Home
		{ SDL_SCANCODE_RIGHT, Key(0x08, 1) }, // Right arrow
		{ SDL_SCANCODE_DOWN, Key(0x10, 1) }, // Down arrow
		{ SDL_SCANCODE_LEFT, Key(0x20, 1) }, // Left arrow
		{ SDL_SCANCODE_UP, Key(0x40, 1) }, // Up arrow
		{ SDL_SCANCODE_LALT, Key(0x80, 1) }, // BASIC
		{ SDL_SCANCODE_RALT, Key(0x80, 1) }, // BASIC

		// Column 2
		{ SDL_SCANCODE_J, Key(0x01, 2) }, // J
		{ SDL_SCANCODE_K, Key(0x02, 2) }, // Z
		{ SDL_SCANCODE_L, Key(0x04, 2) }, // L
		{ SDL_SCANCODE_M, Key(0x08, 2) }, // M
		{ SDL_SCANCODE_B, Key(0x10, 2) }, // B
		{ SDL_SCANCODE_V, Key(0x20, 2) }, // V
		{ SDL_SCANCODE_C, Key(0x40, 2) }, // C

		// Column 3
		{ SDL_SCANCODE_H, Key(0x01, 3) }, // H
		{ SDL_SCANCODE_G, Key(0x02, 3) }, // G
		{ SDL_SCANCODE_F, Key(0x04, 3) }, // F
		{ SDL_SCANCODE_D, Key(0x08, 3) }, // D
		{ SDL_SCANCODE_S, Key(0x10, 3) }, // S
		{ SDL_SCANCODE_Q, Key(0x20, 3)}, // Q
		{ SDL_SCANCODE_TAB, Key(0x40, 3) }, // RAZ

		// Column 4
		{ SDL_SCANCODE_U, Key(0x01, 4) }, // U
		{ SDL_SCANCODE_I, Key(0x02, 4) }, // I
		{ SDL_SCANCODE_O, Key(0x04, 4) }, // O
		{ SDL_SCANCODE_P, Key(0x08, 4) }, // P
		{ SDL_SCANCODE_SLASH, Key(0x10, 4) }, // /
		{ SDL_SCANCODE_KP_DIVIDE, Key(0x10, 4) }, // /
		{ SDL_SCANCODE_RIGHTBRACKET, Key(0x20, 4) }, // *
		{ SDL_SCANCODE_SEMICOLON, Key(0x20, 4) }, // *
		{ SDL_SCANCODE_KP_MULTIPLY, Key(0x20, 4) }, // *
		{ SDL_SCANCODE_RETURN, Key(0x40, 4) }, // ENTER
		{ SDL_SCANCODE_KP_ENTER, Key(0x40, 4) }, // ENTER

		// Column 5
		{ SDL_SCANCODE_Y, Key(0x01, 5) }, // Y
		{ SDL_SCANCODE_T, Key(0x02, 5) }, // T
		{ SDL_SCANCODE_R, Key(0x04, 5) }, // R
		{ SDL_SCANCODE_E, Key(0x08, 5) }, // E
		{ SDL_SCANCODE_Z, Key(0x10, 5) }, // Z
		{ SDL_SCANCODE_A, Key(0x20, 5) }, // A
		{ SDL_SCANCODE_LCTRL, Key(0x40, 5) }, // CONTROL
		{ SDL_SCANCODE_RCTRL, Key(0x40, 5) }, // CONTROL

		// Column 6
		{ SDL_SCANCODE_7, Key(0x01, 6) }, // 7
		{ SDL_SCANCODE_KP_7, Key(0x01, 6) }, // 7
		{ SDL_SCANCODE_8, Key(0x02, 6) }, // 8
		{ SDL_SCANCODE_KP_8, Key(0x02, 6) }, // 8
		{ SDL_SCANCODE_9, Key(0x04, 6) }, // 9
		{ SDL_SCANCODE_KP_9, Key(0x04, 6) }, // 9
		{ SDL_SCANCODE_0, Key(0x08, 6) }, // 0
		{ SDL_SCANCODE_KP_0, Key(0x08, 6) }, // 0
		{ SDL_SCANCODE_MINUS, Key(0x10, 6) }, // -
		{ SDL_SCANCODE_KP_MINUS, Key(0x10, 6) }, // -
		{ SDL_SCANCODE_EQUALS, Key(0x20, 6) }, // +
		{ SDL_SCANCODE_KP_PLUS, Key(0x20, 6) }, // +
		{ SDL_SCANCODE_BACKSLASH, Key(0x40, 6) }, // ACCENT
		{ SDL_SCANCODE_APOSTROPHE, Key(0x40, 6) }, // ACCENT

		// Column 7
		{ SDL_SCANCODE_6, Key(0x01, 7) }, // 6
		{ SDL_SCANCODE_KP_6, Key(0x01, 7) }, // 6
		{ SDL_SCANCODE_5, Key(0x02, 7) }, // 5
		{ SDL_SCANCODE_KP_5, Key(0x02, 7) }, // 5
		{ SDL_SCANCODE_4, Key(0x04, 7) }, // 4
		{ SDL_SCANCODE_KP_4, Key(0x04, 7) }, // 4
		{ SDL_SCANCODE_3, Key(0x08, 7) }, // 3
		{ SDL_SCANCODE_KP_3, Key(0x08, 7) }, // 3
		{ SDL_SCANCODE_2, Key(0x10, 7) }, // 2
		{ SDL_SCANCODE_KP_2, Key(0x10, 7) }, // 2
		{ SDL_SCANCODE_1, Key(0x20, 7) }, // 1
		{ SDL_SCANCODE_KP_1, Key(0x20, 7) }, // 1
		{ SDL_SCANCODE_ESCAPE, Key(0x40, 7) }, // STOP
	};

	static KeyMap s_keyMapTO7 =
	{
		// Column 0
		{ SDL_SCANCODE_LSHIFT, Key(0x01, 0) }, // SHIFT
		{ SDL_SCANCODE_RSHIFT, Key(0x01, 0) }, // SHIFT

		// Column 1
		{ SDL_SCANCODE_W, Key(0x01, 1) }, // W
		{ SDL_SCANCODE_UP, Key(0x02, 1) }, // UP
		{ SDL_SCANCODE_C, Key(0x04, 1) }, // C
		{ SDL_SCANCODE_TAB, Key(0x08, 1) }, // RAZ
		{ SDL_SCANCODE_RETURN, Key(0x10, 1) }, // Entree
		{ SDL_SCANCODE_KP_ENTER, Key(0x10, 1) }, // Entree
		{ SDL_SCANCODE_LCTRL, Key(0x20, 1) }, // CONTROL
		{ SDL_SCANCODE_RCTRL, Key(0x20, 1) }, // CONTROL
		{ SDL_SCANCODE_BACKSLASH, Key(0x40, 1) }, // Accent
		{ SDL_SCANCODE_APOSTROPHE, Key(0x40, 1) }, // Accent
		{ SDL_SCANCODE_ESCAPE, Key(0x80, 1) }, // STOP

		// Column 2
		{ SDL_SCANCODE_X, Key(0x01, 2) }, // X
		{ SDL_SCANCODE_LEFT, Key(0x02, 2) }, // LEFT
		{ SDL_SCANCODE_V, Key(0x04, 2) }, // V
		{ SDL_SCANCODE_Q, Key(0x08, 2) }, // Q
		{ SDL_SCANCODE_RIGHTBRACKET, Key(0x10, 2) }, // :*
		{ SDL_SCANCODE_SEMICOLON, Key(0x10, 2) }, // :*
		{ SDL_SCANCODE_KP_MULTIPLY, Key(0x10, 2) }, // :*
		{ SDL_SCANCODE_A, Key(0x20, 2) }, // A
		{ SDL_SCANCODE_EQUALS, Key(0x40, 2) }, // ;+
		{ SDL_SCANCODE_KP_PLUS, Key(0x40, 2) }, // ;+
		{ SDL_SCANCODE_1, Key(0x80, 2) }, // 1
		{ SDL_SCANCODE_KP_1, Key(0x80, 2) }, // 1

		// Column 3
		{ SDL_SCANCODE_SPACE, Key(0x01, 3) }, // Espace
		{ SDL_SCANCODE_DOWN, Key(0x02, 3) }, // DOWN
		{ SDL_SCANCODE_B, Key(0x04, 3) }, // B
		{ SDL_SCANCODE_S, Key(0x08, 3) }, // S
		{ SDL_SCANCODE_SLASH, Key(0x10, 3) }, // /?
		{ SDL_SCANCODE_KP_DIVIDE, Key(0x10, 3) }, // /?
		{ SDL_SCANCODE_Z, Key(0x20, 3) }, // Z
		{ SDL_SCANCODE_1, Key(0x40, 3) }, // _=
		{ SDL_SCANCODE_2, Key(0x80, 3) }, // 2
		{ SDL_SCANCODE_KP_2, Key(0x80, 3) }, // 2

		// Column 4
		{ SDL_SCANCODE_LEFTBRACKET, Key(0x01, 4) }, // @^
		{ SDL_SCANCODE_RIGHT, Key(0x02, 4) }, // RIGHT
		{ SDL_SCANCODE_M, Key(0x04, 4) }, // M
		{ SDL_SCANCODE_D, Key(0x08, 4) }, // D
		{ SDL_SCANCODE_P, Key(0x10, 4) }, // P
		{ SDL_SCANCODE_E, Key(0x20, 4) }, // E
		{ SDL_SCANCODE_0, Key(0x40, 4) }, // 0
		{ SDL_SCANCODE_3, Key(0x80, 4) }, // 3
		{ SDL_SCANCODE_KP_3, Key(0x80, 4) }, // 3

		// Column 5
		{ SDL_SCANCODE_PERIOD, Key(0x01, 5) }, // .>
		{ SDL_SCANCODE_KP_PERIOD, Key(0x01, 5) }, // .
		{ SDL_SCANCODE_HOME, Key(0x02, 5) }, // Home
		{ SDL_SCANCODE_L, Key(0x04, 5) }, // L
		{ SDL_SCANCODE_F, Key(0x08, 5) }, // F
		{ SDL_SCANCODE_O, Key(0x10, 5) }, // O
		{ SDL_SCANCODE_R, Key(0x20, 5) }, // R
		{ SDL_SCANCODE_9, Key(0x40, 5) }, // 9
		{ SDL_SCANCODE_KP_9, Key(0x40, 5) }, // 9
		{ SDL_SCANCODE_4, Key(0x80, 5) }, // 4
		{ SDL_SCANCODE_KP_4, Key(0x80, 5) }, // 4

		// Column 6
		{ SDL_SCANCODE_COMMA, Key(0x01, 6) }, // ,<
		{ SDL_SCANCODE_INSERT, Key(0x02, 6) }, // INSERT
		{ SDL_SCANCODE_K, Key(0x04, 6) }, // K
		{ SDL_SCANCODE_G, Key(0x08, 6) }, // G
		{ SDL_SCANCODE_I, Key(0x10, 6) }, // I
		{ SDL_SCANCODE_T, Key(0x20, 6) }, // T
		{ SDL_SCANCODE_8, Key(0x40, 6) }, // 8
		{ SDL_SCANCODE_KP_8, Key(0x40, 6) }, // 8
		{ SDL_SCANCODE_5, Key(0x80, 6) }, // 5
		{ SDL_SCANCODE_KP_5, Key(0x80, 6) }, // 5

		// Column 7
		{ SDL_SCANCODE_N, Key(0x01, 7) }, // N
		{ SDL_SCANCODE_DELETE, Key(0x02, 7) }, // EFF
		{ SDL_SCANCODE_BACKSPACE, Key(0x02, 7) }, // EFF
		{ SDL_SCANCODE_J, Key(0x04, 7) }, // J
		{ SDL_SCANCODE_H, Key(0x08, 7) }, // H
		{ SDL_SCANCODE_U, Key(0x10, 7) }, // U
		{ SDL_SCANCODE_Y, Key(0x20, 7) }, // Y
		{ SDL_SCANCODE_7, Key(0x40, 7) }, // 7
		{ SDL_SCANCODE_KP_7, Key(0x40, 7) }, // 7
		{ SDL_SCANCODE_6, Key(0x80, 7) }, // 6
		{ SDL_SCANCODE_KP_6, Key(0x80, 7) }, // 6
	};

	DeviceKeyboardThomson::DeviceKeyboardThomson() : Logger("kbd"), m_currKeymap(&s_keyMapMO5)
	{
		Reset();
	}

	void DeviceKeyboardThomson::SetModel(Model model)
	{
		switch (model)
		{
			case Model::MO5: m_currKeymap = &s_keyMapMO5; break;
			case Model::TO7: m_currKeymap = &s_keyMapTO7; break;
			default:
				LogPrintf(LOG_ERROR, "Unknown Model");
				break;
		}
	}

}
