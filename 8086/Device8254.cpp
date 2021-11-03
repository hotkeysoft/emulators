#include "Device8254.h"
#include "PortAggregator.h"

#include <assert.h>

using emul::GetHByte;
using emul::GetLByte;
using emul::SetLByte;
using emul::SetHByte;

namespace pit
{
	Counter::Counter(Device8254* parent, BYTE id, const char* label) :
		Logger(label),
		m_parent(parent),
		m_id(id)
	{
		assert(parent);
	}

	void Counter::Init()
	{
		Connect(m_parent->GetBaseAdress() + m_id, static_cast<PortConnector::INFunction>(&Counter::ReadData));
		Connect(m_parent->GetBaseAdress() + m_id, static_cast<PortConnector::OUTFunction>(&Counter::WriteData));

		SetGate(true);
	}

	void Counter::Tick()
	{
		if (m_newValue)
		{
			m_newValue = false;
			bool wasRunning = m_run;
			m_run = true;

			switch (m_mode)
			{
			case CounterMode::Mode0:
			{
				m_out = false;
				m_value = m_n;
				size_t ticks = (size_t)GetMaxValue() + 1;
				m_periodMicro = (float)ticks * 1000000 / (float)s_clockSpeed;
				LogPrintf(LOG_INFO, "[%zu] Mode0: Starting Count, interval = %0.2fus", emul::g_ticks, m_periodMicro);
				// Start counting on next tick
				return;
			}
			case CounterMode::Mode2:
			{
				if (!wasRunning)
				{
					m_value = m_n;
				}
				size_t ticks = GetMaxValue();
				m_periodMicro = (float)ticks * 1000000 / (float)s_clockSpeed;
				LogPrintf(LOG_INFO, "[%zu] Mode2: Starting Count, period = %0.2fus", emul::g_ticks, m_periodMicro);
				break;
			}
			case CounterMode::Mode3:
			{
				size_t ticks = GetMaxValue();
				m_periodMicro = (float)ticks * 1000000 / (float)s_clockSpeed;
				float freq = (float)s_clockSpeed / (float)ticks;
				LogPrintf(LOG_INFO, "Mode3: Frequency = %0.2fHz", freq);
				break;
			}
			}
		} 

		m_lastGate = true;
		if (!m_gate)
		{
			m_lastGate = false;
			// Mode 0 not affected by gate, only pauses count
			if (m_mode != CounterMode::Mode0)
			{
				// Mode 2 & 3, if gate is low, out goes high.
				// When back high again, reset counter to n
				m_out = true;
			}
			return;
		}

		if (!m_run)
		{
			return;
		}

		switch (m_mode)
		{
		case CounterMode::Mode0:
			--m_value;
			if (m_value == 0)
			{
				LogPrintf(LOG_DEBUG, "[%zu] Mode0: Count Done", emul::g_ticks);
				m_run = false;
				m_out = true;
			}
			break;
		case CounterMode::Mode2:
			--m_value;

			if (m_lastGate == false)
			{
				LogPrintf(LOG_DEBUG, "[%zu] Mode2: HI, Reset after Gate 0->1", emul::g_ticks);
				m_out = true;
				m_value = m_n;
			}
			else if (m_value == 1)
			{
				m_out = false;
				LogPrintf(LOG_DEBUG, "[%zu] Mode2: LOW", emul::g_ticks);
			}
			else if (m_value == 0)
			{
				LogPrintf(LOG_DEBUG, "[%zu] Mode2: HI, reset", emul::g_ticks);
				m_out = true;
				m_value = m_n;
			}
			break;
		case CounterMode::Mode3:
			m_value -= (m_value & 1) ? 1 : 2;

			if (m_lastGate == false)
			{
				LogPrintf(LOG_WARNING, "[%zu] Mode3: HI, Reset after Gate 0->1", emul::g_ticks);
				m_out = true;
				m_value = m_n;
			}
			else if (m_value == 0)
			{
				m_out = !m_out;

				m_value = m_n & (m_out ? ~0 : ~1);
			}
			break;
		default:
			break;
		}

		m_lastGate = true;
	}

	BYTE Counter::ReadData()
	{
		BYTE ret;
		WORD value = (m_latched) ? m_latchedValue : m_value;

		switch (m_rwMode)
		{
		case RWMode::RW_LSB:
			ret = GetLByte(value);
			m_latched = false;
			break;

		case RWMode::RW_MSB:
			ret = GetHByte(value);
			m_latched = false;
			break;

		case RWMode::RW_LSBMSB:
			if (!m_flipFlopLSBMSB)
			{
				ret = GetLByte(value);
			}
			else
			{
				ret = GetHByte(value);
				m_latched = false;
			}
			m_flipFlopLSBMSB = !m_flipFlopLSBMSB;
			break;

		default:
			throw std::exception("Get:RWMode: Not implemented");
		}
		return ret;
	}

	void Counter::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteData, value=%02Xh", value);
		switch (m_rwMode)
		{
		case RWMode::RW_LSBMSB:
			if (!m_flipFlopLSBMSB)
			{
				SetLByte(m_n, value);
				if (m_mode == CounterMode::Mode0)
				{
					m_run = false;
				}
			}
			else
			{
				SetHByte(m_n, value);
				m_newValue = true;
			}
			m_flipFlopLSBMSB = !m_flipFlopLSBMSB;
			break;
		case RWMode::RW_LSB:
			SetHByte(m_n, 0);
			SetLByte(m_n, value);
			m_newValue = true;
			break;
		case RWMode::RW_MSB:
			SetLByte(m_n, 0);
			SetHByte(m_n, value);
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

	void Counter::SetRWMode(RWMode rw)
	{
		m_rwMode = rw;

		switch (rw)
		{
		case RWMode::RW_LSB:
			LogPrintf(LOG_INFO, "SetRWMode: LSB");
			break;

		case RWMode::RW_MSB:
			LogPrintf(LOG_INFO, "SetRWMode: MSB");
			break;

		case RWMode::RW_LSBMSB:
			LogPrintf(LOG_INFO, "SetRWMode: LSBMSB");
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
			throw std::exception("SetMode: 1: Not implemented");
			break;
		case CounterMode::Mode2:
			LogPrintf(LOG_INFO, "SetMode: 2 - RATE GENERATOR");
			m_out = true;
			break;
		case CounterMode::Mode3:
			LogPrintf(LOG_INFO, "SetMode: 3 - SQUARE WAVE MODE");
			m_out = true;
			break;
		case CounterMode::Mode4:
			LogPrintf(LOG_INFO, "SetMode: 4 - SOFTWARE TRIGGERED MODE");
			LogPrintf(LOG_ERROR, "Mode 4 Not implemented");
			throw std::exception("SetMode: 4: Not implemented");
			break;
		case CounterMode::Mode5:
			LogPrintf(LOG_INFO, "SetMode: 5 - HARDWARE TRIGGERED STROBE");
			LogPrintf(LOG_ERROR, "Mode 5 Not implemented");
			throw std::exception("SetMode: 5: Not implemented");
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
		m_counters{
			{this, 0, "pit.t0"},
			{this, 1, "pit.t1"},
			{this, 2, "pit.t2"}
		},
		m_baseAddress(baseAddress)
	{
		Reset();
		s_clockSpeed = clockSpeedHz;
	}

	void Device8254::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_counters[0].EnableLog(minSev);
		m_counters[1].EnableLog(LOG_OFF); // RAM Refresh, noise in logs
		m_counters[2].EnableLog(minSev);
	}

	void Device8254::Reset()
	{

	}

	void Device8254::Init()
	{
		// Register ports 0..2
		m_counters[0].Init();
		m_counters[1].Init();
		m_counters[2].Init();

		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device8254::WriteControl));
	}

	bool Device8254::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_counters[0]);
		dest.Connect(m_counters[1]);
		dest.Connect(m_counters[2]);
		return PortConnector::ConnectTo(dest);
	}

	void Device8254::WriteControl(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteControl, value=%02Xh", value);

		int counterSel = (value >> 6);
		if (counterSel == 4)
		{
			throw std::exception("SC: Readback: Not implemented");
		}

		Counter& counter = m_counters[counterSel];

		switch (value & (CTRL_RW1 | CTRL_RW0))
		{
		case 0:
			counter.LatchValue(); 
			// Not a control command, latch value and return
			// without setting rw/mode/bcd
			return;

		case CTRL_RW0: counter.SetRWMode(RWMode::RW_LSB); break;
		case CTRL_RW1: counter.SetRWMode(RWMode::RW_MSB); break;
		case CTRL_RW1 | CTRL_RW0: counter.SetRWMode(RWMode::RW_LSBMSB); break;
		default:
			throw std::exception("not possible");
		}

		CounterMode mode;
		switch (value & (CTRL_M2 | CTRL_M1 | CTRL_M0))
		{
		case 0: 
			mode = CounterMode::Mode0; break;
		case CTRL_M0:
			mode = CounterMode::Mode1; break;

		case CTRL_M1:
		case CTRL_M2|CTRL_M1:
			mode = CounterMode::Mode2; break;

		case CTRL_M1 | CTRL_M0:
		case CTRL_M2 | CTRL_M1 | CTRL_M0:
			mode = CounterMode::Mode3; break;

		case CTRL_M2:
			mode = CounterMode::Mode4; break;

		case CTRL_M2 | CTRL_M0:
			mode = CounterMode::Mode5; break;

		default:
			throw std::exception("Mode: Not implemented");
		}

		counter.SetMode(mode);
		counter.SetBCD(value & CTRL_BCD);
	}

	void Device8254::Tick()
	{
		m_counters[0].Tick();
		m_counters[1].Tick();
		m_counters[2].Tick();
	}

	Counter& Device8254::GetCounter(size_t counter)
	{
		assert(counter < 3);
		return m_counters[counter];
	}

}
