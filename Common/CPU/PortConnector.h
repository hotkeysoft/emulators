#pragma once
#include <functional>
#include <map>
#include <tuple>

namespace emul
{
	enum class PortConnectorMode {
		UNDEFINED,
		BYTE_LOW, // 256 entry port map, uses port LSB as key
		BYTE_HI, // 256 entry port map, use port MSB as key
		WORD // 65536 entry port map
	};


	typedef WORD(*GetPortFunc)(WORD);


	class PortConnector : virtual public Logger
	{
	public:
		PortConnector();
		virtual ~PortConnector();

		static void Init(PortConnectorMode mode);

		static void Clear();

		typedef void (PortConnector::* OUTFunction)(BYTE);
		typedef BYTE(PortConnector::* INFunction)();

		class PortHandler
		{
		public:
			PortHandler() : owner(nullptr), inFunc(nullptr) {}
			PortHandler(PortConnector* owner, OUTFunction func) : owner(owner), outFunc(func) {}
			PortHandler(PortConnector* owner, INFunction func) : owner(owner), inFunc(func) {}
			~PortHandler();

			bool IsSet() const { return inFunc != nullptr; }

			void Chain(PortHandler);

			BYTE In() const { return (owner->*inFunc)(); }
			void Out(BYTE value) const
			{
				const PortHandler* next = this;
				do
				{
					((next->owner)->*(next->outFunc))(value);
					next = next->chained;
				} while (next);
			}

		protected:
			PortConnector* owner = 0;
			union
			{
				OUTFunction outFunc;
				INFunction inFunc;
			};
			PortHandler* chained = nullptr;
		};

		using InputPortMap = std::vector<PortHandler>;
		using OutputPortMap = std::vector<PortHandler>;

		bool In(WORD port, BYTE& value);
		bool Out(WORD port, BYTE value);

	protected:
		bool Connect(WORD portNb, INFunction inFunc);
		bool Connect(WORD portNb, OUTFunction outFunc, bool share = false);

		bool DisconnectInput(WORD portNb);
		bool DisconnectOutput(WORD portNb);

		static WORD GetCurrentPort() { return m_currentPort; }

	protected:
		static bool IsInit() { return m_outputPorts.size() && m_inputPorts.size() && m_portConnectorMode != PortConnectorMode::UNDEFINED; }
		static PortConnectorMode GetPortConnectorMode() { return m_portConnectorMode; }

		inline PortHandler& GetInputPort(WORD port) const { return m_inputPorts[m_getPortFunc(port)]; }
		inline PortHandler& GetOutputPort(WORD port) const { return m_outputPorts[m_getPortFunc(port)]; }
	private:
		static OutputPortMap m_outputPorts;
		static InputPortMap m_inputPorts;

		static GetPortFunc m_getPortFunc;

		static WORD m_currentPort;
		static PortConnectorMode m_portConnectorMode;
	};
}
