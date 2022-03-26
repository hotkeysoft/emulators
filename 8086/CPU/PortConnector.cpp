#include "stdafx.h"

#include "PortConnector.h"
#include "../Config.h"

namespace emul
{
	PortConnector::OutputPortMap PortConnector::m_outputPorts;
	PortConnector::InputPortMap PortConnector::m_inputPorts;

	PortConnector::PortConnector() : Logger("PORT")
	{
	}

	PortConnector::~PortConnector()
	{
	}

	void PortConnector::Clear()
	{
		m_inputPorts.clear();
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

		m_inputPorts[portNb] = std::make_tuple(this, inFunc);

		return true;
	}

	bool PortConnector::Connect(WORD portNb, OUTFunction outFunc, bool share)
	{
		LogPrintf(LOG_INFO, "Connect output port 0x%04X", portNb);

		if (!share && m_outputPorts.count(portNb) > 0)
		{
			LogPrintf(LOG_ERROR, "Port already exists");
			return false;
		}

		m_outputPorts.insert(std::make_pair(portNb, std::make_tuple(this, outFunc)));

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
			LogPrintf(LOG_DEBUG, "Disconnect input port 0x%04X (not connected)", portNb);
			return false;
		}
	}

	bool PortConnector::DisconnectOutput(WORD portNb)
	{	
		size_t count = m_outputPorts.erase(portNb);
		if (count > 1)
		{
			LogPrintf(LOG_WARNING, "Multiple listeners disconnected at port 0x%04X", portNb);
			return true;
		}
		else if (count == 1)
		{
			LogPrintf(LOG_INFO, "Disconnect output port 0x%04X", portNb);
			return true;
		}
		else
		{
			LogPrintf(LOG_DEBUG, "Disconnect output port 0x%04X (not connected)", portNb);
			return false;
		}
	}

	bool PortConnector::In(WORD port, BYTE& value)
	{
		auto it = m_inputPorts.find(port);
		if (it == m_inputPorts.end())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "PortConnector::In: port 0x%04X not allocated", port);
#endif
			value = 0xFF;
			return false;
		}

		auto inFunc = it->second;
		PortConnector* owner = std::get<0>(inFunc);
		INFunction& func = std::get<1>(inFunc);

		value = (owner->*func)();
		return true;
	}

	bool PortConnector::Out(WORD port, BYTE value)
	{
#ifdef _DEBUG
		if (m_outputPorts.count(port) == 0)
		{
			LogPrintf(LOG_WARNING, "PortConnector::Out(0x%04X, 0x%02X): port not allocated", port, value);
			return false;
		}
#endif

		auto listeners = m_outputPorts.equal_range(port);
		std::for_each(listeners.first, listeners.second, [value](const auto& listener)
		{
			auto outFunc = listener.second;

			PortConnector* owner = std::get<0>(outFunc);
			OUTFunction& func = std::get<1>(outFunc);

			(owner->*func)(value);
		});

		return true;
	}
}