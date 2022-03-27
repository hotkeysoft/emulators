#pragma once

#include "DeviceKeyboard.h"

using emul::BYTE;

namespace kbd
{
	class DeviceKeyboardTandy : public DeviceKeyboard
	{
	public:
		DeviceKeyboardTandy();

		DeviceKeyboardTandy(const DeviceKeyboardTandy&) = delete;
		DeviceKeyboardTandy& operator=(const DeviceKeyboardTandy&) = delete;
		DeviceKeyboardTandy(DeviceKeyboardTandy&&) = delete;
		DeviceKeyboardTandy& operator=(DeviceKeyboardTandy&&) = delete;

		virtual void Init(ppi::DevicePPI* ppi, pic::Device8259* pic) override;

		virtual void Tick() override;

	protected:

	};
}
