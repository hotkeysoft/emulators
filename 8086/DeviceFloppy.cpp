#include "DeviceFloppy.h"

namespace fdc
{
	DeviceFloppy::DeviceFloppy(WORD baseAddress) :
		Logger("fdc"),
		m_baseAddress(baseAddress),
		m_motor3(false),
		m_motor2(false),
		m_motor1(false),
		m_motor0(false),
		m_enableIRQDMA(false),
		m_driveSel(0)
	{
		Reset();
	}

	void DeviceFloppy::Reset()
	{
	}

	void DeviceFloppy::Init()
	{
		// STATUS_REGISTER_A = 0x3F0, // read-only
		// STATUS_REGISTER_B = 0x3F1, // read-only
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadStatusRegA));
		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadStatusRegB));

		// DIGITAL_OUTPUT_REGISTER = 0x3F2,
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataOutputReg));

		// TAPE_DRIVE_REGISTER = 0x3F3,
		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadTapeReg));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteTapeReg));

		//MAIN_STATUS_REGISTER = 0x3F4, // read-only
		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadMainStatusReg));
		//DATARATE_SELECT_REGISTER = 0x3F4, // write-only
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataRateSelectReg));

		//DATA_FIFO = 0x3F5,
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadDataFIFO));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteDataFIFO));
		
		//DIGITAL_INPUT_REGISTER = 0x3F7, // read-only
		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&DeviceFloppy::ReadDigitalInputReg));
		//CONFIGURATION_CONTROL_REGISTER = 0x3F7  // write-only
		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&DeviceFloppy::WriteConfigControlReg));
	}

	BYTE DeviceFloppy::ReadStatusRegA()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegB");
		return 0;
	}

	BYTE DeviceFloppy::ReadStatusRegB()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegB");
		return 0;
	}

	void DeviceFloppy::WriteDataOutputReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataOutputReg, value=%02X", value);

		m_motor0 = (value & DOR::MOTA);
		m_motor1 = (value & DOR::MOTB);
		m_motor2 = (value & DOR::MOTC);
		m_motor3 = (value & DOR::MOTD);

		LogPrintf(LOG_INFO, "Drive 0 Motor: %s", m_motor0 ? "ON" : "OFF");
		LogPrintf(LOG_INFO, "Drive 1 Motor: %s", m_motor1 ? "ON" : "OFF");
		LogPrintf(LOG_INFO, "Drive 2 Motor: %s", m_motor2 ? "ON" : "OFF");
		LogPrintf(LOG_INFO, "Drive 3 Motor: %s", m_motor3 ? "ON" : "OFF");

		m_enableIRQDMA = (value & DOR::IRQ);
		LogPrintf(LOG_INFO, "IRQ/DMA: %s", m_motor0 ? "Enabled" : "Disabled");

		bool reset = (value & DOR::RESET);
		LogPrintf(LOG_INFO, "RESET: %s", reset ? "Normal" : "Clear");

		switch (value & (DOR::DSEL1 | DOR::DSEL0))
		{
		case 0: m_driveSel = 0;
		case DOR::DSEL0: m_driveSel = 1;
		case DOR::DSEL1: m_driveSel = 2;
		case DOR::DSEL1 | DOR::DSEL0: m_driveSel = 3; break;
		default:
			throw std::exception("not possible");
		}

		LogPrintf(LOG_INFO, "Drive Select: %d", m_driveSel);

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

	BYTE DeviceFloppy::ReadMainStatusReg()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadMainStatusReg");
		return 0;
	}

	void DeviceFloppy::WriteDataRateSelectReg(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataRateSelectReg, value=%02X", value);
	}

	BYTE DeviceFloppy::ReadDataFIFO()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadDataFIFO");
		return 0;
	}
	void DeviceFloppy::WriteDataFIFO(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteDataFIFO, value=%02X", value);
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

	void DeviceFloppy::Tick()
	{
	}
}
