#include "stdafx.h"

#include "Device8255CPC464.h"
#include <IO/DeviceKeyboard.h>

using emul::GetBit;

namespace ppi
{
	void Device8255CPC464::Reset()
	{
		Device8255::Reset();

		SetCassetteInput(false);
		SetPrinterBusy(false);
		SetExpansionPortPin(false);
		SetRefreshRate(ScreenRefreshRate::REFRESH_50HZ);
		SetDistributorID(DistributorID::Amstrad);
		SetVSync(false);
	}

	BYTE Device8255CPC464::PORTA_IN()
	{
		LogPrintf(LOG_INFO, "PORTA: IN, value=%02x", m_portAData);
		//return m_portAData;

		const auto line = GetKeyboardLine();
		return (line != KeyboardLine::INVALID) ? m_keyboard->GetRowData((BYTE)line) : 0xFF;
	}
	void Device8255CPC464::PORTA_OUT(BYTE value)
	{
		m_portAData = value;
		LogPrintf(LOG_INFO, "PORTA: OUT, value=%02x", value);
	}

	BYTE Device8255CPC464::PORTB_IN()
	{
		LogPrintf(LOG_INFO, "PORTB: IN [%cTAPE_R] [%cBUSY] [%cEXP] [%cLK4][%cLK3][%cLK2][%cLK1] [%cVSYNC]",
			GetBit(m_portBData, 7) ? ' ' : '/',
			GetBit(m_portBData, 6) ? ' ' : '/',
			GetBit(m_portBData, 5) ? ' ' : '/',
			GetBit(m_portBData, 4) ? ' ' : '/',
			GetBit(m_portBData, 3) ? ' ' : '/',
			GetBit(m_portBData, 2) ? ' ' : '/',
			GetBit(m_portBData, 1) ? ' ' : '/',
			GetBit(m_portBData, 0) ? ' ' : '/'
		);
		return m_portBData;
	}
	void Device8255CPC464::PORTB_OUT(BYTE value)
	{
		m_portBData = value;
		LogPrintf(LOG_INFO, "PORTB: OUT, value=%02x", value);
	}

	BYTE Device8255CPC464::PORTC_IN()
	{
		LogPrintf(LOG_INFO, "PORTC: IN, value=%02x", m_portCData);
		return m_portCData;
	}

	void Device8255CPC464::PORTC_OUT(BYTE value)
	{
		m_portCData = value;
		LogPrintf(LOG_DEBUG, "PORTC: OUT, value=%02x", value);
		LogPrintf(LOG_INFO, "PORTC: [%cBDIR][%cBC1] [%cTAPE_W][%cTAPE_MOT] [KBD%d]",
			GetBDIR() ? ' ' : '/',
			GetBC1() ? ' ' : '/',
			GetCassetteOutput() ? ' ' : '/',
			GetCassetteMotorOut() ? ' ' : '/',
			GetKeyboardLine());

	}

}
