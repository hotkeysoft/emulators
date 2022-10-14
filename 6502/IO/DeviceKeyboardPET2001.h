#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include "../ComputerPET2001.h"
#include "DeviceKeyboard.h"
#include <map>

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardPET2001 : public DeviceKeyboard
	{
	public:
		DeviceKeyboardPET2001();
		virtual ~DeviceKeyboardPET2001() {}

		DeviceKeyboardPET2001(const DeviceKeyboardPET2001&) = delete;
		DeviceKeyboardPET2001& operator=(const DeviceKeyboardPET2001&) = delete;
		DeviceKeyboardPET2001(DeviceKeyboardPET2001&&) = delete;
		DeviceKeyboardPET2001& operator=(DeviceKeyboardPET2001&&) = delete;

		virtual void Reset() override { m_keyGrid.fill(0xFF); }

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { emul::SetBitMask(m_keyGrid[row], col, !pressed); }

		void SetModel(emul::ComputerPET2001::Model model);
		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }
		virtual BYTE GetRowData(BYTE row) const override { return m_keyGrid[row]; }

	protected:
		using KeyGrid = std::array<BYTE, 10>;

		KeyGrid m_keyGrid;
		events::KeyMap* m_currKeymap;
	};
}
