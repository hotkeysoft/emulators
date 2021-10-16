#include "Device8250.h"

using emul::GetHByte;
using emul::GetLByte;
using emul::SetHByte;
using emul::SetLByte;

namespace uart
{
	Device8250::Device8250(WORD baseAddress, size_t clockSpeedHz) : 
		Logger("UART8250"), 
		m_baseAddress(baseAddress)
	{
		Reset();
		s_clockSpeed = clockSpeedHz;
	}

	void Device8250::Reset()
	{
		m_interruptEnable.dataAvailableInterrupt = false;
		m_interruptEnable.txEmpty = false;
		m_interruptEnable.errorOrBreak = false;
		m_interruptEnable.statusChange = false;

		m_lineControl.wordLengthSelect = 0;
		m_lineControl.stopBits = false;
		m_lineControl.parityEnable = false;
		m_lineControl.parityEven = false;
		m_lineControl.parityStick = false;
		m_lineControl.setBreak = false;
		m_lineControl.divisorLatchAccess = false;

		m_modemControl.dtr = false;
		m_modemControl.rts = false;
		m_modemControl.out1 = false;
		m_modemControl.out2 = false;
		
		m_lineStatus.dataReady = false;
		m_lineStatus.overrunError = false;
		m_lineStatus.parityError = false;
		m_lineStatus.framingError = false;
		m_lineStatus.breakInterrupt = false;
		m_lineStatus.txHoldingRegisterEmpty = true;
		m_lineStatus.txShiftRegisterEmpty = true;

		m_modemStatus.cts = false;
		m_modemStatus.dsr = false;
		m_modemStatus.ri = false;
		m_modemStatus.dcd = false;

		m_lastModemStatus.cts = false;
		m_lastModemStatus.dsr = false;
		m_lastModemStatus.ri = false;
		m_lastModemStatus.dcd = false;

		m_modemStatusDelta.cts = false;
		m_modemStatusDelta.dsr = false;
		m_modemStatusDelta.ri = false;
		m_modemStatusDelta.dcd = false;

		m_serialOut = true;

		m_intr.rxLineStatus = false;
		m_intr.rxDataAvailable = false;
		m_intr.txHoldingRegisterEmpty = false;
		m_intr.modemStatus = false;

		UpdateDataConfig();
	}

	void Device8250::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8250::Read0));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8250::Write0));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8250::Read1));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8250::Write1));

		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&Device8250::ReadInterruptIdentification));

		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&Device8250::ReadLineControl));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device8250::WriteLineControl));

		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&Device8250::ReadModemControl));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&Device8250::WriteModemControl));

		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&Device8250::ReadLineStatus));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&Device8250::WriteLineStatus));

		Connect(m_baseAddress + 6, static_cast<PortConnector::INFunction>(&Device8250::ReadModemStatus));
		Connect(m_baseAddress + 6, static_cast<PortConnector::OUTFunction>(&Device8250::WriteModemStatus));
	}

	BYTE Device8250::Read0()
	{
		return m_lineControl.divisorLatchAccess ? ReadDivisorLatchLSB() : ReadReceiver();
	}
	void Device8250::Write0(BYTE value)
	{
		m_lineControl.divisorLatchAccess ? WriteDivisorLatchLSB(value) : WriteTransmitter(value);
	}

	BYTE Device8250::Read1()
	{
		return m_lineControl.divisorLatchAccess ? ReadDivisorLatchMSB() : ReadInterruptEnable();
	}
	void Device8250::Write1(BYTE value)
	{
		m_lineControl.divisorLatchAccess ? WriteDivisorLatchMSB(value) : WriteInterruptEnable(value);
	}

	BYTE Device8250::ReadDivisorLatchLSB()
	{
		LogPrintf(LOG_DEBUG, "ReadDivisorLatchLSB, value=%02x", GetLByte(m_divisorLatch));
		return GetLByte(m_divisorLatch);
	}
	void Device8250::WriteDivisorLatchLSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDivisorLatchLSB, value=%02x", value);
		SetLByte(m_divisorLatch, value);
	}

	BYTE Device8250::ReadDivisorLatchMSB()
	{
		LogPrintf(LOG_DEBUG, "ReadDivisorLatchMSB, value=%02x", GetHByte(m_divisorLatch));
		return GetHByte(m_divisorLatch);
	}
	void Device8250::WriteDivisorLatchMSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDivisorLatchLSB, value=%02x", value);
		SetHByte(m_divisorLatch, value);
	}

	BYTE Device8250::ReadReceiver()
	{
		m_intr.rxDataAvailable = false;
		m_lineStatus.dataReady = false;

		LogPrintf(LOG_INFO, "ReadReceiver, value=%02x", 0xFF);
		return 0xFF;
	}
	void Device8250::WriteTransmitter(BYTE value)
	{
		m_intr.txHoldingRegisterEmpty = false;
		m_lineStatus.txHoldingRegisterEmpty = false;

		LogPrintf(LOG_INFO, "WriteTransmitter, value=%02x", value);
	}

	BYTE Device8250::ReadInterruptEnable()
	{
		BYTE ret =
			(m_interruptEnable.statusChange << 3) |
			(m_interruptEnable.errorOrBreak << 2) |
			(m_interruptEnable.txEmpty << 1) |
			(m_interruptEnable.dataAvailableInterrupt << 0);

		LogPrintf(LOG_DEBUG, "ReadInterruptEnable, value=%02x", ret);
		return ret;
	}
	void Device8250::WriteInterruptEnable(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteInterruptEnable, value=%02x", value);

		m_interruptEnable.statusChange = value & 0x08;
		m_interruptEnable.errorOrBreak = value & 0x04;
		m_interruptEnable.txEmpty = value & 0x02;
		m_interruptEnable.dataAvailableInterrupt = value & 0x01;

		LogPrintf(LOG_INFO, "WriteInterruptEnable: [%cSTATUS_CHNG %cERR_BRK %cTXR_EMPTY %cDATA_AVAIL]",
			m_interruptEnable.statusChange ? ' ' : '/',
			m_interruptEnable.errorOrBreak ? ' ' : '/',
			m_interruptEnable.txEmpty ? ' ' : '/',
			m_interruptEnable.dataAvailableInterrupt ? ' ' : '/');
	}

	BYTE Device8250::ReadInterruptIdentification()
	{
		BYTE ret;
		// In priority order
		if (m_intr.rxLineStatus)
		{
			LogPrintf(LOG_INFO, "ReadInterruptIdentification: Receiver Error");
			ret = 0b110;
		} 
		else if (m_intr.rxDataAvailable)
		{
			LogPrintf(LOG_INFO, "ReadInterruptIdentification: Receiver Data Available");
			ret = 0b100;
		}
		else if (m_intr.txHoldingRegisterEmpty)
		{
			// Reset interrupt 
			m_intr.txHoldingRegisterEmpty = false;

			LogPrintf(LOG_INFO, "ReadInterruptIdentification: Transmitter Holding Register Empty");
			ret = 0b010;
		}
		else if (m_intr.modemStatus)
		{
			LogPrintf(LOG_INFO, "ReadInterruptIdentification: MODEM status (CTS | DSR | RI | DCD)");
			ret = 0b000;
		}
		else // no interrupt
		{
			//LogPrintf(LOG_DEBUG, "ReadInterruptIdentification: None");
			ret = 0b001;
		}
		return ret;
	}

	BYTE Device8250::ReadLineControl()
	{
		BYTE ret =
			(m_lineControl.divisorLatchAccess << 7) |
			(m_lineControl.setBreak << 6) |
			(m_lineControl.parityStick << 5) |
			(m_lineControl.parityEven << 4) |
			(m_lineControl.parityEnable << 3) |
			(m_lineControl.stopBits << 2) |
			(m_lineControl.wordLengthSelect & 3);

		LogPrintf(LOG_DEBUG, "ReadLineControl, value=%02x", ret);

		return ret;
	}
	void Device8250::WriteLineControl(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteLineControl, value=%02x", value);

		m_lineControl.divisorLatchAccess = value & 0x80;
		m_lineControl.setBreak = value & 0x40;
		LogPrintf(LOG_INFO, "WriteLineControl: [%cDLAB %cBREAK]",
			m_lineControl.divisorLatchAccess ? ' ' : '/',
			m_lineControl.setBreak ? ' ' : '/');

		m_lineControl.parityStick = value & 0x20;
		m_lineControl.parityEven = value & 0x10;
		m_lineControl.parityEnable = value & 0x08;
		m_lineControl.stopBits = value & 0x04;
		m_lineControl.wordLengthSelect = value & 0x03;
		UpdateDataConfig();
	}

	BYTE Device8250::ReadModemControl()
	{
		BYTE ret =
			(m_modemControl.loopback << 4) |
			(m_modemControl.out2 << 3) |
			(m_modemControl.out1 << 2) |
			(m_modemControl.rts << 1) |
			(m_modemControl.dtr << 0);

		LogPrintf(LOG_DEBUG, "ReadModemControl, value=%02x", ret);
		return ret;
	}
	void Device8250::WriteModemControl(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteModemControl, value=%02x", value);

		m_modemControl.loopback = value & 0x10;
		m_modemControl.out2 = value & 0x08;
		m_modemControl.out1 = value & 0x04;
		m_modemControl.rts = value & 0x02;
		m_modemControl.dtr = value & 0x01;

		LogPrintf(LOG_INFO, "WriteModemControl: [%cLOOPBACK %cOUT2 %cOUT1 %cRTS %cDTR]",
			m_modemControl.loopback ? ' ' : '/',
			m_modemControl.out2 ? ' ' : '/',
			m_modemControl.out1 ? ' ' : '/',
			m_modemControl.rts ? ' ' : '/',
			m_modemControl.dtr ? ' ' : '/');
	}

	BYTE Device8250::ReadLineStatus()
	{
		// Reset interrupt
		m_intr.rxLineStatus = false;

		BYTE ret =
			(m_lineStatus.txShiftRegisterEmpty << 6) |
			(m_lineStatus.txHoldingRegisterEmpty << 5) |
			(m_lineStatus.breakInterrupt << 4) |
			(m_lineStatus.framingError << 3) |
			(m_lineStatus.parityError << 2) |
			(m_lineStatus.overrunError << 1) |
			(m_lineStatus.dataReady << 0);

		LogPrintf(LOG_DEBUG, "ReadLineStatus, value=%02x", ret);

		LogPrintf(LOG_INFO, "ReadLineStatus: [%cTSRE %cTHRE %cBI %cFE %cPE %cOE %cDR]",
			m_lineStatus.txShiftRegisterEmpty ? ' ' : '/',
			m_lineStatus.txHoldingRegisterEmpty ? ' ' : '/',
			m_lineStatus.breakInterrupt ? ' ' : '/',
			m_lineStatus.framingError ? ' ' : '/',
			m_lineStatus.parityError ? ' ' : '/',
			m_lineStatus.overrunError ? ' ' : '/',
			m_lineStatus.dataReady ? ' ' : '/');

		// Reset error flags
		m_lineStatus.overrunError = false;
		m_lineStatus.parityError = false;
		m_lineStatus.framingError = false;
		m_lineStatus.breakInterrupt = false;

		return ret;
	}

	// Port is "officially" read-only. However it can be written to for factory testing
	//
	// Setting the bits 0-5 will trigger the approprate interrupt
	void Device8250::WriteLineStatus(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteLineStatus, value=%02x", value);

		m_lineStatus.txHoldingRegisterEmpty = value & 0x20;
		m_lineStatus.breakInterrupt = value & 0x10;
		m_lineStatus.framingError = value & 0x08;
		m_lineStatus.parityError = value & 0x04;
		m_lineStatus.overrunError = value & 0x02;
		m_lineStatus.dataReady = value & 0x01;

		LogPrintf(LOG_DEBUG, "WriteLineStatus: [%cTSRE %cTHRE %cBI %cFE %cPE %cOE %cDR]",
			m_lineStatus.txShiftRegisterEmpty ? ' ' : '/',
			m_lineStatus.txHoldingRegisterEmpty ? ' ' : '/',
			m_lineStatus.breakInterrupt ? ' ' : '/',
			m_lineStatus.framingError ? ' ' : '/',
			m_lineStatus.parityError ? ' ' : '/',
			m_lineStatus.overrunError ? ' ' : '/',
			m_lineStatus.dataReady ? ' ' : '/');

		// Set up interrupts
		m_intr.rxDataAvailable = m_lineStatus.dataReady;
		m_intr.rxLineStatus = value & 0b11110;
		m_intr.txHoldingRegisterEmpty = m_lineStatus.txHoldingRegisterEmpty;
	}

	BYTE Device8250::ReadModemStatus()
	{
		// Reset interrupt
		m_intr.modemStatus = false;
		
		BYTE ret =
			(m_modemStatus.dcd << 7) |
			(m_modemStatus.ri << 6) |
			(m_modemStatus.dsr << 5) |
			(m_modemStatus.cts << 4) |
			(m_modemStatusDelta.dcd << 3) |
			(m_modemStatusDelta.ri << 2) |
			(m_modemStatusDelta.dsr << 1) |
			(m_modemStatusDelta.cts << 0);

		LogPrintf(LOG_DEBUG, "ReadModemStatus, value=%02x", ret);

		LogPrintf(LOG_DEBUG, "ReadModemStatus: [%cDCD %cRI %cDSR %cCTS | %cDDCD %cTERI %cDDSR %cDCTS]",
			m_modemStatus.dcd ? ' ' : '/',
			m_modemStatus.ri ? ' ' : '/',
			m_modemStatus.dsr ? ' ' : '/',
			m_modemStatus.cts ? ' ' : '/',
			m_modemStatusDelta.dcd ? ' ' : '/',
			m_modemStatusDelta.ri ? ' ' : '/',
			m_modemStatusDelta.dsr ? ' ' : '/',
			m_modemStatusDelta.cts ? ' ' : '/');

		// Save current state for next deltas
		m_lastModemStatus = m_modemStatus;

		// Reset Deltas
		m_modemStatusDelta.dcd = false;
		m_modemStatusDelta.ri = false;
		m_modemStatusDelta.dsr = false;
		m_modemStatusDelta.cts = false;

		return ret;
	}
	
	// Port is "officially" read-only. However it can be written to for factory testing
	//
	// Setting the bits 0-3 will trigger the approprate interrupt
	// Bits 4-7 are read-only
	void Device8250::WriteModemStatus(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteModemStatus, value=%02x", value);

		m_modemStatusDelta.dcd = value & 0x08;
		m_modemStatusDelta.ri = value & 0x04;
		m_modemStatusDelta.dsr = value & 0x02;
		m_modemStatusDelta.cts = value & 0x01;

		LogPrintf(LOG_DEBUG, "WriteModemStatus: [%cDCD %cRI %cDSR %cCTS | %cDDCD %cTERI %cDDSR %cDCTS]",
			m_modemStatus.dcd ? ' ' : '/',
			m_modemStatus.ri ? ' ' : '/',
			m_modemStatus.dsr ? ' ' : '/',
			m_modemStatus.cts ? ' ' : '/',
			m_modemStatusDelta.dcd ? ' ' : '/',
			m_modemStatusDelta.ri ? ' ' : '/',
			m_modemStatusDelta.dsr ? ' ' : '/',
			m_modemStatusDelta.cts ? ' ' : '/');
	}

	void Device8250::UpdateDataConfig()
	{
		m_dataConfig.dataLength = m_lineControl.wordLengthSelect + 5;

		if (!m_lineControl.parityEnable)
		{
			m_dataConfig.parity = Parity::NONE;
		}
		else if (m_lineControl.parityStick)
		{
			m_dataConfig.parity = m_lineControl.parityEven ? Parity::SPACE : Parity::MARK;
		}
		else
		{
			m_dataConfig.parity = m_lineControl.parityEven ? Parity::EVEN : Parity::ODD;
		}

		if (!m_lineControl.stopBits)
		{
			m_dataConfig.stopBits = StopBits::ONE;
		}
		else
		{
			m_dataConfig.stopBits = m_dataConfig.dataLength == 5 ? StopBits::ONE_AND_HALF : StopBits::TWO;
		}

		LogPrintf(LOG_INFO, "UpdateDataConfig: [%c-%d-%c]",
			m_dataConfig.parity,
			m_dataConfig.dataLength,
			m_dataConfig.stopBits);
	}

	void Device8250::Tick()
	{
		// Modem status line changes

		// TODO: modem interrupts in loopback

		if (m_lastModemStatus.dcd != m_modemStatus.dcd)
		{
			m_modemStatusDelta.dcd = true;
		}

		if (!m_lastModemStatus.ri && m_modemStatus.ri) // 0->1 transition
		{
			m_modemStatusDelta.ri = true;
		}

		if (m_lastModemStatus.dsr != m_modemStatus.dsr)
		{
			m_modemStatusDelta.dsr = true;
		}

		if (m_lastModemStatus.cts != m_modemStatus.cts)
		{
			m_modemStatusDelta.cts = true;
		}

		if (m_modemStatusDelta.cts || m_modemStatusDelta.dcd || m_modemStatusDelta.dsr || m_modemStatusDelta.ri)
		{
			m_intr.modemStatus = true;
		}
	}

	void Device8250::SetCTS(bool set)
	{
		LogPrintf(LOG_INFO, "SetCTS, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.cts = set;
		}
	}
	void Device8250::SetDSR(bool set)
	{
		LogPrintf(LOG_INFO, "SetDSR, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.dsr = set;
		}
	}
	void Device8250::SetDCD(bool set)
	{
		LogPrintf(LOG_INFO, "SetDCD, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.dcd = set;
		}
	}
	void Device8250::SetRI(bool set)
	{
		LogPrintf(LOG_INFO, "SetRI, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.ri = set;
		}
	}

	bool Device8250::IsInterrupt() const
	{
		return (m_intr.modemStatus && m_interruptEnable.statusChange) ||
			(m_intr.rxDataAvailable && m_interruptEnable.dataAvailableInterrupt) ||
			(m_intr.rxLineStatus && m_interruptEnable.errorOrBreak) ||
			(m_intr.txHoldingRegisterEmpty && m_interruptEnable.txEmpty);
	}
}
