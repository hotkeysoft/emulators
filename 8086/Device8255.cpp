#include "Device8255.h"

namespace ppi
{
	Device8255::Device8255(WORD baseAddress) : 
		Logger("8255"), 
		m_baseAddress(baseAddress)
	{
		Reset();
	}

	void Device8255::Reset()
	{
		LogPrintf(LOG_INFO, "Reset");
		SetControlWord(DEFAULT_CONTROLWORD);
		m_portAData = 0;
		m_portBData = 0;
		m_portCData = 0;
	}

	void Device8255::Init()
	{
		Connect(m_baseAddress + 0, static_cast<PortConnector::INFunction>(&Device8255::PORTA_IN));
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device8255::PORTA_OUT));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device8255::PORTB_IN));
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device8255::PORTB_OUT));

		Connect(m_baseAddress + 2, static_cast<PortConnector::INFunction>(&Device8255::PORTC_IN));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&Device8255::PORTC_OUT));

		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&Device8255::CONTROL_IN));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device8255::CONTROL_OUT));
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
			LogPrintf(LOG_INFO, "GROUP A   : MODE 0");
			break;
		case CTRL_GA_MODE1:
			LogPrintf(LOG_INFO, "GROUP A   : MODE 1");
			throw std::exception("GROUP A MODE 1 not implemented");
		default:
			LogPrintf(LOG_INFO, "GROUP A   : MODE 2");
			throw std::exception("GROUP A MODE 2 not implemented");
		}

		if (ctrl & CTRL_GB_MODE1)
		{
			LogPrintf(LOG_INFO, "GROUP B   : MODE 1");
			throw std::exception("GROUP B MODE 1 not implemented");
		}
		else
		{
			LogPrintf(LOG_INFO, "GROUP B   : MODE 0");
		}

		// Assume mode 0 for now

		m_portADirection = (m_controlWord & CTRL_GA_A_DIR) ? DIRECTION::INPUT : DIRECTION::OUTPUT;
		m_portBDirection = (m_controlWord & CTRL_GB_B_DIR) ? DIRECTION::INPUT : DIRECTION::OUTPUT;
		m_portCHDirection = (m_controlWord & CTRL_GA_C_DIR_H) ? DIRECTION::INPUT : DIRECTION::OUTPUT;
		m_portCLDirection = (m_controlWord & CTRL_GB_C_DIR_L) ? DIRECTION::INPUT : DIRECTION::OUTPUT;

		LogPrintf(LOG_INFO, "PORT A    : %s", GetPortDirectionStr(m_portADirection));
		LogPrintf(LOG_INFO, "PORT B    : %s", GetPortDirectionStr(m_portBDirection));
		LogPrintf(LOG_INFO, "PORT C(hi): %s", GetPortDirectionStr(m_portCHDirection));
		LogPrintf(LOG_INFO, "PORT C(lo): %s", GetPortDirectionStr(m_portCLDirection));

		m_portAData = 0;
		m_portBData = 0;
		m_portCData = 0;

		return;
	}

	bool Device8255::IsSoundON()
	{
		return (m_portBData & 2);
	}
}
