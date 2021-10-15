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
		m_lineControl.wordLengthSelect = 0;
		m_lineControl.stopBits = false;
		m_lineControl.parityEnable = false;
		m_lineControl.parityEven = false;
		m_lineControl.parityStick = false;
		m_lineControl.setBreak = false;
		m_lineControl.divisorLatchAccess = false;

		m_reg.interruptEnable = 0;
		m_reg.modemControl = 0;
		m_reg.lineStatus = 0b01100000;
		m_reg.modemStatus = 0;
		m_reg.scratch = 0;

		m_io.out1 = false;
		m_io.out2 = false;
		m_io.rts = false;
		m_io.dtr = false;
		m_io.serialOut = true;

		m_intr.receiverError = false;
		m_intr.receiverDataAvailable = false;
		m_intr.transmitterEmpty = false;
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
		Connect(m_baseAddress + 6, static_cast<PortConnector::INFunction>(&Device8250::ReadModemStatus));

		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&Device8250::ReadScratch));
		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&Device8250::WriteScratch));
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
		// Reset interrupt
		m_intr.receiverDataAvailable = false;

		LogPrintf(LOG_INFO, "ReadReceiver, value=%02x", 0xFF);
		return 0xFF;
	}
	void Device8250::WriteTransmitter(BYTE value)
	{
		// Reset interrupt
		m_intr.transmitterEmpty = false;

		LogPrintf(LOG_INFO, "WriteTransmitter, value=%02x", value);
	}

	BYTE Device8250::ReadInterruptEnable()
	{
		LogPrintf(LOG_INFO, "ReadInterruptEnable, value=%02x", 0x00);
		return 0x00;
	}
	void Device8250::WriteInterruptEnable(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteInterruptEnable, value=%02x", value);
	}

	BYTE Device8250::ReadInterruptIdentification()
	{
		BYTE ret;
		// In priority order
		if (m_intr.receiverError)
		{
			LogPrintf(LOG_INFO, "ReadInterruptIdentification: Receiver Error");
			ret = 0b110;
		} 
		else if (m_intr.receiverDataAvailable)
		{
			LogPrintf(LOG_INFO, "ReadInterruptIdentification: Receiver Data Available");
			ret = 0b100;
		}
		else if (m_intr.transmitterEmpty)
		{
			// Reset interrupt 
			m_intr.transmitterEmpty = false;

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
			LogPrintf(LOG_INFO, "ReadInterruptIdentification: None");
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

		LogPrintf(LOG_DEBUG, "ReadLineControl, value=%02x", 0xFF);

		return 0xFF;
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
		LogPrintf(LOG_INFO, "ReadModemControl, value=%02x", m_reg.modemControl);
		return m_reg.modemControl;
	}
	void Device8250::WriteModemControl(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteModemControl, value=%02x", value);
		m_reg.modemControl = value;
	}

	BYTE Device8250::ReadLineStatus()
	{
		// Reset interrupt
		m_intr.receiverError = false;

		LogPrintf(LOG_INFO, "ReadLineStatus, value=%02x", m_reg.lineStatus);

		return m_reg.lineStatus;
	}

	BYTE Device8250::ReadModemStatus()
	{
		// Reset interrupt
		m_intr.modemStatus = false;
		
		LogPrintf(LOG_INFO, "ReadModemStatus, value=%02x", m_reg.modemStatus);
		return m_reg.modemStatus;
	}

	BYTE Device8250::ReadScratch()
	{
		LogPrintf(LOG_INFO, "ReadScratch, value=%02x", m_reg.scratch);
		return m_reg.scratch;
	}
	void Device8250::WriteScratch(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteScratch, value=%02x", value);
		m_reg.scratch = value;
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
	}
}
