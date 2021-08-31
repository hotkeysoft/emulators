#include "DeviceFloppy.h"

namespace fdc
{
	const size_t RESET_DELAY_US = 10 * 1000; // 10 ms
	const size_t RQM_DELAY_US = 5; // 5 us

	DeviceFloppy::DeviceFloppy(WORD baseAddress, size_t clockSpeedHz) :
		Logger("fdc"),
		m_baseAddress(baseAddress),
		m_clockSpeed(clockSpeedHz),
		m_currOpWait(0),
		m_state(STATE::CMD_WAIT),
		m_nextState(STATE::CMD_WAIT),
		m_interruptPending(false),
		m_st0(0),
		m_pcn(0),
		m_commandBusy(false),
		m_driveActive{ false, false, false, false },
		m_dataInputOutput(DataDirection::CPU2FDC),
		m_dataRegisterReady(true),
		m_motor{ false, false, false, false },
		m_enableIRQDMA(false),
		m_driveSel(0),
		m_currCommand(nullptr)
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
		
		m_st0 = 0xC0; // TODO: Find why in datasheet

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

	size_t DeviceFloppy::DelayToTicks(size_t delayUS)
	{
		return delayUS * m_clockSpeed / 1000000;
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
		LogPrintf(Logger::LOG_DEBUG, "ReadMainStatusReg [%cRQM %cDIO %cNDMA %cBUSY %cACTD %cATCD %cACTB %cACTA]",
			m_dataRegisterReady ? ' ' : '/',
			(m_dataInputOutput == DataDirection::FDC2CPU) ? ' ' : '/',
			false ? ' ' : '/',
			m_commandBusy ? ' ' : '/',
			m_driveActive[3] ? ' ' : '/',
			m_driveActive[2] ? ' ' : '/',
			m_driveActive[1] ? ' ' : '/',
			m_driveActive[0] ? ' ' : '/');

		BYTE status =
			(RQM * m_dataRegisterReady) |
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
		BYTE result = 0xFF;
		LogPrintf(Logger::LOG_DEBUG, "ReadDataFIFO");
		switch (m_state)
		{
		case STATE::RESULT_WAIT:
			m_commandBusy = true;
			RQMDelay(STATE::RESULT_WAIT);
			result = m_fifo.front();
			m_fifo.pop_front();
			break;
		default:
			throw std::exception("Unexpected state");
		}
		LogPrintf(Logger::LOG_DEBUG, "Return Result: %02X", result);
		return result;
	}
	void DeviceFloppy::WriteDataFIFO(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataFIFO, value=%02X", value);

		m_fifo.push_back(value);

		switch (m_state)
		{
		case STATE::CMD_WAIT:
			m_commandBusy = true;
			RQMDelay(STATE::CMD_READ);
			break;
		case STATE::PARAM_WAIT:
			m_commandBusy = true;
			RQMDelay(STATE::PARAM_WAIT);
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
			m_currOpWait = DelayToTicks(RESET_DELAY_US);
			LogPrintf(Logger::LOG_INFO, "Start RESET, count=%zu", m_currOpWait);
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
		case STATE::RQM_DELAY:
			if (--m_currOpWait == 0)
			{
				LogPrintf(Logger::LOG_DEBUG, "RQM Delay done");
				m_dataRegisterReady = true;
				m_state = m_nextState;
			}
			break;
		case STATE::CMD_WAIT:
			m_dataRegisterReady = true;
			m_commandBusy = false;
			break;
		case STATE::CMD_READ:
			LogPrintf(Logger::LOG_INFO, "Read Command");
			ReadCommand();
			break;
		case STATE::CMD_EXEC_DELAY:
			if (--m_currOpWait == 0)
			{
				m_state = STATE::CMD_EXEC_DONE;
			}
			break;
		case STATE::CMD_EXEC_DONE:
			LogPrintf(Logger::LOG_INFO, "Command Execution done");
			m_driveActive[0] = false;
			m_driveActive[1] = false;
			m_driveActive[2] = false;
			m_driveActive[3] = false;
			m_dataRegisterReady = true;
			m_dataInputOutput = DataDirection::FDC2CPU;
			m_state = STATE::RESULT_WAIT;
			if (m_currCommand->interrupt)
			{
				LogPrintf(LOG_INFO, "Interrupt Pending");
				SetInterruptPending();
			}
			break;
		case STATE::PARAM_WAIT:
			if (m_currCommand->paramCount == m_fifo.size())
			{
				LogPrintf(Logger::LOG_INFO, "Read all [%d] parameters, start execution phase", m_fifo.size());

				ExecuteCommand();
			}
			break;
		case STATE::RESULT_WAIT:
			if (m_fifo.size() == 0)
			{
				LogPrintf(Logger::LOG_INFO, "Client Read all results, ready for next command");
				m_commandBusy = false;
				m_dataInputOutput = DataDirection::CPU2FDC;
				m_state = STATE::CMD_WAIT;
			}
			break;
		default: throw std::exception("Unknown state");
		}
	}

	void DeviceFloppy::RQMDelay(STATE nextState)
	{
		m_currOpWait = DelayToTicks(RQM_DELAY_US);
		LogPrintf(Logger::LOG_INFO, "Start RQM Delay, count=%zu", m_currOpWait);
		m_dataRegisterReady = false;
		m_state = STATE::RQM_DELAY;
		m_nextState = nextState;
	}
	void DeviceFloppy::ReadCommand()
	{
		CMD commandID = (CMD)m_fifo.front();
		m_fifo.pop_front();

		LogPrintf(LOG_INFO, "ReadCommand, cmd = %02X", commandID);

		CommandMap::const_iterator it = m_commandMap.find(commandID);
		if (it != m_commandMap.end())
		{
			m_currCommand = &(it->second);
			LogPrintf(LOG_INFO, "Command: [%s], parameters: [%d]", m_currCommand->name, m_currCommand->paramCount);
			m_state = STATE::PARAM_WAIT;
		}
		else
		{
			LogPrintf(LOG_ERROR, "Unknown command");
			m_state = STATE::CMD_ERROR;
			m_currCommand = nullptr;
			throw std::exception("Unknown command");
		}
	}
	void DeviceFloppy::ExecuteCommand()
	{
		m_state = STATE::CMD_EXEC;
		ExecFunc func = m_currCommand->func;
		m_currOpWait = (this->*func)();
		LogPrintf(LOG_INFO, "Command pushed [%d] results. Execution Delay: [%d]", m_fifo.size(), m_currOpWait);

		m_state = m_currOpWait ? STATE::CMD_EXEC_DELAY : STATE::CMD_EXEC_DONE;
	}

	size_t DeviceFloppy::NotImplemented()
	{
		LogPrintf(LOG_ERROR, "Command [%s] not implemented", m_currCommand->name);
		m_fifo.clear();
		throw std::exception("not implemented");
		return 0;
	}

	size_t DeviceFloppy::SenseInterrupt()
	{
		LogPrintf(LOG_INFO, "COMMAND: SenseInterrupt");


		// Response: ST0, PCN
		m_fifo.push_back(m_st0);
		m_fifo.push_back(m_pcn);

		// If after a reset, return C0 for each drives
		// TODO: Check that we after a reset and not in an error condition
		if (m_st0 & 0xC0)
		{
			++m_st0;
			if (m_st0 == 0xC4)
			{
				// done
				m_st0 = 0;
			}

		}

		return DelayToTicks(5);
	}

	size_t DeviceFloppy::Recalibrate()
	{
		// TODO: Support parallel seek/recalibrate
		BYTE driveNumber = m_fifo.front() % 3;
		m_fifo.pop_front();

		LogPrintf(LOG_INFO, "COMMAND: Recalibrate drive [%d]", driveNumber);

		m_driveActive[driveNumber] = true;
		m_st0 = SE | driveNumber; // Set Seek end + head + drive
		m_pcn = 0;

		// Response: none
		return DelayToTicks(100 * 1000); // 100 ms;
	}

	size_t DeviceFloppy::Seek()
	{
		// TODO: Support parallel seek/recalibrate
		BYTE param = m_fifo.front();
		m_fifo.pop_front();

		BYTE driveNumber = param % 3;
		bool head = param & 4;

		int cylinder = m_fifo.front();
		m_fifo.pop_front();

		BYTE travel = abs(cylinder - m_pcn);

		LogPrintf(LOG_INFO, "COMMAND: Seek d=[%d] h=[%d] cyl=[%d] (travel: [%d])", driveNumber, head, cylinder, travel);

		m_driveActive[driveNumber] = true;
		m_st0 = SE | (param & 7); // Set Seek end + head + drive
		m_pcn = cylinder;

		return DelayToTicks(travel * 1000); // TODO: Get real world number range
	}

	size_t DeviceFloppy::SenseDriveStatus()
	{
		BYTE param = m_fifo.front();
		m_fifo.pop_front();

		BYTE driveNumber = param & 3;
		bool head = param & 4;

		LogPrintf(LOG_INFO, "COMMAND: Sense Drive Status d=[%d] h=[%d]", driveNumber, head);

		m_st3 = RDY | (TRK0 * (m_pcn == 0)) | DSDR | (param & 7);

		// Response: ST3
		m_fifo.push_back(m_st3);
		return DelayToTicks(5);
	}
}
