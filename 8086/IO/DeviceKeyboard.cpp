#include "stdafx.h"

#include "DeviceKeyboard.h"

namespace kbd
{
	DeviceKeyboard::DeviceKeyboard() : Logger("KBD")
	{
	}

	void DeviceKeyboard::Init(ppi::DevicePPI* ppi, pic::Device8259* pic)
	{
		assert(ppi);
		assert(pic);
		m_ppi = ppi;
		m_pic = pic;
	}
}
