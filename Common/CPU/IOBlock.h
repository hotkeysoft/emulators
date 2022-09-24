#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/MemoryBlockBase.h>
#include <CPU/IOConnector.h>

namespace emul
{
	class IOBlock : public MemoryBlockBase
	{
	public:
		IOBlock(const char* id, DWORD size);

		virtual ~IOBlock();

		virtual BYTE read(ADDRESS offset) override;
		virtual void write(ADDRESS offset, BYTE data) override;

		void AddDevice(IOConnector& device, ADDRESS mask);

	protected:
		using IOHandlers = std::vector<IOConnector::IOHandler*>;

		IOHandlers m_readHandlers;
		IOHandlers m_writeHandlers;

		class DefaultHandler : public IOConnector
		{
		public:
			IOConnector::IOHandler m_defaultReadHandler;
			IOConnector::IOHandler m_defaultWriteHandler;

			DefaultHandler();

			BYTE DefaultRead() { return 0xFF; }
			void DefaultWrite(BYTE value) { }
		} m_defaultHandler;
	};
}