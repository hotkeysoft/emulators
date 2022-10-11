#include "stdafx.h"

#include "Device6522.h"

using emul::GetMSB;

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
		default: throw std::exception("Not possible");
		}
	}

	VIAPort::VIAPort(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x0F) // Addresses 0-F are decoded by device
	{
	}

	void VIAPort::Init(bool isPortB)
	{
		Reset();

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

	void VIAPort::Reset()
	{
		DDR = 0;
		OR = 0;
		IR = 0xFF; // TODO: Assume pullups for now
		C1 = false;
		C2 = false;
	}

	BYTE VIAPort::ReadInputRegister()
	{
		BYTE value = IR;

		// For output pins, mix with output register
		value &= (~DDR); // Clear output pin data
		value |= (OR & DDR); // Set output pin data from OR

		LogPrintf(LOG_DEBUG, "Read InputRegister, value=%02X", value);
		return value;
	}
	void VIAPort::WriteOutputRegister(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write OutputRegister, value=%02X", value);
		OR = value;
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
		to["C1"] = C1;
		to["C2"] = C2;
	}

	void VIAPort::Deserialize(const json& from)
	{
		DDR = from["DDR"];
		IR = from["IR"];
		OR = from["OR"];
		C1 = from["C1"];
		C2 = from["C2"];
	}

	Device6522::Device6522(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x0F), // Addresses 0-F are decoded by device
		m_portA(id + ".A"),
		m_portB(id + ".B")
	{
	}

	void Device6522::Init()
	{
		Reset();

		m_portA.Init(false);
		m_portB.Init(true);

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
		Connect(0x8, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT2LatchesL));
		Connect(0x9, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT2CounterH));
		Connect(0xA, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteSR));
		Connect(0xB, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteACR));
		Connect(0xC, static_cast<IOConnector::WRITEFunction>(&Device6522::WritePCR));
		Connect(0xD, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteIFR));
		Connect(0xE, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteIER));
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

	// 6 - T1L-L: T1 Low-Order Latches
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

	// 7 - T1L-H: T1 High-Order Latches
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
	void Device6522::WriteT2LatchesL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2LatchesL, value=%02X", value);
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
		LogPrintf(LOG_WARNING, "ReadSR (Not implemented)");
		return 0xFF;
	}
	void Device6522::WriteSR(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteSR, value=%02X (Not implemented)", value);
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
		LogPrintf(LOG_DEBUG, "ReadIFR, value=%02X", value);
		return value;
	}
	void Device6522::WriteIFR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIFR, value=%02X", value);
		m_interrupt.SetIFR(value);
		UpdateIFR();
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
		LogPrintf(LOG_INFO, " T2 Mode         : %s", ACR.GetTimer2Mode() == AuxControl::T2Mode::TIMED_INTERRUPT ? "INTERRUPT" : "PULSE PB6");

		LogPrintf(LOG_INFO, " Shift Reg Mode  : %d", ACR.GetShiftRegisterMode());

		LogPrintf(LOG_INFO, " PortA Latching  : %d", ACR.GetPortALatchingEnabled());
		LogPrintf(LOG_INFO, " PortB Latching  : %d", ACR.GetPortBLatchingEnabled());

		// Not much implemented atm
		if (ACR.GetPB7TimerOutput())
		{
			LogPrintf(LOG_WARNING, "ACR: T1 Output on PB7 not implemented");
		}
		if (ACR.GetTimer1Mode() == AuxControl::T1Mode::CONTINUOUS)
		{
			LogPrintf(LOG_WARNING, "ACR: T1 Continuous not implemented");
		}
		if (ACR.GetTimer2Mode() == AuxControl::T2Mode::PULSE_PB6)
		{
			LogPrintf(LOG_WARNING, "ACR: T2 Pulse Mode not implemented");
		}
		if (ACR.GetShiftRegisterMode() != AuxControl::SRMode::DISABLED)
		{
			LogPrintf(LOG_WARNING, "ACR: Shift Register not implemented");
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
		else if (m_counter == 0)
		{
			m_load = true;
			ret = m_armed; // Triggers interrupt only once
			m_armed = false;
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
		BYTE ifr = (m_interruptFlags & m_interruptEnable);
		bool any = (ifr != 0);
		emul::SetBit(ifr, 7, any);
		return ifr;
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
