#include "stdafx.h"

#include "Device146818.h"

using emul::GetBit;

namespace rtc
{
	static BYTE Dec2BCD(BYTE dec, BYTE clamp = 59)
	{
		dec = std::min(clamp, dec);
		return ((dec / 10) << 4) | (dec % 10);
	}

	Device146818::Device146818(WORD baseAddress) : 
		Logger("rtc"), 
		m_baseAddress(baseAddress)
	{
	}

	void Device146818::RegisterB::Reset()
	{
		periodicInterruptEn = false;
		alarmInterruptEn = false;
		updateEndedInterruptEn = false;
		squareWave = false;
	}

	void Device146818::RegisterB::FromByte(BYTE value)
	{
		setClock = GetBit(value, 7);
		periodicInterruptEn = GetBit(value, 6);
		alarmInterruptEn = GetBit(value, 5);
		updateEndedInterruptEn = GetBit(value, 4);
		squareWave = GetBit(value, 3);
		dataMode = (RegisterB::DataMode)GetBit(value, 2);
		time24hMode = GetBit(value, 1);
		daylightSavings = GetBit(value, 0);
	}

	BYTE Device146818::RegisterB::ToByte() const
	{
		return
			(setClock << 7) |
			(periodicInterruptEn << 6) |
			(alarmInterruptEn << 5) |
			(updateEndedInterruptEn << 4) |
			(squareWave << 3) |
			((int)dataMode << 2) |
			(time24hMode << 1) |
			(daylightSavings << 0);
	}

	void Device146818::Reset()
	{
		m_currAddress = Address::RTC_INVALID;

		m_registerB.Reset();
		// Synchronize with raw register value
		m_data[(int)Address::RTC_REG_B] = m_registerB.ToByte();
	}

	void Device146818::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device146818::WriteAddress));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device146818::ReadData));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device146818::WriteData));

		// Temp manual initialization

		//// No HDD, VGA
		//// 
		//// Floppy disks
		//m_data[16] = 68; // 2 x 1.44
		//// Hard disks
		//m_data[18] = 0; // None
		//// Display / coprocessor / floppy
		//m_data[20] = 65;
		//// Base memory
		//m_data[21] = 128;
		//m_data[22] = 2;
		//// Expansion memory
		//m_data[23] = 0;
		//m_data[24] = 0;
		//// Checksum
		//m_data[46] = 1;
		//m_data[47] = 7;

		// No HDD, VGA
		// 
		// Floppy disks
		m_data[16] = 68; // 2 x 1.44
		// Hard disks
		m_data[18] = 0; // None
		// Display / coprocessor / floppy
		m_data[20] = 65;
		// Base memory
		m_data[21] = 128;
		m_data[22] = 2;
		// Expansion memory
		m_data[23] = 0;
		m_data[24] = 4;
		// Checksum
		m_data[46] = 1;
		m_data[47] = 11;

	}

	const char* Device146818::GetCurrentRegisterName() const
	{
		if (m_currAddress < Address::_RTC_FIRST_NAMED ||
			m_currAddress >= Address::_RTC_LAST_NAMED)
		{
			return "";
		}

		static const char* names[(int)Address::_RTC_LAST_NAMED + 1] = {
			"RTC_SECONDS",
			"RTC_SECONDS_ALARM",
			"RTC_MINUTES",
			"RTC_MINUTES_ALARM",
			"RTC_HOURS",
			"RTC_HOURS_ALARM",
			"RTC_DAY_OF_WEEK",
			"RTC_DATE_OF_MONTH",
			"RTC_MONTH",
			"RTC_YEAR",
			"RTC_REG_A",
			"RTC_REG_B",
			"RTC_REG_C",
			"RTC_REG_D",
		};
		return names[(int)m_currAddress];
	}

	void Device146818::UpdateCurrentTime()
	{
		LogPrintf(LOG_INFO, "Update Current Time");
		time_t now = time(nullptr);
		m_now = *localtime(&now);
	}

	BYTE Device146818::ReadCurrentRegister() const
	{
		if (m_currAddress == Address::RTC_INVALID)
		{
			throw std::exception("Invalid address");
		}

		return m_data[(int)m_currAddress];
	}

	void Device146818::WriteCurrentRegister(BYTE value)
	{
		if (m_currAddress == Address::RTC_INVALID)
		{
			throw std::exception("Invalid address");
		}

		m_data[(int)m_currAddress] = value;
	}

	void Device146818::WriteAddress(BYTE value)
	{
		value &= 63;
		LogPrintf(LOG_TRACE, "WriteAddress, value=%02x", value);
		m_currAddress = (Address)value;
	}

	BYTE Device146818::ReadData()
	{
		LogPrintf(LOG_TRACE, "ReadData");

		if ((m_currAddress >= Address::RTC_NVRAM_START) && (m_currAddress <= Address::RTC_NVRAM_END))
		{
			BYTE value = ReadCurrentRegister();
			LogPrintf(LOG_DEBUG, "ReadData[%02x] = %02x", m_currAddress, value);
			return value;
		}
		else switch (m_currAddress)
		{
		case Address::RTC_SECONDS:
			LogPrintf(LOG_DEBUG, "Read Seconds");
			break;
		case Address::RTC_MINUTES:
			LogPrintf(LOG_DEBUG, "Read Minutes");
			break;
		case Address::RTC_HOURS:
			LogPrintf(LOG_DEBUG, "Read Hours");
			break;
		case Address::RTC_DAY_OF_WEEK:
			LogPrintf(LOG_DEBUG, "Read Day of week");
			break;
		case Address::RTC_DATE_OF_MONTH:
			LogPrintf(LOG_DEBUG, "Read Date of month");
			break;
		case Address::RTC_MONTH:
			LogPrintf(LOG_DEBUG, "Read Month");
			break;
		case Address::RTC_YEAR:
			LogPrintf(LOG_DEBUG, "Read Year");
			break;

		case Address::RTC_SECONDS_ALARM:
		case Address::RTC_MINUTES_ALARM:
		case Address::RTC_HOURS_ALARM:
		case Address::RTC_REG_A:
		case Address::RTC_REG_B:
		{
			BYTE value = ReadCurrentRegister();
			LogPrintf(LOG_DEBUG, "ReadData[%s] = %02x", GetCurrentRegisterName(), value);
			return value;
		}

		case Address::RTC_REG_C: return ReadRegisterC();
		case Address::RTC_REG_D: return ReadRegisterD();

		default:
			LogPrintf(LOG_WARNING, "Invalid Address");
		}

		return 0xFF;
	}

	void Device146818::WriteData(BYTE value)
	{
		LogPrintf(LOG_TRACE, "WriteData, value=%02x", value);

		if ((m_currAddress >= Address::RTC_NVRAM_START) && (m_currAddress <= Address::RTC_NVRAM_END))
		{
			LogPrintf(LOG_DEBUG, "WriteData[%02x] = %02x", m_currAddress, value);
			WriteCurrentRegister(value);
		}
		else switch (m_currAddress)
		{
		case Address::RTC_SECONDS:
			LogPrintf(LOG_DEBUG, "Write Seconds");
			break;
		case Address::RTC_MINUTES:
			LogPrintf(LOG_DEBUG, "Write Minutes");
			break;
		case Address::RTC_HOURS:
			LogPrintf(LOG_DEBUG, "Write Hours");
			break;
		case Address::RTC_DAY_OF_WEEK:
			LogPrintf(LOG_DEBUG, "Write Day of week");
			break;
		case Address::RTC_DATE_OF_MONTH:
			LogPrintf(LOG_DEBUG, "Write Date of month");
			break;
		case Address::RTC_MONTH:
			LogPrintf(LOG_DEBUG, "Write Month");
			break;
		case Address::RTC_YEAR:
			LogPrintf(LOG_DEBUG, "Write Year");
			break;

		case Address::RTC_SECONDS_ALARM:
		case Address::RTC_MINUTES_ALARM:
		case Address::RTC_HOURS_ALARM:
			LogPrintf(LOG_DEBUG, "WriteData[%s] = %02x", GetCurrentRegisterName(), value);
			WriteCurrentRegister(value);
			break;
		case Address::RTC_REG_A:
			WriteRegisterA(value);
			break;
		case Address::RTC_REG_B:
			WriteRegisterB(value);
			break;

		case Address::RTC_REG_C:
		case Address::RTC_REG_D:
		default:
			LogPrintf(LOG_WARNING, "Invalid Address or read-only register");
		}
	}

	void Device146818::WriteRegisterA(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteRegisterA");
	}

	void Device146818::WriteRegisterB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteRegisterB, value=%02x", value);
		
		// Keep raw data and exploded register in sync
		WriteCurrentRegister(value);
		m_registerB.FromByte(value);

		LogPrintf(Logger::LOG_INFO, "WriteRegisterB: [%cSET %cPIE %cAIE %cUIE %cSQWE DM[%s] %c24H %cDSE] ",
			m_registerB.setClock ? ' ' : '/',
			m_registerB.periodicInterruptEn ? ' ' : '/',
			m_registerB.alarmInterruptEn ? ' ' : '/',
			m_registerB.updateEndedInterruptEn ? ' ' : '/',
			m_registerB.squareWave ? ' ' : '/',
			(m_registerB.dataMode == RegisterB::DataMode::BCD) ? "BCD" : "BIN",
			m_registerB.time24hMode ? ' ' : '/',
			m_registerB.daylightSavings ? ' ' : '/');
	}

	BYTE Device146818::ReadRegisterC()
	{
		LogPrintf(LOG_INFO, "ReadRegisterC");
		return 0;
	}
	BYTE Device146818::ReadRegisterD()
	{
		// Bit0-6: Reserved(0)
		// Bit7: 1 = Valid RAM and Time
		BYTE value = (1 << 7);

		LogPrintf(LOG_INFO, "ReadRegisterD, value=%02x [VRT]", value);
		return value;
	}

	void Device146818::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["currAddress"] = m_currAddress;

		to["data"] = m_data;

		m_registerB.FromByte(m_data[(int)Address::RTC_REG_B]);
	}
	void Device146818::Deserialize(const json& from)
	{
		WORD baseAddress = from["baseAddress"];
		if (baseAddress != m_baseAddress)
		{
			throw emul::SerializableException("Device146818: Incompatible baseAddress");
		}
		m_currAddress = from["currAddress"];

		m_data = from["data"];
	}
}
