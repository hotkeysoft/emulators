#include "Device8255PCjr.h"

namespace ppi
{
	Device8255PCjr::Device8255PCjr(WORD baseAddress) :
		Logger("8255PCjr"),
		Device8255(baseAddress)
	{
	}

	// PORT A: KEYBOARD DATA (OUTPUT)
	// ---------
	// PA7-PA0: Keystroke Storage
	BYTE Device8255PCjr::PORTA_IN()
	{
		LogPrintf(LOG_INFO, "Read Keyboard Storage, value=%02X", m_portAData);
		return m_portAData;
	}
	void Device8255PCjr::PORTA_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "Write Keyboard Storage, value: %d", value);
		m_portAData = value;
	}
	
	// PORT B: OUTPUT
	// ---------
	// PB7: (Reserved)
	// PB6: SPKR Switch 1
	// PB5: SPKR Switch 0
	// PB4: HI: Disable Internal Beeper and Cassette Motor Relay
	// PB3: HI: Cassette Motor OFF
	// PB2: HI: Alphanumeric mode / LOW: Graphics mode
	// PB1: Speaker Data
	// PB0: Timer2 Gate (Speaker)

	BYTE Device8255PCjr::PORTB_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTB IN, value=%02X", m_portBData);
		return m_portBData;
	}
	void Device8255PCjr::PORTB_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTB OUT, value=%02X", value);

		// First time we're call display all bit states, otherwise show changed only
		BYTE diff = m_portBData ^ value;
		static bool firstSet = true;
		if (firstSet)
		{
			diff = 0xFF;
			firstSet = false;
		}

		if (diff & 0x40) LogPrintf(LOG_INFO, "PB6: SPKR Switch 1 %s", value & 0x40 ? "HI" : "LOW");
		if (diff & 0x40) LogPrintf(LOG_INFO, "PB5: SPKR Switch 0 %s", value & 0x40 ? "HI" : "LOW");
		if (diff & 0x10) LogPrintf(LOG_INFO, "PB4: Internal Beeper/Cassette Motor Relay %s", value & 0x10 ? "DISABLE" : "ENABLE");
		if (diff & 0x08) LogPrintf(LOG_INFO, "PB3: Cassette Motor %s", value & 0x08 ? "OFF" : "ON");
		if (diff & 0x04) LogPrintf(LOG_INFO, "PB2: Video Mode %s", value & 0x04 ? "ALPHA" : "GRAPHICS");
		if (diff & 0x02) LogPrintf(LOG_INFO, "PB1: SPEAKER Data %s", value & 0x02 ? "ON" : "OFF");
		if (diff & 0x01) LogPrintf(LOG_INFO, "PB0: TIMER2 Gate %s", value & 0x01 ? "HI" : "LOW");

		m_portBData = value;
	}

	// PORT C: INPUT
	// ---------

	// PC7: (CFG) Keyboard Cable Connected: HI: no, LOW: yes

	// PC6: Keyboard Data
	// PC5: Timer channel 2 monitoring
	// PC4: Cassette Data In
	// 
	// PC3: (CFG) 64KB Expansion Installed: HI: no, LOW: yes
	// PC2: (CFG) Diskette Drive Card Installed: HI: no, LOW: yes
	// PC1: (CFG) Internal MODEM Card Installed: HI: no, LOW: yes
	// 
	// PC0: Keyboard Latched
	BYTE Device8255PCjr::PORTC_IN()
	{
		BYTE ret = (m_config & 0b10001110) | 
			(m_keyboardDataBit << 6) |
			(m_timer2Out << 5) | 
			(m_cassetteDataBit << 4) |
			(m_nmiLatch << 0);

		LogPrintf(LOG_DEBUG, "PORTC IN, ret=%02X", ret);
		return ret;
	}
	void Device8255PCjr::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTC OUT, value=%02X", value);
	}

	// PC7: (CFG) Keyboard Cable Connected: HI: no, LOW: yes
	void Device8255PCjr::SetKeyboardConnected(bool value)
	{
		emul::SetBit(m_config, 7, !value);
	}

	// PC3: (CFG) 64KB Expansion Installed: HI: no, LOW: yes
	void Device8255PCjr::SetRAMExpansion(bool value)
	{
		emul::SetBit(m_config, 3, !value);
	}

	// PC2: (CFG) Diskette Drive Card Installed: HI: no, LOW: yes
	void Device8255PCjr::SetDisketteCard(bool value)
	{
		emul::SetBit(m_config, 2, !value);
	}

	// PC1: (CFG) Internal MODEM Card Installed: HI: no, LOW: yes
	void Device8255PCjr::SetModemCard(bool value)
	{
		emul::SetBit(m_config, 1, !value);
	}

}
