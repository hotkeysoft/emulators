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
		//CR.data = 0;
		//CR.ClearIRQFlags();
		//ISC = 0;
	}

	// 0 - Read PIBA/DDRA
	//BYTE Device6846::ReadPortData()
	//{
	//	// Reading the output register resets IRQ1 & IRQ2
	//	// of the control register
	//	CR.ClearIRQFlags();

	//	m_parent->OnReadPort(this);
	//	BYTE value = IR;

	//	// For output pins, mix with output register
	//	value &= (~DDR); // Clear output pin data
	//	value |= (OR & DDR); // Set output pin data from OR

	//	LogPrintf(LOG_DEBUG, "Read PortData, value=%02X", value);
	//	return value;
	//}

	BYTE Device6846::ReadStatus()
	{
		LogPrintf(LOG_DEBUG, "ReadStatus");
		return 0xFF;
	}

	BYTE Device6846::ReadControl()
	{
		LogPrintf(LOG_DEBUG, "ReadControl");
		return 0xFF;
	}
	void Device6846::WriteControl(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteControl, value=%02x", value);
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
		//// Reading the output register resets IRQ1 & IRQ2
		//// of the control register
		//CR.ClearIRQFlags();

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
		LogPrintf(LOG_DEBUG, "ReadTimerControl");
		return 0xFF;
	}
	void Device6846::WriteTimerControl(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteTimerControl, value=%02x", value);
	}

	BYTE Device6846::ReadTimerMSB()
	{
		LogPrintf(LOG_DEBUG, "ReadTimerMSB");
		return 0xFF;
	}
	void Device6846::WriteTimerMSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteTimerMSB, value=%02x", value);
	}

	BYTE Device6846::ReadTimerLSB()
	{
		LogPrintf(LOG_DEBUG, "ReadTimerLSB");
		return 0xFF;
	}
	void Device6846::WriteTimerLSB(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteTimerLSB, value=%02x", value);
	}


	//// 1 - Read/Write ControlRegister
	//BYTE Device6846::ReadControlRegister()
	//{
	//	BYTE value = CR.data |
	//		(CR.GetIRQ2Flag() << 6) |
	//		(CR.GetIRQ1Flag() << 7);
	//	LogPrintf(LOG_TRACE, "Read ControlRegister, value=%02X", value);

	//	// The most common reason to read this is for irq status
	//	LogPrintf(LOG_DEBUG, "Read ControlRegister [%cIRQ1] [%cIRQ2]",
	//		CR.GetIRQ1Flag() ? ' ' : '/',
	//		CR.GetIRQ2Flag() ? ' ' : '/');

	//	if (IsLog(LOG_TRACE))
	//	{
	//		LogPrintf(LOG_DEBUG, "Read ControlRegister:");
	//		LogControlRegister();
	//	}

	//	return value;
	//}
	//void Device6846::WriteControlRegister(BYTE value)
	//{
	//	LogPrintf(LOG_DEBUG, "WriteControlRegister, value=%02X", value);
	//	CR.data = value & 0b00111111;

	//	if (IsLog(LOG_INFO))
	//	{
	//		LogPrintf(LOG_INFO, "Set ControlRegister:");
	//		LogControlRegister();
	//	}

	//	CR.IRQ1Latch.SetTrigger(CR.GetIRQ1PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);
	//	CR.IRQ2Latch.SetTrigger(CR.GetIRQ2PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE );
	//}

	//void Device6846::LogControlRegister()
	//{
	//	LogPrintf(LOG_INFO, " IRQ1 flag: %d", CR.GetIRQ1Flag());
	//	LogPrintf(LOG_INFO, " IRQ2 flag: %d", CR.GetIRQ2Flag());

	//	if (CR.GetC2OutputMode())
	//	{
	//		LogPrintf(LOG_INFO, " C2 pin: OUT");
	//		LogPrintf(LOG_INFO, " C2 output ctrl : %d", CR.GetC2OutputControl());
	//		LogPrintf(LOG_INFO, " C2 output      : %d", CR.GetC2Output());

	//		if (CR.GetC2OutputControl() == false)
	//		{
	//			LogPrintf(LOG_ERROR, "C2 output ctrl = 0 not supported");
	//		}
	//	}
	//	else
	//	{
	//		LogPrintf(LOG_INFO, " C2 pin: IN");
	//		LogPrintf(LOG_INFO, " IRQ2 pos transition    : %d", CR.GetIRQ2PositiveTransition());
	//		LogPrintf(LOG_INFO, " CPU IRQ Enable for IRQ2: %d", CR.GetCPUIRQEnableForIRQ2());
	//	}

	//	LogPrintf(LOG_INFO, " DDR/Output Select      : %s", CR.GetORSelect() ? "OUTPUT" : "DIRECTION");

	//	LogPrintf(LOG_INFO, " IRQ1 pos transition    : %d", CR.GetIRQ1PositiveTransition());
	//	LogPrintf(LOG_INFO, " CPU IRQ Enable for IRQ1: %d", CR.GetCPUIRQEnableForIRQ1());

	//}

	//void Device6846::ControlRegister::Serialize(json& to)
	//{
	//	to["data"] = data;
	//	IRQ1Latch.Serialize(to["IRQ1"]);
	//	IRQ2Latch.Serialize(to["IRQ2"]);
	//}
	//void Device6846::ControlRegister::Deserialize(const json& from)
	//{
	//	data = from["data"];
	//	IRQ1Latch.Deserialize(from["IRQ1"]);
	//	IRQ2Latch.Deserialize(from["IRQ2"]);
	//}

	void Device6846::Serialize(json& to)
	{
		//to["DDR"] = DDR;
		//to["IR"] = IR;
		//to["OR"] = OR;
		//to["ISC"] = ISC;
		//CR.Serialize(to["CR"]);
	}

	void Device6846::Deserialize(const json& from)
	{
		//DDR = from["DDR"];
		//IR = from["IR"];
		//OR = from["OR"];
		//ISC = from["ISC"];
		//CR.Deserialize(from["CR"]);
	}
}
