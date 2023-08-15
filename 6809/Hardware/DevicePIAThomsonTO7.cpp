#include "stdafx.h"

#include "DevicePIAThomsonTO7.h"
#include <IO/DeviceKeyboard.h>

namespace pia::thomson
{
	DevicePIAThomsonTO7::DevicePIAThomsonTO7() : m_pia6520("pia6520")
	{
	}

	void DevicePIAThomsonTO7::Init(kbd::DeviceKeyboard* kbd)
	{
		m_pia6520.Init(true);

		SetKeyboard(kbd);
	}

	void DevicePIAThomsonTO7::Serialize(json& to)
	{
		m_pia6520.Serialize(to["6520"]);
	}

	void DevicePIAThomsonTO7::Deserialize(const json& from)
	{
		m_pia6520.Deserialize(from["6520"]);
	}
}
