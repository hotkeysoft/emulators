#include "PortConnector.h"

PortConnector::PortConnector() : Logger("PORT")
{
}

PortConnector::~PortConnector()
{
}

bool PortConnector::Connect(BYTE portNb, INFunction inFunc)
{
	LogPrintf(LOG_INFO, "Connect input port 0x%02X", portNb);

	auto it = m_inputPorts.find(portNb);
	if (it != m_inputPorts.end())
	{
		LogPrintf(LOG_ERROR, "Port already exists");
		return false;
	}

	m_inputPorts[portNb] = std::make_tuple(this, inFunc);

	return true;
}

bool PortConnector::Connect(BYTE portNb, OUTFunction outFunc)
{
	LogPrintf(LOG_INFO, "Connect output port 0x%02X", portNb);

	auto it = m_outputPorts.find(portNb);
	if (it != m_outputPorts.end())
	{
		LogPrintf(LOG_ERROR, "Port already exists");
		return false;
	}

	m_outputPorts[portNb] = std::make_tuple(this, outFunc);

	return true;
}

bool PortConnector::In(BYTE port, BYTE & value)
{
	auto it = m_inputPorts.find(port);
	if (it == m_inputPorts.end())
	{
		LogPrintf(LOG_WARNING, "PortConnector::In: port 0x%02X not allocated", port);
		return false;
	}

	auto inFunc = it->second;
	PortConnector* owner = std::get<0>(inFunc);
	INFunction& func = std::get<1>(inFunc);

	value = (owner->*func)();
	return true;
}

bool PortConnector::Out(BYTE port, BYTE value)
{
	auto it = m_outputPorts.find(port);
	if (it == m_outputPorts.end())
	{
		LogPrintf(LOG_WARNING, "PortConnector::Out: port 0x%02X not allocated", port);
		return false;
	}

	auto outFunc = it->second;
	PortConnector* owner = std::get<0>(outFunc);
	OUTFunction& func = std::get<1>(outFunc);

	(owner->*func)(value);

	return true;
}
