#include "Device8254.h"

namespace pit
{
	Counter::Counter(const char* label) : Logger(label),
		m_rwMode(RWMode::RW_LSB),
		m_mode(CounterMode::Mode0),
		m_bcd(false),
		m_n(0),
		m_value(0),
		m_latched(false),
		m_latchedValue(0)
	{
	}

	void Counter::Tick()
	{
		// TODO: Mode 2, LSB only
		--m_value;
		if ((m_value & 0x00FF) == 0)
		{
			m_value = m_n;
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
		case RWMode::RW_MSBLSB:
		default:
			throw std::exception("Get:RWMode: Not implemented");
		}
		return ret;
	}

	void Counter::Set(BYTE value)
	{
		switch (m_rwMode)
		{
		case RWMode::RW_LSB:
			LogPrintf(LOG_DEBUG, "SetLSB: %02X", value);
			SetLSB(value);
			break;

		case RWMode::RW_MSB:
		case RWMode::RW_MSBLSB:
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

	BYTE Counter::GetMSB()
	{
		LogPrintf(LOG_DEBUG, "GetMSB");
		return 0;
	}
	BYTE Counter::GetLSB()
	{
		LogPrintf(LOG_DEBUG, "GetLSB");
		return 0;
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
		case RWMode::RW_MSBLSB:
		default:
			throw std::exception("SetRWMode: Not implemented");
		}
	}

	void Counter::SetMode(CounterMode mode)
	{
		m_mode = mode;

		switch (mode)
		{
		case CounterMode::Mode2:
			LogPrintf(LOG_DEBUG, "SetMode: 2");
			break;

		case CounterMode::Mode1:
		case CounterMode::Mode3:
		case CounterMode::Mode4:
		case CounterMode::Mode5:
		default:
			throw std::exception("SetMode: Not implemented");
		}
	}

	void Counter::SetBCD(bool bcd)
	{
		LogPrintf(LOG_DEBUG, "SetBCD: %d", bcd);
		m_bcd = bcd;
	}


	Device8254::Device8254(WORD baseAddress) : 
		Logger("PIT8254"), 
		m_counter0("PIT_T0"),
		m_counter1("PIT_T1"),
		m_counter2("PIT_T2"),
		m_baseAddress(baseAddress)
	{
		Reset();
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
		case CTRL_RW1 | CTRL_RW0: counter->SetRWMode(RWMode::RW_MSBLSB); break;
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
}
