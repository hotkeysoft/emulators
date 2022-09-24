#include "stdafx.h"

#include <CPU/IOBlock.h>

namespace emul
{
	IOBlock::DefaultHandler::DefaultHandler() :
		Logger("DefaultIOHandler"),
		IOConnector(0),
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

	void IOBlock::AddDevice(IOConnector& device, WORD mask, WORD maskBits)
	{
		WORD deviceMask = device.GetDeviceIOMask();
		if (deviceMask == 0)
		{
			LogPrintf(LOG_WARNING, "AddDevice(%s): device IO mask is zero, nothing to map", device.GetModuleID());
			return;
		}

		if (maskBits == 0)
		{
			maskBits = mask;
		}

		mask &= maskBits;

		for (WORD addr = 0; addr < m_size; ++addr)
		{
			if ((addr & maskBits) != mask)
				continue;

			// TODO: Chaining
			auto readHandler = device.m_readMap.find(addr & deviceMask);
			if (readHandler != device.m_readMap.end())
			{
				if (m_readHandlers[addr] != &m_defaultHandler.m_defaultReadHandler)
				{
					LogPrintf(LOG_WARNING, "AddDevice(%s): There is already a device mapped to IOREAD address %04X, overwriting", device.GetModuleID(), addr);
				}
				m_readHandlers[addr] = &(readHandler->second);
				LogPrintf(LOG_DEBUG, "AddDevice: Connect IOREAD address %04X", addr);
			}

			// TODO: Chaining
			auto writeHandler = device.m_writeMap.find(addr & deviceMask);
			if (writeHandler != device.m_writeMap.end())
			{
				if (m_writeHandlers[addr] != &m_defaultHandler.m_defaultWriteHandler)
				{
					LogPrintf(LOG_WARNING, "AddDevice(%s): There is already a device mapped to IOWRITE address %04X, overwriting", device.GetModuleID(), addr);
				}
				m_writeHandlers[addr] = &(writeHandler->second);
				LogPrintf(LOG_DEBUG, "AddDevice: Connect IOWRITE address %04X", addr);
			}
		}
	}
}
