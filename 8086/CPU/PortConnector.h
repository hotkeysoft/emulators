#pragma once
#include "../Common.h"
#include <Logger.h>
#include <functional>
#include <map>
#include <tuple>

namespace emul
{
	class PortAggregator;

	class PortConnector : virtual public Logger
	{
	public:
		PortConnector();
		virtual ~PortConnector();

		typedef void (PortConnector::* OUTFunction)(BYTE);
		typedef BYTE(PortConnector::* INFunction)();

		typedef std::map<WORD, std::tuple<PortConnector*, OUTFunction> > OutputPortMap;
		typedef std::map<WORD, std::tuple<PortConnector*, INFunction > > InputPortMap;

		bool In(WORD port, BYTE& value);
		bool Out(WORD port, BYTE value);

		InputPortMap& GetInputPorts() { return m_inputPorts; }
		OutputPortMap& GetOutputPorts() { return m_outputPorts; }
	
		virtual bool ConnectTo(PortAggregator& dest);
	protected:
		bool Connect(WORD portNb, INFunction inFunc);
		bool Connect(WORD portNb, OUTFunction outFunc);

	protected:
		OutputPortMap m_outputPorts;
		InputPortMap m_inputPorts;
	};
}
