#pragma once
#include "Device6522.h"

namespace joy { class DeviceJoystick; }
namespace joy { class DeviceJoystickDigital; }

namespace via
{
	class Device6522VIC_VIA1 : public Device6522
	{
	public:
		Device6522VIC_VIA1(std::string id = "VIA1") : Device6522(id), Logger(id.c_str()) {}

		void Init(joy::DeviceJoystick* m_joystick);

		// CA1
		void SetRestore(bool set) { m_portA.SetC1(set); }

		// CA2
		bool GetCassetteMotorOut() const { return !m_portA.GetC2(); }

		// PA6 - Cassette switch
		void SetCassetteSwitchIn(bool set) { m_portA.SetInputBit(6, !set); }

	protected:
		virtual void OnReadPort(VIAPort* src) override;

		joy::DeviceJoystickDigital* m_joystick = nullptr;
	};
}
