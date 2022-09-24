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

		// mask and maskBits allows partial address decoding
		// mask     = b01000000 and
		// maskBits = b01100000 are equivalent to
		//            (x10xxxxx) (where 'x' are 'don't care').
		// Device will be activated for all addresses that match:
		// 1[10]00000, 0[10]10000, etc.
		//
		// If maskBits is not set, this is equivalent of having mask = maskBits.
		// So only the 1 in the mask will be used to activate the device.
		void AddDevice(IOConnector& device, WORD mask, WORD maskBits = 0);

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