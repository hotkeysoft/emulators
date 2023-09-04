#include "stdafx.h"

#include "Device6522.h"

using emul::GetMSB;
using hscommon::EdgeDetectLatch;

namespace via
{
	static const char* GetC2OperationStr(C2Operation op)
	{
		switch (op)
		{
		case C2Operation::IN_NEG_EDGE:     return "INPUT, Negative active edge";
		case C2Operation::IN_NEG_EDGE_INT: return "INPUT, Negative active edge, Independent interrupt";
		case C2Operation::IN_POS_EDGE:     return "INPUT, Positive active edge";
		case C2Operation::IN_POS_EDGE_INT: return "INPUT, Positive active edge, Independent interrupt";
		case C2Operation::OUT_HANDSHAKE:   return "OUTPUT, Handshake";
		case C2Operation::OUT_PULSE:       return "OUTPUT, Pulse";
		case C2Operation::OUT_LOW:         return "OUTPUT, LOW";
		case C2Operation::OUT_HIGH:        return "OUTPUT, HIGH";
		default: NODEFAULT;
		}
	}

	static const char* GetShiftRegisterModeStr(ShiftRegisterMode mode)
	{
		switch (mode)
		{
		case ShiftRegisterMode::DISABLED:          return "Disabled";
		case ShiftRegisterMode::SHIFT_IN_T2:       return "INPUT, T2";
		case ShiftRegisterMode::SHIFT_IN_CLK:      return "INPUT, CLK";
		case ShiftRegisterMode::SHIFT_IN_EXTCLK:   return "INPUT, EXT CLK";
		case ShiftRegisterMode::SHIFT_OUT_T2_FREE: return "OUTPUT, T2 FREE";
		case ShiftRegisterMode::SHIFT_OUT_T2:      return "OUTPUT, T2";
		case ShiftRegisterMode::SHIFT_OUT_CLK:     return "OUTPUT, CLK";
		case ShiftRegisterMode::SHIFT_OUT_EXTCLK:  return "OUTPUT, EXT CLK";
		default: NODEFAULT;
		}
	}

	VIAPort::VIAPort(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x0F) // Addresses 0-F are decoded by device
	{
	}

	void VIAPort::Init(Device6522* parent, bool isPortB, bool initIO)
	{
		assert(parent);
		m_parent = parent;

		Reset();

		if (initIO)
		{
			WORD offset = isPortB ? 0 : 1;
			Connect(0 + offset, static_cast<IOConnector::READFunction>(&VIAPort::ReadInputRegister));
			Connect(0 + offset, static_cast<IOConnector::WRITEFunction>(&VIAPort::WriteOutputRegister));

			Connect(2 + offset, static_cast<IOConnector::READFunction>(&VIAPort::ReadDataDirectionRegister));
			Connect(2 + offset, static_cast<IOConnector::WRITEFunction>(&VIAPort::WriteDataDirectionRegister));

			if (!isPortB)
			{
				Connect(0xF, static_cast<IOConnector::READFunction>(&VIAPort::ReadInputRegisterNoHandshake));
				Connect(0xF, static_cast<IOConnector::WRITEFunction>(&VIAPort::WriteOutputRegisterNoHandshake));
			}
		}
	}

	void VIAPort::Reset()
	{
		DDR = 0;
		OR = 0;
		IR = 0xFF; // TODO: Assume pullups for now
		C1.ResetLatch();
		C1.SetTrigger(EdgeDetectLatch::Trigger::NEGATIVE);
		C2 = false;
	}

	BYTE VIAPort::ReadInputRegister()
	{
		// Resets C1 and C2 (TODO: C2 input not implemented)
		C1.ResetLatch();
		m_parent->m_interrupt.ClearInterrupt(InterruptFlag::CA1);
		LogPrintf(LOG_DEBUG, "Reset C1 Latch");

		m_parent->OnReadPort(this);
		BYTE value = IR;

		// For output pins, mix with output register
		value &= (~DDR); // Clear output pin data
		value |= (OR & DDR); // Set output pin data from OR

		LogPrintf(LOG_DEBUG, "Read InputRegister, value=%02X", value);
		return value;
	}
	void VIAPort::WriteOutputRegister(BYTE value)
	{
		// Resets C1 and C2 (TODO: C2 input not implemented)
		C1.ResetLatch();
		m_parent->m_interrupt.ClearInterrupt(InterruptFlag::CA2);
		LogPrintf(LOG_DEBUG, "Reset C1 Latch");

		LogPrintf(LOG_DEBUG, "Write OutputRegister, value=%02X", value);
		OR = value;
		m_parent->OnWritePort(this);
	}

	BYTE VIAPort::ReadInputRegisterNoHandshake()
	{
		LogPrintf(LOG_DEBUG, "Read InputRegisterNoHandshake");
		// TODO
		return ReadInputRegister();
	}
	void VIAPort::WriteOutputRegisterNoHandshake(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write OutputRegisterNoHandshake, value=%02X", value);
		// TODO
		WriteOutputRegister(value);
	}

	BYTE VIAPort::ReadDataDirectionRegister()
	{
		BYTE value = DDR;
		LogPrintf(LOG_DEBUG, "Read DataDirectionRegister, value=%02X", value);
		return value;
	}
	void VIAPort::WriteDataDirectionRegister(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write DataDirectionRegister, value=%02X", value);
		DDR = value;
		LogPrintf(LOG_INFO, "Set DataDirection:");
		for (int i = 7; i >= 0; --i)
		{
			LogPrintf(LOG_INFO, " PIN %d: %s", i, (GetDataDirection(i) == DataDirection::INPUT) ? "IN" : "OUT");
		}
	}

	void VIAPort::SetC1InterruptActiveEdge(ActiveEdge edge)
	{
		C1.SetTrigger(edge == ActiveEdge::POS_EDGE ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);
	}

	void VIAPort::SetC2Operation(C2Operation op)
	{
		// CA2 Operation
		switch (op)
		{
		case C2Operation::OUT_LOW:
			C2 = false;
			break;
		case C2Operation::OUT_HIGH:
			C2 = true;
			break;

		case C2Operation::IN_NEG_EDGE:
		case C2Operation::IN_NEG_EDGE_INT:
		case C2Operation::IN_POS_EDGE:
		case C2Operation::IN_POS_EDGE_INT:
		case C2Operation::OUT_HANDSHAKE:
		case C2Operation::OUT_PULSE:
		default:
			LogPrintf(LOG_ERROR, "C2 Operation Not supported: %s", GetC2OperationStr(op));
			break;
		}
	}

	void VIAPort::Serialize(json& to)
	{
		to["DDR"] = DDR;
		to["IR"] = IR;
		to["OR"] = OR;
		C1.Serialize(to["C1"]);
		to["C2"] = C2;
	}

	void VIAPort::Deserialize(const json& from)
	{
		DDR = from["DDR"];
		IR = from["IR"];
		OR = from["OR"];
		C1.Deserialize(from["C1"]);
		C2 = from["C2"];
	}

	Device6522::Device6522(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x0F), // Addresses 0-F are decoded by device
		m_portA(id + ".A"),
		m_portB(id + ".B")
	{
	}

	void Device6522::Init(bool initIO)
	{
		Reset();

		m_portA.Init(this, false, initIO);
		m_portB.Init(this, true, initIO);

		if (initIO)
		{
			// Attach connections made in children objects
			Attach(m_portA);
			Attach(m_portB);

			Connect(0x4, static_cast<IOConnector::READFunction>(&Device6522::ReadT1CounterL));
			Connect(0x5, static_cast<IOConnector::READFunction>(&Device6522::ReadT1CounterH));
			Connect(0x6, static_cast<IOConnector::READFunction>(&Device6522::ReadT1LatchL));
			Connect(0x7, static_cast<IOConnector::READFunction>(&Device6522::ReadT1LatchH));
			Connect(0x8, static_cast<IOConnector::READFunction>(&Device6522::ReadT2CounterL));
			Connect(0x9, static_cast<IOConnector::READFunction>(&Device6522::ReadT2CounterH));
			Connect(0xA, static_cast<IOConnector::READFunction>(&Device6522::ReadSR));
			Connect(0xB, static_cast<IOConnector::READFunction>(&Device6522::ReadACR));
			Connect(0xC, static_cast<IOConnector::READFunction>(&Device6522::ReadPCR));
			Connect(0xD, static_cast<IOConnector::READFunction>(&Device6522::ReadIFR));
			Connect(0xE, static_cast<IOConnector::READFunction>(&Device6522::ReadIER));

			Connect(0x4, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchL));
			Connect(0x5, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1CounterH));
			Connect(0x6, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchL));
			Connect(0x7, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchH));
			Connect(0x8, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT2LatchL));
			Connect(0x9, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT2CounterH));
			Connect(0xA, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteSR));
			Connect(0xB, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteACR));
			Connect(0xC, static_cast<IOConnector::WRITEFunction>(&Device6522::WritePCR));
			Connect(0xD, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteIFR));
			Connect(0xE, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteIER));
		}
	}

	void Device6522::Reset()
	{
		LogPrintf(LOG_INFO, "Reset");
		m_interrupt.Clear();
		UpdateIER();
		UpdateIFR();

		PCR.Clear();
		UpdatePCR();

		ACR.Clear();
		UpdateACR();

		TIMER1.Reset();
		TIMER2.Reset();

		m_portA.Reset();
		m_portB.Reset();

		m_shiftRegister = 0;
	}

	// 4 - T1C-L: T1 Low-Order Counter
	BYTE Device6522::ReadT1CounterL()
	{
		BYTE value = TIMER1.GetCounterLow();

		// Reading this register resets the Timer1 interrupt flag
		m_interrupt.ClearInterrupt(InterruptFlag::TIMER1);

		LogPrintf(LOG_DEBUG, "ReadT1CounterL, value=%02X", value);
		return value;
	}
	// 5 - T1C-H: T1 High-Order Counter
	BYTE Device6522::ReadT1CounterH()
	{
		BYTE value = TIMER1.GetCounterHigh();

		LogPrintf(LOG_DEBUG, "ReadT1CounterH, value=%02X", value);
		return value;
	}
	void Device6522::WriteT1CounterH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1CounterH, value=%02X", value);

		// Writing this register resets the Timer1 interrupt flag
		m_interrupt.ClearInterrupt(InterruptFlag::TIMER1);

		TIMER1.SetCounterHighLatch(value);
		TIMER1.Load();
	}

	// 6 - T1L-L: T1 Low-Order Latch
	BYTE Device6522::ReadT1LatchL()
	{
		BYTE value = TIMER1.GetCounterLowLatch();

		LogPrintf(LOG_DEBUG, "ReadT1LatchL, value=%02X", value);
		return value;
	}
	void Device6522::WriteT1LatchL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1LatchL, value=%02X", value);
		TIMER1.SetCounterLowLatch(value);
	}

	// 7 - T1L-H: T1 High-Order Latch
	BYTE Device6522::ReadT1LatchH()
	{
		BYTE value = TIMER1.GetCounterHighLatch();

		LogPrintf(LOG_DEBUG, "ReadT1LatchH, value=%02X", value);
		return value;
	}
	void Device6522::WriteT1LatchH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1LatchH, value=%02X", value);
		TIMER1.SetCounterHighLatch(value);
	}

	// 8 - T2C-L: T2 Low-Order Counter
	BYTE Device6522::ReadT2CounterL()
	{
		BYTE value = TIMER2.GetCounterLow();

		// Reading this register resets the Timer2 interrupt flag
		m_interrupt.ClearInterrupt(InterruptFlag::TIMER2);

		LogPrintf(LOG_DEBUG, "ReadT2CounterL, value=%02X", value);
		return value;
	}
	void Device6522::WriteT2LatchL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2LatchL, value=%02X", value);
		TIMER2.SetCounterLowLatch(value);
	}

	// 9 - T2C-H: T2 High-Order Counter
	BYTE Device6522::ReadT2CounterH()
	{
		BYTE value = TIMER2.GetCounterHigh();
		LogPrintf(LOG_DEBUG, "ReadT2CounterH, value=%02X", value);
		return value;
	}
	void Device6522::WriteT2CounterH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2CounterH, value=%02X", value);

		// Writing this register resets the Timer2 interrupt flag
		m_interrupt.ClearInterrupt(InterruptFlag::TIMER2);

		TIMER2.SetCounterHighLatch(value);
		TIMER2.Load();
	}

	// A - SR: Shift Register
	BYTE Device6522::ReadSR()
	{
		m_interrupt.ClearInterrupt(InterruptFlag::SR);
		BYTE value = m_shiftRegister;
		LogPrintf(LOG_DEBUG, "ReadSR, value=%02X", value);
		return value;
	}
	void Device6522::WriteSR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteSR, value=%02X", value);
		m_shiftRegister = value;

		ShiftRegisterMode srMode = ACR.GetShiftRegisterMode();
		switch (srMode)
		{
		case ShiftRegisterMode::DISABLED:
			// No effect;
			break;
		case ShiftRegisterMode::SHIFT_OUT_T2_FREE:
		case ShiftRegisterMode::SHIFT_OUT_T2:
			if (m_interrupt.IsInterruptSet(InterruptFlag::SR))
			{
				TIMER2.Load();
			}
			m_interrupt.ClearInterrupt(InterruptFlag::SR);
			break;
		default:
			LogPrintf(LOG_WARNING, "WriteSR: Mode not supported: %s", GetShiftRegisterModeStr(srMode));
			break;
		}
	}

	// B - ACR: Auxiliary Control Register
	BYTE Device6522::ReadACR()
	{
		BYTE value = ACR.Get();
		LogPrintf(LOG_DEBUG, "ReadACR, value=%02X", value);
		return value;
	}
	void Device6522::WriteACR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteACR, value=%02X", value);
		ACR.Set(value);
		UpdateACR();
	}

	// C - PCR: Peripheral Control Register
	BYTE Device6522::ReadPCR()
	{
		BYTE value = PCR.Get();
		LogPrintf(LOG_DEBUG, "ReadPCR, value=%02X", value);
		return value;
	}
	void Device6522::WritePCR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WritePCR, value=%02X", value);
		PCR.Set(value);
		UpdatePCR();
	}

	// D - IFR: Interrupt Flag Register
	BYTE Device6522::ReadIFR()
	{
		BYTE value = m_interrupt.GetIFR();
		LogPrintf(LOG_TRACE, "ReadIFR, value=%02X", value);
		return value;
	}
	void Device6522::WriteIFR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIFR, value=%02X", value);
		m_interrupt.SetIFR(value);
		UpdateIFR();

		if (Interrupt::IsInterruptEnabled(value, InterruptFlag::TIMER1)) TIMER1.Disarm();
		if (Interrupt::IsInterruptEnabled(value, InterruptFlag::TIMER2)) TIMER2.Disarm();
		if (Interrupt::IsInterruptEnabled(value, InterruptFlag::CB1)) m_portB.ResetC1();
		if (Interrupt::IsInterruptEnabled(value, InterruptFlag::CB2)) m_portB.ResetC2();
		// TODO: SR
		if (Interrupt::IsInterruptEnabled(value, InterruptFlag::CA1)) m_portA.ResetC1();
		if (Interrupt::IsInterruptEnabled(value, InterruptFlag::CA2)) m_portA.ResetC2();
	}

	// E - IER: Interrupt Enable Register
	BYTE Device6522::ReadIER()
	{
		BYTE value = m_interrupt.GetIER();
		LogPrintf(LOG_DEBUG, "ReadIER, value=%02X", value);
		return value;
	}
	void Device6522::WriteIER(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIER, value=%02X", value);
		m_interrupt.SetIER(value);
		UpdateIER();
	}

	void Device6522::UpdateACR()
	{
		LogPrintf(LOG_INFO, "Auxiliary Control Register");
		LogPrintf(LOG_INFO, " T1 Output on PB7: %d", ACR.GetPB7TimerOutput());
		LogPrintf(LOG_INFO, " T1 Mode         : %s", ACR.GetTimer1Mode() == AuxControl::T1Mode::CONTINUOUS ? "CONTINUOUS" : "ONE SHOT");
		LogPrintf(LOG_INFO, " T2 Mode         : %s", ACR.GetTimer2Mode() == AuxControl::T2Mode::ONE_SHOT? "ONE SHOT" : "PULSE PB6");

		ShiftRegisterMode srMode = ACR.GetShiftRegisterMode();
		LogPrintf(LOG_INFO, " Shift Reg Mode  : %s", GetShiftRegisterModeStr(srMode));

		LogPrintf(LOG_INFO, " PortA Latching  : %d", ACR.GetPortALatchingEnabled());
		LogPrintf(LOG_INFO, " PortB Latching  : %d", ACR.GetPortBLatchingEnabled());

		// Not much implemented atm
		if (ACR.GetPB7TimerOutput())
		{
			LogPrintf(LOG_WARNING, "ACR: T1 Output on PB7 not implemented");
		}
		if (ACR.GetTimer2Mode() == AuxControl::T2Mode::PULSE_COUNTER_PB6)
		{
			LogPrintf(LOG_WARNING, "ACR: T2 Pulse Counter Mode not implemented");
		}

		switch (srMode)
		{
		case ShiftRegisterMode::DISABLED:
		case ShiftRegisterMode::SHIFT_OUT_T2_FREE:
			// OK
			break;
		default:
			LogPrintf(LOG_WARNING, "ACR: Shift Register mode not implemented: %s", GetShiftRegisterModeStr(srMode));
			break;
		}
		if (ACR.GetPortALatchingEnabled())
		{
			LogPrintf(LOG_WARNING, "ACR: Port A Latching not implemented");
		}
		if (ACR.GetPortBLatchingEnabled())
		{
			LogPrintf(LOG_WARNING, "ACR: Port B Latching not implemented");
		}
	}

	void Device6522::UpdatePCR()
	{
		LogPrintf(LOG_INFO, "Update Peripheral Control Register");
		LogPrintf(LOG_INFO, " CA1 Interrupt active edge: %s", PCR.GetCA1InterruptActiveEdge() == ActiveEdge::NEG_EDGE ? "NEG" : "POS");
		LogPrintf(LOG_INFO, " CA2 Operation: %s", GetC2OperationStr(PCR.GetCA2Operation()));

		LogPrintf(LOG_INFO, " CB1 Interrupt active edge: %s", PCR.GetCB1InterruptActiveEdge() == ActiveEdge::NEG_EDGE ? "NEG" : "POS");
		LogPrintf(LOG_INFO, " CB2 Operation: %s", GetC2OperationStr(PCR.GetCB2Operation()));

		m_portA.SetC1InterruptActiveEdge(PCR.GetCA1InterruptActiveEdge());
		m_portB.SetC1InterruptActiveEdge(PCR.GetCB1InterruptActiveEdge());

		m_portA.SetC2Operation(PCR.GetCA2Operation());
		m_portB.SetC2Operation(PCR.GetCB2Operation());
	}

	void Device6522::UpdateIER()
	{
		LogPrintf(LOG_INFO, "Interrupt Register Mask [%cTI1 %cTI2 %cCB1 %cCB2 %cSR %cCA1 %cCA2]",
			(m_interrupt.IsInterruptEnabled(InterruptFlag::TIMER1) ? ' ' : '/'),
			(m_interrupt.IsInterruptEnabled(InterruptFlag::TIMER2) ? ' ' : '/'),
			(m_interrupt.IsInterruptEnabled(InterruptFlag::CB1) ? ' ' : '/'),
			(m_interrupt.IsInterruptEnabled(InterruptFlag::CB2) ? ' ' : '/'),
			(m_interrupt.IsInterruptEnabled(InterruptFlag::SR) ? ' ' : '/'),
			(m_interrupt.IsInterruptEnabled(InterruptFlag::CA1) ? ' ' : '/'),
			(m_interrupt.IsInterruptEnabled(InterruptFlag::CA2) ? ' ' : '/')
		);
	}

	void Device6522::UpdateIFR()
	{
		LogPrintf(LOG_INFO, "Interrupt Flag     [%cANY %cTI1 %cTI2 %cCB1 %cCB2 %cSR %cCA1 %cCA2]",
			(m_interrupt.IsInterruptSet(InterruptFlag::ANY) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::TIMER1) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::TIMER2) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::CB1) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::CB2) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::SR) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::CA1) ? ' ' : '/'),
			(m_interrupt.IsInterruptSet(InterruptFlag::CA2) ? ' ' : '/')
		);
	}

	void Device6522::Timer::Reset()
	{
		m_armed = false;
		m_latch = 0;
		m_counter = 0;
	}

	bool Device6522::Timer::Tick()
	{
		bool ret = false;

		if (m_load)
		{
			m_counter = m_latch;
			m_load = false;
		}
		else if (m_latch && (m_counter == 0))
		{
			m_load = true;
			ret = true;
		}
		else
		{
			--m_counter;
		}

		return ret;
	}

	void Device6522::Timer::Serialize(json& to)
	{
		to["latch"] = m_latch;
		to["armed"] = m_armed;
		to["counter"] = m_counter;
	}
	void Device6522::Timer::Deserialize(const json& from)
	{
		m_latch = from["latch"];
		m_armed = from["armed"];
		m_counter = from["counter"];
	}

	void Device6522::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_portA.EnableLog(minSev);
		m_portB.EnableLog(minSev);
	}

	BYTE Device6522::Interrupt::GetIFR() const
	{
		BYTE ifr = m_interruptFlags;
		bool any = ((ifr & m_interruptEnable) != 0);
		emul::SetBit(ifr, 7, any);
		return ifr;
	}

	void Device6522::Tick()
	{
		if (m_portA.GetC1())
		{
			LogPrintf(LOG_TRACE, "CA1 Interrupt set");
			m_interrupt.SetInterrupt(InterruptFlag::CA1);
		}
		if (m_portB.GetC1())
		{
			LogPrintf(LOG_TRACE, "CB1 Interrupt set");
			m_interrupt.SetInterrupt(InterruptFlag::CB1);
		}

		if (TIMER1.Tick())
		{
			LogPrintf(LOG_DEBUG, "Timer1 triggered");
			if (TIMER1.IsArmed())
			{
				m_interrupt.SetInterrupt(InterruptFlag::TIMER1);
				if (ACR.GetTimer1Mode() == AuxControl::T1Mode::ONE_SHOT)
				{
					TIMER1.Disarm();
				}
			}
		}
		if (TIMER2.Tick())
		{
			LogPrintf(LOG_DEBUG, "Timer2 triggered");
			if (TIMER2.IsArmed())
			{
				m_interrupt.SetInterrupt(InterruptFlag::TIMER2);
				TIMER2.Disarm();
			}

			if (ACR.GetShiftRegisterMode() == ShiftRegisterMode::SHIFT_OUT_T2_FREE)
			{
				// TODO: Should pulse CB1 but I don't think this is used much
				// On the PET CB1 is tape input
				Shift();
			}
		}
	}

	void Device6522::Shift()
	{
		LogPrintf(LOG_DEBUG, "[%zu] SHIFT %02X", emul::g_ticks, m_shiftRegister);
		// TODO: Only free-running mode now
		bool msb = emul::GetMSB(m_shiftRegister);
		m_portB.SetC2(msb);
		m_shiftRegister <<= 1;
		emul::SetBit(m_shiftRegister, 0, msb);
	}

	void Device6522::Interrupt::Serialize(json& to)
	{
		to["IER"] = m_interruptEnable;
		to["IFR"] = m_interruptFlags;
	}
	void Device6522::Interrupt::Deserialize(const json& from)
	{
		m_interruptEnable = from["IER"];
		m_interruptFlags = from["IFR"];
	}

	void Device6522::Serialize(json& to)
	{
		m_portA.Serialize(to["portA"]);
		m_portB.Serialize(to["portB"]);

		to["SR"] = m_shiftRegister;

		TIMER1.Serialize(to["timer1"]);
		TIMER2.Serialize(to["timer2"]);

		m_interrupt.Serialize(to["interrupt"]);

		to["PCR"] = PCR.Get();
		to["ACR"] = ACR.Get();
	}

	void Device6522::Deserialize(const json& from)
	{
		m_portA.Deserialize(from["portA"]);
		m_portB.Deserialize(from["portB"]);

		m_shiftRegister = from["SR"];

		TIMER1.Deserialize(from["timer1"]);
		TIMER2.Deserialize(from["timer2"]);

		m_interrupt.Deserialize(from["interrupt"]);
		UpdateIER();
		UpdateIFR();

		PCR.Set(from["PCR"]);
		UpdatePCR();
		ACR.Set(from["ACR"]);
		UpdateACR();
	}
}
