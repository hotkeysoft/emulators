#include "Device8254.h"

namespace pit
{
	Counter::Counter(const char* label) : Logger(label),
		m_rwMode(RWMode::RW_LSB),
		m_mode(CounterMode::Mode0),
		m_bcd(false),
		m_gate(false),
		m_out(false),
		m_run(false),
		m_flipFlopLSBMSB(false),
		m_newValue(false),
		m_n(0),
		m_value(0),
		m_latched(false),
		m_latchedValue(0)
	{
	}

	void Counter::Tick()
	{
		if (!m_gate)
		{
			return;
		}

		if (m_newValue)
		{
			m_newValue = false;
			m_run = true;

			switch (m_mode)
			{
			case CounterMode::Mode0:
			{
				m_value = m_n;
				WORD ticks = 1 + (m_n ? m_n : GetMaxValue());
				float intervalMicro = (float)ticks * 1000000 / (float)s_clockSpeed;
				LogPrintf(LOG_INFO, "Starting Count, interval = %0.2fus", intervalMicro);
				// Start counting on next tick
				return;
			}
			case CounterMode::Mode2:
			{
				size_t ticks = m_n ? m_n : GetMaxValue();
				float intervalMicro = (float)ticks * 1000000 / (float)s_clockSpeed;
				LogPrintf(LOG_INFO, "Starting Count, period = %0.2fus", intervalMicro);
				break;
			}
			case CounterMode::Mode3:
			{
				size_t ticks = m_n ? m_n : GetMaxValue();
				float freq = (float)s_clockSpeed / (float)ticks;
				LogPrintf(LOG_INFO, "Frequency = %0.2fHz", freq);
				break;
			}
			}
		}

		if (!m_run)
		{
			return;
		}

		// Decrease counter, wrap if 0
		if (m_value == 0)
		{
			m_value = GetMaxValue();
		}
		else
		{
			--m_value;
		}

		switch (m_mode)
		{
		case CounterMode::Mode0:
			if (m_value == 0)
			{
				LogPrintf(LOG_INFO, "Count Done");
				m_run = false;
				m_out = true;
			}
			break;
		case CounterMode::Mode2:
			if (m_value == 1)
			{
				m_out = false;
			}
			else if (m_value == 0)
			{
				m_value = m_n;
				m_out = true;
			}
			break;
		default:
			break;
		}
	}

	BYTE Counter::Get()
	{
		BYTE ret;
		switch (m_rwMode)
		{
		case RWMode::RW_LSB:
			if (m_latched)
			{
				m_latched = false;
				ret = (BYTE)m_latchedValue;
				LogPrintf(LOG_DEBUG, "GetLSB(latched): %02X", ret);
			}
			else
			{
				ret = (BYTE)m_value;
				LogPrintf(LOG_DEBUG, "GetLSB(live): %02X", ret);
			}
			break;

		case RWMode::RW_MSB:
			if (m_latched)
			{
				m_latched = false;
				ret = (BYTE)(m_latchedValue >> 8);
				LogPrintf(LOG_DEBUG, "GetMSB(latched): %02X", ret);
			}
			else
			{
				ret = (BYTE)(m_value >> 8);
				LogPrintf(LOG_DEBUG, "GetMSB(live): %02X", ret);
			}
			break;

		case RWMode::RW_LSBMSB:
		default:
			throw std::exception("Get:RWMode: Not implemented");
		}
		return ret;
	}

	void Counter::Set(BYTE value)
	{
		switch (m_rwMode)
		{
		case RWMode::RW_LSBMSB:
			if (!m_flipFlopLSBMSB)
			{
				SetLSB(value);
			}
			else
			{
				SetMSB(value);
				m_newValue = true;
			}
			m_flipFlopLSBMSB = !m_flipFlopLSBMSB;
			break;
		case RWMode::RW_LSB: 
			SetLSB(value); 
			m_newValue = true;
			break;
		case RWMode::RW_MSB: 
			SetMSB(value); 
			m_newValue = true;
			break;
		default:
			throw std::exception("Set:RWMode: Not implemented");
		}
	}

	void Counter::LatchValue()
	{
		if (!m_latched)
		{
			LogPrintf(LOG_DEBUG, "Latch value: %04X", m_value);

			m_latchedValue = m_value;
			m_latched = true;
		}
	}

	void Counter::SetMSB(BYTE value)
	{
		m_n = (value<<8);
		LogPrintf(LOG_DEBUG, "Value: %04X", m_n);
	}
	void Counter::SetLSB(BYTE value)
	{
		m_n = value;
		LogPrintf(LOG_DEBUG, "Value: %04X", m_n);
	}

	void Counter::SetRWMode(RWMode rw)
	{
		m_rwMode = rw;

		switch (rw)
		{
		case RWMode::RW_LSB:
			LogPrintf(LOG_DEBUG, "SetRWMode: LSB");
			break;

		case RWMode::RW_MSB:
			LogPrintf(LOG_DEBUG, "SetRWMode: MSB");
			break;

		case RWMode::RW_LSBMSB:
			LogPrintf(LOG_DEBUG, "SetRWMode: LSBMSB");
			m_flipFlopLSBMSB = false;
			break;

		default:
			throw std::exception("SetRWMode: Not implemented");
		}
	}

	void Counter::SetMode(CounterMode mode)
	{
		m_mode = mode;

		m_newValue = false;
		m_run = false;

		switch (mode)
		{
		case CounterMode::Mode0:
			LogPrintf(LOG_INFO, "SetMode: 0 - INTERRUPT ON TERMINAL COUNT");
			m_out = false;
			break;
		case CounterMode::Mode1:
			LogPrintf(LOG_INFO, "SetMode: 1 - HARDWARE RETRIGGERABLE ONE-SHOT");
			break;
		case CounterMode::Mode2:
			LogPrintf(LOG_INFO, "SetMode: 2 - RATE GENERATOR");
			m_out = false;
			break;
		case CounterMode::Mode3:
			LogPrintf(LOG_INFO, "SetMode: 3 - SQUARE WAVE MODE");
			break;
		case CounterMode::Mode4:
			LogPrintf(LOG_INFO, "SetMode: 4 - SOFTWARE TRIGGERED MODE");
			LogPrintf(LOG_ERROR, "Mode 4 Not implemented");
			break;
		case CounterMode::Mode5:
			LogPrintf(LOG_INFO, "SetMode: 5 - HARDWARE TRIGGERED STROBE");
			LogPrintf(LOG_ERROR, "Mode 5 Not implemented");
			break;
		default:
			throw std::exception("SetMode: Not implemented");
		}
	}

	void Counter::SetBCD(bool bcd)
	{
		LogPrintf(LOG_DEBUG, "SetBCD: %d", bcd);
		m_bcd = bcd;
		if (m_bcd)
		{
			LogPrintf(LOG_ERROR, "BCD mode not implemented");
		}
	}


	Device8254::Device8254(WORD baseAddress, size_t clockSpeedHz) : 
		Logger("PIT8254"), 
		m_counter0("PIT_T0"),
		m_counter1("PIT_T1"),
		m_counter2("PIT_T2"),
		m_baseAddress(baseAddress)
	{
		Reset();
		s_clockSpeed = clockSpeedHz;
	}

	void Device8254::EnableLog(bool enable, SEVERITY minSev)
	{
		m_counter0.EnableLog(enable, minSev);
		m_counter1.EnableLog(enable, minSev);
		m_counter2.EnableLog(enable, minSev);
	}

	void Device8254::Reset()
	{

	}

	void Device8254::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8254::T0_IN));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8254::T0_OUT));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8254::T1_IN));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8254::T1_OUT));

		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&Device8254::T2_IN));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&Device8254::T2_OUT));

		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device8254::CONTROL_OUT));

		m_counter0.SetGate(true);
		m_counter1.SetGate(true);
		m_counter2.SetGate(true);
	}

	BYTE Device8254::T0_IN()
	{
		return m_counter0.Get();
	}
	void Device8254::T0_OUT(BYTE value)
	{
		m_counter0.Set(value);
	}

	BYTE Device8254::T1_IN()
	{
		return m_counter1.Get();
	}

	void Device8254::T1_OUT(BYTE value)
	{
		m_counter1.Set(value);
	}

	BYTE Device8254::T2_IN()
	{
		return m_counter2.Get();
	}
	void Device8254::T2_OUT(BYTE value)
	{
		m_counter2.Set(value);
	}

	void Device8254::CONTROL_OUT(BYTE value)
	{
		Counter* counter = 0;

		switch (value & (CTRL_SC1 | CTRL_SC0))
		{
		case 0:        counter = &m_counter0; break;
		case CTRL_SC0: counter = &m_counter1; break;
		case CTRL_SC1: counter = &m_counter2; break;
		case CTRL_SC1 | CTRL_SC0:
		default:
			throw std::exception("SC: Not implemented");
		}

		switch (value & (CTRL_RW1 | CTRL_RW0))
		{
		case 0:
			counter->LatchValue(); 
			// Not a control command, latch value and return
			// without setting rw/mode/bcd
			return;

		case CTRL_RW0: counter->SetRWMode(RWMode::RW_LSB); break;
		case CTRL_RW1: counter->SetRWMode(RWMode::RW_MSB); break;
		case CTRL_RW1 | CTRL_RW0: counter->SetRWMode(RWMode::RW_LSBMSB); break;
		default:
			throw std::exception("RW: Not implemented");
		}

		switch (value & (CTRL_M2 | CTRL_M1 | CTRL_M0))
		{
		case 0:
			counter->SetMode(CounterMode::Mode0); break;
		case CTRL_M0:
			counter->SetMode(CounterMode::Mode1); break;

		case CTRL_M1:
		case CTRL_M2|CTRL_M1:
			counter->SetMode(CounterMode::Mode2); break;

		case CTRL_M1 | CTRL_M0:
		case CTRL_M2 | CTRL_M1 | CTRL_M0:
			counter->SetMode(CounterMode::Mode3); break;

		case CTRL_M2:
			counter->SetMode(CounterMode::Mode4); break;

		case CTRL_M2 | CTRL_M0:
			counter->SetMode(CounterMode::Mode5); break;

		default:
			throw std::exception("Mode: Not implemented");
		}

		counter->SetBCD(value & CTRL_BCD);
	}

	void Device8254::Tick()
	{
		m_counter0.Tick();
		m_counter1.Tick();
		m_counter2.Tick();
	}

	Counter& Device8254::GetCounter(size_t counter)
	{
		switch (counter)
		{
		case 0: return m_counter0;
		case 1: return m_counter1;
		case 2: return m_counter2;
		default:
			throw std::exception("Device8254::GetCounter:: invalid counter id");
		}
	}

}
