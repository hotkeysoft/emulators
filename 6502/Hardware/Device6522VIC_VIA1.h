#pragma once
#include "Device6522.h"

namespace via
{
	class Device6522VIC_VIA1 : public Device6522
	{
	public:
		Device6522VIC_VIA1(std::string id = "VIA1") : Device6522(id), Logger(id.c_str()) {}

		// CA1
		void SetRestore(bool set) { m_portA.SetC1(set); }

		// CA2
		bool GetCassetteMotorOut() const { return !m_portA.GetC2(); }
	};
}
