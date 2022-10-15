#pragma once

#include <CPU/CPUCommon.h>
#include "DeviceJoystick.h"

namespace joy
{
	class DeviceJoystickDigital : public DeviceJoystick
	{
	public:
		DeviceJoystickDigital() {}
		virtual ~DeviceJoystickDigital() {}

		virtual void Init() override;

		DeviceJoystickDigital(const DeviceJoystickDigital&) = delete;
		DeviceJoystickDigital& operator=(const DeviceJoystickDigital&) = delete;
		DeviceJoystickDigital(DeviceJoystickDigital&&) = delete;
		DeviceJoystickDigital& operator=(DeviceJoystickDigital&&) = delete;

		bool GetUp() const { return m_joysticks[0].axisValue[1] <= (-m_threshold); }
		bool GetDown() const { return m_joysticks[0].axisValue[1] >= (m_threshold); }
		bool GetLeft() const { return m_joysticks[0].axisValue[0] <= (-m_threshold); }
		bool GetRight() const { return m_joysticks[0].axisValue[0] >= (m_threshold); }
		bool GetFire() const { return m_joysticks[0].buttons[0]; }

	protected:
		int16_t m_threshold = 10000;
	};
}
