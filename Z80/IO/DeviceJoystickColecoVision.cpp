#include "stdafx.h"

#include "DeviceJoystickColecoVision.h"
#include <IO/InputEvents.h>

using events::KeyMap;
using events::Key;

namespace joy
{
	// Keypad is not scanned like a matrix. Only one key at a
	// time is allowed
	static KeyMap s_keyMapColecoKeypad =
	{
		//{ , Key(0x0, 0) }, // invalid
		{ SDL_SCANCODE_KP_8, Key(0x1, 0) },
		{ SDL_SCANCODE_KP_4, Key(0x2, 0) },
		{ SDL_SCANCODE_KP_5, Key(0x3, 0) },
		//{ , Key(0x4, 0) }, // invalid
		{ SDL_SCANCODE_KP_7, Key(0x5, 0) },
		{ SDL_SCANCODE_KP_PERIOD, Key(0x6, 0) }, // #
		{ SDL_SCANCODE_KP_2, Key(0x7, 0) },
		//{ , Key(0x8, 0) }, // invalid
		{ SDL_SCANCODE_KP_MULTIPLY, Key(0x9, 0) }, // *
		{ SDL_SCANCODE_KP_0, Key(0xA, 0) },
		{ SDL_SCANCODE_KP_9, Key(0xB, 0) },
		{ SDL_SCANCODE_KP_3, Key(0xC, 0) },
		{ SDL_SCANCODE_KP_1, Key(0xD, 0) },
		{ SDL_SCANCODE_KP_6, Key(0xE, 0) },
		//{ , Key(0xF, 0) }, // invalid - no button down
	};

	DeviceJoystickColecoVision::DeviceJoystickColecoVision() : m_currKeymap(&s_keyMapColecoKeypad), Logger("ColecoJoystick")
	{
		Reset();
	}

	void DeviceJoystickColecoVision::Init()
	{
		Reset();

		// TODO: Need masking in io ports to simplify this

		for (WORD offset = 0; offset < 32; ++offset)
		{
			Connect(0x80 + offset, static_cast<PortConnector::OUTFunction>(&DeviceJoystickColecoVision::SetKeypadMode));
			Connect(0xC0 + offset, static_cast<PortConnector::OUTFunction>(&DeviceJoystickColecoVision::SetJoystickMode));
		}

		for (WORD offset = 0; offset < 32; offset += 2)
		{
			Connect(0xE0 + offset + 0, static_cast<PortConnector::INFunction>(&DeviceJoystickColecoVision::ReadController1));
			Connect(0xE0 + offset + 1, static_cast<PortConnector::INFunction>(&DeviceJoystickColecoVision::ReadController2));
		}
	}

	void DeviceJoystickColecoVision::Reset()
	{
		m_keypad = 0xFF;
	}

	void DeviceJoystickColecoVision::SetKeypadMode(BYTE)
	{
		LogPrintf(LOG_DEBUG, "Set Keypad Mode");
		m_mode = Mode::KEYPAD;
	}
	void DeviceJoystickColecoVision::SetJoystickMode(BYTE)
	{
		LogPrintf(LOG_DEBUG, "Set Joystick Mode");
		m_mode = Mode::JOYSTICK;
	}

	BYTE DeviceJoystickColecoVision::ReadController1()
	{
		LogPrintf(LOG_DEBUG, "ReadController1");
		return m_mode == Mode::KEYPAD ? (m_keypad | 0x70) : ~GetJoystick();
	}

	BYTE DeviceJoystickColecoVision::ReadController2()
	{
		LogPrintf(LOG_DEBUG, "ReadController2");
		// Not supported for now
		return 0xFF;
	}
}
