#include "stdafx.h"

#include "DeviceFloppy.h"
#include <array>
#include <FileUtil.h>

using hscommon::fileUtil::File;
using emul::GetBit;

namespace fdc
{
	const size_t RESET_DELAY_US = 10 * 1000; // 10 ms
	const size_t RQM_DELAY_US = 5; // 5 us

	DeviceFloppy::DeviceFloppy(WORD baseAddress, size_t clockSpeedHz) :
		Logger("fdc"),
		m_baseAddress(baseAddress),
		m_clockSpeed(clockSpeedHz)
	{
	}

	void DeviceFloppy::Reset()
	{
		m_driveActive[0] = false;
		m_driveActive[1] = false;
		m_driveActive[2] = false;
		m_driveActive[3] = false;

		m_dataRegisterReady = false;
		m_executionPhase = false;
		m_dataInputOutput = DataDirection::CPU2FDC;

		m_st0 = 0xC0; // TODO: Find why in datasheet
		m_srt = 16;
		m_hlt = 254;
		m_hut = 240;
		m_nonDMA = true;

		m_fifo.clear();

		m_dmaPending = false;
		m_interruptPending = false;

		m_state = STATE::RESET_START;
	}

	bool DeviceFloppy::ClearDiskImage(BYTE drive)
	{
		if (drive > 3)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: invalid drive number %d", drive);
			return false;
		}

		LogPrintf(LOG_INFO, "ClearDiskImage: unloading drive %d", drive);
		m_images[drive].Clear();
		SetDiskChanged();
		return true;
	}

	bool DeviceFloppy::LoadDiskImage(BYTE drive, const char* path)
	{
		if (drive > 3)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: invalid drive number %d", drive);
			return false;
		}

		LogPrintf(LOG_INFO, "LoadDiskImage: loading %s in drive %d", path, drive);

		// Find geometry
		struct stat stat_buf;
		int rc = stat(path, &stat_buf);
		uint32_t size = stat_buf.st_size;

		const auto it = m_geometries.find(size);
		if (it == m_geometries.end())
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: Unsupported image file, size=%d", size);
			return false;
		}

		SetDiskChanged();

		const Geometry& geometry = it->second;

		LogPrintf(LOG_INFO, "LoadImage: Detected image geometry: %s", geometry.name);

		m_images[drive].Clear();
		m_images[drive].data.resize(size);

		File f(path, "rb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: error opening binary file");
			return false;
		}

		size_t bytesRead = fread(&m_images[drive].data[0], sizeof(char), size, f);
		if (size != bytesRead)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: error reading binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "LoadDiskImage: read %d bytes", bytesRead);
		}

		m_images[drive].path = path;
		m_images[drive].loaded = true;
		m_images[drive].geometry = geometry;

		return true;
	}

	bool DeviceFloppy::SaveDiskImage(BYTE drive, const char* path)
	{
		if (drive > 3)
		{
			LogPrintf(LOG_ERROR, "SaveDiskImage: invalid drive number %d", drive);
			return false;
		}

		if (!m_images[drive].loaded)
		{
			LogPrintf(LOG_ERROR, "SaveDiskImage: image not loaded for drive %d", drive);
			return false;
		}

		LogPrintf(LOG_INFO, "SaveDiskImage: saving floppy %d to file %s", drive, path);

		File f(path, "wb");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "SaveDiskImage: error opening binary file");
			return false;
		}

		size_t size = m_images[drive].data.size();
		size_t bytesWritten = fwrite(&m_images[drive].data[0], sizeof(char), size, f);
		if (size != bytesWritten)
		{
			LogPrintf(LOG_ERROR, "SaveDiskImage: error writing binary file");
			return false;
		}
		else
		{
			LogPrintf(LOG_INFO, "SaveDiskImage: written %d bytes", bytesWritten);
		}

		return true;
	}

	void DeviceFloppy::Init()
	{
		//MAIN_STATUS_REGISTER = 0x3F4, // read-only
		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadMainStatusReg));

		//DATA_FIFO = 0x3F5,
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadDataFIFO));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataFIFO));

		SetDiskChanged();
	}

	size_t DeviceFloppy::DelayToTicks(size_t delayUS)
	{
		return delayUS * m_clockSpeed / 1000000;
	}

	BYTE DeviceFloppy::ReadMainStatusReg()
	{
		// In non-DMA mode a read or write clears the interrupt pin
		if (m_nonDMA)
		{
			ClearInterrupt();
		}

		LogPrintf(Logger::LOG_TRACE, "ReadMainStatusReg [%cRQM %cDIO %cEXM %cBUSY %cACTD %cATCD %cACTB %cACTA]",
			m_dataRegisterReady ? ' ' : '/',
			(m_dataInputOutput == DataDirection::FDC2CPU) ? ' ' : '/',
			(m_executionPhase && m_nonDMA) ? ' ' : '/',
			m_commandBusy ? ' ' : '/',
			m_driveActive[3] ? ' ' : '/',
			m_driveActive[2] ? ' ' : '/',
			m_driveActive[1] ? ' ' : '/',
			m_driveActive[0] ? ' ' : '/');

		BYTE status =
			(RQM * m_dataRegisterReady) |
			(DIO * (m_dataInputOutput == DataDirection::FDC2CPU)) |
			(EXM * (m_executionPhase && m_nonDMA)) |
			(BUSY * m_commandBusy) |
			(ACTD * m_driveActive[3]) |
			(ACTC * m_driveActive[2]) |
			(ACTB * m_driveActive[1]) |
			(ACTA * m_driveActive[0]);

		return status;
	}

	BYTE DeviceFloppy::ReadDataFIFO()
	{
		// In non-DMA mode a read or write clears the interrupt pin
		if (m_nonDMA)
		{
			ClearInterrupt();
		}

		BYTE result = 0xFF;
		LogPrintf(Logger::LOG_TRACE, "ReadDataFIFO");
		switch (m_state)
		{
		case STATE::RESULT_WAIT:
			// In DMA mode a reading the first result clears the interrupt pin
			if (!m_nonDMA)
			{
				ClearInterrupt();
			}
			m_commandBusy = true;
			RQMDelay(STATE::RESULT_WAIT);
			result = Pop();
			break;
		case STATE::DMA_WAIT:
		case STATE::NDMA_WAIT:
			result = Pop();
			DMAAcknowledge();
			ReadSector();
			break;
		default:
			LogPrintf(Logger::LOG_ERROR, "ReadDataFIFO() Unexpected State: %d", m_state);
			throw std::exception("Unexpected state");
		}
		LogPrintf(Logger::LOG_DEBUG, "ReadDataFIFO(): Return Result: %02X", result);
		return result;
	}

	void DeviceFloppy::WriteDataFIFO(BYTE value)
	{
		// In non-DMA mode a read or write clears the interrupt pin
		if (m_nonDMA)
		{
			ClearInterrupt();
		}

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
		case STATE::DMA_WAIT:
		case STATE::NDMA_WAIT:
			DMAAcknowledge();
			WriteSector();
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
			LogPrintf(Logger::LOG_DEBUG, "Start RESET, count=%zu", m_currOpWait);
			m_state = STATE::RESET_ACTIVE;
			m_commandBusy = true;
			break;
		case STATE::RESET_ACTIVE:
			if (m_currOpWait % 100 == 0)
			{
				LogPrintf(Logger::LOG_DEBUG, "RESET in progress, count=%zu", m_currOpWait);
			}
			if (--m_currOpWait == 0)
			{
				m_state = STATE::RESET_DONE;
			}
			break;
		case STATE::RESET_DONE:
			LogPrintf(Logger::LOG_DEBUG, "End RESET");
			m_state = STATE::CMD_WAIT;
			SetInterruptPending();
			break;
		case STATE::RQM_DELAY:
			if (--m_currOpWait == 0)
			{
				LogPrintf(Logger::LOG_TRACE, "RQM Delay done");
				m_dataRegisterReady = m_fifo.size();
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
			LogPrintf(Logger::LOG_DEBUG, "Command Execution done");
			m_driveActive[0] = false;
			m_driveActive[1] = false;
			m_driveActive[2] = false;
			m_driveActive[3] = false;
			m_dataRegisterReady = true;
			m_executionPhase = false;

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

			if (m_currCommand && m_currCommand->interrupt)
			{
				LogPrintf(LOG_DEBUG, "Interrupt Pending");
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
			LogPrintf(Logger::LOG_DEBUG, "Start Read");
			m_state = STATE::READ_EXEC;
			m_currOpWait = DelayToTicks(100 * 1000);
			break;
		case STATE::READ_EXEC:
			LogPrintf(Logger::LOG_DEBUG, "Read Sector");
			ReadSector();
			break;
		case STATE::RW_DONE:
			LogPrintf(Logger::LOG_DEBUG, "End Read/Write");
			RWSectorEnd();
			m_state = STATE::CMD_EXEC_DONE;
			break;
		case STATE::WRITE_START:
			LogPrintf(Logger::LOG_DEBUG, "Start Write");
			m_state = STATE::WRITE_EXEC;
			m_currOpWait = DelayToTicks(100 * 1000);
			break;
		case STATE::WRITE_EXEC:
			LogPrintf(Logger::LOG_DEBUG, "Write Sector");
			WriteSector();
			break;
		case STATE::DMA_WAIT:
			LogPrintf(Logger::LOG_TRACE, "DMA Wait");
			if (--m_currOpWait == 0)
			{
				m_state = STATE::CMD_ERROR;
			}
			break;
		case STATE::NDMA_WAIT:
			LogPrintf(Logger::LOG_TRACE, "NDMA Wait");
			m_dataRegisterReady = true;
			if (--m_currOpWait == 0)
			{
				m_dataRegisterReady = false;
				m_state = STATE::CMD_ERROR;
			}
			break;
		case STATE::CMD_ERROR:
			LogPrintf(Logger::LOG_WARNING, "Command Error");
			m_st0 = ST0_INT_IC;
			m_fifo.clear();
			Push(m_st0);
			m_state = STATE::CMD_EXEC_DONE;
			break;
		case STATE::ABNORMAL_TERMINATION:
			LogPrintf(Logger::LOG_WARNING, "Abnormal Termination");
			m_st0 |= ST0_INT_AT;
			m_fifo.clear();
			Push(m_st0);
			m_state = STATE::CMD_EXEC_DONE;
			break;
		case STATE::NOT_READY:
			LogPrintf(Logger::LOG_WARNING, "Drive Not Ready");
			m_st0 |= ST0_NR;
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
		LogPrintf(Logger::LOG_TRACE, "Start RQM Delay, count=%zu", m_currOpWait);
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
			LogPrintf(LOG_WARNING, "Unknown command %d", m_currcommandID);
			m_state = STATE::CMD_EXEC_DONE;
			m_currCommand = nullptr;
			m_currcommandID = 0;
		}
	}
	void DeviceFloppy::ExecuteCommand()
	{
		m_state = STATE::CMD_EXEC;
		m_executionPhase = true;
		m_dataRegisterReady = false;
		ExecFunc func = m_currCommand->func;
		m_nextState = (this->*func)();
		LogPrintf(LOG_DEBUG, "Command pushed [%d] results. Next State: [%d]", m_fifo.size(), m_nextState);

		m_state = m_currOpWait ? STATE::CMD_EXEC_DELAY : m_nextState;
	}

	void DeviceFloppy::DMAAcknowledge()
	{
		m_dmaPending = false;
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

		m_dataRegisterReady = false;
		m_driveActive[driveNumber] = true;
		m_st0 = ST0_SE | driveNumber; // Set Seek end + head + drive
		m_pcn = 0;

		assert(m_fifo.size() == 0);
		// Response: none, Interrupt
		m_currOpWait = DelayToTicks(100 * 1000); // 100 ms
		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::ReadID()
	{
		BYTE param = Pop();

		BYTE driveNumber = param % 3;
		bool head = param & 4;

		// TODO: Hack for initial ID read.
		// Need to keep track of currSector/etc for each drive
		// And reset when a new disk image is loaded
		if (m_currSector == 0)
		{
			FloppyDisk& disk = m_images[driveNumber];
			m_currSector = disk.geometry.sectOffset;
		}

		LogPrintf(LOG_INFO, "COMMAND: Read ID d=[%d] h=[%d]", driveNumber, head);

		m_dataRegisterReady = false;
		m_driveActive[driveNumber] = true;
		m_st0 = param & 7; // head + drive

		assert(m_fifo.size() == 0);

		m_fifo.clear();

		Push(m_st0);
		Push(0/*m_st1*/); // TODO: Error stuff, check details
		Push(0/*m_st2*/); // TODO: Error stuff, check details
		Push(m_pcn);
		Push(m_currHead);
		Push(m_currSector);
		Push(2); // N

		m_currOpWait = DelayToTicks(1 * 1000); // 1 ms

		ClearDiskChanged();

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

		m_dataRegisterReady = false;
		m_driveActive[driveNumber] = true;
		m_st0 = ST0_SE | (param & 7); // Set Seek end + head + drive
		m_pcn = cylinder;

		assert(m_fifo.size() == 0);

		m_currOpWait = DelayToTicks(travel * (size_t)m_srt * 1000);

		ClearDiskChanged();

		return STATE::CMD_EXEC_DONE;
	}

	DeviceFloppy::STATE DeviceFloppy::SenseDriveStatus()
	{
		BYTE param = Pop();

		BYTE driveNumber = param & 3;
		bool head = param & 4;

		LogPrintf(LOG_INFO, "COMMAND: Sense Drive Status d=[%d] h=[%d]", driveNumber, head);

		m_st3 = ST3_RDY | (ST3_TRK0 * (m_pcn == 0)) | ST3_DSDR | (param & 7);

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

	void DeviceFloppy::PushSector(fdc::FloppyDisk& disk, uint32_t offset)
	{
		for (size_t b = 0; b < 512; ++b)
		{
			Push(disk.data[offset + b]);
		}

		LogPrintHex(LOG_DEBUG, &disk.data[offset], 512);
	}

	DeviceFloppy::STATE DeviceFloppy::ReadData()
	{
		BYTE param = Pop();

		m_currDrive = param & 3;
		bool head = param & 4;

		BYTE c = Pop();
		BYTE h = Pop();
		BYTE r = Pop();
		BYTE n = Pop();
		BYTE eot = Pop();
		BYTE gpl = Pop();
		BYTE dtl = Pop();

		m_currHead = head;
		m_currSector = r;
		m_maxSector = eot;
		m_multiTrack = GetBit(m_currcommandID, 7);

		LogPrintf(LOG_INFO, "COMMAND: Read Data drive=[%d] head=[%d] multitrack=[%d]", m_currDrive, head, m_multiTrack);
		LogPrintf(LOG_INFO, "|Params: cyl=[%d] head=[%d] sector=[%x] number=[%d] endOfTrack=[%x] gapLength=[%d] dtl=[%d]", c, h, r, n, eot, gpl, dtl);

		// TODO: Error handling, validation
		m_pcn = c;
		//if (c != m_pcn)
		//{
		//	throw std::exception("cylinder != current cylinder");
		//}

		// Only this configuration is supported at the moment
		assert(n == 2);		// All floppy drives use 512 bytes/sector
		assert(dtl == 0xFF); // All floppy drives use 512 bytes/sector

		assert(m_fifo.size() == 0);

		FloppyDisk& disk = m_images[m_currDrive];

		if (!disk.loaded)
		{
			m_st0 = ST0_NR | (param & 7); // Not Ready + head + drive

			m_currOpWait = DelayToTicks(100 * 1000); // 100ms
			return STATE::NOT_READY;
		}

		// TODO: Only if head is not already loaded
		m_currOpWait = DelayToTicks((size_t)m_hlt * 1000);

		if (r < 1 || r > (disk.geometry.sect + disk.geometry.sectOffset))
		{
			LogPrintf(LOG_ERROR, "Invalid sector [%d]", r);
			throw std::exception("Invalid sector");
		}
		if (c >= disk.geometry.cyl)
		{
			LogPrintf(LOG_ERROR, "Invalid cylinder [%d]", c);
			throw std::exception("Invalid cylinder");
		}
		if (h > disk.geometry.head)
		{
			LogPrintf(LOG_ERROR, "Invalid head [%d]", h);
			throw std::exception("Invalid head");
		}

		// Put the whole sector in the fifo
		uint32_t offset = disk.geometry.CHS2A(m_pcn, m_currHead, m_currSector);
		PushSector(disk, offset);

		return STATE::READ_START;
	}

	// Same as readdata but start a sector 1
	DeviceFloppy::STATE DeviceFloppy::ReadTrack()
	{
		FloppyDisk& disk = m_images[m_currDrive];

		BYTE param = Pop();

		m_currDrive = param & 3;
		bool head = param & 4;

		BYTE c = Pop();
		BYTE h = Pop();
		BYTE r = Pop();
		BYTE n = Pop();
		BYTE eot = Pop();
		BYTE gpl = Pop();
		BYTE dtl = Pop();

		m_currHead = head;
		m_currSector = disk.geometry.sectOffset + 1;
		m_maxSector = eot;
		m_multiTrack = false; // No multitrack bit in command id

		LogPrintf(LOG_INFO, "COMMAND: Read Track drive=[%d] head=[%d] multitrack=[%d]", m_currDrive, head, m_multiTrack);
		LogPrintf(LOG_INFO, "|Params: cyl=[%d] head=[%d] sector=[%x] number=[%d] endOfTrack=[%x] gapLength=[%d] dtl=[%d]", c, h, r, n, eot, gpl, dtl);

		// TODO: Error handling, validation
		m_pcn = c;
		//if (c != m_pcn)
		//{
		//	throw std::exception("cylinder != current cylinder");
		//}

		// Only this configuration is supported at the moment
		assert(n == 2);      // All floppy drives use 512 bytes/sector
		assert(dtl == 0xFF); // All floppy drives use 512 bytes/sector

		assert(m_fifo.size() == 0);

		if (!disk.loaded)
		{
			m_st0 = ST0_NR | (param & 7); // Not Ready + head + drive

			m_currOpWait = DelayToTicks(100 * 1000); // 100ms
			return STATE::NOT_READY;
		}

		// TODO: Only if head is not already loaded
		m_currOpWait = DelayToTicks((size_t)m_hlt * 1000);

		if (c >= disk.geometry.cyl)
		{
			LogPrintf(LOG_ERROR, "Invalid cylinder [%d]", c);
			throw std::exception("Invalid cylinder");
		}
		if (h > disk.geometry.head)
		{
			LogPrintf(LOG_ERROR, "Invalid head [%d]", h);
			throw std::exception("Invalid head");
		}

		// Put the whole sector in the fifo
		uint32_t offset = disk.geometry.CHS2A(m_pcn, m_currHead, m_currSector);
		PushSector(disk, offset);

		return STATE::READ_START;
	}

	void DeviceFloppy::ReadSector()
	{
		LogPrintf(LOG_TRACE, "ReadSector, fifo=%d", m_fifo.size());
		// Exit Conditions

		FloppyDisk& disk = m_images[m_currDrive];
		// TODO: Handle not loaded

		if (m_fifo.size() == 0)
		{
			bool isEOT = UpdateCurrPos();
			if (isEOT)
			{
				LogPrintf(LOG_DEBUG, "ReadSector done, end of track");
				m_currOpWait = DelayToTicks(10);
				m_nextState = STATE::RW_DONE;
				m_state = STATE::CMD_EXEC_DELAY;
				return;
			}

			LogPrintf(LOG_DEBUG, "ReadSector done, caching next sector %d", m_currSector);
			// Put the whole sector in the fifo
			// TODO: Avoid duplication, check out of bounds
			uint32_t offset = disk.geometry.CHS2A(m_pcn, m_currHead, m_currSector);
			PushSector(disk, offset);
		}

		SetDMAPending();

		// Timeout
		m_currOpWait = DelayToTicks(100 * 1000);

		m_state = m_nonDMA ? STATE::NDMA_WAIT : STATE::DMA_WAIT;
	}

	void DeviceFloppy::RWSectorEnd()
	{
		LogPrintf(LOG_DEBUG, "ReadSectorEnd, fifo=%d", m_fifo.size());

		ClearDiskChanged();

		m_st0 = ST0_INT_NT;

		m_fifo.clear();

		Push(m_st0);
		Push(0/*m_st1*/); // TODO: Error stuff, check details
		Push(0/*m_st2*/); // TODO: Error stuff, check details
		Push(m_pcn);
		Push(m_currHead);
		Push(m_currSector);
		Push(2); // N
	}

	DeviceFloppy::STATE DeviceFloppy::WriteData()
	{
		BYTE param = Pop();

		m_currDrive = param & 3;
		bool head = param & 4;

		BYTE c = Pop();
		BYTE h = Pop();
		BYTE r = Pop();
		BYTE n = Pop();
		BYTE eot = Pop();
		BYTE gpl = Pop();
		BYTE dtl = Pop();

		m_currHead = head;
		m_currSector = r;
		m_maxSector = eot;
		m_multiTrack = GetBit(m_currcommandID, 7);

		LogPrintf(LOG_INFO, "COMMAND: Write Data drive=[%d] head=[%d] multitrack=[%d]", m_currDrive, head, m_multiTrack);
		LogPrintf(LOG_INFO, "|Params: cyl=[%d] head=[%d] sector=[%x] number=[%d] endOfTrack=[%x] gapLength=[%d] dtl=[%d]", c, h, r, n, eot, gpl, dtl);

		// TODO: Error handling, validation
		m_pcn = c;
		//if (c != m_pcn)
		//{
		//	throw std::exception("cylinder != current cylinder");
		//}

		// Only this configuration is supported at the moment
		assert(n == 2);      // All floppy drives use 512 bytes/sector
		assert(dtl == 0xFF); // All floppy drives use 512 bytes/sector

		assert(m_fifo.size() == 0);

		FloppyDisk& disk = m_images[m_currDrive];

		if (!disk.loaded)
		{
			m_st0 = ST0_INT_NR | (param & 7); // Not Ready + head + drive

			m_currOpWait = DelayToTicks(100 * 1000); // 100ms
			return STATE::NOT_READY;
		}

		// TODO: Only if head is not already loaded
		m_currOpWait = DelayToTicks((size_t)m_hlt * 1000);

		if (r < 1 || r > (disk.geometry.sect + disk.geometry.sectOffset))
		{
			LogPrintf(LOG_ERROR, "Invalid sector [%d]", r);
			throw std::exception("Invalid sector");
		}
		if (c >= disk.geometry.cyl)
		{
			LogPrintf(LOG_ERROR, "Invalid cylinder [%d]", c);
			throw std::exception("Invalid cylinder");
		}
		if (h > disk.geometry.head)
		{
			LogPrintf(LOG_ERROR, "Invalid head [%d]", h);
			throw std::exception("Invalid head");
		}

		return STATE::WRITE_START;
	}

	void DeviceFloppy::WriteSector()
	{
		LogPrintf(LOG_DEBUG, "WriteSector, fifo=%d", m_fifo.size());

		// Exit Conditions

		FloppyDisk& disk = m_images[m_currDrive];
		// TODO: Handle not loaded

		if (m_fifo.size() == 512)
		{
			uint32_t offset = disk.geometry.CHS2A(m_pcn, m_currHead, m_currSector);
			for (size_t b = 0; b < 512; ++b)
			{
				disk.data[offset + b] = Pop();
			}

			bool isEOT = UpdateCurrPos();
			if (isEOT)
			{
				m_dataRegisterReady = false;
				m_currOpWait = DelayToTicks(10);
				m_nextState = STATE::RW_DONE;
				m_state = STATE::CMD_EXEC_DELAY;
				return;
			}

			LogPrintf(LOG_DEBUG, "WriteSector done, writing next sector %d", m_currSector);
		}

		SetDMAPending();

		// Timeout
		m_currOpWait = DelayToTicks(100 * 1000);

		m_state = m_nonDMA ? STATE::NDMA_WAIT : STATE::DMA_WAIT;
	}

	void DeviceFloppy::DMATerminalCount()
	{
		LogPrintf(LOG_DEBUG, "DMATerminalCount");

		// TODO: Check that we are in correct state;
		m_dmaPending = false;
		m_currOpWait = DelayToTicks(10);
		m_nextState = STATE::RW_DONE;
		m_state = STATE::CMD_EXEC_DELAY;
	}

	bool DeviceFloppy::UpdateCurrPos()
	{
		const FloppyDisk& disk = m_images[m_currDrive];
		BYTE maxSector = m_nonDMA ? std::min(m_maxSector, (BYTE)(disk.geometry.sect + disk.geometry.sectOffset)) : (disk.geometry.sect + disk.geometry.sectOffset);

		bool isEOT = (m_currSector >= maxSector);
		LogPrintf(LOG_DEBUG, "UpdateCurrPos mt=[%d], head=[%d], EOT=[%d]", m_multiTrack, m_currHead, isEOT);

		if (m_multiTrack)
		{
			m_pcn += (isEOT && m_currHead) ? 1 : 0;
			m_currHead ^= isEOT ? 1 : 0; // Toggle head
		}
		else
		{
			m_pcn += isEOT ? 1 : 0;
		}
		m_currSector = isEOT ? (disk.geometry.sectOffset + 1) : (m_currSector + 1);
		LogPrintf(LOG_DEBUG, "UpdateCurrPos done: cyl=[%d] head=[%d] sector=[%d]", m_pcn, m_currHead, m_currSector);

		// TODO: In multitrack mode, should continue if eot and head passes from 0 to 1
		// Does anyone uses this?
		return isEOT;
	}

	void DeviceFloppy::Serialize(json& to)
	{
		std::array<std::string, 4> images;
		for (int i=0; i<4; ++i)
		{
			if (m_images[i].loaded)
			{
				images[i] = m_images[i].path.string();
			}
		}
		to["images"] = images;
	}
	void DeviceFloppy::Deserialize(const json& from)
	{
		std::array<std::string, 4> images;
		images = from["images"];

		for (int i = 0; i < 4; ++i)
		{
			if (images[i].size())
			{
				LoadDiskImage(i, images[i].c_str());
			}
			else
			{
				ClearDiskImage(i);
			}
		}
	}
}
