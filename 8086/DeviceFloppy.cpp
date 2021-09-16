#include "DeviceFloppy.h"
#include <assert.h>

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
		m_nonDMA(true),
		m_commandBusy(false),
		m_driveActive{ false, false, false, false },
		m_dataInputOutput(DataDirection::CPU2FDC),
		m_dataRegisterReady(true),
		m_motor{ false, false, false, false },
		m_enableIRQDMA(false),
		m_driveSel(0),
		m_currCommand(nullptr),
		m_currcommandID(0)
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
		m_srt = 16;
		m_hlt = 254;
		m_hut = 240;
		m_nonDMA = true;

		m_fifo.clear();

		m_state = STATE::RESET_START;
	}

	bool DeviceFloppy::LoadDiskImage(BYTE drive, const char* path)
	{
		if (drive > 3)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: invalid drive number %d", drive);
			return false;
		}

		m_currImage[drive].clear();

		LogPrintf(LOG_INFO, "LoadDiskImage: loading %s in drive %d", path, drive);

		struct stat stat_buf;
		int rc = stat(path, &stat_buf);
		size_t size = stat_buf.st_size;
		size_t expected = 512 * 40 * 8;

		if (size != expected)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: Unsupported image file, size=%d", size);
			return false;
		}

		m_currImage[drive].clear();
		m_currImage[drive].resize(expected);

		FILE* f = fopen(path, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: error opening binary file");
			return false;
		}

		size_t bytesRead = fread(&m_currImage[drive][0], sizeof(char), size, f);
		if (size != bytesRead)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: error reading binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "LoadDiskImage: read %d bytes", bytesRead);
		}

		fclose(f);
		return true;
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
			result = Pop();
			break;
		case STATE::DMA_WAIT:
			result = Pop();
			DMAAcknowledge();
			ReadSector();
			break;
		default:
			LogPrintf(Logger::LOG_ERROR, "ReadDataFIFO() Unexpected State: %d", m_state);
			throw std::exception("Unexpected state");
		}
		LogPrintf(Logger::LOG_DEBUG, "Return Result: %02X", result);
		return result;
	}

	void DeviceFloppy::WriteDataFIFO(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataFIFO, value=%02X", value);

		Push(value);

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
			LogPrintf(Logger::LOG_ERROR, "WriteDataFIFO() Unexpected State: %d", m_state);
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
			LogPrintf(Logger::LOG_DEBUG, "Read Command");
			ReadCommand();
			break;
		case STATE::CMD_EXEC_DELAY:
			if (--m_currOpWait == 0)
			{
				m_state = m_nextState;
			}
			break;
		case STATE::CMD_EXEC_DONE:
			LogPrintf(Logger::LOG_INFO, "Command Execution done");
			m_driveActive[0] = false;
			m_driveActive[1] = false;
			m_driveActive[2] = false;
			m_driveActive[3] = false;
			m_dataRegisterReady = true;

			// No results, ready for next command
			if (m_fifo.size() == 0)
			{
				m_dataInputOutput = DataDirection::CPU2FDC;
				m_state = STATE::CMD_WAIT;
			}
			else
			{
				m_dataInputOutput = DataDirection::FDC2CPU;
				m_state = STATE::RESULT_WAIT;
			}

			if (m_currCommand->interrupt)
			{
				LogPrintf(LOG_INFO, "Interrupt Pending");
				SetInterruptPending();
			}
			break;
		case STATE::PARAM_WAIT:
			if (m_currCommand->paramCount == m_fifo.size())
			{
				LogPrintf(Logger::LOG_DEBUG, "Read all [%d] parameters, start execution phase", m_fifo.size());

				ExecuteCommand();
			}
			break;
		case STATE::RESULT_WAIT:
			if (m_fifo.size() == 0)
			{
				LogPrintf(Logger::LOG_DEBUG, "Client Read all results, ready for next command");
				m_commandBusy = false;
				m_dataInputOutput = DataDirection::CPU2FDC;
				m_state = STATE::CMD_WAIT;
			}
			break;
		case STATE::READ_START:
			LogPrintf(Logger::LOG_INFO, "Start Read");
			m_state = STATE::READ_EXEC;
			m_currOpWait = DelayToTicks(100 * 1000);
			break;
		case STATE::READ_EXEC:
			LogPrintf(Logger::LOG_INFO, "Read Sector");
			ReadSector();
			break;
		case STATE::READ_DONE:
			LogPrintf(Logger::LOG_INFO, "End Read");
			ReadSectorEnd();
			m_state = STATE::CMD_EXEC_DONE;
			break;
		case STATE::DMA_WAIT:
			LogPrintf(Logger::LOG_DEBUG, "DMA Wait");
			if (--m_currOpWait == 0)
			{
				m_state = STATE::CMD_ERROR;
			}
			break;
		case STATE::CMD_ERROR:
			LogPrintf(Logger::LOG_INFO, "Command Error");
			m_st0 = 0x80;
			m_fifo.clear();
			Push(m_st0);
			m_state = STATE::CMD_EXEC_DONE;
			break;
		default: 
			LogPrintf(Logger::LOG_ERROR, "Tick() Unknown State: %d", m_state);
			throw std::exception("Unknown state");
		}
	}

	void DeviceFloppy::RQMDelay(STATE nextState)
	{
		m_currOpWait = DelayToTicks(RQM_DELAY_US);
		LogPrintf(Logger::LOG_DEBUG, "Start RQM Delay, count=%zu", m_currOpWait);
		m_dataRegisterReady = false;
		m_state = STATE::RQM_DELAY;
		m_nextState = nextState;
	}
	void DeviceFloppy::ReadCommand()
	{
		m_currcommandID = Pop();

		LogPrintf(LOG_DEBUG, "ReadCommand, cmd = %02X", m_currcommandID);

		// Only check 5 lower bits, hi bits are parameters or fixed 0/1
		CommandMap::const_iterator it = m_commandMap.find((CMD)(m_currcommandID & 0b00011111));
		if (it != m_commandMap.end())
		{
			m_currCommand = &(it->second);
			LogPrintf(LOG_DEBUG, "Command: [%s], parameters: [%d]", m_currCommand->name, m_currCommand->paramCount);
			m_state = STATE::PARAM_WAIT;
		}
		else
		{
			LogPrintf(LOG_ERROR, "Unknown command");
			m_state = STATE::CMD_ERROR;
			m_currCommand = nullptr;
			m_currcommandID = 0;
			throw std::exception("Unknown command");
		}
	}
	void DeviceFloppy::ExecuteCommand()
	{
		m_state = STATE::CMD_EXEC;
		ExecFunc func = m_currCommand->func;
		m_nextState = (this->*func)();
		LogPrintf(LOG_DEBUG, "Command pushed [%d] results. Next State: [%d]", m_fifo.size(), m_nextState);

		m_state = m_currOpWait ? STATE::CMD_EXEC_DELAY : m_nextState;
	}

	void DeviceFloppy::DMAAcknowledge()
	{
		m_dmaPending = false;
		//m_state = STATE::DMA_ACK;
	}

	DeviceFloppy::STATE DeviceFloppy::NotImplemented()
	{
		LogPrintf(LOG_ERROR, "Command [%s] not implemented", m_currCommand->name);
		m_fifo.clear();
		throw std::exception("not implemented");
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::SenseInterrupt()
	{
		LogPrintf(LOG_INFO, "COMMAND: SenseInterrupt");

		assert(m_fifo.size() == 0);

		// Response: ST0, PCN
		Push(m_st0);
		Push(m_pcn);

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

		// Response: none
		m_currOpWait = 5;
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::Recalibrate()
	{
		// TODO: Support parallel seek/recalibrate
		BYTE driveNumber = Pop() % 3;

		LogPrintf(LOG_INFO, "COMMAND: Recalibrate drive [%d]", driveNumber);

		m_driveActive[driveNumber] = true;
		m_st0 = SE | driveNumber; // Set Seek end + head + drive
		m_pcn = 0;

		assert(m_fifo.size() == 0);
		// Response: none, Interrupt
		m_currOpWait = DelayToTicks(100 * 1000); // 100 ms
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::Seek()
	{
		// TODO: Support parallel seek/recalibrate
		BYTE param = Pop();

		BYTE driveNumber = param % 3;
		bool head = param & 4;

		int cylinder = Pop();

		size_t travel = abs(cylinder - m_pcn);

		LogPrintf(LOG_INFO, "COMMAND: Seek d=[%d] h=[%d] cyl=[%d] (travel: [%d])", driveNumber, head, cylinder, travel);

		m_driveActive[driveNumber] = true;
		m_st0 = SE | (param & 7); // Set Seek end + head + drive
		m_pcn = cylinder;

		assert(m_fifo.size() == 0);
		// Response: Interrupt
		m_currOpWait = DelayToTicks(travel * (size_t)m_srt * 1000);
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::SenseDriveStatus()
	{
		BYTE param = Pop();

		BYTE driveNumber = param & 3;
		bool head = param & 4;

		LogPrintf(LOG_INFO, "COMMAND: Sense Drive Status d=[%d] h=[%d]", driveNumber, head);

		m_st3 = RDY | (TRK0 * (m_pcn == 0)) | DSDR | (param & 7);

		assert(m_fifo.size() == 0);
		// Response: ST3
		Push(m_st3);
		m_currOpWait = DelayToTicks(5);
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::Specify()
	{
		BYTE param = Pop();

		m_srt = 16-(param >> 4); // Step Rate Time (1-16 ms, 0xF=1, 0xE=2, etc)
		m_hut = param << 4; // Head Unload Time (16-240ms in 16ms increment)
		if (m_hut == 0) m_hut = 255;

		param = Pop();

		m_hlt = (param & 0b11111110); // Head Load Time (2-254ms in 2ms increment)
		if (m_hlt == 0) m_hlt = 255;

		m_nonDMA = (param & 1);

		LogPrintf(LOG_INFO, "COMMAND: Specify srt=[%d] hut=[%d] hlt=[%d] nonDMA=[%d]", m_srt, m_hut, m_hlt, m_nonDMA);

		assert(m_fifo.size() == 0);
		// No Response, no interrupt
		m_currOpWait = DelayToTicks(5);
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::ReadData()
	{
		BYTE param = Pop();

		BYTE driveNumber = param & 3;
		bool head = param & 4;

		BYTE c = Pop();
		BYTE h = Pop();
		BYTE r = Pop();
		BYTE n = Pop();
		BYTE eot = Pop();
		BYTE gpl = Pop();
		BYTE dtl = Pop();

		LogPrintf(LOG_INFO, "COMMAND: Read Data drive=[%d] head=[%d]", driveNumber, head);
		LogPrintf(LOG_INFO, "|Params: cyl=[%d] head=[%d] sector=[%d] number=[%d] endOfTrack=[%d] gapLength=[%d] dtl=[%d]", c, h, r, n, eot, gpl, dtl);

		m_currHead = head;
		m_currSector = r;
		m_maxSector = eot;

		// TODO: Error handling, validation
		m_pcn = c;
		//if (c != m_pcn)
		//{
		//	throw std::exception("cylinder != current cylinder");
		//}

		// TODO: Multitrack
		// (m_currcommandID & 0x80) //MT

		// Only this configuration is supported at the moment
		assert(m_enableIRQDMA == true);
		assert(m_nonDMA == false);
		assert(n == 2);		// All floppy drives use 512 bytes/sector
		assert(dtl == 0xFF); // All floppy drives use 512 bytes/sector

		assert(m_fifo.size() == 0);

		m_driveActive[driveNumber] = true;
		// TODO: Only if head is not already loaded
		m_currOpWait = DelayToTicks((size_t)m_hlt * 1000);

		if (r < 1 || r > 8)
		{
			LogPrintf(LOG_ERROR, "Invalid sector [%d]", r);
			throw std::exception("Invalid sector");
		}
		if (c >= 40)
		{
			LogPrintf(LOG_ERROR, "Invalid cylinder [%d]", c);
			throw std::exception("Invalid cylinder");
		}
		if (h > 1)
		{
			LogPrintf(LOG_ERROR, "Invalid head [%d]", h);
			throw std::exception("Invalid head");
		}

		// Put the whole sector in the fifo
		for (size_t b = 0; b < 512; ++b)
		{
			// TODO: adjust to floppy geometry
			int offset = 512 * ((c * 8) + (r - 1));
			Push(m_currImage[driveNumber][offset+b]);
		}

		return STATE::READ_START;
	}

	void DeviceFloppy::ReadSector()
	{
		LogPrintf(LOG_DEBUG, "ReadSector, fifo=%d", m_fifo.size());
		// Exit Conditions

		if (m_fifo.size() == 0)
		{
			++m_currSector;
			if (m_currSector > 8)
			{
				m_currSector = 1;

				// TODO for now don't auto move to next track
				m_currOpWait = DelayToTicks(10);
				m_nextState = STATE::READ_DONE;
				m_state = STATE::CMD_EXEC_DELAY;
				return;
			}

			LogPrintf(LOG_INFO, "ReadSector done, reading next sector %d", m_currSector);
			// Put the whole sector in the fifo
			// TODO: Avoid duplication
			BYTE driveActive = 0;
			for (BYTE d = 0; d < 3; ++d)
			{
				if (m_driveActive[d])
				{
					driveActive = d;
					break;
				}
			}
			for (size_t b = 0; b < 512; ++b)
			{
				// TODO: adjust to floppy geometry
				int offset = 512 * ((m_pcn * 8) + (m_currSector - 1));
				Push(m_currImage[driveActive][offset + b]);
			}
		}

		SetDMAPending();

		// Timeout
		m_currOpWait = DelayToTicks(100 * 1000);

		m_state = STATE::DMA_WAIT;
	}

	void DeviceFloppy::ReadSectorEnd()
	{
		LogPrintf(LOG_INFO, "ReadSectorEnd, fifo=%d", m_fifo.size());
		m_st0 = 0;

		m_fifo.clear();

		Push(m_st0);
		Push(0/*m_st1*/); // TODO: Error stuff, check details
		Push(0/*m_st2*/); // TODO: Error stuff, check details
		Push(m_pcn);
		Push(m_currHead);
		Push(m_currSector);
		Push(2); // N
	}

	void DeviceFloppy::DMATerminalCount()
	{
		LogPrintf(LOG_INFO, "DMATerminalCount");

		// TODO: Check that we are in correct state;
		m_dmaPending = false;
		m_currOpWait = DelayToTicks(10);
		m_nextState = STATE::READ_DONE;
		m_state = STATE::CMD_EXEC_DELAY;
	}
}
