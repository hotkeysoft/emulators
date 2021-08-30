#include "DeviceFloppy.h"

namespace fdc
{
	const size_t RESET_DELAY_MS = 10;

	DeviceFloppy::DeviceFloppy(WORD baseAddress, size_t clockSpeedHz) :
		Logger("fdc"),
		m_baseAddress(baseAddress),
		m_clockSpeed(clockSpeedHz),
		m_currOpWait(0),
		m_state(STATE::CMD_WAIT),
		m_interruptPending(false),
		m_commandBusy(false),
		m_driveActive{ false, false, false, false },
		m_dataInputOutput(DataDirection::CPU2FDC),
		m_dataRegisterReady(true),
		m_motor{ false, false, false, false },
		m_enableIRQDMA(false),
		m_driveSel(0)
	{
	}

	void DeviceFloppy::Reset()
	{
		m_driveActive[0] = false;
		m_driveActive[1] = false;
		m_driveActive[2] = false;
		m_driveActive[3] = false;

		m_dataRegisterReady = true;
		m_dataInputOutput = DataDirection::CPU2FDC;

		m_motor[0] = false;
		m_motor[1] = false;
		m_motor[2] = false;
		m_motor[3] = false;

		m_enableIRQDMA = false;
		m_driveSel = 0;
		m_state = STATE::RESET_START;
	}

	void DeviceFloppy::Init()
	{
		// STATUS_REGISTER_A = 0x3F0, // read-only (PS/2)
		// STATUS_REGISTER_B = 0x3F1, // read-only (PS/2)
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadStatusRegA));
		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadStatusRegB));

		// DIGITAL_OUTPUT_REGISTER = 0x3F2,
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataOutputReg));

		// TAPE_DRIVE_REGISTER = 0x3F3, // Unused, returns 0
		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadTapeReg));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteTapeReg));

		//MAIN_STATUS_REGISTER = 0x3F4, // read-only
		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadMainStatusReg));

		//DATARATE_SELECT_REGISTER = 0x3F4, // write-only (PS/2)
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataRateSelectReg));

		//DATA_FIFO = 0x3F5,
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadDataFIFO));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataFIFO));
		
		//DIGITAL_INPUT_REGISTER = 0x3F7, // read-only (AT)
		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadDigitalInputReg));
		//CONFIGURATION_CONTROL_REGISTER = 0x3F7  // write-only (AT)
		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteConfigControlReg));
	}

	size_t DeviceFloppy::DelayToTicks(size_t delayMS)
	{
		return delayMS * m_clockSpeed / 1000;
	}

	// BEGIN Unused|AT|PS/2 Registers - No operation
	BYTE DeviceFloppy::ReadStatusRegA()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegA");
		return 0xFF;
	}

	BYTE DeviceFloppy::ReadStatusRegB()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegB");
		return 0xFF; // PS/2 only
	}

	BYTE DeviceFloppy::ReadDigitalInputReg()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadDigitalInputReg");
		return 0;
	}

	void DeviceFloppy::WriteConfigControlReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteConfigControlReg, value=%02X", value);
	}

	void DeviceFloppy::WriteDataRateSelectReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataRateSelectReg, value=%02X", value);
	}

	BYTE DeviceFloppy::ReadTapeReg()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadTapeRegFIFO");
		return 0;
	}
	void DeviceFloppy::WriteTapeReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteTapeReg, value=%02X", value);
	}
	// END Unused|AT|PS/2 Registers


	void DeviceFloppy::WriteDataOutputReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataOutputReg, value=%02X", value);

		if (!(value & DOR::RESET))
		{
			LogPrintf(LOG_INFO, "RESET");
			Reset();
			return;
		}

		m_motor[0] = (value & DOR::MOTA);
		m_motor[1] = (value & DOR::MOTB);
		m_motor[2] = (value & DOR::MOTC);
		m_motor[3] = (value & DOR::MOTD);

		for (size_t drive = 0; drive < 4; ++drive)
		{
			LogPrintf(LOG_INFO, "Drive %d Motor: %s", drive, m_motor[drive] ? "ON" : "OFF");
		}

		m_enableIRQDMA = (value & DOR::IRQ);
		LogPrintf(LOG_INFO, "IRQ/DMA: %s", m_enableIRQDMA ? "Enabled" : "Disabled");

		switch (value & (DOR::DSEL1 | DOR::DSEL0))
		{
		case 0: m_driveSel = 0; break;
		case DOR::DSEL0: m_driveSel = 1; break;
		case DOR::DSEL1: m_driveSel = 2; break;
		case DOR::DSEL1 | DOR::DSEL0: m_driveSel = 3; break;
		default: throw std::exception("not possible");
		}

		LogPrintf(LOG_INFO, "Drive Select: %d", m_driveSel);
	}

	BYTE DeviceFloppy::ReadMainStatusReg()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadMainStatusReg [%cMRQ %cDIO %cNDMA %cBUSY %cACTD %cATCD %cACTB %cACTA]",
			m_dataRegisterReady ? ' ' : '/',
			(m_dataInputOutput == DataDirection::FDC2CPU) ? ' ' : '/',
			false ? ' ' : '/',
			m_commandBusy ? ' ' : '/',
			m_driveActive[3] ? ' ' : '/',
			m_driveActive[2] ? ' ' : '/',
			m_driveActive[1] ? ' ' : '/',
			m_driveActive[0] ? ' ' : '/');

		BYTE status =
			(MRQ * m_dataRegisterReady) |
			(DIO * (m_dataInputOutput == DataDirection::FDC2CPU)) |
			(NDMA * 0) |
			(BUSY * m_commandBusy) |
			(ACTD * m_driveActive[3]) |
			(ACTC * m_driveActive[2]) |
			(ACTB * m_driveActive[1]) |
			(ACTA * m_driveActive[0]);

		return status;
	}

	BYTE DeviceFloppy::ReadDataFIFO()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadDataFIFO");
		return 0;
	}
	void DeviceFloppy::WriteDataFIFO(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataFIFO, value=%02X", value);

		m_fifo.push_back(value);

		switch (m_state)
		{
		case STATE::CMD_WAIT:
			m_state = STATE::CMD_READ;
			break;
		default:
			throw std::exception("Unexpected state");
		}

	}

	void DeviceFloppy::Tick()
	{
		switch (m_state)
		{
		case STATE::RESET_START:
			m_currOpWait = DelayToTicks(RESET_DELAY_MS);
			LogPrintf(Logger::LOG_INFO, "Start RESET, count=%d", m_currOpWait);
			m_state = STATE::RESET_ACTIVE;
			m_commandBusy = true;
			break;
		case STATE::RESET_ACTIVE:
			if (--m_currOpWait == 0)
			{
				m_state = STATE::RESET_DONE;
			}
			break;
		case STATE::RESET_DONE:
			LogPrintf(Logger::LOG_INFO, "End RESET, interrupt");
			m_state = STATE::CMD_WAIT;
			SetInterruptPending();
			break;
		case STATE::CMD_WAIT:
			m_dataRegisterReady = true;
			m_commandBusy = false;
			break;

		default: throw std::exception("Unknown state");
		}
	}
}
