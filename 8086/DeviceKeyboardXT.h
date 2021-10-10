#pragma once

#include "Common.h"
#include "Logger.h"

#include "DeviceKeyboard.h"

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardXT : public DeviceKeyboard
	{
	public:
		DeviceKeyboardXT();

		DeviceKeyboardXT(const DeviceKeyboardXT&) = delete;
		DeviceKeyboardXT& operator=(const DeviceKeyboardXT&) = delete;
		DeviceKeyboardXT(DeviceKeyboardXT&&) = delete;
		DeviceKeyboardXT& operator=(DeviceKeyboardXT&&) = delete;

		virtual void Init(ppi::Device8255* ppi, pic::Device8259* pic) override;

		virtual void Tick() override;

	protected:

	};
}
