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

		FILE* f = fopen(path, "rb");
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
		//// In non-DMA mode a read or write clears the interrupt pin
		//if (m_nonDMA)
		//{
		//	ClearInterrupt();
		//}

		LogPrintf(Logger::LOG_DEBUG, "ReadStatus [%cIRQ %cDRQ %cBSY %cDATAcmd %cINout %cREQ]",
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
		// In non-DMA mode a read or write clears the interrupt pin
		//if (m_nonDMA)
		//{
		//	ClearInterrupt();
		//}

		BYTE result = 0xFF;
		LogPrintf(Logger::LOG_DEBUG, "ReadDataFIFO");
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
			m_state = STATE::PARAM_WAIT;
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
			LogPrintf(Logger::LOG_INFO, "Command Execution done");
			m_dataRegisterReady = true;
			m_controlData = ControlData::CONTROL;

			// No results, ready for next command
			if (m_fifo.size() == 0)
			{
				m_dataInputOutput = DataDirection::CPU2HDC;
				m_state = STATE::CMD_WAIT;
			}
			else
			{
				m_dataInputOutput = DataDirection::HDC2CPU;
				m_state = STATE::RESULT_WAIT;
			}

			if (m_currCommand->interrupt)
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
		case STATE::DMA_WAIT:
			LogPrintf(Logger::LOG_DEBUG, "DMA Wait");
			if (--m_currOpWait == 0)
			{
				m_state = STATE::CMD_ERROR;
			}
			break;
		case STATE::NDMA_WAIT:
			LogPrintf(Logger::LOG_DEBUG, "NDMA Wait");
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

	DeviceHardDrive::STATE DeviceHardDrive::NotImplemented()
	{
		LogPrintf(LOG_ERROR, "Command [%s] not implemented", m_currCommand->name);
		m_fifo.clear();
		throw std::exception("not implemented");
		return STATE::CMD_EXEC_DONE;
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

}
