#include "stdafx.h"
#include "Device8042AT.h"
#include "CPU/CPU80286.h"

using emul::GetBit;

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
		BYTE value =
			(m_status.parityError << 7) |
			(m_status.receiveTimeout << 6) |
			(m_status.transmitTimeout << 5) |
			(!m_status.inhibit << 4) |
			((BYTE)m_status.mode << 3) |
			(m_status.systemFlag << 2) |
			(m_status.inputBufferFull << 1) |
			(m_status.outputBufferFull << 0);

		LogPrintf(LOG_DEBUG, "ReadStatus: [%cPE %cRTO %cTTO %cINHIBIT MODE[%s] %cSYSFLAG %cINFULL %cOUTFULL]",
			(m_status.parityError ? ' ' : '/'),
			(m_status.receiveTimeout ? ' ' : '/'),
			(m_status.transmitTimeout ? ' ' : '/'),
			(m_status.inhibit ? ' ' : '/'),
			((BYTE)m_status.mode ? "CMD" : "DAT"),
			(m_status.systemFlag ? ' ' : '/'),
			(m_status.inputBufferFull ? ' ' : '/'),
			(m_status.outputBufferFull ? ' ' : '/'));

		return value;
	}

	void Device8042AT::WriteCommand(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Command, value=%02x", value);
		m_status.mode = Status::Mode::COMMAND;

		m_commandParameter = false;

		Command command = (Command)value;
		if ((command >= Command::CMD_PULSE_OUTPUT_PORT_MIN) && 
			(command <= Command::CMD_PULSE_OUTPUT_PORT_MAX))
		{
			LogPrintf(LOG_INFO, "Pulse bits %02x", value & 0x0F);

			// Restart CPU
			if ((value & 1) == 0)
			{
				LogPrintf(LOG_WARNING, "***RESTART CPU***");
				m_cpu->Reset();
			} 
			else if ((value & 0x0F) != 0x0F)
			{
				LogPrintf(LOG_ERROR, "Pulse bits %02x Not implemented", value);
				throw std::exception("Not implemented");
			}
		}
		else switch (command)
		{
		case Command::CMD_READ_COMMAND_BYTE:
			ReadCommandByte();
			break;
		case Command::CMD_WRITE_COMMAND_BYTE:
			m_activeCommand = command;
			m_commandParameter = true;
			break;
		case Command::CMD_SELF_TEST:
			LogPrintf(LOG_INFO, "Self Test");
			StartSelfTest();
			break;
		case Command::CMD_INTERFACE_TEST:
			LogPrintf(LOG_INFO, "Interface Test");
			// Checks for stuck CLK/DATA lines
			WriteOutputBuffer(0); // No error
			break;
		case Command::CMD_DIAGNOSTIC_DUMP:
			LogPrintf(LOG_INFO, "Diagnostic Dump");
			LogPrintf(LOG_ERROR, "Diagnostic Dump Not Implemented");
			throw std::exception("Not implemented");
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
			ReadConfigurationSwitches();
			break;
		case Command::CMD_READ_OUTPUT_PORT:
			LogPrintf(LOG_INFO, "Read Output Port");
			LogPrintf(LOG_ERROR, "Read Output Port Not Implemented");
			throw std::exception("Not implemented");
			break;
		case Command::CMD_WRITE_OUTPUT_PORT:
			LogPrintf(LOG_INFO, "Write Output Port");
			m_activeCommand = command;
			m_commandParameter = true;
			break;
		case Command::CMD_READ_TEST_INPUTS:
			LogPrintf(LOG_INFO, "Read Test Inputs");
			// CLK (B0) line is driven low when inhibit = true
			// DATA(B1) doesn't really matter (serial stream)
			WriteOutputBuffer(m_status.inhibit ? 0 : 1); // Read CLK(B0) and Data(B1) bits
			break;
		default:
			LogPrintf(LOG_ERROR, "Invalid Command %02x", command);
			break;
		}
	}

	void Device8042AT::ReadConfigurationSwitches()
	{
		BYTE value =
			(!m_switches.keyboardLock << 7) |
			((BYTE)m_switches.display << 6) |
			(!m_switches.manufacturing << 5) |
			(m_switches.ramsel << 4);

		LogPrintf(LOG_INFO, "ReadConfigurationSwitches [%cKEYLOCK DISP[%s] %cPOSTLOOP %cRAMSEL]",
			(m_switches.keyboardLock ? ' ' : '/'),
			((m_switches.display == DISPLAY::MDA) ? "MDA" : "CGA"),
			(m_switches.manufacturing ? ' ' : '/'),
			(m_switches.ramsel ?' ' : '/'));
		
		WriteOutputBuffer(value);
	}

	BYTE Device8042AT::ReadPortB()
	{
		BYTE value = 
			(m_portB.parityCheckOccurred << 7) |
			(m_portB.channelCheckOccurred << 6) |
			(m_portB.timer2Output << 5) |
			((m_portB.refresh == 0) << 4) |
			(m_portB.channelCheckEnabled << 3) |
			(m_portB.parityCheckEnabled << 2) |
			(m_portB.speakerDataEnabled << 1) |
			(m_portB.timer2Gate << 0);

		LogPrintf(LOG_TRACE, "ReadPortB [%cPCHK %cCCHK %cTIMER2 %cRFSH %CCHKEN %cPCHKEN %cSPKEN %cTIMER2GATE]",
			(m_portB.parityCheckOccurred ? ' ' : '/'),
			(m_portB.channelCheckOccurred ? ' ' : '/'),
			(m_portB.timer2Output ? ' ' : '/'),
			((m_portB.refresh == 0) ? ' ' : '/'),
			(m_portB.channelCheckEnabled ? ' ' : '/'),
			(m_portB.parityCheckEnabled ? ' ' : '/'),
			(m_portB.speakerDataEnabled ? ' ' : '/'),
			(m_portB.timer2Gate ? ' ' : '/'));

		return value;
	}
	void Device8042AT::WritePortB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write Port B, value=%02x", value);

		m_portB.channelCheckEnabled = GetBit(value, 3);
		m_portB.parityCheckEnabled = GetBit(value, 2);
		m_portB.speakerDataEnabled = GetBit(value, 1);
		m_portB.timer2Gate = GetBit(value, 0);
	}

	void Device8042AT::ReadCommandByte()
	{
		BYTE value =
			(m_commandByte.pcCompatibilityMode << 6) |
			(m_commandByte.pcComputerMode << 5) |
			(m_commandByte.disableKeyboard << 4) |
			(m_commandByte.inhibitOverride << 3) |
			(m_status.systemFlag << 2) |
			(m_commandByte.outputBufferFullInterrupt << 0);

		LogPrintf(LOG_DEBUG, "Read Command Byte, value=%02x", value);
		WriteOutputBuffer(value);
	}
	void Device8042AT::WriteCommandByte()
	{
		BYTE value = ReadInputBuffer();

		m_commandByte.pcCompatibilityMode = GetBit(value, 6);
		m_commandByte.pcComputerMode = GetBit(value, 5);
		m_commandByte.disableKeyboard = GetBit(value, 4);
		m_commandByte.inhibitOverride = GetBit(value, 3);
		m_status.systemFlag = GetBit(value, 2);
		m_commandByte.outputBufferFullInterrupt = GetBit(value, 0);

		LogPrintf(LOG_INFO, "WriteCommandByte [%cPCCOMPAT %cPCMODE %cKBDDISABLE %cINHOVR %cSYSFLAG %cINTERRUPT]",
			(m_commandByte.pcCompatibilityMode ? ' ' : '/'),
			(m_commandByte.pcComputerMode ? ' ' : '/'),
			(m_commandByte.disableKeyboard ? ' ' : '/'),
			(m_commandByte.inhibitOverride ? ' ' : '/'),
			(m_status.systemFlag ? ' ' : '/'),
			(m_commandByte.outputBufferFullInterrupt ? ' ' : '/'));

		m_activeCommand = Command::CMD_NONE;
	}

	void Device8042AT::WriteOutputPort()
	{
		BYTE value = ReadInputBuffer();

		// Bit 7: Keyboard DATA (unconnected)
		// Bit 6: Keyboard CLK (unconnected)
		// Bit 5: Input buffer full (unconnected)
		// Bit 4: Output buffer full
		// Bit 2,3 reserved, unconnected
		// Bit 1: 1 = A20 Line enabled, 0 = A20 Forced to 0
		// Bit 0: 0 = CPU Reset

		// We only care about bit 0, 1 and 4

		// Bit 4 is connected to IRQ1, normally reflects m_status.outputBufferFull
		// We do it the other way around
		m_status.outputBufferFull = GetBit(value, 4);

		// A20 Gate
		m_cpu->ForceA20Low(!GetBit(value, 1));

		// CPU Reset
		if (!GetBit(value, 0))
		{
			m_cpu->Reset();
		}

		m_activeCommand = Command::CMD_NONE;
	}

	void Device8042AT::StartSelfTest()
	{
		LogPrintf(LOG_INFO, "Start Self Test");
		m_activeCommand = Command::CMD_SELF_TEST;
		m_commandDelay = 1000;
	}

	void Device8042AT::EndSelfTest()
	{
		LogPrintf(LOG_INFO, "End Self Test");
		WriteOutputBuffer(0x55);
		m_status.systemFlag = true;
		m_activeCommand = Command::CMD_NONE;
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
		if (m_activeCommand != Command::CMD_NONE)
		{
			switch (m_activeCommand)
			{
			case Command::CMD_SELF_TEST:
				if (--m_commandDelay == 0)
				{
					EndSelfTest();
				}
				break;
			case Command::CMD_WRITE_COMMAND_BYTE:
				if (m_status.inputBufferFull)
				{
					WriteCommandByte();
				}
				break;
			case Command::CMD_WRITE_OUTPUT_PORT:
				if (m_status.inputBufferFull)
				{
					WriteOutputPort();
				}
				break;
			default:
				LogPrintf(LOG_WARNING, "Invalid active command");
				break;
			}
		}
	}

}
