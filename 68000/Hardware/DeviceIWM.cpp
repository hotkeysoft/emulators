#include "stdafx.h"
#include "DeviceIWM.h"

namespace floppy::woz
{
	void DeviceIWM::Reset()
	{
		LogPrintf(LOG_DEBUG, "Reset");
		m_stateRegister = 0;
		m_stateRegister = 0;
		m_dataRegister = 0;
		m_statusRegister = 0;
		m_writeHandshakeRegister = 0;
		m_modeRegister = 0;
	}

	void DeviceIWM::SetStateRegister(BYTE a3a2a1a0)
	{
		bool a0 = emul::GetBit(a3a2a1a0, 0);
		int address = (a3a2a1a0 >> 1) & 7;
		LogPrintf(LOG_DEBUG, "SetStateRegister, set bit[%d] = %d", address, a0);
		emul::SetBit(m_stateRegister, address, a0);

		if (address == 6 || address == 7)
		{
			UpdateState();
		}
	}

	void DeviceIWM::Write(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write, value=%02X", value);

		switch (m_currState)
		{
		case State::WRITE_MODE: WriteMode(value); break;
		case State::WRITE_DATA: WriteData(value); break;
		}
	}
	BYTE DeviceIWM::Read()
	{
		LogPrintf(LOG_DEBUG, "Read");

		switch (m_currState)
		{
		case State::NONE: return 0xFF;
		case State::READ_DATA: return ReadData();
		case State::READ_STATUS: return ReadStatus();
		case State::READ_WRITE_HANDSHAKE: return ReadWriteHandshake();
		default:
			LogPrintf(LOG_WARNING, "Read(): Unexpected state");
		}

		return 0xFF;
	}

	void DeviceIWM::UpdateState()
	{
		switch (GetL7L6())
		{
		case 0:
			m_currState = IsMotorOn() ? State::READ_DATA : State::NONE;
			break;
		case 1:
			m_currState = State::READ_STATUS;
			break;
		case 2:
			m_currState = State::READ_WRITE_HANDSHAKE;
			break;
		case 3:
			m_currState = IsMotorOn() ? State::WRITE_DATA : State::WRITE_MODE;
			break;
		default:
			NODEFAULT;
		}
	}

	BYTE DeviceIWM::ReadStatus()
	{
		BYTE value = // Bits 0-4 == bits 0-4 of the mode register
			(m_modeRegister & 0b11111) |
			(IsMotorOn()) << 5 | // Drive 0 or 1 active
			(0) << 6 | // MZ, Reserved, reads 0
			(0) << 7; // Sense input

		LogPrintf(LOG_DEBUG, "Read Status, value = %2X", value);
		return value;
	}

	BYTE DeviceIWM::ReadData()
	{
		LogPrintf(LOG_INFO, "Read Data");
		return 0xFF;
	}

	BYTE DeviceIWM::ReadWriteHandshake()
	{
		LogPrintf(LOG_INFO, "Read Write Handshake");
		return 0xFF;
	}

	void DeviceIWM::WriteMode(BYTE value)
	{
		LogPrintf(LOG_INFO, "Write Mode, value=%02X", value);
		m_modeRegister = value;
	}

	void DeviceIWM::WriteData(BYTE value)
	{
		LogPrintf(LOG_INFO, "Write Data, value=%02X", value);
	}
}
