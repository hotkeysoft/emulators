#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>

namespace kbd
{
	class DeviceKeyboardMC10 : public DeviceKeyboard, public Logger
	{
	public:
		DeviceKeyboardMC10();
		virtual ~DeviceKeyboardMC10() {}

		DeviceKeyboardMC10(const DeviceKeyboardMC10&) = delete;
		DeviceKeyboardMC10& operator=(const DeviceKeyboardMC10&) = delete;
		DeviceKeyboardMC10(DeviceKeyboardMC10&&) = delete;
		DeviceKeyboardMC10& operator=(DeviceKeyboardMC10&&) = delete;

		virtual void Reset() override { m_keyGrid.fill(0xFF); }

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { emul::SetBitMask(m_keyGrid[row], col, !pressed); }

		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }
		virtual BYTE GetRowData(BYTE row) const override { return m_keyGrid[row]; }

	protected:
		using KeyGrid = std::array<emul::BYTE, 8>;

		KeyGrid m_keyGrid;
		events::KeyMap* m_currKeymap;
	};
}
