#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>

namespace kbd
{
	class DeviceKeyboardCPC : public DeviceKeyboard, public Logger
	{
	public:
		DeviceKeyboardCPC();
		virtual ~DeviceKeyboardCPC() {}

		DeviceKeyboardCPC(const DeviceKeyboardCPC&) = delete;
		DeviceKeyboardCPC& operator=(const DeviceKeyboardCPC&) = delete;
		DeviceKeyboardCPC(DeviceKeyboardCPC&&) = delete;
		DeviceKeyboardCPC& operator=(DeviceKeyboardCPC&&) = delete;

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
