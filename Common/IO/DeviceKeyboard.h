#pragma once

#include <CPU/CPUCommon.h>
#include <IO/InputEvents.h>

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboard : public Logger
	{
	public:
		DeviceKeyboard() : Logger("kbd") {}
		virtual ~DeviceKeyboard() {}

		DeviceKeyboard(const DeviceKeyboard&) = delete;
		DeviceKeyboard& operator=(const DeviceKeyboard&) = delete;
		DeviceKeyboard(DeviceKeyboard&&) = delete;
		DeviceKeyboard& operator=(DeviceKeyboard&&) = delete;

		virtual void Reset() = 0;

		virtual void InputKey(BYTE row, BYTE col, bool pressed) = 0;

		virtual events::KeyMap& GetKeymap() const = 0;
		virtual BYTE GetRowData(BYTE row) const = 0;
	};
}
