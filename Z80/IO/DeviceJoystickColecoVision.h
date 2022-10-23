#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>
#include <IO/DeviceJoystickDigital.h>

#include <map>

using emul::BYTE;

namespace joy
{
	class DeviceJoystickColecoVision :
		public kbd::DeviceKeyboard,
		public joy::DeviceJoystickDigital,
		public emul::PortConnector
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

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { m_keypad = pressed ? row : 0xFF; }

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

		BYTE GetKeypad() const {
			return (m_keypad & 0x0F) | // 0..3
				(1 << 4) |
				(1 << 5) |
				((!GetFire2()) << 6) |
				(1 << 7);
		}

		BYTE GetJoystick() const {
			const BYTE ret =
				(GetUp() << 0) |
				(GetRight() << 1) |
				(GetDown() << 2) |
				(GetLeft() << 3) |
				(GetFire() << 6);
			return ~ret;
		}

		events::KeyMap* m_currKeymap;
	};
}
