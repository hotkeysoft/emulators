#pragma once
#include "Common.h"
#include "Logger.h"
#include <functional>
#include <map>
#include <tuple>

class PortConnector : virtual public Logger
{
public:
	PortConnector();
	virtual ~PortConnector();

	typedef void (PortConnector::*OUTFunction)(BYTE);
	typedef BYTE (PortConnector::*INFunction)();


	typedef std::map<BYTE, std::tuple<PortConnector*, OUTFunction> > OutputPortMap;
	typedef std::map<BYTE, std::tuple<PortConnector*, INFunction > > InputPortMap;

	bool In(BYTE port, BYTE &value);
	bool Out(BYTE port, BYTE value);

	InputPortMap& GetInputPorts() { return m_inputPorts; }
	OutputPortMap& GetOutputPorts() { return m_outputPorts; }

protected:
	bool Connect(BYTE portNb, INFunction inFunc);
	bool Connect(BYTE portNb, OUTFunction outFunc);

protected:
	OutputPortMap m_outputPorts;
	InputPortMap m_inputPorts;
};

