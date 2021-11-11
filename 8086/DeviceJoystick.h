#pragma once

#include "Common.h"
#include "PortConnector.h"

using emul::BYTE;
using emul::PortConnector;

namespace joy
{
	class DeviceJoystick : public PortConnector
	{
	public:
		DeviceJoystick(WORD baseAddress, size_t baseClock);

		DeviceJoystick(const DeviceJoystick&) = delete;
		DeviceJoystick& operator=(const DeviceJoystick&) = delete;
		DeviceJoystick(DeviceJoystick&&) = delete;
		DeviceJoystick& operator=(DeviceJoystick&&) = delete;

		virtual void Init();

		virtual void Tick();

		void SetConnected(uint8_t id, bool connected);
		void SetButtonState(uint8_t id, uint8_t button, bool pressed);
		void SetAxisState(uint8_t id, uint8_t axis, uint8_t value);

	protected:
		WORD m_baseAddress;
		size_t m_baseClock;

		BYTE ReadJoystick();
		void WriteJoystick(BYTE);

		static void DecrementCount(uint8_t& count) { if (count) --count; }

		struct JoystickState
		{
			bool connected = false;
			bool buttons[2] = { false, false };
			uint8_t axisValue[2] = { 128, 128 };

			uint8_t axisCounter[2] = { 0, 0 };
		};

		JoystickState m_joysticks[2];
	};
}
