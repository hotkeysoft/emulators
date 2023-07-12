#include "stdafx.h"

#include "Device8255.h"

namespace ppi
{
	Device8255::Device8255() :
		Logger("8255")
	{
	}

	void Device8255::Reset()
	{
		LogPrintf(LOG_DEBUG, "Reset");
		SetControlWord(DEFAULT_CONTROLWORD);
		m_portAData = 0;
		m_portBData = 0;
		m_portCData = 0;
	}

	// TODO: Combine both Init() functions
	void Device8255::Init(WORD basePort)
	{
		Reset();

		Connect(basePort + 0, static_cast<PortConnector::INFunction>(&Device8255::PORTA_IN));
		Connect(basePort + 0, static_cast<PortConnector::OUTFunction>(&Device8255::PORTA_OUT));

		Connect(basePort + 1, static_cast<PortConnector::INFunction>(&Device8255::PORTB_IN));
		Connect(basePort + 1, static_cast<PortConnector::OUTFunction>(&Device8255::PORTB_OUT));

		Connect(basePort + 2, static_cast<PortConnector::INFunction>(&Device8255::PORTC_IN));
		Connect(basePort + 2, static_cast<PortConnector::OUTFunction>(&Device8255::PORTC_OUT));

		Connect(basePort + 3, static_cast<PortConnector::INFunction>(&Device8255::CONTROL_IN));
		Connect(basePort + 3, static_cast<PortConnector::OUTFunction>(&Device8255::CONTROL_OUT));
	}

	void Device8255::Init(emul::BitMaskB mask)
	{
		Connect(mask.Merge("00"), static_cast<PortConnector::INFunction>(&Device8255::PORTA_IN));
		Connect(mask.Merge("00"), static_cast<PortConnector::OUTFunction>(&Device8255::PORTA_OUT));

		Connect(mask.Merge("01"), static_cast<PortConnector::INFunction>(&Device8255::PORTB_IN));
		Connect(mask.Merge("01"), static_cast<PortConnector::OUTFunction>(&Device8255::PORTB_OUT));

		Connect(mask.Merge("10"), static_cast<PortConnector::INFunction>(&Device8255::PORTC_IN));
		Connect(mask.Merge("10"), static_cast<PortConnector::OUTFunction>(&Device8255::PORTC_OUT));

		Connect(mask.Merge("11"), static_cast<PortConnector::INFunction>(&Device8255::CONTROL_IN));
		Connect(mask.Merge("11"), static_cast<PortConnector::OUTFunction>(&Device8255::CONTROL_OUT));
	}

	BYTE Device8255::CONTROL_IN()
	{
		LogPrintf(LOG_DEBUG, "GET Control word, ret=%02X", m_controlWord);
		return m_controlWord | CTRL_MODESETFLAG;
	}
	void Device8255::CONTROL_OUT(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "SET Control word, value=%02X", value);
		SetControlWord(value);
	}

	void Device8255::SetControlWord(BYTE ctrl)
	{
		m_controlWord = ctrl;
		if (!(ctrl | CTRL_MODESETFLAG))
		{
			throw std::exception("Single bit set not implemented");
		}

		switch (ctrl & (CTRL_GA_MODE2 | CTRL_GA_MODE1))
		{
		case 0:
			LogPrintf(LOG_DEBUG, "GROUP A   : MODE 0");
			break;
		case CTRL_GA_MODE1:
			LogPrintf(LOG_DEBUG, "GROUP A   : MODE 1");
			throw std::exception("GROUP A MODE 1 not implemented");
		default:
			LogPrintf(LOG_DEBUG, "GROUP A   : MODE 2");
			throw std::exception("GROUP A MODE 2 not implemented");
		}

		if (ctrl & CTRL_GB_MODE1)
		{
			LogPrintf(LOG_DEBUG, "GROUP B   : MODE 1");
			throw std::exception("GROUP B MODE 1 not implemented");
		}
		else
		{
			LogPrintf(LOG_DEBUG, "GROUP B   : MODE 0");
		}

		// Assume mode 0 for now

		m_portADirection = (m_controlWord & CTRL_GA_A_DIR) ? DIRECTION::INPUT : DIRECTION::OUTPUT;
		m_portBDirection = (m_controlWord & CTRL_GB_B_DIR) ? DIRECTION::INPUT : DIRECTION::OUTPUT;
		m_portCHDirection = (m_controlWord & CTRL_GA_C_DIR_H) ? DIRECTION::INPUT : DIRECTION::OUTPUT;
		m_portCLDirection = (m_controlWord & CTRL_GB_C_DIR_L) ? DIRECTION::INPUT : DIRECTION::OUTPUT;

		LogPrintf(LOG_DEBUG, "PORT A    : %s", GetPortDirectionStr(m_portADirection));
		LogPrintf(LOG_DEBUG, "PORT B    : %s", GetPortDirectionStr(m_portBDirection));
		LogPrintf(LOG_DEBUG, "PORT C(hi): %s", GetPortDirectionStr(m_portCHDirection));
		LogPrintf(LOG_DEBUG, "PORT C(lo): %s", GetPortDirectionStr(m_portCLDirection));

		//m_portAData = 0;
		//m_portBData = 0;
		//m_portCData = 0;

		return;
	}

	bool Device8255::IsSoundON()
	{
		return (m_portBData & 2);
	}
}
