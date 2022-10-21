#pragma once

#include <CPU/CPUCommon.h>
#include <map>

using emul::BYTE;

namespace joy
{
	class DeviceJoystick : public Logger
	{
	public:
		DeviceJoystick() : Logger("joy") {}
		virtual ~DeviceJoystick() {}

		DeviceJoystick(const DeviceJoystick&) = delete;
		DeviceJoystick& operator=(const DeviceJoystick&) = delete;
		DeviceJoystick(DeviceJoystick&&) = delete;
		DeviceJoystick& operator=(DeviceJoystick&&) = delete;

		virtual void Init();
		virtual void Reset();

		virtual void SetConnected(uint8_t id, bool connected);
		virtual void SetButtonState(uint8_t id, uint8_t button, bool pressed);
		virtual void SetAxisState(uint8_t id, uint8_t axis, int16_t value);

	protected:
		struct JoystickState
		{
			void Reset();
			bool connected = false;
			bool buttons[2] = { false, false };
			int16_t axisValue[2] = { };
		};

		JoystickState m_joysticks[2];
	};
}
