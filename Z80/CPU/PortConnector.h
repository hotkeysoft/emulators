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

		class PortHandler
		{
		public:
			PortHandler(PortConnector* owner, OUTFunction func) : owner(owner), outFunc(func) {}
			PortHandler(PortConnector* owner, INFunction func) : owner(owner), inFunc(func) {}
			~PortHandler();

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

		using InputPortMap = std::unordered_map<WORD, PortHandler> ;
		using OutputPortMap = std::unordered_map<WORD, PortHandler> ;

		bool In(WORD port, BYTE& value);
		bool Out(WORD port, BYTE value);
	
	protected:
		bool Connect(WORD portNb, INFunction inFunc);
		bool Connect(WORD portNb, OUTFunction outFunc, bool share = false);

		bool DisconnectInput(WORD portNb);
		bool DisconnectOutput(WORD portNb);

		static WORD GetCurrentPort() { return m_currentPort; }

	protected:
		static OutputPortMap m_outputPorts;
		static InputPortMap m_inputPorts;
	private:
		static WORD m_currentPort;
	};
}
