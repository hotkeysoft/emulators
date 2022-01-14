#include "DeviceKeyboardPCjr.h"
#include "../Hardware/Device8255PCjr.h"
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
		for (int i = 0; i < 8; ++i)
		{
			Connect(m_baseAddress + i, static_cast<PortConnector::INFunction>(&DeviceKeyboardPCjr::ReadPortA0));
			Connect(m_baseAddress + i, static_cast<PortConnector::OUTFunction>(&DeviceKeyboardPCjr::WritePortA0));
		}
	}

	BYTE DeviceKeyboardPCjr::ReadPortA0()
	{
		LogPrintf(LOG_DEBUG, "Clear NMI Latch");
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

	bool DeviceKeyboardPCjr::NMIPending()
	{
		static bool lastNMI = false;
		bool nmi = m_nmiLatch && m_portA0.enableNMI;
		bool ret = (nmi && !lastNMI); // Trigger only on low-to-high transitions
		lastNMI = nmi;
		return ret;
	}

	void DeviceKeyboardPCjr::LoadKey(BYTE key)
	{
		LogPrintf(LOG_DEBUG, "LoadKey [%02Xh]", key);

		// Start bit
		PushBit(1);

		// Char data
		BYTE parity = 1; // odd parity
		for (int i = 0; i < 8; ++i)
		{
			parity += (key & 1);
			PushBit(key & 1);
			key >>= 1;
		}

		// Parity
		PushBit(parity & 1);

		// Stop bits
		for (int i = 0; i < 11; ++i)
		{
			PushStop();
		}
	}

	void DeviceKeyboardPCjr::PushBit(bool bit)
	{
		// Biphase signal
		m_keySerialData.push_back(bit);
		m_keySerialData.push_back(!bit);
	}

	void DeviceKeyboardPCjr::PushStop()
	{
		m_keySerialData.push_back(0);
		m_keySerialData.push_back(0);
	}

	void DeviceKeyboardPCjr::SendBit()
	{
		ppi::Device8255PCjr* ppi = (ppi::Device8255PCjr*)m_ppi;

		bool bit = m_keySerialData.front();
		m_keySerialData.pop_front();

		ppi->SetKeyboardDataBit(bit);
		LogPrintf(LOG_DEBUG, "SendBit, b=%d", bit);
		if (bit)
		{
			m_nmiLatch = true;
		}
	}

	void DeviceKeyboardPCjr::Tick()
	{
		ppi::Device8255PCjr* ppi = (ppi::Device8255PCjr*)m_ppi;

		static WORD wait = 0;
		if (wait)
		{
			--wait;
		}
		else if (IsSendingKey())
		{
			SendBit();
			wait = 263; // 220 microseconds
		}
		else if (m_keyBufRead != m_keyBufWrite)
		{
			// Prepare key for serial transmission
			LoadKey(m_keyBuf[m_keyBufRead++]);
		}

		ppi->SetNMILatch(m_nmiLatch);
	}
}
