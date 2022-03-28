#include "stdafx.h"
#include "Device8042AT.h"
#include "CPU/CPU80286.h"

namespace ppi
{
	Device8042AT::Device8042AT(WORD baseAddress) :
		DevicePPI(baseAddress),
		Logger("8042")
	{
	}

	void Device8042AT::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8042AT::ReadOutputBuffer));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8042AT::WriteInputBuffer));

		// 8255 PortB compatibility
		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8042AT::ReadPortB));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8042AT::WritePortB));

		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&Device8042AT::ReadStatus));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&Device8042AT::WriteCommand));
	}

	void Device8042AT::SetCPU(emul::CPU8086* cpu)
	{
		assert(cpu);
		// Need minimum 286 for A20 line control
		m_cpu = dynamic_cast<emul::CPU80286*>(cpu);
		if (m_cpu == nullptr)
		{
			LogPrintf(LOG_ERROR, "Fatal: Keyboard Controller needs a 286+ CPU");
			throw std::exception("Need a 286+ CPU");
		}
	}

	BYTE Device8042AT::ReadInputBuffer()
	{
		m_status.inputBufferFull = false;
		return m_inputBuffer;
	}

	void Device8042AT::WriteInputBuffer(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteInputBuffer, value=%02x", value);
		m_status.mode = Status::Mode::DATA;
		m_status.inputBufferFull = true;
		m_inputBuffer = value;
	}

	BYTE Device8042AT::ReadOutputBuffer()
	{
		m_status.outputBufferFull = false;
		LogPrintf(LOG_INFO, "ReadOutputBuffer, value=%02x", m_outputBuffer);
		return m_outputBuffer;
	}

	void Device8042AT::WriteOutputBuffer(BYTE value)
	{
		m_status.outputBufferFull = true;
		m_outputBuffer = value;
	}

	BYTE Device8042AT::ReadStatus()
	{
		LogPrintf(LOG_TRACE, "Read Status");

		BYTE value =
			(m_status.parityError << 7) |
			(m_status.receiveTimeout << 6) |
			(m_status.transmitTimeout << 5) |
			(m_status.inhibit << 4) |
			((BYTE)m_status.mode << 3) |
			(m_status.selfTestOK << 2) |
			(m_status.inputBufferFull << 1) |
			(m_status.outputBufferFull << 0);

		LogPrintf(LOG_DEBUG, "ReadStatus: [%cPE %cRTO %cTTO %cINHIBIT MODE[%s] %cSELFTESTOK %cINFULL %cOUTFULL]",
			(m_status.parityError ? ' ' : '/'),
			(m_status.receiveTimeout ? ' ' : '/'),
			(m_status.transmitTimeout ? ' ' : '/'),
			(m_status.inhibit ? ' ' : '/'),
			((BYTE)m_status.mode ? "CMD" : "DAT"),
			(m_status.selfTestOK ? ' ' : '/'),
			(m_status.inputBufferFull ? ' ' : '/'),
			(m_status.outputBufferFull ? ' ' : '/'));

		return value;
	}

	void Device8042AT::WriteCommand(BYTE value)
	{
		LogPrintf(LOG_TRACE, "Write Command, value=%02x", value);
		m_status.mode = Status::Mode::COMMAND;

		Command command = (Command)value;
		if ((command >= Command::CMD_PULSE_OUTPUT_PORT_MIN) && 
			(command < Command::CMD_PULSE_OUTPUT_PORT_MAX))
		{
			LogPrintf(LOG_WARNING, "Pulse bits %02x", value & 0x0F);

			// Restart CPU
			if (((int)command & 1) == 0)
			{
				LogPrintf(LOG_WARNING, "***RESTART CPU***");
				m_cpu->Reset();
			}
		}
		else switch (command)
		{
		case Command::CMD_READ_COMMAND_BYTE:
			LogPrintf(LOG_INFO, "Read Command Byte");
			break;
		case Command::CMD_WRITE_COMMAND_BYTE:
			LogPrintf(LOG_INFO, "Write Command Byte");
			break;
		case Command::CMD_SELF_TEST:
			LogPrintf(LOG_INFO, "Self Test");
			StartSelfTest();
			break;
		case Command::CMD_INTERFACE_TEST:
			LogPrintf(LOG_INFO, "Interface Test");
			break;
		case Command::CMD_DIAGNOSTIC_DUMP:
			LogPrintf(LOG_INFO, "Diagnostic Dump");
			break;
		case Command::CMD_DISABLE_KEYBOARD:
			LogPrintf(LOG_INFO, "Disable Keyboard");
			m_status.inhibit = true;
			break;
		case Command::CMD_ENABLE_KEYBOARD:
			LogPrintf(LOG_INFO, "Enable Keyboard");
			m_status.inhibit = false;
			break;
		case Command::CMD_READ_INPUT_PORT:
			LogPrintf(LOG_INFO, "Read Input Port");
			WriteOutputBuffer(0x55);
			break;
		case Command::CMD_READ_OUTPUT_PORT:
			LogPrintf(LOG_INFO, "Read Output Port");
			break;
		case Command::CMD_WRITE_OUTPUT_PORT:
			LogPrintf(LOG_INFO, "Write Output Port");
			break;
		case Command::CMD_READ_TEST_INPUTS:
			LogPrintf(LOG_INFO, "Test Inputs");
			break;
		default:
			LogPrintf(LOG_ERROR, "Invalid Command %02x", command);
			break;
		}
	}

	BYTE Device8042AT::ReadPortB()
	{
		LogPrintf(LOG_DEBUG, "Read Port B, refresh=%d", m_portB.refresh);

		return ((m_portB.refresh == 0) << 4);
	}
	void Device8042AT::WritePortB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Port B, value=%02x", value);
	}

	void Device8042AT::StartSelfTest()
	{
		LogPrintf(LOG_INFO, "Start Self Test");
		m_selfTest = true;
		m_commandDelay = 1000;
	}

	void Device8042AT::EndSelfTest()
	{
		LogPrintf(LOG_INFO, "End Self Test");
		m_selfTest = false;
		WriteOutputBuffer(0x55);
		m_status.selfTestOK = true;
	}

	void Device8042AT::SetRefresh(bool refreshBit)
	{
		// Toggle at the 0->1 transition
		static bool lastRefresh = false;
		if (!lastRefresh && refreshBit)
		{
			++m_portB.refresh;
			m_portB.refresh &= 7;
		}
	}

	void Device8042AT::Tick()
	{
		if (m_selfTest && m_commandDelay)
		{
			if (--m_commandDelay == 0)
			{
				EndSelfTest();
			}
		}
	}

}
