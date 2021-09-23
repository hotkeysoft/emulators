#include "Device8255.h"

namespace ppi
{
	Device8255::Device8255(WORD baseAddress) : 
		Logger("8255"), 
		m_baseAddress(baseAddress)
	{
		Reset();
	}

	void Device8255::Reset()
	{
		LogPrintf(LOG_INFO, "Reset");
		SetControlWord(DEFAULT_CONTROLWORD);
		m_portAData = 0;
		m_portBData = 0;
		m_portCData = 0;
	}

	void Device8255::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8255::PORTA_IN));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8255::PORTA_OUT));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8255::PORTB_IN));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8255::PORTB_OUT));

		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&Device8255::PORTC_IN));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&Device8255::PORTC_OUT));

		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&Device8255::CONTROL_IN));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device8255::CONTROL_OUT));
	}

	// PORT A: KEYBOARD DATA (INPUT)
	// ---------
	// PA7-PA0: Received keyboard byte
	// 
	// Configured as OUTPUT during POST
	// Outputs diagnostic code
	BYTE Device8255::PORTA_IN()
	{
		LogPrintf(LOG_DEBUG, "Read Keyboard, current key=%02X", m_currentKey);
		return m_currentKey;
	}
	void Device8255::PORTA_OUT(BYTE value)
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

	BYTE Device8255::PORTB_IN()
	{
		LogPrintf(LOG_DEBUG, "PORTB IN");
		return m_portBData;
	}
	void Device8255::PORTB_OUT(BYTE value)
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

	BYTE Device8255::PORTC_IN()
	{
		// TODO: PC4-7 == 0 for now

		bool PB6 = (m_portBData & 0x08);

		BYTE ret;
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
	void Device8255::PORTC_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "PORTC OUT, value=%02X", value);
	}

	BYTE Device8255::CONTROL_IN()
	{
		LogPrintf(LOG_DEBUG, "GET Control word, ret=%02X", m_controlWord);
		return m_controlWord | CTRL_MODESETFLAG;
	}
	void Device8255::CONTROL_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "SET Control word, value=%02X", value);
		SetControlWord(value);
	}

	void Device8255::SetControlWord(BYTE ctrl)
	{
		m_controlWord = ctrl;
		if (!(ctrl | CTRL_MODESETFLAG))
		{
			throw std::exception("Single bit set not implemented");
		}

		switch (ctrl & (CTRL_GA_MODE2 | CTRL_GA_MODE1))
		{
		case 0:
			LogPrintf(LOG_INFO, "GROUP A   : MODE 0");
			break;
		case CTRL_GA_MODE1:
			LogPrintf(LOG_INFO, "GROUP A   : MODE 1");
			throw std::exception("GROUP A MODE 1 not implemented");
		default:
			LogPrintf(LOG_INFO, "GROUP A   : MODE 2");
			throw std::exception("GROUP A MODE 2 not implemented");
		}

		if (ctrl & CTRL_GB_MODE1)
		{
			LogPrintf(LOG_INFO, "GROUP B   : MODE 1");
			throw std::exception("GROUP B MODE 1 not implemented");
		}
		else
		{
			LogPrintf(LOG_INFO, "GROUP B   : MODE 0");
		}

		// Assume mode 0 for now

		m_portADirection = (m_controlWord & CTRL_GA_A_DIR) ? INPUT : OUTPUT;
		m_portBDirection = (m_controlWord & CTRL_GB_B_DIR) ? INPUT : OUTPUT;
		m_portCHDirection = (m_controlWord & CTRL_GA_C_DIR_H) ? INPUT : OUTPUT;
		m_portCLDirection = (m_controlWord & CTRL_GB_C_DIR_L) ? INPUT : OUTPUT;

		LogPrintf(LOG_INFO, "PORT A    : %s", GetPortDirectionStr(m_portADirection));
		LogPrintf(LOG_INFO, "PORT B    : %s", GetPortDirectionStr(m_portBDirection));
		LogPrintf(LOG_INFO, "PORT C(hi): %s", GetPortDirectionStr(m_portCHDirection));
		LogPrintf(LOG_INFO, "PORT C(lo): %s", GetPortDirectionStr(m_portCLDirection));

		m_portAData = 0;
		m_portBData = 0;
		m_portCData = 0;

		return;
	}

	void Device8255::SetRAMConfig(RAMSIZE r)
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

	void Device8255::SetDisplayConfig(DISPLAY d)
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

	void Device8255::SetFloppyCount(BYTE count)
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

	void Device8255::SetPOSTLoop(bool set)
	{
		m_switches &= ~SW_POST_LOOP;
		m_switches |= (set ? 0 : SW_POST_LOOP);
	}

	void Device8255::SetMathCoprocessor(bool set)
	{
		m_switches &= ~SW_COPROCESSOR;
		m_switches |= (set ? SW_COPROCESSOR : 0);
	}

	void Device8255::SetCurrentKeyCode(BYTE keyCode)
	{ 
		m_currentKey = keyCode;
	}

	bool Device8255::IsSoundON()
	{
		return (m_portBData & 3) == 3;
	}
}
