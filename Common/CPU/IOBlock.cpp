#include "stdafx.h"

#include <CPU/IOBlock.h>

namespace emul
{
	IOBlock::DefaultHandler::DefaultHandler() :
		Logger("DefaultIOHandler"),
		m_defaultReadHandler(this, static_cast<IOConnector::READFunction>(&IOBlock::DefaultHandler::DefaultRead)),
		m_defaultWriteHandler(this, static_cast<IOConnector::WRITEFunction>(&IOBlock::DefaultHandler::DefaultWrite))
	{

	}

	IOBlock::IOBlock(const char* id, DWORD size) :
		MemoryBlockBase(id, size, MemoryType::IO)
	{
		m_readHandlers.resize(size);
		std::fill(m_readHandlers.begin(), m_readHandlers.end(), &m_defaultHandler.m_defaultReadHandler);

		m_writeHandlers.resize(size);
		std::fill(m_writeHandlers.begin(), m_writeHandlers.end(), &m_defaultHandler.m_defaultWriteHandler);
	}

	IOBlock::~IOBlock()
	{

	}

	BYTE IOBlock::read(ADDRESS offset)
	{
		assert(offset < m_size);
		return m_readHandlers[offset]->Read();
	}

	void IOBlock::write(ADDRESS offset, BYTE data)
	{
		assert(offset < m_size);
		m_writeHandlers[offset]->Write(data);
	}

	void IOBlock::AddDevice(IOConnector& device, ADDRESS mask)
	{
		for (WORD addr = 0; addr < m_size; ++addr)
		{
			// TODO: Chaining
			auto readHandler = device.m_readMap.find(addr & mask);
			if (readHandler != device.m_readMap.end())
			{
				m_readHandlers[addr] = &(readHandler->second);
				LogPrintf(LOG_TRACE, "AddDevice: Connect IOREAD address %042X", addr);
			}

			// TODO: Chaining
			auto writeHandler = device.m_writeMap.find(addr & mask);
			if (writeHandler != device.m_writeMap.end())
			{
				m_writeHandlers[addr] = &(writeHandler->second);
				LogPrintf(LOG_TRACE, "AddDevice: Connect IOWRITE address %04X", addr);
			}
		}
	}
}
