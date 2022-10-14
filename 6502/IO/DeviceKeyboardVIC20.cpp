#include "stdafx.h"

#include "DeviceKeyboardVIC20.h"
#include "InputEvents.h"

using events::KeyMap;
using events::Key;

namespace kbd
{
	static KeyMap s_keyMapNormal =
	{
		// Column 0
		{ SDL_SCANCODE_1, Key(0x01, 0) }, // 1
		{ SDL_SCANCODE_KP_1, Key(0x01, 0) }, // 1
		{ SDL_SCANCODE_3, Key(0x02, 0) }, // 3
		{ SDL_SCANCODE_KP_3, Key(0x02, 0) }, // 3
		{ SDL_SCANCODE_5, Key(0x04, 0) }, // 5
		{ SDL_SCANCODE_KP_5, Key(0x04, 0) }, // 5
		{ SDL_SCANCODE_7, Key(0x08, 0) }, // 7
		{ SDL_SCANCODE_KP_7, Key(0x08, 0) }, // 7
		{ SDL_SCANCODE_9, Key(0x10, 0) }, // 9
		{ SDL_SCANCODE_KP_9, Key(0x10, 0) }, // 9
		{ SDL_SCANCODE_KP_PLUS, Key(0x20, 0) }, // +
		{ SDL_SCANCODE_RIGHTBRACKET, Key(0x40, 0) }, // Pound
		{ SDL_SCANCODE_BACKSPACE, Key(0x80, 0) }, // INST/DEL

		// Column 1
		{ SDL_SCANCODE_GRAVE, Key(0x01, 1) }, // left arrow
		{ SDL_SCANCODE_W, Key(0x02, 1) }, // W
		{ SDL_SCANCODE_R, Key(0x04, 1) }, // R
		{ SDL_SCANCODE_Y, Key(0x08, 1) }, // Y
		{ SDL_SCANCODE_I, Key(0x10, 1) }, // I
		{ SDL_SCANCODE_P, Key(0x20, 1) }, // P
		{ SDL_SCANCODE_KP_MULTIPLY, Key(0x40, 1) }, // *
		{ SDL_SCANCODE_RETURN, Key(0x80, 1) }, // ENTER
		{ SDL_SCANCODE_KP_ENTER, Key(0x80, 1) }, // ENTER

		// Column 2
		{ SDL_SCANCODE_TAB, Key(0x01, 2) }, // CTRL
		{ SDL_SCANCODE_A, Key(0x02, 2) }, // A
		{ SDL_SCANCODE_D, Key(0x04, 2) }, // D
		{ SDL_SCANCODE_G, Key(0x08, 2) }, // G
		{ SDL_SCANCODE_J, Key(0x10, 2) }, // J
		{ SDL_SCANCODE_L, Key(0x20, 2) }, // L
		{ SDL_SCANCODE_APOSTROPHE, Key(0x40, 2) }, // ;
		{ SDL_SCANCODE_RIGHT, Key(0x80, 2) }, // CURSOR LEFT/RIGHT

		// Column 3
		{ SDL_SCANCODE_RCTRL, Key(0x01, 3) }, // RUN/STOP
		{ SDL_SCANCODE_LSHIFT, Key(0x02, 3) }, // LEFT SHIFT
		{ SDL_SCANCODE_X, Key(0x04, 3) }, // X
		{ SDL_SCANCODE_V, Key(0x08, 3) }, // V
		{ SDL_SCANCODE_N, Key(0x10, 3) }, // N
		{ SDL_SCANCODE_COMMA, Key(0x20, 3) }, // ,
		{ SDL_SCANCODE_SLASH, Key(0x40, 3) }, // /
		{ SDL_SCANCODE_DOWN, Key(0x80, 3) }, // CURSOR UP/DOWN

		// Column 4
		{ SDL_SCANCODE_SPACE, Key(0x01, 4) }, // SPACE
		{ SDL_SCANCODE_Z, Key(0x02, 4) }, // Z
		{ SDL_SCANCODE_C, Key(0x04, 4) }, // C
		{ SDL_SCANCODE_B, Key(0x08, 4) }, // B
		{ SDL_SCANCODE_M, Key(0x10, 4) }, // M
		{ SDL_SCANCODE_PERIOD, Key(0x20, 4) }, // .
		{ SDL_SCANCODE_RSHIFT, Key(0x40, 4) }, // RIGHT SHIFT
		{ SDL_SCANCODE_F1, Key(0x80, 4) }, // F1

		// Column 5
		{ SDL_SCANCODE_LCTRL, Key(0x01, 5) }, // CBM
		{ SDL_SCANCODE_S, Key(0x02, 5) }, // S
		{ SDL_SCANCODE_F, Key(0x04, 5) }, // F
		{ SDL_SCANCODE_H, Key(0x08, 5) }, // H
		{ SDL_SCANCODE_K, Key(0x10, 5) }, // K
		{ SDL_SCANCODE_SEMICOLON, Key(0x20, 5) }, // :
		{ SDL_SCANCODE_EQUALS, Key(0x40, 5) }, // =
		{ SDL_SCANCODE_F3, Key(0x80, 5) }, // F3

		// Column 6
		{ SDL_SCANCODE_Q, Key(0x01, 6) }, // Q
		{ SDL_SCANCODE_E, Key(0x02, 6) }, // E
		{ SDL_SCANCODE_T, Key(0x04, 6) }, // T
		{ SDL_SCANCODE_U, Key(0x08, 6) }, // U
		{ SDL_SCANCODE_O, Key(0x10, 6) }, // O
		{ SDL_SCANCODE_LEFTBRACKET, Key(0x20, 6) }, // @
		{ SDL_SCANCODE_PAGEUP, Key(0x40, 6) }, // Up arrow
		{ SDL_SCANCODE_F5, Key(0x80, 6) }, // F5

		// Column 7
		{ SDL_SCANCODE_2, Key(0x01, 7) }, // 2
		{ SDL_SCANCODE_KP_2, Key(0x01, 7) }, // 2
		{ SDL_SCANCODE_4, Key(0x02, 7) }, // 4
		{ SDL_SCANCODE_KP_4, Key(0x02, 7) }, // 4
		{ SDL_SCANCODE_6, Key(0x04, 7) }, // 6
		{ SDL_SCANCODE_KP_6, Key(0x04, 7) }, // 6
		{ SDL_SCANCODE_8, Key(0x08, 7) }, // 8
		{ SDL_SCANCODE_KP_8, Key(0x08, 7) }, // 8
		{ SDL_SCANCODE_0, Key(0x10, 7) }, // 0
		{ SDL_SCANCODE_KP_0, Key(0x10, 7) }, // 0
		{ SDL_SCANCODE_MINUS, Key(0x20, 7) }, // -
		{ SDL_SCANCODE_KP_MINUS, Key(0x20, 7) }, // -
		{ SDL_SCANCODE_HOME, Key(0x40, 7) }, // HOME
		{ SDL_SCANCODE_F7, Key(0x80, 7) }, // F7
	};

	DeviceKeyboardVIC20::DeviceKeyboardVIC20() : m_currKeymap(&s_keyMapNormal)
	{
		Reset();
	}
}
