#include "stdafx.h"
#include "DeviceIWM.h"
#include "Storage/DeviceFloppy.h"

using emul::GetBit;
using fdd::StepDirection;

namespace floppy::woz
{
	void DeviceIWM::Reset()
	{
		LogPrintf(LOG_DEBUG, "Reset");
		m_stateRegister = 0;
		m_dataRegister = 0;
		m_statusRegister = 0;
		m_writeHandshakeRegister = 0;
		m_modeRegister = 0;
		m_sel = 0;
	}

	void DeviceIWM::Init(fdd::DeviceFloppy* floppy1, fdd::DeviceFloppy* floppy2)
	{
		assert(floppy1);
		assert(floppy2);

		m_floppy1 = floppy1;
		m_floppy2 = floppy2;
	}

	void DeviceIWM::SetStateRegister(BYTE a3a2a1a0)
	{
		bool a0 = GetBit(a3a2a1a0, 0);
		int address = (a3a2a1a0 >> 1) & 7;
		LogPrintf(LOG_DEBUG, "SetStateRegister, set bit[%d] = %d", address, a0);
		emul::SetBit(m_stateRegister, address, a0);

		switch (address)
		{
		case 6:
		case 7:
			UpdateState();
			break;
		case 3:
			if (a0) // LSTRB high
			{
				WriteRegister();
			}
			break;
		case 4: // Drive selection
		case 5:
			if (!IsMotorOn())
			{
				m_floppy1->SetSelected(false);
				m_floppy2->SetSelected(false);
			}
			else
			{
				m_floppy1->SetSelected(!GetDriveSel());
				m_floppy2->SetSelected(GetDriveSel());
			}
			break;
		}
	}

	void DeviceIWM::Write(BYTE value)
	{
		LogPrintf(LOG_TRACE, "Write, value=%02X", value);

		switch (m_currState)
		{
		case State::WRITE_MODE: WriteMode(value); break;
		case State::WRITE_DATA: WriteData(value); break;
		}
	}
	BYTE DeviceIWM::Read()
	{
		LogPrintf(LOG_TRACE, "Read");

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
		bool statusBit = true;

		switch (((m_stateRegister & 7) << 1) | (m_sel ? 1 : 0))
		{
		case 0b0000: LogPrintf(LOG_INFO, "Read [Step direction]");
			statusBit = (GetFloppy()->GetStepDirection() == StepDirection::OUTER);
			break;
		case 0b0001: LogPrintf(LOG_INFO, "Read [Disk in place]"); 
			statusBit = !GetFloppy()->IsDiskLoaded();
			break;
		case 0b0010: LogPrintf(LOG_INFO, "Read [Step]");
			statusBit = !GetFloppy()->IsSeeking();
			break;
		case 0b0011: LogPrintf(LOG_INFO, "Read [Write Protect]"); 
			statusBit = false; // TODO
			break;
		case 0b0100: LogPrintf(LOG_INFO, "Read [Motor ON]");
			statusBit = !GetFloppy()->IsMotorEnabled();
			break;
		case 0b0101: LogPrintf(LOG_INFO, "Read [Is Track 0]");
			statusBit = !GetFloppy()->IsTrack0();
			break;
		case 0b0110: LogPrintf(LOG_INFO, "Read [Eject]"); 
			statusBit = true; // Always returns 1
			break;
		case 0b0111: LogPrintf(LOG_DEBUG, "Read [Tachometer][%d]", GetFloppy()->GetMotorPulse());
			statusBit = GetFloppy()->GetMotorPulse();
			break;
		case 0b1000: LogPrintf(LOG_INFO, "Read [Head 0 Data]");
			// TODO: Switches head
			break;
		case 0b1001: LogPrintf(LOG_INFO, "Read [Head 1 Data]");
			// TODO: Switches head
			break;
		case 0b1100: LogPrintf(LOG_INFO, "Read [Sides]"); 
			// TODO: 800k drive
			statusBit = false;
			break;
		case 0b1101: LogPrintf(LOG_INFO, "Read [Reserved 1101]"); 
			statusBit = false;
			break;
		case 0b1110: LogPrintf(LOG_INFO, "Read [Drive Installed]"); 
			statusBit = !GetFloppy()->IsConnected();
			break;
		case 0b1111: LogPrintf(LOG_INFO, "Read [Reserved 1111]"); 
			statusBit = false;
			break;
		default:
			LogPrintf(LOG_WARNING, "ReadStatus: Invalid register");
		}

		BYTE value = // Bits 0-4 == bits 0-4 of the mode register
			(m_modeRegister & 0b11111) |
			(IsMotorOn()) << 5 | // Drive 0 or 1 active
			(0) << 6 | // MZ, Reserved, reads 0
			(statusBit) << 7; // Sense input

		LogPrintf(LOG_TRACE, "Read Status, value = %2X", value);

		return value;
	}

	BYTE DeviceIWM::ReadData()
	{
		BYTE value = GetFloppy()->ReadByte();

		LogPrintf(LOG_TRACE, "Read Data, value = %02X", value);

		return value;
	}

	BYTE DeviceIWM::ReadWriteHandshake()
	{
		LogPrintf(LOG_INFO, "Read Write Handshake");
		return 0xFF;
	}

	void DeviceIWM::WriteMode(BYTE value)
	{
		LogPrintf(LOG_TRACE, "Write Mode, value=%02X", value);
		m_modeRegister = value;

		LogPrintf(LOG_INFO, "Mode: ");
		LogPrintf(LOG_INFO, " Clock Speed: %d MHz", GetBit(value, 4) ? 8 : 7);
		LogPrintf(LOG_INFO, " Bit cell time: %d us/bit", GetBit(value, 3) ? 2 : 4);
		LogPrintf(LOG_INFO, " Motor Off Timer: %s", GetBit(value, 2) ? "no delay" : "1 second");
		LogPrintf(LOG_INFO, " Handshake protocol: %s", GetBit(value, 1) ? "Async" : "Sync");
		LogPrintf(LOG_INFO, " Latch Mode: %s", GetBit(value, 0) ? "ON" : "OFF");
	}

	void DeviceIWM::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Data, value=%02X", value);
	}

	void DeviceIWM::WriteRegister()
	{
		bool value = GetBit(m_stateRegister, 2); // CA2 contains the value to write

		switch (((m_stateRegister & 3) << 1) | (m_sel ? 1 : 0))
		{
		case 0b000: LogPrintf(LOG_INFO, "Write [Direction]: %s", value ? "Outer (to 0)" : "Inner (to 80)");
			GetFloppy()->SetStepDirection(value ? StepDirection::OUTER : StepDirection::INNER);
			break;
		case 0b010: LogPrintf(LOG_INFO, "Write [Step]: %d", value);
			GetFloppy()->Step();
			break;
		case 0b100: LogPrintf(LOG_INFO, "Write [Motor]: %s", value ? "OFF" : "ON");
			GetFloppy()->EnableMotor(!value);
			break;
		case 0b110: LogPrintf(LOG_INFO, "Write [Eject]: %s", value ? "EJECT" : "No");
			break;
		default:
			LogPrintf(LOG_WARNING, "WriteRegister: Invalid register");
		}
	}
}
