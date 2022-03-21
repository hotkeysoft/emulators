#include "stdafx.h"

#include "Device8250.h"

using emul::GetHByte;
using emul::GetLByte;
using emul::SetHByte;
using emul::SetLByte;
using emul::GetBit;

using emul::SerializableException;
using emul::SerializationError;

namespace uart
{
	Device8250::Device8250(WORD baseAddress, BYTE irq, size_t clockSpeedHz) : 
		Logger("UART8250"), 
		m_baseAddress(baseAddress),
		m_irq(irq)
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
		LogPrintf(LOG_INFO, "Baud rate: [%d]", GetBaudRate());
	}

	BYTE Device8250::ReadDivisorLatchMSB()
	{
		LogPrintf(LOG_DEBUG, "ReadDivisorLatchMSB, value=%02x", GetHByte(m_divisorLatch));
		return GetHByte(m_divisorLatch);
	}
	void Device8250::WriteDivisorLatchMSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteDivisorLatchMSB, value=%02x", value);
		SetHByte(m_divisorLatch, value);
		LogPrintf(LOG_INFO, "Baud rate: [%d]", GetBaudRate());
	}

	BYTE Device8250::ReadReceiver()
	{
		m_intr.rxDataAvailable = false;
		m_lineStatus.dataReady = false;

		LogPrintf(LOG_TRACE, "ReadReceiver, value=%02x", m_rxBufferRegister);
		return m_rxBufferRegister;
	}
	void Device8250::WriteTransmitter(BYTE value)
	{
		m_intr.txHoldingRegisterEmpty = false;
		m_lineStatus.txHoldingRegisterEmpty = false;
		m_txHoldingRegister = value;

		LogPrintf(LOG_TRACE, "WriteTransmitter, value=%02x", value);

		m_transmitDelay = 100000;
	}

	BYTE Device8250::ReadInterruptEnable()
	{
		BYTE ret =
			(m_interruptEnable.statusChange << 3) |
			(m_interruptEnable.errorOrBreak << 2) |
			(m_interruptEnable.txEmpty << 1) |
			(m_interruptEnable.dataAvailableInterrupt << 0);

		LogPrintf(LOG_TRACE, "ReadInterruptEnable, value=%02x", ret);
		return ret;
	}
	void Device8250::WriteInterruptEnable(BYTE value)
	{
		LogPrintf(LOG_TRACE, "WriteInterruptEnable, value=%02x", value);

		m_interruptEnable.statusChange = GetBit(value, 3);
		m_interruptEnable.errorOrBreak = GetBit(value, 2);
		m_interruptEnable.txEmpty = GetBit(value, 1);
		m_interruptEnable.dataAvailableInterrupt = GetBit(value, 0);

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
			LogPrintf(LOG_TRACE, "ReadInterruptIdentification: Receiver Error");
			ret = 0b110;
		} 
		else if (m_intr.rxDataAvailable)
		{
			LogPrintf(LOG_TRACE, "ReadInterruptIdentification: Receiver Data Available");
			ret = 0b100;
		}
		else if (m_intr.txHoldingRegisterEmpty)
		{
			// Reset interrupt 
			m_intr.txHoldingRegisterEmpty = false;

			LogPrintf(LOG_TRACE, "ReadInterruptIdentification: Transmitter Holding Register Empty");
			ret = 0b010;
		}
		else if (m_intr.modemStatus)
		{
			LogPrintf(LOG_TRACE, "ReadInterruptIdentification: MODEM status (CTS | DSR | RI | DCD)");
			ret = 0b000;
		}
		else // no interrupt
		{
			//LogPrintf(LOG_TRACE, "ReadInterruptIdentification: None");
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
		LogPrintf(LOG_TRACE, "WriteLineControl, value=%02x", value);

		m_lineControl.divisorLatchAccess = GetBit(value, 7);
		m_lineControl.setBreak = GetBit(value, 6);
		LogPrintf(LOG_DEBUG, "WriteLineControl: [%cDLAB %cBREAK]",
			m_lineControl.divisorLatchAccess ? ' ' : '/',
			m_lineControl.setBreak ? ' ' : '/');

		m_lineControl.parityStick = GetBit(value, 5);
		m_lineControl.parityEven = GetBit(value, 4);
		m_lineControl.parityEnable = GetBit(value, 3);
		m_lineControl.stopBits = GetBit(value, 2);
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

		LogPrintf(LOG_TRACE, "ReadModemControl, value=%02x", ret);
		return ret;
	}
	void Device8250::WriteModemControl(BYTE value)
	{
		LogPrintf(LOG_TRACE, "WriteModemControl, value=%02x", value);

		ModemControlRegister m_old = m_modemControl;

		m_modemControl.loopback = GetBit(value, 4);
		m_modemControl.out2 = GetBit(value, 3);
		m_modemControl.out1 = GetBit(value, 2);
		m_modemControl.rts = GetBit(value, 1);
		m_modemControl.dtr = GetBit(value, 0);

		LogPrintf(LOG_DEBUG, "WriteModemControl: [%cLOOPBACK %cOUT2 %cOUT1 %cRTS %cDTR]",
			m_modemControl.loopback ? ' ' : '/',
			m_modemControl.out2 ? ' ' : '/',
			m_modemControl.out1 ? ' ' : '/',
			m_modemControl.rts ? ' ' : '/',
			m_modemControl.dtr ? ' ' : '/');

		if (m_old.out2 != m_modemControl.out2) { OnOUT2(m_modemControl.out2); }
		if (m_old.out1 != m_modemControl.out1) { OnOUT1(m_modemControl.out1); }
		if (m_old.rts != m_modemControl.rts) { OnRTS(m_modemControl.rts); }
		if (m_old.dtr != m_modemControl.dtr) { OnDTR(m_modemControl.dtr); }
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

		LogPrintf(LOG_TRACE, "ReadLineStatus, value=%02x", ret);

		LogPrintf(LOG_DEBUG, "ReadLineStatus: [%cTSRE %cTHRE %cBI %cFE %cPE %cOE %cDR]",
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

		m_lineStatus.txHoldingRegisterEmpty = GetBit(value, 6);
		m_lineStatus.breakInterrupt = GetBit(value, 5);
		m_lineStatus.framingError = GetBit(value, 4);
		m_lineStatus.parityError = GetBit(value, 3);
		m_lineStatus.overrunError = GetBit(value, 1);
		m_lineStatus.dataReady = GetBit(value, 0);

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

		const bool& loop = m_modemControl.loopback;
		
		const bool& dcd = loop ? m_modemControl.out2 : m_modemStatus.dcd;
		const bool& ri  = loop ? m_modemControl.out1 : m_modemStatus.ri;
		const bool& dsr = loop ? m_modemControl.dtr  : m_modemStatus.dsr;
		const bool& cts = loop ? m_modemControl.rts  : m_modemStatus.cts;

		BYTE ret =
			(dcd << 7) |
			(ri  << 6) |
			(dsr << 5) |
			(cts << 4) |
			(m_modemStatusDelta.dcd << 3) |
			(m_modemStatusDelta.ri  << 2) |
			(m_modemStatusDelta.dsr << 1) |
			(m_modemStatusDelta.cts << 0);

		LogPrintf(LOG_TRACE, "ReadModemStatus, value=%02x", ret);

		LogPrintf(LOG_DEBUG, "ReadModemStatus: [%cDCD %cRI %cDSR %cCTS | %cDDCD %cTERI %cDDSR %cDCTS]",
			dcd ? ' ' : '/',
			ri  ? ' ' : '/',
			dsr ? ' ' : '/',
			cts ? ' ' : '/',
			m_modemStatusDelta.dcd ? ' ' : '/',
			m_modemStatusDelta.ri  ? ' ' : '/',
			m_modemStatusDelta.dsr ? ' ' : '/',
			m_modemStatusDelta.cts ? ' ' : '/');

		// Save current state for next deltas
		m_lastModemStatus.dcd = dcd;
		m_lastModemStatus.ri  = ri;
		m_lastModemStatus.dsr = dsr;
		m_lastModemStatus.cts = cts;

		// Reset Deltas
		m_modemStatusDelta.dcd = false;
		m_modemStatusDelta.ri  = false;
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
		LogPrintf(LOG_TRACE, "WriteModemStatus, value=%02x", value);

		m_modemStatusDelta.dcd = GetBit(value, 3);
		m_modemStatusDelta.ri  = GetBit(value, 2);
		m_modemStatusDelta.dsr = GetBit(value, 1);
		m_modemStatusDelta.cts = GetBit(value, 0);

		LogPrintf(LOG_DEBUG, "WriteModemStatus: [%cDCD %cRI %cDSR %cCTS | %cDDCD %cTERI %cDDSR %cDCTS]",
			m_modemStatus.dcd ? ' ' : '/',
			m_modemStatus.ri  ? ' ' : '/',
			m_modemStatus.dsr ? ' ' : '/',
			m_modemStatus.cts ? ' ' : '/',
			m_modemStatusDelta.dcd ? ' ' : '/',
			m_modemStatusDelta.ri  ? ' ' : '/',
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

	void Device8250::SetCTS(bool set)
	{
		LogPrintf(LOG_DEBUG, "SetCTS, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.cts = set;
		}
	}
	void Device8250::SetDSR(bool set)
	{
		LogPrintf(LOG_DEBUG, "SetDSR, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.dsr = set;
		}
	}
	void Device8250::SetDCD(bool set)
	{
		LogPrintf(LOG_DEBUG, "SetDCD, value=%d", set);

		if (!m_modemControl.loopback)
		{
			m_modemStatus.dcd = set;
		}
	}
	void Device8250::SetRI(bool set)
	{
		LogPrintf(LOG_DEBUG, "SetRI, value=%d", set);

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

	void Device8250::InputData(BYTE value)
	{
		m_rxBufferRegister = value;
		m_intr.rxDataAvailable = true;
		m_lineStatus.dataReady = true;
	}

	void Device8250::Tick()
	{
		// Modem status line changes

		const bool& loop = m_modemControl.loopback;

		const bool& dcd = loop ? m_modemControl.out2 : m_modemStatus.dcd;
		const bool& ri  = loop ? m_modemControl.out1 : m_modemStatus.ri;
		const bool& dsr = loop ? m_modemControl.dtr  : m_modemStatus.dsr;
		const bool& cts = loop ? m_modemControl.rts  : m_modemStatus.cts;

		if (m_lastModemStatus.dcd != dcd) m_modemStatusDelta.dcd = true;
		if (m_lastModemStatus.ri && !ri)  m_modemStatusDelta.ri  = true; // 1->0 transition
		if (m_lastModemStatus.dsr != dsr) m_modemStatusDelta.dsr = true;
		if (m_lastModemStatus.cts != cts) m_modemStatusDelta.cts = true;

		if (m_modemStatusDelta.cts || m_modemStatusDelta.dcd || m_modemStatusDelta.dsr || m_modemStatusDelta.ri)
		{
			m_intr.modemStatus = true;
		}

		if (!m_lineStatus.txHoldingRegisterEmpty)
		{
			// Shortcut: connect tx and rx in loopback mode
			if (m_modemControl.loopback)
			{
				m_rxBufferRegister = m_txHoldingRegister;
				m_lineStatus.dataReady = true;
				m_intr.rxDataAvailable = true;

				m_lineStatus.txHoldingRegisterEmpty = true;
				m_intr.txHoldingRegisterEmpty = true;
			}
			else
			{
				// Data transmission delay
				if (--m_transmitDelay == 0)
				{
					m_lineStatus.txHoldingRegisterEmpty = true;
					m_intr.txHoldingRegisterEmpty = true;
					OnDataTransmit(m_txHoldingRegister);
				}
			}
		}
	}

	void Device8250::DataConfig::Serialize(json& to)
	{
		to["dataLength"] = dataLength;
		to["parity"] = parity;
		to["stopBits"] = stopBits;
	}
	void Device8250::InterruptEnableRegister::Serialize(json& to)
	{
		to["dataAvailableInterrupt"] = dataAvailableInterrupt;
		to["txEmpty"] = txEmpty;
		to["errorOrBreak"] = errorOrBreak;
		to["statusChange"] = statusChange;
	}
	void Device8250::LineControlRegister::Serialize(json& to)
	{
		to["wordLengthSelect"] = wordLengthSelect;
		to["stopBits"] = stopBits;
		to["parityEnable"] = parityEnable;
		to["parityEven"] = parityEven;
		to["parityStick"] = parityStick;
		to["setBreak"] = setBreak;
		to["divisorLatchAccess"] = divisorLatchAccess;
	}
	void Device8250::ModemControlRegister::Serialize(json& to)
	{
		to["dtr"] = dtr;
		to["rts"] = rts;
		to["out1"] = out1;
		to["out2"] = out2;
		to["loopback"] = loopback;
	}
	void Device8250::LineStatusRegister::Serialize(json& to)
	{
		to["dataReady"] = dataReady;
		to["overrunError"] = overrunError;
		to["parityError"] = parityError;
		to["framingError"] = framingError;
		to["breakInterrupt"] = breakInterrupt;
		to["txHoldingRegisterEmpty"] = txHoldingRegisterEmpty;
		to["txShiftRegisterEmpty"] = txShiftRegisterEmpty;
	}
	void Device8250::ModemStatusRegister::Serialize(json& to)
	{
		to["cts"] = cts;
		to["dsr"] = dsr;
		to["ri"] = ri;
		to["dcd"] = dcd;
	}
	void Device8250::INTR::Serialize(json& to)
	{
		to["rxLineStatus"] = rxLineStatus;
		to["rxDataAvailable"] = rxDataAvailable;
		to["txHoldingRegisterEmpty"] = txHoldingRegisterEmpty;
		to["modemStatus"] = modemStatus;
	}
	void Device8250::Serialize(json& to)
	{
		to["clockSpeed"] = s_clockSpeed;

		to["baseAddress"] = m_baseAddress;
		to["irq"] = m_irq;

		m_dataConfig.Serialize(to["dataConfig"]);
		m_interruptEnable.Serialize(to["interruptEnable"]);
		m_lineControl.Serialize(to["lineControl"]);
		m_modemControl.Serialize(to["modemControl"]);
		m_lineStatus.Serialize(to["lineStatus"]);
		m_modemStatus.Serialize(to["modemStatus"]);
		m_lastModemStatus.Serialize(to["lastModemStatus"]);
		m_modemStatusDelta.Serialize(to["modemStatusDelta"]);
		m_intr.Serialize(to["intr"]);

		to["divisorLatch"] = m_divisorLatch;
		to["rxBufferRegister"] = m_rxBufferRegister;
		to["txHoldingRegister"] = m_txHoldingRegister;
		to["transmitDelay"] = m_transmitDelay;
		to["m_serialOut"] = m_serialOut;
	}

	void Device8250::DataConfig::Deserialize(const json& from)
	{
		dataLength = from["dataLength"];
		parity = from["parity"];
		stopBits = from["stopBits"];
	}
	void Device8250::InterruptEnableRegister::Deserialize(const json& from)
	{
		dataAvailableInterrupt = from["dataAvailableInterrupt"];
		txEmpty = from["txEmpty"];
		errorOrBreak = from["errorOrBreak"];
		statusChange = from["statusChange"];
	}
	void Device8250::LineControlRegister::Deserialize(const json& from)
	{
		wordLengthSelect = from["wordLengthSelect"];
		stopBits = from["stopBits"];
		parityEnable = from["parityEnable"];
		parityEven = from["parityEven"];
		parityStick = from["parityStick"];
		setBreak = from["setBreak"];
		divisorLatchAccess = from["divisorLatchAccess"];
	}
	void Device8250::ModemControlRegister::Deserialize(const json& from)
	{
		dtr = from["dtr"];
		rts = from["rts"];
		out1 = from["out1"];
		out2 = from["out2"];
		loopback = from["loopback"];
	}
	void Device8250::LineStatusRegister::Deserialize(const json& from)
	{
		dataReady = from["dataReady"];
		overrunError = from["overrunError"];
		parityError = from["parityError"];
		framingError = from["framingError"];
		breakInterrupt = from["breakInterrupt"];
		txHoldingRegisterEmpty = from["txHoldingRegisterEmpty"];
		txShiftRegisterEmpty = from["txShiftRegisterEmpty"];
	}
	void Device8250::ModemStatusRegister::Deserialize(const json& from)
	{
		cts = from["cts"];
		dsr = from["dsr"];
		ri = from["ri"];
		dcd = from["dcd"];
	}
	void Device8250::INTR::Deserialize(const json& from)
	{
		rxLineStatus = from["rxLineStatus"];
		rxDataAvailable = from["rxDataAvailable"];
		txHoldingRegisterEmpty = from["txHoldingRegisterEmpty"];
		modemStatus = from["modemStatus"];
	}
	void Device8250::Deserialize(const json& from)
	{
		size_t clockSpeed = from["clockSpeed"];
		if (clockSpeed != s_clockSpeed)
		{
			throw emul::SerializableException("Device8250: Incompatible clockSpeed", SerializationError::COMPAT);
		}

		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("Device8250: Incompatible baseAddress", SerializationError::COMPAT);
		}

		BYTE irq = from["irq"];
		if (irq != m_irq)
		{
			throw emul::SerializableException("Device8250: Incompatible IRQ", SerializationError::COMPAT);
		}

		m_dataConfig.Deserialize(from["dataConfig"]);
		m_interruptEnable.Deserialize(from["interruptEnable"]);
		m_lineControl.Deserialize(from["lineControl"]);
		m_modemControl.Deserialize(from["modemControl"]);
		m_lineStatus.Deserialize(from["lineStatus"]);
		m_modemStatus.Deserialize(from["modemStatus"]);
		m_lastModemStatus.Deserialize(from["lastModemStatus"]);
		m_modemStatusDelta.Deserialize(from["modemStatusDelta"]);
		m_intr.Deserialize(from["intr"]);

		m_divisorLatch = from["divisorLatch"];
		m_rxBufferRegister = from["rxBufferRegister"];
		m_txHoldingRegister = from["txHoldingRegister"];
		m_transmitDelay = from["transmitDelay"];
		m_serialOut = from["m_serialOut"];
	}

}
