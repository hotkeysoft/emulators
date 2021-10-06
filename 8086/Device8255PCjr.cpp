#include "Device8255PCjr.h"

namespace ppi
{
	Device8255PCjr::Device8255PCjr(WORD baseAddress) :
		Logger("8255PCjr"),
		Device8255(baseAddress)
	{
	}

	// PORT A: KEYBOARD DATA (INPUT)
	// ---------
	// PA7-PA0: Received keyboard byte
	// 
	// Configured as OUTPUT during POST
	// Outputs diagnostic code
	BYTE Device8255PCjr::PORTA_IN()
	{
		LogPrintf(LOG_DEBUG, "Read Keyboard, current key=%02X", m_currentKey);
		return m_currentKey;
	}
	void Device8255PCjr::PORTA_OUT(BYTE value)
	{
		LogPrintf(LOG_INFO, "POST: %d", value);
	}
	
	// PORT B:
	// ---------
	// PB7: HIGH: Clear KSR + Clear IRQ1 / LOW: Normal state
	// PB6: Keyboard clock line: HIGH: Normal state / LOW: Pull clock line low (triggers keyboard self-test)
	// PB5: ENABLEI/OCK(2) / LOW: Enable detection of RAM parity errors on expansion cards
	// PB4: ENBRAMPCK (6) / LOW: Enable detection of RAM parity errors on motherboard
	// PB3: DIP Switch block select (0: 1-4 / 1: 5-8) (TODO: Determine 0-1)
	// PB2: (JUMPER E5 - UNUSED)
	// PB1: SPKRDATA (8) / Data for speaker
	// PB0: TIM2GATESPK (8) / Gate for timer 2

	BYTE Device8255PCjr::PORTB_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTB IN");
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

		if (diff & 0x80) LogPrintf(LOG_INFO, "PB7: KSR+IRQ1 %s", value & 0x80 ? "Clear" : "Normal");
		if (diff & 0x40) LogPrintf(LOG_INFO, "PB6: KBD Clock %s", value & 0x40 ? "Normal" : "LOW: Self-test");
		if (diff & 0x20) LogPrintf(LOG_INFO, "PB5: EX RAM Parity Check %s", value & 0x20 ? "OFF" : "ON");
		if (diff & 0x10) LogPrintf(LOG_INFO, "PB4: MB RAM Parity Check %s", value & 0x10 ? "OFF" : "ON");
		if (diff & 0x08) LogPrintf(LOG_INFO, "PB3: DIP Switch Block Select %s", value & 0x08 ? "1-4" : "5-8");
		if (diff & 0x04) LogPrintf(LOG_INFO, "PB2: UNUSED %s", value & 0x04 ? "1" : "0");
		if (diff & 0x02) LogPrintf(LOG_INFO, "PB1: SPEAKER Data %s", value & 0x02 ? "ON" : "OFF");
		if (diff & 0x01) LogPrintf(LOG_INFO, "PB0: TIMER2 Gate %s", value & 0x01 ? "HI" : "LOW");

		m_portBData = value;
	}

	// PORT C:
	// ---------
	// PC7: PCK (6) (IN) RAM parity error occured on motherboard
	// PC6: I/O CHCK (2) (IN) RAM parity error occured on expansion cards
	// PC5: T/C2OUT (8) (IN) Timer channel 2 monitoring
	// PC4: SPK (8) (IN) Speaker monitoring

	// PC0-PC3: (IN) SW1 Configuration Switches 1-4 / 5-8 (selected by PB3)

	BYTE Device8255PCjr::PORTC_IN()
	{
		// TODO: PC4-7 == 0 for now

		bool PB6 = (m_portBData & 0x08);

		BYTE ret = 0;

		LogPrintf(LOG_DEBUG, "PORTC IN, ret=%02X", ret);
		return ret;
	}
	void Device8255PCjr::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTC OUT, value=%02X", value);
	}

	void Device8255PCjr::SetCurrentKeyCode(BYTE keyCode)
	{ 
		m_currentKey = keyCode;
	}
}
