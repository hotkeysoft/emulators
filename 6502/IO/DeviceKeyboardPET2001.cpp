#include "stdafx.h"

#include "DeviceKeyboardPET2001.h"
#include "InputEvents.h"

using events::KeyMap;
using events::Key;
using emul::ComputerPET2001;

namespace kbd
{
	static KeyMap s_keyMapNormal =
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

		// Row 1
		{ SDL_SCANCODE_2, Key(1, 0x01) }, // "
		{ SDL_SCANCODE_4, Key(1, 0x02) }, // $
		{ SDL_SCANCODE_6, Key(1, 0x04) }, // '
		{ SDL_SCANCODE_8, Key(1, 0x08) }, // Backslash
		{ SDL_SCANCODE_0, Key(1, 0x10) }, // )
		//{ , Key(1, 0x20) }, // NC
		{ SDL_SCANCODE_DOWN, Key(1, 0x40) }, // CRSR UP DOWN
		{ SDL_SCANCODE_BACKSPACE, Key(1, 0x80) }, // DEL

		// Row 2
		{ SDL_SCANCODE_Q, Key(2, 0x01) }, // q
		{ SDL_SCANCODE_E, Key(2, 0x02) }, // e
		{ SDL_SCANCODE_T, Key(2, 0x04) }, // t
		{ SDL_SCANCODE_U, Key(2, 0x08) }, // u
		{ SDL_SCANCODE_O, Key(2, 0x10) }, // o
		{ SDL_SCANCODE_BACKSLASH, Key(2, 0x20) }, // PI/EXP
		{ SDL_SCANCODE_KP_7, Key(2, 0x40) }, // Keypad 7
		{ SDL_SCANCODE_KP_9, Key(2, 0x80) }, // Keypad 9

		// Row 3
		{ SDL_SCANCODE_W, Key(3, 0x01) }, // w
		{ SDL_SCANCODE_R, Key(3, 0x02) }, // r
		{ SDL_SCANCODE_Y, Key(3, 0x04) }, // y
		{ SDL_SCANCODE_I, Key(3, 0x08) }, // i
		{ SDL_SCANCODE_P, Key(3, 0x10) }, // p
		//{ , Key(3, 0x20) }, // NC
		{ SDL_SCANCODE_KP_8, Key(3, 0x40) }, // Keypad 8
		{ SDL_SCANCODE_KP_DIVIDE, Key(3, 0x80) }, // Keypad /

		// Row 4
		{ SDL_SCANCODE_A, Key(4, 0x01) }, // a
		{ SDL_SCANCODE_D, Key(4, 0x02) }, // d
		{ SDL_SCANCODE_G, Key(4, 0x04) }, // g
		{ SDL_SCANCODE_J, Key(4, 0x08) }, // j
		{ SDL_SCANCODE_L, Key(4, 0x10) }, // l
		//{ , Key(4, 0x20) }, // NC
		{ SDL_SCANCODE_KP_4, Key(4, 0x40) }, // Keypad 4
		{ SDL_SCANCODE_KP_6, Key(4, 0x80) }, // Keypad 6

		// Row 5
		{ SDL_SCANCODE_S, Key(5, 0x01) }, // s
		{ SDL_SCANCODE_F, Key(5, 0x02) }, // f
		{ SDL_SCANCODE_H, Key(5, 0x04) }, // h
		{ SDL_SCANCODE_K, Key(5, 0x08) }, // k
		{ SDL_SCANCODE_SEMICOLON, Key(5, 0x10) }, // :
		//{ , Key(5, 0x20) }, // NC
		{ SDL_SCANCODE_KP_5, Key(5, 0x40) }, // Keypad 5
		{ SDL_SCANCODE_KP_MULTIPLY, Key(5, 0x80) }, // Keypad *

		// Row 6
		{ SDL_SCANCODE_Z, Key(6, 0x01) }, // z
		{ SDL_SCANCODE_C, Key(6, 0x02) }, // c
		{ SDL_SCANCODE_B, Key(6, 0x04) }, // b
		{ SDL_SCANCODE_M, Key(6, 0x08) }, // m
		{ SDL_SCANCODE_PERIOD, Key(6, 0x10) }, // ;
		{ SDL_SCANCODE_RETURN, Key(6, 0x20) }, // RETURN
		{ SDL_SCANCODE_KP_ENTER, Key(6, 0x20) }, // RETURN
		{ SDL_SCANCODE_KP_1, Key(6, 0x40) }, // Keypad 1
		{ SDL_SCANCODE_KP_3, Key(6, 0x80) }, // Keypad 3

		// Row 7
		{ SDL_SCANCODE_X, Key(7, 0x01) }, // x
		{ SDL_SCANCODE_V, Key(7, 0x02) }, // v
		{ SDL_SCANCODE_N, Key(7, 0x04) }, // n
		{ SDL_SCANCODE_COMMA, Key(7, 0x08) }, // ,
		{ SDL_SCANCODE_SLASH, Key(7, 0x10) }, // ?
		//{ , Key(7, 0x20) }, // NC
		{ SDL_SCANCODE_KP_2, Key(7, 0x40) }, // Keypad 2
		{ SDL_SCANCODE_KP_PLUS, Key(7, 0x80) }, // Keypad +

		// Row 8
		{ SDL_SCANCODE_LSHIFT, Key(8, 0x01) }, // LEFT SHIFT
		{ SDL_SCANCODE_GRAVE, Key(8, 0x02) }, // @
		{ SDL_SCANCODE_RIGHTBRACKET, Key(8, 0x04) }, // ]
		//{ , Key(8, 0x08) }, // NC
		{ SDL_SCANCODE_PAGEDOWN, Key(8, 0x10) }, // >
		{ SDL_SCANCODE_RSHIFT, Key(8, 0x20) }, // RIGHT SHIFT
		{ SDL_SCANCODE_KP_0, Key(8, 0x40) }, // Keypad 0
		{ SDL_SCANCODE_KP_MINUS, Key(8, 0x80) }, // Keypad -

		// Row 9
		{ SDL_SCANCODE_LCTRL, Key(9, 0x01) }, // REVERSE OFF
		{ SDL_SCANCODE_LEFTBRACKET, Key(9, 0x02) }, // [
		{ SDL_SCANCODE_SPACE, Key(9, 0x04) }, // SPACE
		{ SDL_SCANCODE_PAGEUP, Key(9, 0x08) }, // <
		{ SDL_SCANCODE_RCTRL, Key(9, 0x10) }, // RUN STOP
		//{ , Key(9, 0x20) }, // NC
		{ SDL_SCANCODE_KP_PERIOD, Key(9, 0x40) }, // Keypad .
		{ SDL_SCANCODE_EQUALS, Key(9, 0x80) }, // Keypad =
	};

	static KeyMap s_keyMapBusiness =
	{
		// Row 0
		{ SDL_SCANCODE_2, Key(0, 0x01) }, // 2
		{ SDL_SCANCODE_5, Key(0, 0x02) }, // 5
		{ SDL_SCANCODE_8, Key(0, 0x04) }, // 8
		{ SDL_SCANCODE_MINUS, Key(0, 0x08) }, // -
		{ SDL_SCANCODE_KP_8, Key(0, 0x10) }, // Keypad 8
		{ SDL_SCANCODE_RIGHT, Key(0, 0x20) }, // Right cursor
		// { , Key(0, 0x40) }, // Both shifts?
		// { , Key(0, 0x80) }, // . (???)

		// Row 1
		{ SDL_SCANCODE_1, Key(1, 0x01) }, // 1
		{ SDL_SCANCODE_4, Key(1, 0x02) }, // 4
		{ SDL_SCANCODE_7, Key(1, 0x04) }, // 7
		{ SDL_SCANCODE_0, Key(1, 0x08) }, // 0
		{ SDL_SCANCODE_KP_7, Key(1, 0x10) }, // Keypad 7
		{ SDL_SCANCODE_PAGEUP, Key(1, 0x20) }, // Up arrow (power)
		// { , Key(1, 0x40) }, // NC
		{ SDL_SCANCODE_KP_9, Key(1, 0x80) }, // Keypad 9

		// Row 2
		{ SDL_SCANCODE_ESCAPE, Key(2, 0x01) }, // ESC
		{ SDL_SCANCODE_S, Key(2, 0x02) }, // s
		{ SDL_SCANCODE_F, Key(2, 0x04) }, // f
		{ SDL_SCANCODE_H, Key(2, 0x08) }, // h
		{ SDL_SCANCODE_RIGHTBRACKET, Key(2, 0x10) }, // ]
		{ SDL_SCANCODE_K, Key(2, 0x20) }, // k
		{ SDL_SCANCODE_SEMICOLON, Key(2, 0x40) }, // ;
		{ SDL_SCANCODE_KP_5, Key(2, 0x80) }, // Keypad 5

		// Row 3
		{ SDL_SCANCODE_A, Key(3, 0x01) }, // a
		{ SDL_SCANCODE_D, Key(3, 0x02) }, // d
		{ SDL_SCANCODE_G, Key(3, 0x04) }, // g
		{ SDL_SCANCODE_J, Key(3, 0x08) }, // j
		{ SDL_SCANCODE_RETURN, Key(3, 0x10) }, // RETURN
		{ SDL_SCANCODE_KP_ENTER, Key(3, 0x10) }, // RETURN
		{ SDL_SCANCODE_L, Key(3, 0x20) }, // l
		{ SDL_SCANCODE_APOSTROPHE, Key(3, 0x40) }, // @
		{ SDL_SCANCODE_KP_6, Key(3, 0x80) }, // Keypad 6

		// Row 4
		{ SDL_SCANCODE_TAB, Key(4, 0x01) }, // TAB
		{ SDL_SCANCODE_W, Key(4, 0x02) }, // w
		{ SDL_SCANCODE_R, Key(4, 0x04) }, // r
		{ SDL_SCANCODE_Y, Key(4, 0x08) }, // y
		{ SDL_SCANCODE_BACKSLASH, Key(4, 0x10) }, // Backslash
		{ SDL_SCANCODE_I, Key(4, 0x20) }, // i
		{ SDL_SCANCODE_P, Key(4, 0x40) }, // p
		{ SDL_SCANCODE_BACKSPACE, Key(4, 0x80) }, // DEL
		{ SDL_SCANCODE_DELETE, Key(4, 0x80) }, // DEL

		// Row 5
		{ SDL_SCANCODE_Q, Key(5, 0x01) }, // q
		{ SDL_SCANCODE_E, Key(5, 0x02) }, // e
		{ SDL_SCANCODE_T, Key(5, 0x04) }, // t
		{ SDL_SCANCODE_U, Key(5, 0x08) }, // u
		{ SDL_SCANCODE_DOWN, Key(5, 0x10) }, // Down cursor
		{ SDL_SCANCODE_O, Key(5, 0x20) }, // o
		{ SDL_SCANCODE_LEFTBRACKET, Key(5, 0x40) }, // [
		{ SDL_SCANCODE_KP_4, Key(5, 0x80) }, // Keypad 4

		// Row 6
		{ SDL_SCANCODE_LSHIFT, Key(6, 0x01) }, // Left shift
		{ SDL_SCANCODE_C, Key(6, 0x02) }, // c
		{ SDL_SCANCODE_B, Key(6, 0x04) }, // b
		{ SDL_SCANCODE_PERIOD, Key(6, 0x08) }, // . (???)
		{ SDL_SCANCODE_KP_PERIOD, Key(6, 0x10) }, // Keypad .
		//{ , Key(6, 0x20) }, // Left shift + TAB + I
		{ SDL_SCANCODE_RSHIFT, Key(6, 0x40) }, // Right shift
		{ SDL_SCANCODE_KP_3, Key(6, 0x80) }, // Keypad 3

		// Row 7
		{ SDL_SCANCODE_Z, Key(7, 0x01) }, // z
		{ SDL_SCANCODE_V, Key(7, 0x02) }, // v
		{ SDL_SCANCODE_N, Key(7, 0x04) }, // n
		{ SDL_SCANCODE_COMMA, Key(7, 0x08) }, // ,
		{ SDL_SCANCODE_KP_0, Key(7, 0x10) }, // Keypad 0
		//{ , Key(7, 0x20) }, // Z + A + L
		{ SDL_SCANCODE_LALT, Key(7, 0x40) }, // Repeat (???)
		{ SDL_SCANCODE_KP_2, Key(7, 0x80) }, // Keypad 2

		// Row 8
		{ SDL_SCANCODE_LCTRL, Key(8, 0x01) }, // RVS OFF
		{ SDL_SCANCODE_X, Key(8, 0x02) }, // x
		{ SDL_SCANCODE_SPACE, Key(8, 0x04) }, // Space
		{ SDL_SCANCODE_M, Key(8, 0x08) }, // m
		{ SDL_SCANCODE_HOME, Key(8, 0x10) }, // HOME
		// { , Key(8, 0x20) }, // RVS + A + L
		{ SDL_SCANCODE_SLASH, Key(8, 0x40) }, // Slash
		{ SDL_SCANCODE_KP_1, Key(8, 0x80) }, // Keypad 1

		// Row 9
		{ SDL_SCANCODE_GRAVE, Key(9, 0x01) }, // <-
		{ SDL_SCANCODE_3, Key(9, 0x02) }, // 3
		{ SDL_SCANCODE_6, Key(9, 0x04) }, // 6
		{ SDL_SCANCODE_9, Key(9, 0x08) }, // 9
		{ SDL_SCANCODE_RCTRL, Key(9, 0x10) }, // RUN STOP
		{ SDL_SCANCODE_EQUALS, Key(9, 0x20) }, // :
		//{ , Key(9, 0x40) }, // NC
		//{ , Key(9, 0x80) }, // TAB ?

	};

	DeviceKeyboardPET2001::DeviceKeyboardPET2001() : Logger("kbd"), m_currKeymap(&s_keyMapNormal)
	{
		Reset();
	}

	void DeviceKeyboardPET2001::SetModel(ComputerPET2001::Model model)
	{
		switch (model)
		{
		case ComputerPET2001::Model::BASIC1:
		case ComputerPET2001::Model::BASIC1p:
		case ComputerPET2001::Model::BASIC2n:
			LogPrintf(LOG_INFO, "Using Normal keymap");
			m_currKeymap = &s_keyMapNormal;
			break;
		case ComputerPET2001::Model::BASIC2b:
		case ComputerPET2001::Model::BASIC4n:
		case ComputerPET2001::Model::BASIC4b:
			LogPrintf(LOG_INFO, "Using Business keymap");
			m_currKeymap = &s_keyMapBusiness;
			break;
		default:
			LogPrintf(LOG_WARNING, "Unknown/Unsupported model: [%s], using default layout",
				ComputerPET2001::ModelToString(model).c_str());
			m_currKeymap = &s_keyMapNormal;
		}
	}
}
