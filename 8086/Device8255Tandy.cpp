#include "Device8255Tandy.h"

namespace ppi
{
	Device8255Tandy::Device8255Tandy(WORD baseAddress) :
		Logger("8255PCjr"),
		Device8255(baseAddress)
	{
	}

	// PORT A: KEYBOARD DATA (OUTPUT)
	// ---------
	// PA7-PA0: Keystroke Storage
	BYTE Device8255Tandy::PORTA_IN()
	{
		LogPrintf(LOG_INFO, "Read Keyboard Storage, value=%02X", m_portAData);
		return m_portAData;
	}
	void Device8255Tandy::PORTA_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "Write Keyboard Storage, value: %d", value);
		m_portAData = value;
	}
	
	// PORT B: OUTPUT
	// ---------
	// PB7: HI: Keyboard Clear
	// PB6: SPKR Switch 1
	// PB5: SPKR Switch 0
	// PB4: Not Used
	// PB3: Not Used
	// PB2: Not Used
	// PB1: Speaker Data Out
	// PB0: Timer2 Gate (Speaker)
	BYTE Device8255Tandy::PORTB_IN()
	{
		LogPrintf(LOG_INFO, "PORTB IN, value=%02X", m_portBData);
		return m_portBData;
	}
	void Device8255Tandy::PORTB_OUT(BYTE value)
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

		if (diff & 0x80) LogPrintf(LOG_INFO, "PB7: Keyboard Clear %s", value & 0x80 ? "ON" : "OFF");
		if (diff & 0x40) LogPrintf(LOG_INFO, "PB6: SPKR Switch 1 %s", value & 0x40 ? "HI" : "LOW");
		if (diff & 0x20) LogPrintf(LOG_INFO, "PB5: SPKR Switch 0 %s", value & 0x20 ? "HI" : "LOW");


		if (diff & 0x02) LogPrintf(LOG_INFO, "PB1: SPEAKER Data %s", value & 0x02 ? "ON" : "OFF");
		if (diff & 0x01) LogPrintf(LOG_INFO, "PB0: TIMER2 Gate %s", value & 0x01 ? "HI" : "LOW");

		m_portBData = value;
	}

	// PORT C
	// 
	// PC4-PC4: Inputs
	// ---------------
	// PC7: Not used
	// PC6: Not used
	// PC5: Timer channel 2 monitoring
	// PC4: Not Used
	// 
	// PC0-PC3: Outputs
	// ----------------
	// PC3: Not used
	// 
	// PC2: Multi-Clock
	// PC1: Multi-Data
	// 
	// PC0: (OUT) Not used
	BYTE Device8255Tandy::PORTC_IN()
	{
		BYTE ret = (m_config & 0b00001111) | 
			(m_timer2Out << 5);

		LogPrintf(LOG_INFO, "PORTC IN  [%cT2OUT %cMCLOCK %cMDATA]",
			m_timer2Out ? ' ' : '/',
			(m_config & 4) ? ' ' : '/',
			(m_config & 2) ? ' ' : '/');

		return ret;
	}
	void Device8255Tandy::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "PORTC OUT, value=%02X", value);
		m_config = value & 0x0F;
	}

}
