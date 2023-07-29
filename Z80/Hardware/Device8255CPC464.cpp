#include "stdafx.h"

#include "Device8255CPC464.h"
#include <IO/DeviceKeyboard.h>
#include <IO/DeviceJoystickDigital.h>
#include "Sound/DeviceAY-3-891x.h"

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
		BYTE value = (m_portADirection == DIRECTION::OUTPUT) ? m_portAData : m_sound->ReadData();
		LogPrintf(LOG_INFO, "PORTA: IN (PSG), value=%02x", value);
		return value;
	}
	void Device8255CPC464::PORTA_OUT(BYTE value)
	{
		m_portAData = value;
		LogPrintf(LOG_INFO, "PORTA: OUT (PSG), value=%02x", value);

		if (m_portADirection == DIRECTION::OUTPUT)
		{
			m_sound->WriteData(value);
		}
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

		if (m_sound)
		{
			m_sound->SetCommand(sound::ay3::Command(value >> 6));
		}
	}

	BYTE Device8255CPC464::OnReadPortA()
	{
		LogPrintf(LOG_DEBUG, "AY3: OnReadPortA");

		const auto line = GetKeyboardLine();
		switch (line)
		{
		case KeyboardLine::LINE_0:
		case KeyboardLine::LINE_1:
		case KeyboardLine::LINE_2:
		case KeyboardLine::LINE_3:
		case KeyboardLine::LINE_4:
		case KeyboardLine::LINE_5:
		case KeyboardLine::LINE_6:
		case KeyboardLine::LINE_7:
		case KeyboardLine::LINE_8:
			return m_keyboard->GetRowData((BYTE)line);
		case KeyboardLine::LINE_9:
			return m_keyboard->GetRowData((BYTE)line) & ReadJoystick1();

		default:
		case KeyboardLine::INVALID:
			LogPrintf(LOG_WARNING, "Invalid keyboard line: %d", line);
			return 0;
		}
	}

	const BYTE JOY_UP = 0x01;
	const BYTE JOY_DOWN = 0x02;
	const BYTE JOY_LEFT = 0x04;
	const BYTE JOY_RIGHT = 0x08;
	const BYTE JOY_FIRE1 = 0x10;
	const BYTE JOY_FIRE2 = 0x20;

	BYTE Device8255CPC464::ReadJoystick1() const
	{
		if (!m_joystick) return 0xFF;

		BYTE val =
			(m_joystick->GetUp() * JOY_UP) |
			(m_joystick->GetDown() * JOY_DOWN) |
			(m_joystick->GetLeft() * JOY_LEFT) |
			(m_joystick->GetRight() * JOY_RIGHT) |
			(m_joystick->GetFire() * JOY_FIRE1) |
			(m_joystick->GetFire2() * JOY_FIRE2);
		return ~val;
	}
}
