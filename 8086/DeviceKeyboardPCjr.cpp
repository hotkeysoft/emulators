#include "DeviceKeyboardPCjr.h"
#include "Device8255PCjr.h"
#include <assert.h>

namespace kbd
{
	DeviceKeyboardPCjr::DeviceKeyboardPCjr(WORD baseAddress) : 
		Logger("kbdPCjr"), 
		DeviceKeyboard(),
		m_baseAddress(baseAddress)
	{
	}

	void DeviceKeyboardPCjr::Init(ppi::Device8255* ppi, pic::Device8259* pic)
	{
		assert(dynamic_cast<ppi::Device8255PCjr*>(ppi));
		DeviceKeyboard::Init(ppi, pic);

		// Port A0 is not strictly part of the keyboard but it handles NMI masking and a keyboard IR bit
		//
		// the other bits are Timer1-related and HRQ(unimplemented)
		Connect(m_baseAddress, static_cast<PortConnector::INFunction>(&DeviceKeyboardPCjr::ReadPortA0));
		Connect(m_baseAddress, static_cast<PortConnector::OUTFunction>(&DeviceKeyboardPCjr::WritePortA0));
	}

	BYTE DeviceKeyboardPCjr::ReadPortA0()
	{
		LogPrintf(LOG_INFO, "Clear NMI Latch");
		m_nmiLatch = false;
		return 0xFF;
	}

	void DeviceKeyboardPCjr::WritePortA0(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WritePort40, value=%02Xh", value);
		
		m_portA0.enableNMI = value & 0x80;
		m_portA0.irTestEnabled = value & 0x40;
		m_portA0.selectCLK1Input = value & 0x20;
		m_portA0.disableHRQ = value & 0x10;

		LogPrintf(Logger::LOG_INFO, "WritePort40 [%cEnableNMI %cIRTest %cSelCLK1Input %cDisableHRQ]",
			m_portA0.enableNMI ? ' ' : '/',
			m_portA0.irTestEnabled ? ' ' : '/',
			m_portA0.selectCLK1Input ? ' ' : '/',
			m_portA0.disableHRQ ? ' ' : '/');
	}

	void DeviceKeyboardPCjr::Tick()
	{
		ppi::Device8255PCjr* ppi = (ppi::Device8255PCjr*)m_ppi;

		static int cooldown = 0;
		if (m_keyBufRead != m_keyBufWrite && !cooldown)
		{
			if (m_portA0.enableNMI)
			{

			}
			else
			{
				// NMI disabled: eat the key, set nmi latch
				LogPrintf(LOG_DEBUG, "NMI Disabled, dropping key");
				++m_keyBufRead;
				m_nmiLatch = true;
			}
			//((ppi::Device8255XT*)m_ppi)->SetCurrentKeyCode(m_keyBuf[m_keyBufRead++]);
			//m_pic->InterruptRequest(1);
			cooldown = 10000;
		}

		ppi->SetNMILatch(m_nmiLatch);

		if (cooldown)
		{
			--cooldown;
		}
	}
}
