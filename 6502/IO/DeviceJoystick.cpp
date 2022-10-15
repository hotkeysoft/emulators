#include "stdafx.h"
#include "DeviceJoystick.h"

namespace joy
{
	void DeviceJoystick::Init()
	{
		Reset();
	}

	void DeviceJoystick::JoystickState::Reset()
	{
		connected = false;
		buttons[0] = false;
		buttons[1] = false;
		axisValue[0] = 0;
		axisValue[1] = 0;
	}

	void DeviceJoystick::Reset()
	{
		m_joysticks[0].Reset();
		m_joysticks[1].Reset();
	}

	void DeviceJoystick::SetConnected(uint8_t id, bool connected)
	{
		assert(id <= 1);
		LogPrintf(LOG_INFO, "SetConnected id[%d]=%d", id, connected);

		m_joysticks[id].Reset();
		m_joysticks[id].connected = connected;
	}

	void DeviceJoystick::SetButtonState(uint8_t id, uint8_t button, bool pressed)
	{
		assert(id <= 1);
		assert(button <= 1);
		LogPrintf(LOG_DEBUG, "SetButtonState id[%d][%d]=%d", id, button, pressed);
		m_joysticks[id].buttons[button] = pressed;
	}

	void DeviceJoystick::SetAxisState(uint8_t id, uint8_t axis, int16_t value)
	{
		assert(id <= 1);
		assert(axis <= 1);
		LogPrintf(LOG_DEBUG, "SetAxisState id[%d][%d]=%d", id, axis, value);
		m_joysticks[id].axisValue[axis] = value;
	}
}