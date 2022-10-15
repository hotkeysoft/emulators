#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <map>
#include "DeviceKeyboard.h"

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardVIC20 : public DeviceKeyboard
	{
	public:
		DeviceKeyboardVIC20();
		virtual ~DeviceKeyboardVIC20() {}

		DeviceKeyboardVIC20(const DeviceKeyboardVIC20&) = delete;
		DeviceKeyboardVIC20& operator=(const DeviceKeyboardVIC20&) = delete;
		DeviceKeyboardVIC20(DeviceKeyboardVIC20&&) = delete;
		DeviceKeyboardVIC20& operator=(DeviceKeyboardVIC20&&) = delete;

		virtual void Reset() override { m_keyGrid.fill(0xFF); }

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { emul::SetBitMask(m_keyGrid[col], row, !pressed); }

		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }

		virtual BYTE GetRowData(BYTE col) const override {
			BYTE ret = 0xFF;
			// TODO: ugh. VIC20 checks for "any key" first by setting col = 0 (meaning, all columns)
			// instead of looking column by columns (which it does later if there is a key pressed).
			// This is simplest and ugliest way to do it.
			for (int i = 0; i < 8; ++i)
			{
				if (!emul::GetBit(col, i))
				{
					ret &= m_keyGrid[i];
				}
			}
			return ret;
		}

		bool IsRestorePressed() const { return m_keyGrid[9] != 0xFF; }

	protected:
		using KeyGrid = std::array<BYTE, 9>;

		KeyGrid m_keyGrid;
		events::KeyMap* m_currKeymap;
	};
}
