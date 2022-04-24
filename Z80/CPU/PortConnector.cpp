#include "stdafx.h"

#include "PortConnector.h"
#include "../Config.h"

namespace emul
{
	PortConnector::OutputPortMap PortConnector::m_outputPorts;
	PortConnector::InputPortMap PortConnector::m_inputPorts;
	WORD PortConnector::m_currentPort;

	PortConnector::PortHandler::~PortHandler()
	{
		PortHandler* handler = this->chained;
		while (handler)
		{
			PortHandler* toDelete = handler;
			handler = handler->chained;
			delete (toDelete);
		}		
	}

	void PortConnector::PortHandler::Chain(PortHandler chained)
	{
		PortHandler* handler = this;
		while (handler->chained)
		{
			handler = handler->chained;
		}
		handler->chained = new PortHandler(chained);
	}

	PortConnector::PortConnector() : Logger("PORT")
	{
	}

	PortConnector::~PortConnector()
	{
	}

	void PortConnector::Clear()
	{
		m_inputPorts.clear();
		m_outputPorts.reserve(200);
		m_outputPorts.clear();
	}

	bool PortConnector::Connect(WORD portNb, INFunction inFunc)
	{
		LogPrintf(LOG_INFO, "Connect input port 0x%04X", portNb);		
		
		if (m_inputPorts.find(portNb) != m_inputPorts.end())
		{
			LogPrintf(LOG_ERROR, "Port already exists");
			return false;
		}

		m_inputPorts.emplace(portNb, PortHandler(this, inFunc));

		return true;
	}

	bool PortConnector::Connect(WORD portNb, OUTFunction outFunc, bool share)
	{
		LogPrintf(LOG_INFO, "Connect output port 0x%04X", portNb);

		OutputPortMap::iterator it = m_outputPorts.find(portNb);
		if (it != m_outputPorts.end())
		{
			if (share)
			{
				LogPrintf(LOG_INFO, "Chaining output port 0x%04X", portNb);
				it->second.Chain(PortHandler(this, outFunc));
			}
			else
			{
				LogPrintf(LOG_ERROR, "Port already exists");
				return false;
			}
		}

		m_outputPorts.emplace(portNb, PortHandler(this, outFunc));

		return true;
	}

	bool PortConnector::DisconnectInput(WORD portNb)
	{
		if (m_inputPorts.erase(portNb))
		{
			LogPrintf(LOG_INFO, "Disconnect input port 0x%04X", portNb);
			return true;
		}
		else
		{
			LogPrintf(LOG_INFO, "Disconnect input port 0x%04X (not connected)", portNb);
			return false;
		}
	}

	bool PortConnector::DisconnectOutput(WORD portNb)
	{
		if (m_outputPorts.erase(portNb))
		{
			LogPrintf(LOG_INFO, "Disconnect output port 0x%04X", portNb);
			return true;
		}
		else
		{
			LogPrintf(LOG_INFO, "Disconnect output port 0x%04X (not connected)", portNb);
			return false;
		}
	}

	bool PortConnector::In(WORD port, BYTE& value)
	{
		m_currentPort = port;
		auto it = m_inputPorts.find(port);
		if (it == m_inputPorts.end())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "PortConnector::In: port 0x%04X not allocated", port);
#endif
			value = 0xFF;
			return false;
		}

		value = it->second.In();
		return true;
	}

	bool PortConnector::Out(WORD port, BYTE value)
	{
		m_currentPort = port;
		auto it = m_outputPorts.find(port);
		if (it == m_outputPorts.end())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "PortConnector::Out(0x%04X, 0x%02X): port not allocated", port, value);
#endif
			return false;
		}

		it->second.Out(value);
		return true;
	}
}