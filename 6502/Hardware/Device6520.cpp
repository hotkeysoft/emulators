#include "stdafx.h"

#include "Device6520.h"

using hscommon::EdgeDetectLatch;

namespace pia
{
	PIAPort::PIAPort(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x03) // Addresses 0-3 are decoded by device
	{
	}

	void PIAPort::Init(Device6520* parent, bool isPortB)
	{
		assert(parent);
		m_parent = parent;

		Reset();

		WORD base = isPortB ? 2 : 0;
		Connect(base + 0, static_cast<IOConnector::READFunction>(&PIAPort::Read0));
		Connect(base + 1, static_cast<IOConnector::READFunction>(&PIAPort::ReadControlRegister));

		Connect(base + 0, static_cast<IOConnector::WRITEFunction>(&PIAPort::Write0));
		Connect(base + 1, static_cast<IOConnector::WRITEFunction>(&PIAPort::WriteControlRegister));
	}

	void PIAPort::Reset()
	{
		DDR = 0;
		OR = 0;
		IR = 0xFF; // TODO: Assume pullups for now
		CR.data = 0;
		CR.ClearIRQFlags();
		ISC = 0;
		C1 = false;
		C2 = false;
	}

	// 0 - Read PIBA/DDRA
	BYTE PIAPort::Read0()
	{
		LogPrintf(LOG_TRACE, "Read0");
		return CR.GetORSelect() ? ReadPortData() : ReadDataDirectionRegister();
	}
	BYTE PIAPort::ReadPortData()
	{
		// Reading the output register resets IRQ1 & IRQ2
		// of the control register
		CR.ClearIRQFlags();

		m_parent->OnReadPort(this);
		BYTE value = IR;

		// For output pins, mix with output register
		value &= (~DDR); // Clear output pin data
		value |= (OR & DDR); // Set output pin data from OR

		LogPrintf(LOG_DEBUG, "Read PortData, value=%02X", value);
		return value;
	}
	BYTE PIAPort::ReadDataDirectionRegister()
	{
		BYTE value = DDR;
		LogPrintf(LOG_DEBUG, "Read DataDirectionRegister, value=%02X", value);
		return value;
	}

	// 0 - Write OutputRegister/DataDirectionRegister
	void PIAPort::Write0(BYTE value)
	{
		LogPrintf(LOG_TRACE, "Write0, value=%02x", value);
		CR.GetORSelect() ? WriteOutputRegister(value) : WriteDataDirectionRegister(value);
	}
	void PIAPort::WriteOutputRegister(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write OutputRegister, value=%02X", value);
		OR = value;
	}
	void PIAPort::WriteDataDirectionRegister(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "Write DataDirectionRegister, value=%02X", value);
		DDR = value;
		LogPrintf(LOG_INFO, "Set DataDirection:");
		for (int i = 7; i >= 0; --i)
		{
			LogPrintf(LOG_INFO, " PIN %d: %s", i, (GetDataDirection(i) == DataDirection::INPUT) ? "IN" : "OUT");
		}
	}

	// 1 - Read/Write ControlRegister
	BYTE PIAPort::ReadControlRegister()
	{
		BYTE value = CR.data |
			(CR.GetIRQ2Flag() << 6) |
			(CR.GetIRQ1Flag() << 7);
		LogPrintf(LOG_TRACE, "Read ControlRegister, value=%02X", value);

		// The most common reason to read this is for irq status
		LogPrintf(LOG_DEBUG, "Read ControlRegister [%cIRQ1] [%cIRQ2]",
			CR.GetIRQ1Flag() ? ' ' : '/',
			CR.GetIRQ2Flag() ? ' ' : '/');

		if (IsLog(LOG_TRACE))
		{
			LogPrintf(LOG_DEBUG, "Read ControlRegister:");
			LogControlRegister();
		}

		return value;
	}
	void PIAPort::WriteControlRegister(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteControlRegister, value=%02X", value);
		CR.data = value & 0b00111111;

		if (IsLog(LOG_INFO))
		{
			LogPrintf(LOG_INFO, "Set ControlRegister:");
			LogControlRegister();
		}

		CR.IRQ1Latch.SetTrigger(CR.GetIRQ1PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE);
		CR.IRQ2Latch.SetTrigger(CR.GetIRQ2PositiveTransition() ? EdgeDetectLatch::Trigger::POSITIVE : EdgeDetectLatch::Trigger::NEGATIVE );
	}

	void PIAPort::LogControlRegister()
	{
		LogPrintf(LOG_INFO, " IRQ1 flag: %d", CR.GetIRQ1Flag());
		LogPrintf(LOG_INFO, " IRQ2 flag: %d", CR.GetIRQ2Flag());

		if (CR.GetC2OutputMode())
		{
			LogPrintf(LOG_INFO, " C2 pin: OUT");
			LogPrintf(LOG_INFO, " C2 output ctrl : %d", CR.GetC2OutputControl());
			LogPrintf(LOG_INFO, " C2 restore ctrl: %d", CR.GetC2RestoreControl());

		}
		else
		{
			LogPrintf(LOG_INFO, " C2 pin: IN");
			LogPrintf(LOG_INFO, " IRQ2 pos transition    : %d", CR.GetIRQ2PositiveTransition());
			LogPrintf(LOG_INFO, " CPU IRQ Enable for IRQ2: %d", CR.GetCPUIRQEnableForIRQ2());
		}

		LogPrintf(LOG_INFO, " DDR/Output Select      : %s", CR.GetORSelect() ? "OUTPUT" : "DIRECTION");

		LogPrintf(LOG_INFO, " IRQ1 pos transition    : %d", CR.GetIRQ1PositiveTransition());
		LogPrintf(LOG_INFO, " CPU IRQ Enable for IRQ1: %d", CR.GetCPUIRQEnableForIRQ1());

	}

	Device6520::Device6520(std::string id) :
		Logger(id.c_str()),
		IOConnector(0x03), // Addresses 0-3 are decoded by device
		m_portA(id + ".A"),
		m_portB(id + ".B")
	{
	}

	void Device6520::Init()
	{
		Reset();

		m_portA.Init(this, false);
		m_portB.Init(this, true);

		// Attach connections made in children objects
		Attach(m_portA);
		Attach(m_portB);
	}

	void Device6520::Reset()
	{
		m_portA.Reset();
		m_portB.Reset();
	}

	void Device6520::EnableLog(SEVERITY minSev)
	{
		Logger::EnableLog(minSev);
		m_portA.EnableLog(minSev);
		m_portB.EnableLog(minSev);
	}
}
