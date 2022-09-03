#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/PortConnector.h>
#include <Serializable.h>

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace rtc
{
	class Device146818 : public PortConnector, public emul::Serializable
	{
	public:
		Device146818(WORD baseAddress);
		~Device146818() {}

		Device146818() = delete;
		Device146818(const Device146818&) = delete;
		Device146818& operator=(const Device146818&) = delete;
		Device146818(Device146818&&) = delete;
		Device146818& operator=(Device146818&&) = delete;

		void Init();
		void Reset();

		// emul::Serializable
		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

	protected:
		const WORD m_baseAddress;

		struct tm m_now;
		void UpdateCurrentTime();

		BYTE ReadCurrentRegister() const;
		void WriteCurrentRegister(BYTE value);
		const char* GetCurrentRegisterName() const;

		enum class Address {
			RTC_SECONDS = 0,
			RTC_SECONDS_ALARM,
			RTC_MINUTES,
			RTC_MINUTES_ALARM,
			RTC_HOURS,
			RTC_HOURS_ALARM,
			RTC_DAY_OF_WEEK,
			RTC_DATE_OF_MONTH,
			RTC_MONTH,
			RTC_YEAR,

			RTC_REG_A,
			RTC_REG_B,
			RTC_REG_C,
			RTC_REG_D,

			RTC_NVRAM_START = 14,
			RTC_NVRAM_END = 63,

			_RTC_FIRST_NAMED = RTC_SECONDS,
			_RTC_LAST_NAMED = RTC_REG_D,

			_RTC_MAX = RTC_NVRAM_END,
			RTC_INVALID = 0xFF
		} m_currAddress = Address::RTC_INVALID;

		void WriteAddress(BYTE value);
		BYTE ReadData();
		void WriteData(BYTE value);

		void WriteRegisterA(BYTE value);

		struct RegisterB
		{
			bool setClock = false; // Set Clock Mode - 1: clock stopped to set values, 0: normal mode
			bool periodicInterruptEn = false; // PIE - 1: Periodic Flag (PF) triggers interrupt
			bool alarmInterruptEn = false; // AIE - Interrupt when alarm values match current time
			bool updateEndedInterruptEn = false; // UIE - Interrupt on Update Flag (UF)
			bool squareWave = false; // SQWE - Square Wave on SQW pin
			enum class DataMode { BCD = 0, BIN } dataMode = DataMode::BCD; // DM - BIN or BCD mode
			bool time24hMode = false; // 24/12 - 1: 24 hours mode, 0: 12 hours
			bool daylightSavings = false; // DSE - 1: enabled

			void FromByte(BYTE value);
			BYTE ToByte() const;
			void Reset();
		} m_registerB;
		void WriteRegisterB(BYTE value);

		BYTE ReadRegisterC();
		BYTE ReadRegisterD();

		std::array<BYTE, (int)Address::_RTC_MAX> m_data = {};
	};
}
