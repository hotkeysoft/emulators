#pragma once

#include "../Serializable.h"
#include "../CPU/PortConnector.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

namespace pit
{
	static size_t s_clockSpeed = 1000000;

	enum class RWMode { RW_LSB, RW_MSB, RW_LSBMSB };
	enum class CounterMode { Mode0, Mode1, Mode2, Mode3, Mode4, Mode5 };

	class Device8254;

	class Counter : public PortConnector, public emul::Serializable
	{
	public:
		Counter(Device8254* parent, BYTE id, const char* label);

		Counter() = delete;
		Counter(const Counter&) = delete;
		Counter& operator=(const Counter&) = delete;
		Counter(Counter&&) = delete;
		Counter& operator=(Counter&&) = delete;

		void Init();

		void Tick();

		float GetPeriodMicro() const { return m_periodMicro; };

		bool GetOutput() const { return m_out; }
		bool GetGate() const { return m_gate; }
		void SetGate(bool gate) { m_gate = gate; }

		void LatchValue();

		void SetRWMode(RWMode rw);
		void SetMode(CounterMode rw);
		void SetBCD(bool bcd);

		virtual void Serialize(json& to);
		virtual void Deserialize(json& from);

	protected:
		Device8254* m_parent = nullptr;
		BYTE m_id = 0;

		BYTE ReadData();
		void WriteData(BYTE value);

		size_t GetMaxValue() const
		{
			return m_n ? m_n : (m_rwMode == RWMode::RW_LSB ? 256 : 65536);
		}

		RWMode m_rwMode = RWMode::RW_LSB;
		CounterMode m_mode = CounterMode::Mode0;
		bool m_bcd = false;

		bool m_gate = true;
		bool m_lastGate = true;
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

	class Device8254 : public PortConnector, public emul::Serializable
	{
	public:
		Device8254(WORD baseAddress, size_t clockSpeedHz = 1000000);

		Device8254() = delete;
		Device8254(const Device8254&) = delete;
		Device8254& operator=(const Device8254&) = delete;
		Device8254(Device8254&&) = delete;
		Device8254& operator=(Device8254&&) = delete;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO);

		void Init();
		void Reset();

		void Tick();

		Counter& GetCounter(size_t counter);

		WORD GetBaseAdress() const { return m_baseAddress; }

		virtual void Serialize(json& to);
		virtual void Deserialize(json& from);

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

		void WriteControl(BYTE value);

		Counter m_counters[3];
	};
}
