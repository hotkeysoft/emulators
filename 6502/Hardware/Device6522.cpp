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
			LogPrintf(LOG_ERROR, "CA2 Operation Not supported: %s", GetC2OperationStr(op));
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
		Connect(0x6, static_cast<IOConnector::READFunction>(&Device6522::ReadT1LatchesL));
		Connect(0x7, static_cast<IOConnector::READFunction>(&Device6522::ReadT1LatchesH));
		Connect(0x8, static_cast<IOConnector::READFunction>(&Device6522::ReadT2CounterL));
		Connect(0x9, static_cast<IOConnector::READFunction>(&Device6522::ReadT2CounterH));
		Connect(0xA, static_cast<IOConnector::READFunction>(&Device6522::ReadSR));
		Connect(0xB, static_cast<IOConnector::READFunction>(&Device6522::ReadACR));
		Connect(0xC, static_cast<IOConnector::READFunction>(&Device6522::ReadPCR));
		Connect(0xD, static_cast<IOConnector::READFunction>(&Device6522::ReadIFR));
		Connect(0xE, static_cast<IOConnector::READFunction>(&Device6522::ReadIER));

		Connect(0x4, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchesL));
		Connect(0x5, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1CounterH));
		Connect(0x6, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchesL));
		Connect(0x7, static_cast<IOConnector::WRITEFunction>(&Device6522::WriteT1LatchesH));
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
		m_portA.Reset();
		m_portB.Reset();

		IER.Clear();
	}

	// 4 - T1C-L: T1 Low-Order Counter
	BYTE Device6522::ReadT1CounterL()
	{
		LogPrintf(LOG_DEBUG, "ReadT1CounterL");
		return 0xFF;
	}

	// 5 - T1C-H: T1 High-Order Counter
	BYTE Device6522::ReadT1CounterH()
	{
		LogPrintf(LOG_DEBUG, "ReadT1CounterH");
		return 0xFF;
	}
	void Device6522::WriteT1CounterH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1CounterH, value=%02X", value);
	}

	// 6 - T1L-L: T1 Low-Order Latches
	BYTE Device6522::ReadT1LatchesL()
	{
		LogPrintf(LOG_DEBUG, "ReadT1LatchesL");
		return 0xFF;
	}
	void Device6522::WriteT1LatchesL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1LatchesL, value=%02X", value);
	}

	// 7 - T1L-H: T1 High-Order Latches
	BYTE Device6522::ReadT1LatchesH()
	{
		LogPrintf(LOG_DEBUG, "ReadT1LatchesH");
		return 0xFF;
	}
	void Device6522::WriteT1LatchesH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT1LatchesH, value=%02X", value);
	}

	// 8 - T2C-L: T2 Low-Order Counter
	BYTE Device6522::ReadT2CounterL()
	{
		LogPrintf(LOG_DEBUG, "ReadT2CounterL");
		return 0xFF;
	}
	void Device6522::WriteT2LatchesL(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2LatchesL, value=%02X", value);
	}

	// 9 - T2C-H: T2 High-Order Counter
	BYTE Device6522::ReadT2CounterH()
	{
		LogPrintf(LOG_DEBUG, "ReadT2CounterH");
		return 0xFF;
	}
	void Device6522::WriteT2CounterH(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteT2CounterH, value=%02X", value);
	}

	// A - SR: Shift Register
	BYTE Device6522::ReadSR()
	{
		LogPrintf(LOG_DEBUG, "ReadSR");
		return 0xFF;
	}
	void Device6522::WriteSR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteSR, value=%02X", value);
	}

	// B - ACR: Auxiliary Control Register
	BYTE Device6522::ReadACR()
	{
		LogPrintf(LOG_DEBUG, "ReadACR");
		return 0xFF;
	}
	void Device6522::WriteACR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteACR, value=%02X", value);
	}

	// C - PCR: Peripheral Control Register
	BYTE Device6522::ReadPCR()
	{
		bool value = PCR.data;
		LogPrintf(LOG_DEBUG, "ReadPCR, value=%02X", value);
		return value;
	}
	void Device6522::WritePCR(BYTE value)
	{
		PCR.data = value;
		LogPrintf(LOG_DEBUG, "WritePCR, value=%02X", value);

		LogPrintf(LOG_INFO, "Write Peripheral Control Register");
		LogPrintf(LOG_INFO, " CA1 Interrupt active edge: %s", PCR.GetCA1InterruptActiveEdge() == ActiveEdge::NEG_EDGE ? "NEG" : "POS");
		LogPrintf(LOG_INFO, " CA2 Operation: %s", GetC2OperationStr(PCR.GetCA2Operation()));

		LogPrintf(LOG_INFO, " CB1 Interrupt active edge: %s", PCR.GetCB1InterruptActiveEdge() == ActiveEdge::NEG_EDGE ? "NEG" : "POS");
		LogPrintf(LOG_INFO, " CB2 Operation: %s", GetC2OperationStr(PCR.GetCB2Operation()));

		m_portA.SetC2Operation(PCR.GetCA2Operation());
		m_portB.SetC2Operation(PCR.GetCB2Operation());
	}

	// D - IFR: Interrupt Flag Register
	BYTE Device6522::ReadIFR()
	{
		LogPrintf(LOG_DEBUG, "ReadIFR");
		return 0xFF;
	}
	void Device6522::WriteIFR(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIFR, value=%02X", value);
}

	// E - IER: Interrupt Enable Register
	BYTE Device6522::ReadIER()
	{
		BYTE value = IER.data;
		LogPrintf(LOG_DEBUG, "ReadIER, value=%02X", value);
		return value;
	}
	void Device6522::WriteIER(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteIER, value=%02X", value);
		IER.Set(value);

		LogPrintf(LOG_INFO, "Set Interrupt Register Mask [%cTI1 %cTI2 %cCB1 %cCB2 %cSR %cCA1 %cCA2]",
			(IER.IsInterruptEnabled(InterruptEnable::TIMER1) ? ' ' : '/'),
			(IER.IsInterruptEnabled(InterruptEnable::TIMER2) ? ' ' : '/'),
			(IER.IsInterruptEnabled(InterruptEnable::CB1) ? ' ' : '/'),
			(IER.IsInterruptEnabled(InterruptEnable::CB2) ? ' ' : '/'),
			(IER.IsInterruptEnabled(InterruptEnable::SR) ? ' ' : '/'),
			(IER.IsInterruptEnabled(InterruptEnable::CA1) ? ' ' : '/'),
			(IER.IsInterruptEnabled(InterruptEnable::CA2) ? ' ' : '/')
		);
	}

	void Device6522::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_portA.EnableLog(minSev);
		m_portB.EnableLog(minSev);
	}

	void Device6522::Serialize(json& to)
	{
		m_portA.Serialize(to["portA"]);
		m_portB.Serialize(to["portB"]);

		to["PCR"] = PCR.data;
		to["IER"] = IER.data;
	}

	void Device6522::Deserialize(const json& from)
	{
		m_portA.Deserialize(from["portA"]);
		m_portB.Deserialize(from["portB"]);

		PCR.data = from["PCR"];
		IER.data = from["IER"];

	}
}
