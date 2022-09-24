#pragma once
#include <functional>
#include <map>
#include <tuple>

namespace emul
{
	class IOConnector : virtual public Logger
	{
	public:
		IOConnector(WORD deviceMask);
		virtual ~IOConnector();

		typedef void (IOConnector::* WRITEFunction)(BYTE);
		typedef BYTE(IOConnector::* READFunction)();

		class IOHandler
		{
		public:
			IOHandler(IOConnector* owner, WRITEFunction func) : owner(owner), writeFunc(func) {}
			IOHandler(IOConnector* owner, READFunction func) : owner(owner), readFunc(func) {}
			~IOHandler();

			void Chain(IOHandler);

			BYTE Read() const { return (owner->*readFunc)(); }
			void Write(BYTE value) const
			{
				const IOHandler* next = this;
				do
				{
					((next->owner)->*(next->writeFunc))(value);
					next = next->chained;
				} while (next);
			}

		protected:
			IOConnector* owner = 0;
			union
			{
				WRITEFunction writeFunc;
				READFunction readFunc;
			};
			IOHandler* chained = nullptr;
		};

		using IOReadMap = std::unordered_map<WORD, IOHandler> ;
		using IOWriteMap = std::unordered_map<WORD, IOHandler> ;

		bool IORead(WORD address, BYTE& value);
		bool IOWrite(WORD address, BYTE value);

		WORD GetDeviceIOMask() const { return m_deviceIOMask; }

	protected:
		bool Connect(WORD address, READFunction readFunc);
		bool Connect(WORD address, WRITEFunction writeFunc, bool share = false);

		// Merge our connections with another group (usually used for child objects)
		void Attach(IOConnector& other);

		IOWriteMap m_writeMap;
		IOReadMap m_readMap;
		WORD m_deviceIOMask;

		friend class IOBlock;
	};
}
