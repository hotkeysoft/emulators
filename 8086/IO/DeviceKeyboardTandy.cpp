#include "stdafx.h"

#include "../Hardware/Device8255Tandy.h"
#include "DeviceKeyboardTandy.h"

namespace kbd
{
	DeviceKeyboardTandy::DeviceKeyboardTandy() : Logger("kbdTandy")
	{
	}

	void DeviceKeyboardTandy::Init(ppi::DevicePPI* ppi, pic::Device8259* pic)
	{
		assert(dynamic_cast<ppi::Device8255Tandy*>(ppi));
		assert(pic);
		DeviceKeyboard::Init(ppi, pic);
		m_pic->InterruptRequest(1, false);
	}

	void DeviceKeyboardTandy::Tick()
	{
		static int cooldown = 0;
		static int lastBusy = false;

		ppi::Device8255Tandy* ppi = (ppi::Device8255Tandy*)(m_ppi);
		bool busy = ppi->IsKeyboardBusy();

		// Busy 1->0, clear IRQ1
		if (lastBusy && !busy)
		{
			LogPrintf(LOG_DEBUG, "Clear interrupt");
			m_pic->InterruptRequest(1, false);
		}
		lastBusy = busy;

		if (cooldown)
		{
			--cooldown;
			return;
		}

		if (!busy && (m_keyBufRead != m_keyBufWrite))
		{
			char currChar = m_keyBuf[m_keyBufRead++];
			LogPrintf(LOG_DEBUG, "Interrupt, key=%02Xh", currChar);
			m_ppi->SetCurrentKeyCode(currChar);
			m_pic->InterruptRequest(1);
			cooldown = 10000;
		}
	}
}
