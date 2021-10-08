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

		float GetPeriodMicro() const { return m_periodMicro; };

		bool GetOutput() const { return m_out; }
		bool GetGate() const { return m_gate; }
		void SetGate(bool gate) { m_gate = gate; }

		void LatchValue();
		BYTE Get();
		void Set(BYTE);

		void SetRWMode(RWMode rw);
		void SetMode(CounterMode rw);
		void SetBCD(bool bcd);

	protected:
		WORD GetMaxValue() const
		{
			return m_n ? m_n : (m_rwMode == RWMode::RW_LSB ? 255 : 65535);
		}

		RWMode m_rwMode = RWMode::RW_LSB;
		CounterMode m_mode = CounterMode::Mode0;
		bool m_bcd = false;

		bool m_gate = false;
		bool m_out = false;
		bool m_run = false;
		bool m_newValue = false;
		bool m_flipFlopLSBMSB = false;
		WORD m_n = 0;
		WORD m_value = 0;

		bool m_latched = false;
		WORD m_latchedValue = 0;

		float m_periodMicro = 0.0f;
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

		Counter& GetCounter(size_t counter);

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
