#pragma once
#include <functional>
#include <map>
#include <tuple>

namespace emul
{
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

		//InputPortMap& GetInputPorts() { return m_inputPorts; }
		//OutputPortMap& GetOutputPorts() { return m_outputPorts; }
	
	protected:
		bool Connect(WORD portNb, INFunction inFunc);
		bool Connect(WORD portNb, OUTFunction outFunc);

		bool DisconnectInput(WORD portNb);
		bool DisconnectOutput(WORD portNb);

	protected:
		static OutputPortMap m_outputPorts;
		static InputPortMap m_inputPorts;
	};
}
