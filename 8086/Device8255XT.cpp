#include "Device8255XT.h"

namespace ppi
{
	Device8255XT::Device8255XT(WORD baseAddress) :
		Logger("8255XT"),
		Device8255(baseAddress)
	{
		Reset();
	}

	void Device8255XT::Reset()
	{
		// For POST test
		// TODO: Need proper keyboard emulation, still get POST error
		m_portAData = 0xAA;
	}

	// PORT A: KEYBOARD DATA (INPUT)
	// ---------
	// PA7-PA0: Received keyboard byte
	// 
	// Configured as OUTPUT during POST
	// Outputs diagnostic code
	BYTE Device8255XT::PORTA_IN()
	{
		LogPrintf(LOG_DEBUG, "Read Keyboard, current key=%02X", m_portAData);
		return m_portAData;
	}
	void Device8255XT::PORTA_OUT(BYTE value)
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

	BYTE Device8255XT::PORTB_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTB IN");
		return m_portBData;
	}
	void Device8255XT::PORTB_OUT(BYTE value)
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

	BYTE Device8255XT::PORTC_IN()
	{
		// TODO: PC4 == 0 for now

		BYTE ret = (m_timer2Out << 5);

		bool PB6 = (m_portBData & 0x08);
		if (PB6)
		{
			ret = (m_switches & 0xF0) >> 4;
		}
		else
		{
			ret = (m_switches & 0x0F);
		}

		LogPrintf(LOG_DEBUG, "PORTC IN, ret=%02X", ret);
		return ret;
	}
	void Device8255XT::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTC OUT, value=%02X", value);
	}

	void Device8255XT::SetRAMConfig(RAMSIZE r)
	{
		m_switches &= ~(SW_RAM_H | SW_RAM_L);
		switch (r)
		{
		case RAMSIZE::RAM_128K: m_switches |= SW_RAM_L; break;
		case RAMSIZE::RAM_192K: m_switches |= SW_RAM_H; break;
		case RAMSIZE::RAM_256K: m_switches |= (SW_RAM_H | SW_RAM_L); break;

		case RAMSIZE::RAM_64K:
		default:
			break;
		}
	}

	void Device8255XT::SetDisplayConfig(DISPLAY d)
	{
		m_switches &= ~(SW_DISPLAY_H | SW_DISPLAY_L);
		switch (d)
		{
		case DISPLAY::COLOR_40x25: m_switches |= SW_DISPLAY_L; break;
		case DISPLAY::COLOR_80x25: m_switches |= SW_DISPLAY_H; break;
		case DISPLAY::MONO_80x25: m_switches |= (SW_DISPLAY_H | SW_DISPLAY_L); break;
		case DISPLAY::NONE:
		default:
			break;
		}
	}

	void Device8255XT::SetFloppyCount(BYTE count)
	{
		m_switches &= ~(SW_FLOPPY_H | SW_FLOPPY_L);
		switch (count)
		{		
		case 2: m_switches |= SW_FLOPPY_L; break;
		case 3: m_switches |= SW_FLOPPY_H; break;
		case 4: m_switches |= (SW_FLOPPY_H | SW_FLOPPY_L); break;

		case 1:
		default: 
			break;
		}
	}

	void Device8255XT::SetPOSTLoop(bool set)
	{
		m_switches &= ~SW_POST_LOOP;
		m_switches |= (set ? 0 : SW_POST_LOOP);
	}

	void Device8255XT::SetMathCoprocessor(bool set)
	{
		m_switches &= ~SW_COPROCESSOR;
		m_switches |= (set ? SW_COPROCESSOR : 0);
	}
}
