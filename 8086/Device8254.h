#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace pit
{
	static size_t s_clockSpeed = 1000000;

	enum class RWMode { RW_LSB, RW_MSB, RW_LSBMSB };
	enum class CounterMode { Mode0, Mode1, Mode2, Mode3, Mode4, Mode5 };

	class Counter : public Logger
	{
	public:
		Counter(const char* label);

		Counter() = delete;
		Counter(const Counter&) = delete;
		Counter& operator=(const Counter&) = delete;
		Counter(Counter&&) = delete;
		Counter& operator=(Counter&&) = delete;

		void Tick();

		BYTE Get();
		void Set(BYTE);

		void SetGate(bool gate) { m_gate = gate; }

		void LatchValue();

		void SetRWMode(RWMode rw);
		void SetMode(CounterMode rw);
		void SetBCD(bool bcd);

	protected:
		WORD GetMaxValue()
		{
			return m_rwMode == RWMode::RW_LSB ? 255 : 65535;
		}

		void SetMSB(BYTE value);
		void SetLSB(BYTE value);

		RWMode m_rwMode;
		CounterMode m_mode;
		bool m_bcd;

		bool m_gate;
		bool m_out;
		bool m_run;
		bool m_newValue;
		bool m_flipFlopLSBMSB;
		WORD m_n;
		WORD m_value;

		bool m_latched;
		WORD m_latchedValue;
	};

	class Device8254 : public PortConnector
	{
	public:
		Device8254(WORD baseAddress, size_t clockSpeedHz = 1000000);

		Device8254() = delete;
		Device8254(const Device8254&) = delete;
		Device8254& operator=(const Device8254&) = delete;
		Device8254(Device8254&&) = delete;
		Device8254& operator=(Device8254&&) = delete;

		virtual void EnableLog(bool enable, SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		BYTE T0_IN();
		void T0_OUT(BYTE value);

		BYTE T1_IN();
		void T1_OUT(BYTE value);

		BYTE T2_IN();
		void T2_OUT(BYTE value);

		void CONTROL_OUT(BYTE value);

	protected:
		enum CTRL {
			CTRL_SC1 = 128,
			CTRL_SC0 = 64,
			CTRL_RW1 = 32,
			CTRL_RW0 = 16,
			CTRL_M2 = 8,
			CTRL_M1 = 4,
			CTRL_M0 = 2,
			CTRL_BCD = 1,
		};

		const WORD m_baseAddress;

		Counter m_counter0;
		Counter m_counter1;
		Counter m_counter2;
	};
}
