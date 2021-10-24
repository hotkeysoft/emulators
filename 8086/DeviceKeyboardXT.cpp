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
		assert(ppi);
		assert(pic);
		DeviceKeyboard::Init(ppi, pic);
	}

	void DeviceKeyboardXT::Tick()
	{
		static int cooldown = 0;
		static bool keySent = false;

		if (cooldown)
		{
			--cooldown;
			return;
		}

		cooldown = 10000;

		if (keySent)
		{
			keySent = false;
			m_pic->InterruptRequest(1, false);
		}
		else if (m_keyBufRead != m_keyBufWrite)
		{
			m_ppi->SetCurrentKeyCode(m_keyBuf[m_keyBufRead++]);
			m_pic->InterruptRequest(1);
			keySent = true;
		}

	}
}
