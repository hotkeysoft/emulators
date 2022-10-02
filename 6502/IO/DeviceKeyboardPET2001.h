#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>

#include <map>

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardPET2001
	{
	public:
		DeviceKeyboardPET2001()
		{
			Reset();
		}
		virtual ~DeviceKeyboardPET2001() {}

		DeviceKeyboardPET2001(const DeviceKeyboardPET2001&) = delete;
		DeviceKeyboardPET2001& operator=(const DeviceKeyboardPET2001&) = delete;
		DeviceKeyboardPET2001(DeviceKeyboardPET2001&&) = delete;
		DeviceKeyboardPET2001& operator=(DeviceKeyboardPET2001&&) = delete;

		void Reset() { m_keyGrid.fill(0xFF); }

		void InputKey(BYTE row, BYTE col, bool pressed) { emul::SetBitMask(m_keyGrid[row], col, !pressed); }

		BYTE GetRowData(BYTE row) { return m_keyGrid[row]; }

	protected:
		using KeyGrid = std::array<BYTE, 10>;

		KeyGrid m_keyGrid;
	};
}
