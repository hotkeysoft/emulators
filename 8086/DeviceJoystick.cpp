#include "DeviceKeyboard.h"
#include <assert.h>
#include "DeviceJoystick.h"

namespace joy
{
	DeviceJoystick::DeviceJoystick(WORD baseAddress, size_t baseClock) :
		Logger("JOY"),
		m_baseAddress(baseAddress),
		m_baseClock(baseClock)
	{
	}

	void DeviceJoystick::Init()
	{
		Connect(m_baseAddress, static_cast<PortConnector::INFunction>(&DeviceJoystick::ReadJoystick));
		Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&DeviceJoystick::WriteJoystick));
	}

	BYTE DeviceJoystick::ReadJoystick()
	{
		BYTE value =
			(!m_joysticks[1].buttons[1] << 7) |
			(!m_joysticks[1].buttons[0] << 6) |
			(!m_joysticks[0].buttons[1] << 5) |
			(!m_joysticks[0].buttons[0] << 4) |
			((m_joysticks[1].axisCounter[1] != 0) << 3) |
			((m_joysticks[1].axisCounter[0] != 0) << 2) |
			((m_joysticks[0].axisCounter[1] != 0) << 1) |
			((m_joysticks[0].axisCounter[0] != 0) << 0);

		LogPrintf(LOG_DEBUG, "ReadJoystick, value=" PRINTF_BIN_PATTERN_INT8, PRINTF_BYTE_TO_BIN_INT8(value));
		return value;
	}

	void DeviceJoystick::WriteJoystick(BYTE)
	{
		// Value is ignored, writing to the port only resets the counters
		LogPrintf(LOG_DEBUG, "WriteJoystick");
		m_joysticks[0].axisCounter[0] = m_joysticks[0].axisValue[0];
		m_joysticks[0].axisCounter[1] = m_joysticks[0].axisValue[1];
		m_joysticks[1].axisCounter[0] = m_joysticks[1].axisValue[0];
		m_joysticks[1].axisCounter[1] = m_joysticks[1].axisValue[1];
	}

	void DeviceJoystick::SetConnected(uint8_t id, bool connected)
	{
		assert(id <= 1);
		LogPrintf(LOG_INFO, "SetConnected id[%d]=%d", id, connected);
		m_joysticks[id].connected = connected;
		m_joysticks[id].axisValue[0] = 128;
		m_joysticks[id].axisValue[1] = 128;
	}

	void DeviceJoystick::SetButtonState(uint8_t id, uint8_t button, bool pressed)
	{
		assert(id <= 1);
		assert(button <= 1);
		LogPrintf(LOG_DEBUG, "SetButtonState id[%d][%d]=%d", id, button, pressed);
		m_joysticks[id].buttons[button] = pressed;
	}

	void DeviceJoystick::SetAxisState(uint8_t id, uint8_t axis, uint8_t value)
	{
		assert(id <= 1);
		assert(axis <= 1);
		LogPrintf(LOG_DEBUG, "SetAxisState id[%d][%d]=%d", id, axis, value);
		m_joysticks[id].axisValue[axis] = value;
	}

	void DeviceJoystick::Tick()
	{
		static int cooldown = 8;

		if (--cooldown)
		{
			return;
		}

		cooldown = 8;
		if (m_joysticks[0].connected)
		{
			DecrementCount(m_joysticks[0].axisCounter[0]);
			DecrementCount(m_joysticks[0].axisCounter[1]);
		}
		if (m_joysticks[1].connected)
		{
			DecrementCount(m_joysticks[1].axisCounter[0]);
			DecrementCount(m_joysticks[1].axisCounter[1]);
		}
	}
}
