#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <IO/DeviceKeyboard.h>

#include <map>

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardZX80 : public DeviceKeyboard
	{
	public:
		DeviceKeyboardZX80();
		virtual ~DeviceKeyboardZX80() {}

		DeviceKeyboardZX80(const DeviceKeyboardZX80&) = delete;
		DeviceKeyboardZX80& operator=(const DeviceKeyboardZX80&) = delete;
		DeviceKeyboardZX80(DeviceKeyboardZX80&&) = delete;
		DeviceKeyboardZX80& operator=(DeviceKeyboardZX80&&) = delete;

		virtual void Reset() override;

		virtual void InputKey(BYTE row, BYTE col, bool pressed) override { emul::SetBitMask(m_keyGrid[row], col, pressed); }

		virtual events::KeyMap& GetKeymap() const override { return *m_currKeymap; }

		virtual BYTE GetRowData(BYTE row) const override { return m_keyGrid.at(row); }

	protected:
		using KeyGrid = std::map<BYTE, BYTE>;

		KeyGrid m_keyGrid;
		events::KeyMap* m_currKeymap;
	};
}
