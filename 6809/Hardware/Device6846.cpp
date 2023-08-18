#include "stdafx.h"

#include "Device6846.h"

using hscommon::EdgeDetectLatch;

namespace pia
{
	Device6846::Device6846(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x07) // Addresses 0-7 are decoded by device
	{
	}

	void Device6846::Init()
	{
		Reset();

		Connect(0, static_cast<IOConnector::READFunction>(&Device6846::ReadStatus));
		Connect(1, static_cast<IOConnector::READFunction>(&Device6846::ReadControl));
		Connect(2, static_cast<IOConnector::READFunction>(&Device6846::ReadDirection));
		Connect(3, static_cast<IOConnector::READFunction>(&Device6846::ReadData));
		Connect(4, static_cast<IOConnector::READFunction>(&Device6846::ReadStatus));
		Connect(5, static_cast<IOConnector::READFunction>(&Device6846::ReadTimerControl));
		Connect(6, static_cast<IOConnector::READFunction>(&Device6846::ReadTimerMSB));
		Connect(7, static_cast<IOConnector::READFunction>(&Device6846::ReadTimerLSB));

		Connect(1, static_cast<IOConnector::WRITEFunction>(&Device6846::WriteControl));
		Connect(2, static_cast<IOConnector::WRITEFunction>(&Device6846::WriteDirection));
		Connect(3, static_cast<IOConnector::WRITEFunction>(&Device6846::WriteData));
		Connect(5, static_cast<IOConnector::WRITEFunction>(&Device6846::WriteTimerControl));
		Connect(6, static_cast<IOConnector::WRITEFunction>(&Device6846::WriteTimerMSB));
		Connect(7, static_cast<IOConnector::WRITEFunction>(&Device6846::WriteTimerLSB));
	}

	void Device6846::Reset()
	{
		DDR = 0;
		OR = 0;
		IR = 0xFF; // TODO: Assume pullups for now

		PCR.Reset();
		TCR.Reset();

		m_counterLatch = 0xFFFF;
		m_counter = 0xFFFF;
		m_timerOutput = false;
		ClearTimerIRQ();
	}

	void Device6846::Tick()
	{
		static constexpr int PRESCALE = 8;
		static int prescaler = PRESCALE;

		// Nothing to do in preset mode
		if (TCR.IsTimerPreset())
		{
			m_counter = m_counterLatch;
			prescaler = PRESCALE;
			return;
		}

		if (TCR.IsDiv8Prescaler() && (--prescaler))
		{
			return;
		}
		prescaler = PRESCALE;

		// Mode 0 only for now
		if (m_counter-- == 0)
		{
			LogPrintf(LOG_TRACE, "[%zu] Time out", emul::g_ticks);
			m_counter = m_counterLatch;
			m_timerOutput = !m_timerOutput;
			SetTimerIRQ();
		}
	}

	BYTE Device6846::ReadStatus()
	{
		// Reading the status allows interrupt flags
		// to be cleared by Read/WriteControl
		if (PCR.GetCP1IRQFlag() || PCR.GetCP2IRQFlag())
		{
			PCR.interruptAcknowledged = true;
		}

		// Reading the status allows timer irq
		// to be cleared by Read/WriteCounter
		AcknowledgeTimerIRQ();

		BYTE value = 0 |
			(GetIRQ() << 7) |
			(PCR.GetCP2IRQFlag() << 2) |
			(PCR.GetCP1IRQFlag() << 1) |
			(m_timerIRQ << 0);

		LogPrintf(LOG_DEBUG, "ReadStatus, value=%2x", value);

		return value;
	}

	BYTE Device6846::ReadControl()
	{
		PCR.ClearInterruptFlags();

		BYTE value = PCR.data;
		LogPrintf(LOG_TRACE, "Read ControlRegister, value=%02X", value);

		return value;
	}
	void Device6846::WriteControl(BYTE value)
	{
		PCR.ClearInterruptFlags();

		LogPrintf(LOG_DEBUG, "WriteControlRegister, value=%02X", value);
		PCR.data = value;

		if (IsLog(LOG_INFO))
		{
			LogPrintf(LOG_INFO, "Set ControlRegister:");
			LogPeripheralControlRegister();
		}

		PCR.CP1IRQLatch.SetTrigger(PCR.GetCP1PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);
		PCR.CP2IRQLatch.SetTrigger(PCR.GetCP2PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);

		if (PCR.IsReset())
		{
			Reset();
		}

		if (PCR.GetCP2OutputControl() == false)
		{
			LogPrintf(LOG_ERROR, "CP2 output ctrl = 0 not supported");
		}
		if (PCR.GetCP1InputLatchControl())
		{
			LogPrintf(LOG_ERROR, "CP1 input latch control = 1 not supported");
		}
	}

	BYTE Device6846::ReadDirection()
	{
		BYTE value = DDR;
		LogPrintf(LOG_DEBUG, "Read DataDirection, value=%02X", value);
		return value;
	}
	void Device6846::WriteDirection(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write DataDirection, value=%02X", value);
		DDR = value;
		LogPrintf(LOG_INFO, "Set DataDirection:");
		for (int i = 7; i >= 0; --i)
		{
			LogPrintf(LOG_INFO, " PIN %d: %s", i, (GetDataDirection(i) == DataDirection::INPUT) ? "IN" : "OUT");
		}
	}

	BYTE Device6846::ReadData()
	{
		OnReadPort();
		BYTE value = IR;

		// For output pins, mix with output register
		value &= (~DDR); // Clear output pin data
		value |= (OR & DDR); // Set output pin data from OR

		LogPrintf(LOG_DEBUG, "Read PortData, value=%02X", value);
		return value;
	}
	void Device6846::WriteData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteData, value=%02x", value);
		OR = value;
		OnWritePort();
	}

	BYTE Device6846::ReadTimerControl()
	{
		BYTE value = TCR.data;
		LogPrintf(LOG_DEBUG, "ReadTimerControl, value=%02X", value);
		return value;
	}
	void Device6846::WriteTimerControl(BYTE value)
	{
		LogPrintf(LOG_INFO, "WriteTimerControl, value=%02x", value);

		TCR.data = value;

		if (IsLog(LOG_INFO))
		{
			LogPrintf(LOG_INFO, "Set Timer Control:");
			LogTimerControlRegister();
		}

		// Only a few modes implemented for now
		switch (TCR.GetTimerMode())
		{
		case TimerMode::CONTINUOUS1:
		case TimerMode::CONTINUOUS2:
			break;
		default:
			LogPrintf(LOG_ERROR, "Timer Operating Mode not implemented: %d", TCR.GetTimerMode());
		}

		if (!TCR.IsClockSourceSystem())
		{
			LogPrintf(LOG_ERROR, "External clock source not implemented");
		}
	}

	void Device6846::LogTimerControlRegister() const
	{
		LogPrintf(LOG_INFO, "Timer State          : %s", TCR.IsTimerPreset() ? "Preset state" : "Enabled");
		LogPrintf(LOG_INFO, "Timer Clock Source   : %s", TCR.IsClockSourceSystem() ? "SYSTEM" : "EXTERNAL");
		LogPrintf(LOG_INFO, "Timer /8 Prescaler   : %d", TCR.IsDiv8Prescaler());

		const char* mode = nullptr;
		switch (TCR.GetTimerMode())
		{
		case TimerMode::CONTINUOUS1: mode = "Continuous (1)"; break;
		case TimerMode::SINGLE_SHOT_CASCADED: mode = "Single Shot (Cascaded)"; break;
		case TimerMode::CONTINUOUS2: mode = "Continuous (2)"; break;
		case TimerMode::SINGLE_SHOT_NORMAL: mode = "Single Shot (Normal)"; break;
		case TimerMode::FREQ_COMPARISON1: mode = "Frequency Comparison (1)"; break;
		case TimerMode::FREQ_COMPARISON2: mode = "Frequency Comparison (2)"; break;
		case TimerMode::PULSE_WIDTH_COMPARISON1: mode = "Pulse Width Comparison (1)"; break;
		case TimerMode::PULSE_WIDTH_COMPARISON2: mode = "Pulse Width Comparison (2)"; break;
		}
		LogPrintf(LOG_INFO, "Timer Operating Mode : %s", mode);
		LogPrintf(LOG_INFO, "Timer Interrupt Enabled : %d", TCR.IsTimerInterruptEnabled());
		LogPrintf(LOG_INFO, "Timer Output Enabled: %d", TCR.IsTimerOutputEnabled());
	}

	BYTE Device6846::ReadTimerMSB()
	{
		if (m_timerIRQAcknowledged)
		{
			ClearTimerIRQ();
		}

		BYTE value = emul::GetHByte(m_counter);
		LogPrintf(LOG_DEBUG, "ReadTimerMSB, value=%02x", value);
		return value;
	}
	void Device6846::WriteTimerMSB(BYTE value)
	{
		m_tempMSB = value;
		LogPrintf(LOG_DEBUG, "WriteTimerMSB, value=%02x", value);
	}

	BYTE Device6846::ReadTimerLSB()
	{
		if (m_timerIRQAcknowledged)
		{
			ClearTimerIRQ();
		}

		BYTE value = emul::GetLByte(m_counter);
		LogPrintf(LOG_DEBUG, "ReadTimerLSB, value=%02x", value);
		return value;
	}
	void Device6846::WriteTimerLSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteTimerLSB, value=%02x", value);
		emul::SetHByte(m_counterLatch, m_tempMSB);
		emul::SetLByte(m_counterLatch, value);
		LogPrintf(LOG_INFO, "Set Counter Latch: %04x", m_counterLatch);

		if (TCR.GetTimerMode() == TimerMode::CONTINUOUS1)
		{
			m_counter = m_counterLatch;
			ClearTimerIRQ();
		}
	}

	void Device6846::LogPeripheralControlRegister() const
	{
		LogPrintf(LOG_INFO, " Reset             : %s", PCR.IsReset() ? "RESET" : "Normal operation");

		LogPrintf(LOG_INFO, " CP1 Interrupt flag: %d", PCR.GetCP2IRQFlag());
		LogPrintf(LOG_INFO, " CP2 Interrupt flag: %d", PCR.GetCP2IRQFlag());

		if (PCR.GetCP2Direction() == DataDirection::OUTPUT)
		{
			LogPrintf(LOG_INFO, " CP2 pin: OUTPUT");
			LogPrintf(LOG_INFO, " CP2 output ctrl : %d", PCR.GetCP2OutputControl());
			LogPrintf(LOG_INFO, " CP2 output      : %d", PCR.GetCP2Output());
		}
		else
		{
			LogPrintf(LOG_INFO, " CP2 pin: INPUT");
			LogPrintf(LOG_INFO, " CP2 pos transition   : %d", PCR.GetCP2PositiveTransition());
			LogPrintf(LOG_INFO, " CP2 Interrupt Enable : %d", PCR.GetCP2InterruptEnabled());
		}

		LogPrintf(LOG_INFO, " CP1 latch control    : %d", PCR.GetCP1InputLatchControl());
		LogPrintf(LOG_INFO, " CP1 pos transition   : %d", PCR.GetCP1PositiveTransition());
		LogPrintf(LOG_INFO, " CP1 Interrupt Enable : %d", PCR.GetCP1InterruptEnabled());
	}

	void Device6846::PeripheralControlRegister::ClearInterruptFlags(bool force)
	{
		if (force || interruptAcknowledged)
		{
			CP1IRQLatch.ResetLatch();
			CP2IRQLatch.ResetLatch();
			interruptAcknowledged = false;
		}
	}

	void Device6846::PeripheralControlRegister::Serialize(json& to)
	{
		to["data"] = data;
		to["interruptAcknowledged"] = interruptAcknowledged;
		CP1IRQLatch.Serialize(to["CP1IRQLatch"]);
		CP2IRQLatch.Serialize(to["CP2IRQLatch"]);
	}
	void Device6846::PeripheralControlRegister::Deserialize(const json& from)
	{
		data = from["data"];
		interruptAcknowledged = from["interruptAcknowledged"];
		CP1IRQLatch.Deserialize(from["CP1IRQLatch"]);
		CP2IRQLatch.Deserialize(from["CP2IRQLatch"]);
	}

	void Device6846::TimerControlRegister::Serialize(json& to)
	{
		to["data"] = data;
	}
	void Device6846::TimerControlRegister::Deserialize(const json& from)
	{
		data = from["data"];
	}

	void Device6846::Serialize(json& to)
	{
		to["DDR"] = DDR;
		to["IR"] = IR;
		to["OR"] = OR;
		PCR.Serialize(to["PCR"]);
		TCR.Serialize(to["TCR"]);

		to["tempMSB"] = m_tempMSB;
		to["counterLatch"] = m_counterLatch;
		to["counter"] = m_counter;
		to["timerOutput"] = m_timerOutput;
		to["timerIRQ"] = m_timerIRQ;
	}

	void Device6846::Deserialize(const json& from)
	{
		DDR = from["DDR"];
		IR = from["IR"];
		OR = from["OR"];
		PCR.Deserialize(from["PCR"]);
		TCR.Deserialize(from["TCR"]);

		m_tempMSB = from["tempMSB"];
		m_counterLatch = from["counterLatch"];
		m_counter = from["counter"];
		m_timerOutput = from["timerOutput"];
		m_timerIRQ = from["timerIRQ"];
	}
}
