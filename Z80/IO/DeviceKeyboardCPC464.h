#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>

namespace kbd
{
	class DeviceKeyboardCPC464 : public DeviceKeyboard, public Logger
	{
	public:
		DeviceKeyboardCPC464();
		virtual ~DeviceKeyboardCPC464() {}

		DeviceKeyboardCPC464(const DeviceKeyboardCPC464&) = delete;
		DeviceKeyboardCPC464& operator=(const DeviceKeyboardCPC464&) = delete;
		DeviceKeyboardCPC464(DeviceKeyboardCPC464&&) = delete;
		DeviceKeyboardCPC464& operator=(DeviceKeyboardCPC464&&) = delete;

		virtual void Reset() override { m_keyGrid.fill(0xFF); }

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { emul::SetBitMask(m_keyGrid[col], row, !pressed); }

		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }
		virtual BYTE GetRowData(BYTE row) const override { return m_keyGrid[row]; }

	protected:
		using KeyGrid = std::array<emul::BYTE, 10>;

		KeyGrid m_keyGrid;
		events::KeyMap* m_currKeymap;
	};
}
