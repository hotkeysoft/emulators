#pragma once

#include "../Common.h"
#include "../CPU/PortConnector.h"

#include <map>

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardZX80
	{
	public:
		DeviceKeyboardZX80() {}
		virtual ~DeviceKeyboardZX80() {}

		DeviceKeyboardZX80(const DeviceKeyboardZX80&) = delete;
		DeviceKeyboardZX80& operator=(const DeviceKeyboardZX80&) = delete;
		DeviceKeyboardZX80(DeviceKeyboardZX80&&) = delete;
		DeviceKeyboardZX80& operator=(DeviceKeyboardZX80&&) = delete;

		void InputKey(BYTE row, BYTE col, bool pressed) { emul::SetBitMask(m_keyGrid[row], col, pressed); }

		BYTE GetRowData(BYTE row) { return m_keyGrid[row]; }

	protected:
		using KeyGrid = std::map<BYTE, BYTE>;

		KeyGrid m_keyGrid;
	};
}
