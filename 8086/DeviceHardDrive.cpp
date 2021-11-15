#include "DeviceHardDrive.h"
#include <assert.h>

using emul::GetBit;

namespace hdd
{
	const size_t RESET_DELAY_US = 10 * 1000; // 10 ms
	const size_t RQM_DELAY_US = 5; // 5 us

	DeviceHardDrive::DeviceHardDrive(WORD baseAddress, size_t clockSpeedHz) :
		Logger("hdd"),
		m_baseAddress(baseAddress),
		m_clockSpeed(clockSpeedHz)
	{
	}

	DeviceHardDrive::~DeviceHardDrive()
	{
		if (m_images[0].data)
		{
			fclose(m_images[0].data);
		}

		if (m_images[1].data)
		{
			fclose(m_images[1].data);
		}
	}

	void DeviceHardDrive::Reset()
	{
		m_fifo.clear();
		m_commandBusy = false;
		m_dataRegisterReady = false;
		m_dmaPending = false;
		m_interruptPending = false;
		m_dataInputOutput = DataDirection::CPU2HDC;
		m_controlData = ControlData::CONTROL;

		m_state = STATE::CMD_WAIT;
	}

	bool DeviceHardDrive::LoadDiskImage(BYTE drive, BYTE type, const char* path)
	{
		if (drive > 1)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: invalid drive number %d", drive);
			return false;
		}

		HardDisk& image = m_images[drive];
		image.loaded = false;
			if (image.data)
			{
				fclose(image.data);
				image.data = nullptr;
			}

		LogPrintf(LOG_INFO, "LoadDiskImage: loading %s in drive %d", path, drive);

		const auto it = m_geometries.find(type);
		if (it == m_geometries.end())
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: Unsupported drive type %d", type);
			return false;
		}
		const Geometry& geometry = it->second;
		LogPrintf(LOG_INFO, "LoadImage: Requested image geometry: %s", geometry.name);

		// Check if geometry is compatible with image size
		struct stat stat_buf;
		int rc = stat(path, &stat_buf);
		uint32_t size = stat_buf.st_size;

		if (size != geometry.GetImageSize())
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: Image size [%d] incompatible with geometry size [%d]", size, geometry.GetImageSize());
			return false;
		}

		FILE* f = fopen(path, "r+b");
		if (!f)
		{
			LogPrintf(LOG_ERROR, "LoadDiskImage: error opening binary file");
			return false;
		}

		image.loaded = true;
		image.data = f;
		image.geometry = geometry;

		return true;
	}

	void DeviceHardDrive::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&DeviceHardDrive::ReadDataFIFO));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&DeviceHardDrive::WriteDataFIFO));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&DeviceHardDrive::ReadStatus));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&DeviceHardDrive::WriteControllerReset));

		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&DeviceHardDrive::ReadOptionJumpers));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceHardDrive::WriteControllerSelectPulse));

		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&DeviceHardDrive::WriteMaskRegister));
	}

	size_t DeviceHardDrive::DelayToTicks(size_t delayUS)
	{
		return delayUS * m_clockSpeed / 1000000;
	}

	BYTE DeviceHardDrive::ReadStatus()
	{
		LogPrintf(Logger::LOG_TRACE, "ReadStatus [%cIRQ %cDRQ %cBSY %cDATAcmd %cINout %cREQ]",
			m_interruptPending ? ' ' : '/',
			m_dmaPending ? ' ' : '/',
			m_commandBusy ? ' ' : '/',
			(m_controlData == ControlData::DATA) ? ' ' : '/',
			(m_dataInputOutput == DataDirection::HDC2CPU) ? ' ' : '/',
			m_dataRegisterReady ? ' ' : '/');

		BYTE status =
			(HWS_REQ * m_dataRegisterReady) |
			(HWS_DIO * (m_dataInputOutput == DataDirection::HDC2CPU)) |
			(HWS_CD * (m_controlData == ControlData::DATA)) |
			(HWS_BUSY * m_commandBusy) |
			(HWS_DRQ * m_dmaPending) |
			(HWS_IRQ * m_interruptPending);

		return status;
	}

	BYTE DeviceHardDrive::ReadDataFIFO()
	{
		BYTE result = 0xFF;
		LogPrintf(Logger::LOG_TRACE, "ReadDataFIFO");
		switch (m_state)
		{
		case STATE::RESULT_WAIT:
			//// In DMA mode a reading the first result clears the interrupt pin
			//if (!m_nonDMA)
			//{
			//	ClearInterrupt();
			//}
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
		LogPrintf(Logger::LOG_DEBUG, "Return Result: %02X", result);
		return result;
	}

	void DeviceHardDrive::WriteDataFIFO(BYTE value)
	{
		//// In non-DMA mode a read or write clears the interrupt pin
		//if (m_nonDMA)
		//{
		//	ClearInterrupt();
		//}

		LogPrintf(Logger::LOG_DEBUG, "WriteDataFIFO, value=%02Xh", value);

		Push(value);

		switch (m_state)
		{
		case STATE::CMD_WAIT:
			m_state = STATE::CMD_READ;
			break;
		case STATE::PARAM_WAIT:
		case STATE::INIT_PARAM_WAIT:
			// No state change
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

	void DeviceHardDrive::WriteControllerReset(BYTE)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteControllerReset");
		Reset();
	}

	BYTE DeviceHardDrive::ReadOptionJumpers()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadOptionJumpers");
		return 0;
	}

	void DeviceHardDrive::WriteControllerSelectPulse(BYTE)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteControllerSelectPulse");

		// TODO: Validate
		m_state = STATE::CMD_WAIT;
		m_dataRegisterReady = true;
		m_commandBusy = true;
		m_controlData = ControlData::DATA;
		m_dataInputOutput = DataDirection::CPU2HDC;
		m_interruptPending = false;
		m_dmaPending = false;
	}

	void DeviceHardDrive::WriteMaskRegister(BYTE value)
	{
		m_irqEnabled = value & 2;
		m_dmaEnabled = value & 1;

		LogPrintf(Logger::LOG_INFO, "WriteMaskRegister, IRQEN=[%d], DMAEN=[%d]", m_irqEnabled, m_dmaEnabled);
	}

	void DeviceHardDrive::Tick()
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
			LogPrintf(Logger::LOG_INFO, "End RESET");
			m_state = STATE::CMD_WAIT;
			SetInterruptPending();
			break;
		case STATE::RQM_DELAY:
			if (--m_currOpWait == 0)
			{
				LogPrintf(Logger::LOG_DEBUG, "RQM Delay done");
				m_dataRegisterReady = m_fifo.size();
				m_state = m_nextState;
			}
			break;
		case STATE::CMD_WAIT:
			//m_dataRegisterReady = true;
			//m_commandBusy = false;
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
			CommandExecutionDone();
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
				m_dataInputOutput = DataDirection::CPU2HDC;
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
		case STATE::RW_DONE:
			LogPrintf(Logger::LOG_INFO, "End Read/Write");
			RWSectorEnd();
			m_state = STATE::CMD_EXEC_DONE;
			break;
		case STATE::WRITE_START:
			LogPrintf(Logger::LOG_INFO, "Start Write");
			m_state = STATE::WRITE_EXEC;
			m_currOpWait = DelayToTicks(100 * 1000);
			break;
		case STATE::WRITE_EXEC:
			LogPrintf(Logger::LOG_INFO, "Write Sector");
			WriteSector();
			break;
		case STATE::SEEK_EXEC:
			if (m_currOpWait)
			{
				--m_currOpWait;
			}
			else
			{
				SeekTo();
			}
			break;
		case STATE::INIT:
			// We end the current command but wait for drive config parameters
			CommandExecutionDone();
			// Wait for 8 config arguments
			m_state = STATE::INIT_PARAM_WAIT;
			break;
		case STATE::INIT_PARAM_WAIT:
			if (m_fifo.size() == 8)
			{
				LogPrintf(Logger::LOG_DEBUG, "Read all [%d] drive parameters", m_fifo.size());
				InitDrive2();
				CommandExecutionDone();
			}
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
			LogPrintf(Logger::LOG_INFO, "Command Error");
			m_commandError = true;
			m_fifo.clear();
//			Push(m_st0);
			m_state = STATE::CMD_EXEC_DONE;
			break;
		default: 
			LogPrintf(Logger::LOG_ERROR, "Tick() Unknown State: %d", m_state);
			throw std::exception("Unknown state");
		}
	}

	void DeviceHardDrive::CommandExecutionDone()
	{
		LogPrintf(Logger::LOG_INFO, "Command Execution done");
		m_dataRegisterReady = true;

		// No results, ready for next command
		if (m_fifo.size() == 0)
		{
			m_controlData = ControlData::CONTROL;
			m_dataInputOutput = DataDirection::CPU2HDC;
			m_state = STATE::CMD_WAIT;
		}
		else
		{
			m_controlData = ControlData::DATA;
			m_dataInputOutput = DataDirection::HDC2CPU;
			m_state = STATE::RESULT_WAIT;
		}

		LogPrintf(LOG_DEBUG, "Interrupt Pending");
		SetInterruptPending();
	}

	void DeviceHardDrive::RQMDelay(STATE nextState)
	{
		m_currOpWait = DelayToTicks(RQM_DELAY_US);
		LogPrintf(Logger::LOG_DEBUG, "Start RQM Delay, count=%zu", m_currOpWait);
		m_dataRegisterReady = false;
		m_state = STATE::RQM_DELAY;
		m_nextState = nextState;
	}

	void DeviceHardDrive::ReadCommand()
	{
		m_currcommandID = Pop();

		LogPrintf(LOG_DEBUG, "ReadCommand, cmd = %02X", m_currcommandID);

		CommandMap::const_iterator it = m_commandMap.find((CMD)(m_currcommandID));
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
	void DeviceHardDrive::ExecuteCommand()
	{
		m_state = STATE::CMD_EXEC;
		m_controlData = ControlData::DATA;
		m_dataRegisterReady = false;
		ExecFunc func = m_currCommand->func;
		m_nextState = (this->*func)();
		LogPrintf(LOG_DEBUG, "Command pushed [%d] results. Next State: [%d]", m_fifo.size(), m_nextState);

		m_state = m_currOpWait ? STATE::CMD_EXEC_DELAY : m_nextState;
	}

	void DeviceHardDrive::DMAAcknowledge()
	{
		m_dmaPending = false;
	}

	void DeviceHardDrive::SeekTo()
	{
		if (m_currCylinder != m_commandBlock.cylinder)
		{
			m_currCylinder += (m_currCylinder > m_commandBlock.cylinder) ? -1 : -1;
			LogPrintf(LOG_DEBUG, "[%zu] Seeking, curr=[%d], target=[%d]", emul::g_ticks, m_currCylinder, m_commandBlock.cylinder);
			m_currOpWait = DelayToTicks(1 * 1000); //TODO
			m_state = STATE::SEEK_EXEC;
		}
		else
		{
			LogPrintf(LOG_DEBUG, "Seek done, currcylinder=[%d]", m_currCylinder);
			m_commandError = false;
			PushStatus();
			m_state = STATE::CMD_EXEC_DONE;
		}
	}

	DeviceHardDrive::STATE DeviceHardDrive::NotImplemented()
	{
		LogPrintf(LOG_ERROR, "Command [%s] not implemented", m_currCommand->name);
		m_fifo.clear();
		throw std::exception("not implemented");
		return STATE::CMD_EXEC_DONE;
	}

	void DeviceHardDrive::ReadCommandBlock()
	{
		BYTE val = Pop();
		m_commandBlock.drive = (val & 0b00100000) >> 5;
		m_commandBlock.head = val & 0b00011111;
		val = Pop();
		m_commandBlock.cylinder = (val & 0b11000000) << 2;
		m_commandBlock.sector = val & 0b00011111;
		val = Pop();
		m_commandBlock.cylinder |= val;
		val = Pop();
		m_commandBlock.blockCount = val;
		val = Pop();
		m_commandBlock.noRetries = val & 0b10000000;
		m_commandBlock.eccRetry = val & 0b01000000;
		m_commandBlock.stepCode = val & 0b00000111;

		LogPrintf(LOG_INFO, "CommandBlock: drive[%d] cyl[%d] head[%d] sect[%d], count[%d], R1[%d] R2[%d] SP[%d]",
			m_commandBlock.drive,
			m_commandBlock.cylinder,
			m_commandBlock.head,
			m_commandBlock.sector,
			m_commandBlock.blockCount,
			m_commandBlock.noRetries,
			m_commandBlock.eccRetry,
			m_commandBlock.stepCode);
	}

	void DeviceHardDrive::PushStatus()
	{
		BYTE status = (m_commandError * 2) | (m_currDrive * 32);
		Push(status);
	}

	DeviceHardDrive::STATE DeviceHardDrive::Diagnostic()
	{	
		LogPrintf(LOG_INFO, "COMMAND: %s", m_currCommand->name);

		ReadCommandBlock();
		assert(m_fifo.size() == 0);

		m_currDrive = m_commandBlock.drive;
		m_commandError = false;
		PushStatus();

		m_currOpWait = DelayToTicks(10 * 1000); // 10 ms
		return STATE::CMD_EXEC_DONE;
	}

	DeviceHardDrive::STATE DeviceHardDrive::Recalibrate()
	{
		LogPrintf(LOG_INFO, "COMMAND: %s", m_currCommand->name);

		ReadCommandBlock();
		assert(m_fifo.size() == 0);

		m_commandBlock.cylinder = 0; // Target cylinder for recalibrate

		m_currDrive = m_commandBlock.drive;
		m_currCylinder = 100; // TODO: TEMP
		m_currOpWait = DelayToTicks(100);
		m_commandError = false;

		return STATE::SEEK_EXEC;
	}

	DeviceHardDrive::STATE DeviceHardDrive::InitDrive()
	{
		LogPrintf(LOG_INFO, "COMMAND: %s", m_currCommand->name);

		ReadCommandBlock();
		assert(m_fifo.size() == 0);

		m_currDrive = m_commandBlock.drive;
		m_currOpWait = DelayToTicks(100);
		m_commandError = false;

		return STATE::INIT;
	}

	void DeviceHardDrive::InitDrive2()
	{
		LogPrintf(LOG_INFO, "InitDrive (Part 2)");
		assert(m_fifo.size() == 8);
		
		WORD cylinders = (Pop() << 8) | Pop();
		LogPrintf(LOG_INFO, "| Number of cylinders: %d", cylinders);

		BYTE heads = Pop();
		LogPrintf(LOG_INFO, "| Number of heads:     %d", heads);

		WORD rmcStartCylinder = (Pop() << 8) | Pop();
		LogPrintf(LOG_INFO, "| RWC start cyl:       %d", rmcStartCylinder);

		WORD precompStartCylinder = (Pop() << 8) | Pop();
		LogPrintf(LOG_INFO, "| Precomp start cyl:   %d", precompStartCylinder);

		BYTE eccBurst = Pop();
		LogPrintf(LOG_INFO, "| Max ECC burst:       %d", eccBurst);

		m_currDrive = m_commandBlock.drive;
		m_commandError = false;
		PushStatus();
	}

	DeviceHardDrive::STATE DeviceHardDrive::WriteDataBuffer()
	{
		LogPrintf(LOG_INFO, "COMMAND: %s", m_currCommand->name);

		ReadCommandBlock();
		assert(m_fifo.size() == 0);

		m_currDrive = m_commandBlock.drive;
		m_currOpWait = DelayToTicks(100);
		m_commandError = false;

		return STATE::WRITE_START;
	}

	DeviceHardDrive::STATE DeviceHardDrive::ReadSectors()
	{
		LogPrintf(LOG_INFO, "COMMAND: %s", m_currCommand->name);

		ReadCommandBlock();
		assert(m_fifo.size() == 0);

		m_currDrive = m_commandBlock.drive;
		m_currCylinder = m_commandBlock.cylinder;
		m_currHead = m_commandBlock.head;
		m_currSector = m_commandBlock.sector;

		return STATE::READ_START;
	}

	DeviceHardDrive::STATE DeviceHardDrive::WriteSectors()
	{
		LogPrintf(LOG_INFO, "COMMAND: %s", m_currCommand->name);

		ReadCommandBlock();
		assert(m_fifo.size() == 0);

		m_currDrive = m_commandBlock.drive;
		m_currCylinder = m_commandBlock.cylinder;
		m_currHead = m_commandBlock.head;
		m_currSector = m_commandBlock.sector;

		if (m_currSector >= m_images[0].geometry.sect)
		{
			LogPrintf(LOG_ERROR, "Invalid sector [%d]", m_currSector);
			throw std::exception("Invalid sector");
		}
		if (m_currCylinder >= m_images[0].geometry.cyl)
		{
			LogPrintf(LOG_ERROR, "Invalid cylinder [%d]", m_currCylinder);
			throw std::exception("Invalid cylinder");
		}
		if (m_currHead > m_images[0].geometry.head)
		{
			LogPrintf(LOG_ERROR, "Invalid head [%d]", m_currHead);
			throw std::exception("Invalid head");
		}

		return STATE::WRITE_START;
	}

	void DeviceHardDrive::ReadSector()
	{
		LogPrintf(LOG_DEBUG, "ReadSector, fifo=%d", m_fifo.size());

		// TODO: Handle not loaded

		if (m_fifo.size() == 0)
		{ 
			if (!m_commandBlock.blockCount)
			{
				LogPrintf(LOG_INFO, "ReadSector done, end of track");
				m_currOpWait = DelayToTicks(10);
				m_nextState = STATE::RW_DONE;
				m_state = STATE::CMD_EXEC_DELAY;
				return;
			}

			uint32_t offset = m_images[0].geometry.CHS2A(m_currCylinder, m_currHead, m_currSector);
			fseek(m_images[0].data, offset, SEEK_SET);
			fread(m_sectorBuffer, 512, 1, m_images[0].data);
			for (size_t b = 0; b < 512; ++b)
			{
				Push(m_sectorBuffer[b]);
			}

			UpdateCurrPos();

			--m_commandBlock.blockCount;
		}

		SetDMAPending();

		// Timeout
		m_currOpWait = DelayToTicks(100 * 1000);

		m_state = m_dmaEnabled ? STATE::DMA_WAIT : STATE::NDMA_WAIT;
	}

	void DeviceHardDrive::WriteSector()
	{
		LogPrintf(LOG_DEBUG, "WriteSector, fifo=%d", m_fifo.size());

		if (m_fifo.size() == 512)
		{
			for (size_t b = 0; b < 512; ++b)
			{
				m_sectorBuffer[b] = Pop();
			}

			// Actual write to disk image
			if (m_currcommandID != WRITE_DATA_BUFFER)
			{
				uint32_t offset = m_images[0].geometry.CHS2A(m_currCylinder, m_currHead, m_currSector);
				fseek(m_images[0].data, offset, SEEK_SET);
				fwrite(m_sectorBuffer, 512, 1, m_images[0].data);
			}

			if (!m_commandBlock.blockCount || m_currcommandID == WRITE_DATA_BUFFER)
			{
				m_currOpWait = DelayToTicks(10);
				m_nextState = STATE::RW_DONE;
				m_state = STATE::CMD_EXEC_DELAY;
				return;
			}

			UpdateCurrPos();

			--m_commandBlock.blockCount;
		}

		SetDMAPending();

		// Timeout
		m_currOpWait = DelayToTicks(100 * 1000);

		m_state = m_dmaEnabled ? STATE::DMA_WAIT : STATE::NDMA_WAIT;
	}

	void DeviceHardDrive::RWSectorEnd()
	{
		LogPrintf(LOG_INFO, "ReadWriteSectorEnd, fifo=%d", m_fifo.size());

		m_fifo.clear();

		m_currDrive = m_commandBlock.drive;
		m_commandError = false;
		PushStatus();
	}

	void DeviceHardDrive::DMATerminalCount()
	{
		LogPrintf(LOG_INFO, "DMATerminalCount");

		// TODO: Check that we are in correct state;
		m_dmaPending = false;
		m_currOpWait = DelayToTicks(10);
		m_nextState = STATE::RW_DONE;
		m_state = STATE::CMD_EXEC_DELAY;
	}

	bool DeviceHardDrive::UpdateCurrPos()
	{
		const HardDisk& disk = m_images[0];

		bool isEOT = (m_currSector >= disk.geometry.sect-1);
		LogPrintf(LOG_DEBUG, "UpdateCurrPos cyl=[%d] head=[%d] sector=[%d], EOT=[%d]", m_currCylinder, m_currHead, m_currSector, isEOT);

		m_currCylinder += (isEOT && (m_currHead == (disk.geometry.head - 1))) ? 1 : 0;
		if (isEOT)
		{
			++m_currHead;
			if (m_currHead >= disk.geometry.head)
			{
				m_currHead = 0;
			}
		}

		m_currSector = isEOT ? 0 : (m_currSector + 1);
		LogPrintf(LOG_INFO, "UpdateCurrPos done: cyl=[%d] head=[%d] sector=[%d]", m_currCylinder, m_currHead, m_currSector);

		return isEOT;
	}
}
