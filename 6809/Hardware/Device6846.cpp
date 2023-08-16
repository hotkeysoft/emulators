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
	}

	BYTE Device6846::ReadStatus()
	{
		// Reading the status allows interrupt flags
		// to be cleared by Read/WriteControl
		PCR.interruptAcknowledged = true;

		BYTE value = 0 |
			(GetIRQ() << 7) |
			(PCR.GetCP2InterruptFlag() << 2) |
			(PCR.GetCP1InterruptFlag() << 1) |
			0; // Timer interrupt

		LogPrintf(LOG_INFO, "ReadStatus, value=%2x", value);

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

		PCR.CP1Latch.SetTrigger(PCR.GetCP1PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);
		PCR.CP2Latch.SetTrigger(PCR.GetCP2PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);

		if (PCR.IsReset())
		{
			Reset();
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
	}

	void Device6846::LogTimerControlRegister() const
	{
		LogPrintf(LOG_INFO, "Timer Output Enabled: %d", TCR.IsTimerOutputEnabled());
		LogPrintf(LOG_INFO, "Timer Interrupt Enabled : %d", TCR.IsTimerInterruptEnabled());
		LogPrintf(LOG_INFO, "Timer Operating Mode : %d", TCR.GetTimerMode());
		LogPrintf(LOG_INFO, "Timer /8 Prescaler   : %d", TCR.IsDiv8Prescaler());
		LogPrintf(LOG_INFO, "Timer Clock Source   : %s", TCR.IsClockSourceSystem() ? "SYSTEM" : "EXTERNAL");
		LogPrintf(LOG_INFO, "Timer Initial Reset  : %s", TCR.IsTimerPreset() ? "Preset state" : "Enabled");

		if (!TCR.IsClockSourceSystem())
		{
			LogPrintf(LOG_ERROR, "External clock source not implemented");
		}
	}

	BYTE Device6846::ReadTimerMSB()
	{
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
		BYTE value = emul::GetLByte(m_counter);
		LogPrintf(LOG_DEBUG, "ReadTimerLSB, value=%02x", value);
		return value;
	}
	void Device6846::WriteTimerLSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteTimerLSB, value=%02x", value);
		emul::SetHByte(m_counterLatch, m_tempMSB);
		emul::SetLByte(m_counterLatch, value);

		LogPrintf(LOG_TRACE, "Set Counter Latch: %04x", m_counterLatch);
	}

	void Device6846::LogPeripheralControlRegister() const
	{
		LogPrintf(LOG_INFO, " Reset             : %s", PCR.IsReset() ? "RESET" : "Normal operation");

		LogPrintf(LOG_INFO, " CP1 Interrupt flag: %d", PCR.GetCP2InterruptFlag());
		LogPrintf(LOG_INFO, " CP2 Interrupt flag: %d", PCR.GetCP2InterruptFlag());

		if (PCR.GetCP2Direction() == DataDirection::OUTPUT)
		{
			LogPrintf(LOG_INFO, " CP2 pin: OUTPUT");
			LogPrintf(LOG_INFO, " CP2 output ctrl : %d", PCR.GetCP2OutputControl());
			LogPrintf(LOG_INFO, " CP2 output      : %d", PCR.GetCP2Output());

			if (PCR.GetCP2OutputControl() == false)
			{
				LogPrintf(LOG_ERROR, "CP2 output ctrl = 0 not supported");
			}
		}
		else
		{
			LogPrintf(LOG_INFO, " CP2 pin: INPUT");
			LogPrintf(LOG_INFO, " CP2 pos transition   : %d", PCR.GetCP2PositiveTransition());
			LogPrintf(LOG_INFO, " CP2 Interrupt Enable : %d", PCR.GetCP2InterruptEnabled());
		}

		LogPrintf(LOG_INFO, " CP1 latch control    : %d", PCR.GetCP1InputLatchControl());
		if (PCR.GetCP1InputLatchControl())
		{
			LogPrintf(LOG_ERROR, "CP1 input latch control = 1 not supported");
		}

		LogPrintf(LOG_INFO, " CP1 pos transition   : %d", PCR.GetCP1PositiveTransition());
		LogPrintf(LOG_INFO, " CP1 Interrupt Enable : %d", PCR.GetCP1InterruptEnabled());
	}

	void Device6846::PeripheralControlRegister::ClearInterruptFlags(bool force)
	{
		if (force || interruptAcknowledged)
		{
			CP1Latch.ResetLatch();
			CP2Latch.ResetLatch();
			interruptAcknowledged = false;
		}
	}

	void Device6846::PeripheralControlRegister::Serialize(json& to)
	{
		to["data"] = data;
		to["interruptAcknowledged"] = interruptAcknowledged;
		CP1Latch.Serialize(to["CP1Latch"]);
		CP2Latch.Serialize(to["CP2Latch"]);
	}
	void Device6846::PeripheralControlRegister::Deserialize(const json& from)
	{
		data = from["data"];
		interruptAcknowledged = from["interruptAcknowledged"];
		CP1Latch.Deserialize(from["CP1Latch"]);
		CP2Latch.Deserialize(from["CP2Latch"]);
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
	}

	void Device6846::Deserialize(const json& from)
	{
		DDR = from["DDR"];
		IR = from["IR"];
		OR = from["OR"];
		PCR.Deserialize(from["PCR"]);
		TCR.Deserialize(from["TCR"]);
	}
}
