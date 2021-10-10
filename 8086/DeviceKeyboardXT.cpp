#include "DeviceKeyboardXT.h"
#include "Device8255XT.h"
#include <assert.h>

namespace kbd
{
	DeviceKeyboardXT::DeviceKeyboardXT() : Logger("kbdXT")
	{
	}

	void DeviceKeyboardXT::Init(ppi::Device8255* ppi, pic::Device8259* pic)
	{
		assert(dynamic_cast<ppi::Device8255XT*>(ppi));
		DeviceKeyboard::Init(ppi, pic);
	}

	void DeviceKeyboardXT::Tick()
	{
		static int cooldown = 0;
		if (m_keyBufRead != m_keyBufWrite && !cooldown)
		{
			((ppi::Device8255XT*)m_ppi)->SetCurrentKeyCode(m_keyBuf[m_keyBufRead++]);
			m_pic->InterruptRequest(1);
			cooldown = 10000;
		}

		if (cooldown)
		{
			--cooldown;
		}
	}
}
