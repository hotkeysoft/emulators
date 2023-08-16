#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>
#include "../ComputerThomson.h"

namespace kbd
{
	class DeviceKeyboardThomson : public DeviceKeyboard, public Logger
	{
	public:
		DeviceKeyboardThomson();
		virtual ~DeviceKeyboardThomson() {}

		DeviceKeyboardThomson(const DeviceKeyboardThomson&) = delete;
		DeviceKeyboardThomson& operator=(const DeviceKeyboardThomson&) = delete;
		DeviceKeyboardThomson(DeviceKeyboardThomson&&) = delete;
		DeviceKeyboardThomson& operator=(DeviceKeyboardThomson&&) = delete;

		virtual void Reset() override { m_keyGrid.fill(0xFF); }

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { emul::SetBitMask(m_keyGrid[col], row, !pressed); }

		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }
		virtual BYTE GetRowData(BYTE row) const override { return m_keyGrid[row]; }

		void SetModel(emul::ComputerThomson::Model model);

	protected:
		using KeyGrid = std::array<emul::BYTE, 8>;

		KeyGrid m_keyGrid;
		events::KeyMap* m_currKeymap;
	};
}
