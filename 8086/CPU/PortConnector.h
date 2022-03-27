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

		static void Clear();

		typedef void (PortConnector::* OUTFunction)(BYTE);
		typedef BYTE(PortConnector::* INFunction)();

		using InputPortMap = std::map<WORD, std::tuple<PortConnector*, INFunction > >;
		using OutputPortMap = std::multimap<WORD, std::tuple<PortConnector*, OUTFunction> >;

		bool In(WORD port, BYTE& value);
		bool Out(WORD port, BYTE value);
	
	protected:
		bool Connect(WORD portNb, INFunction inFunc);
		bool Connect(WORD portNb, OUTFunction outFunc, bool share = false);

		bool DisconnectInput(WORD portNb);
		bool DisconnectOutput(WORD portNb);

		WORD GetCurrentPort() const { return m_currentPort; }

	protected:
		static OutputPortMap m_outputPorts;
		static InputPortMap m_inputPorts;

	private:
		WORD m_currentPort;
	};
}
