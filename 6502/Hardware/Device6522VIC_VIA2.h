#pragma once
#include "Device6522.h"

namespace kbd { class DeviceKeyboard; }

namespace via
{
	class Device6522VIC_VIA2 : public Device6522
	{
	public:
		Device6522VIC_VIA2(std::string id = "VIA2") : Device6522(id), Logger(id.c_str()) {}

		void Init(kbd::DeviceKeyboard* kbd);

		BYTE GetKeyboardColumnSelect() const { return m_portB.GetOutput(); }

		void SetKeyboardRow(BYTE data) { m_portA.SetInput(data); }

	protected:
		virtual void OnReadPort(VIAPort* src) override;

		kbd::DeviceKeyboard* m_keyboard = nullptr;
	};
}

