#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>

#include <map>

using emul::BYTE;

namespace joy
{
	class DeviceJoystickColecoVision : public kbd::DeviceKeyboard, public emul::PortConnector
	{
	public:
		DeviceJoystickColecoVision();
		virtual ~DeviceJoystickColecoVision() {}

		DeviceJoystickColecoVision(const DeviceJoystickColecoVision&) = delete;
		DeviceJoystickColecoVision& operator=(const DeviceJoystickColecoVision&) = delete;
		DeviceJoystickColecoVision(DeviceJoystickColecoVision&&) = delete;
		DeviceJoystickColecoVision& operator=(DeviceJoystickColecoVision&&) = delete;

		void Init();

		virtual void Reset() override;

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { m_keypad = pressed ? ~row : 0xFF; }

		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }

		virtual BYTE GetRowData(BYTE row) const override { return m_keypad; }

	protected:
		enum class Mode {JOYSTICK, KEYPAD};
		Mode m_mode = Mode::JOYSTICK;

		void SetKeypadMode(BYTE);
		void SetJoystickMode(BYTE);
		BYTE ReadController1();
		BYTE ReadController2();

		BYTE m_keypad = 0xFF;
		BYTE m_joystick = 0xFF;

		events::KeyMap* m_currKeymap;
	};
}
