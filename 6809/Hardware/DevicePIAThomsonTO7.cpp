#include "stdafx.h"

#include "DevicePIAThomsonTO7.h"
#include <IO/DeviceKeyboard.h>

namespace pia::thomson
{
	DevicePIAThomsonTO7::DevicePIAThomsonTO7() :
		m_pia6846("pia6846"),
		m_pia6520("pia6520")
	{
	}

	void DevicePIAThomsonTO7::Reset()
	{
		m_pia6846.Reset();
		m_pia6520.Reset();
	}

	void DevicePIAThomsonTO7::Init(kbd::DeviceKeyboard* kbd)
	{
		m_pia6846.Init();
		m_pia6520.Init(kbd);
	}

	void DevicePIAThomsonTO7::Serialize(json& to)
	{
		m_pia6846.Serialize(to["6846"]);
		m_pia6520.Serialize(to["6520"]);
	}

	void DevicePIAThomsonTO7::Deserialize(const json& from)
	{
		m_pia6846.Deserialize(from["6846"]);
		m_pia6520.Deserialize(from["6520"]);
	}
}
