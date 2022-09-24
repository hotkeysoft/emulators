#include "stdafx.h"

#include "IOConnector.h"
#include <Config.h>

namespace emul
{
	IOConnector::IOHandler::~IOHandler()
	{
		IOHandler* handler = this->chained;
		while (handler)
		{
			IOHandler* toDelete = handler;
			handler = handler->chained;
			delete (toDelete);
		}
	}

	void IOConnector::IOHandler::Chain(IOHandler chained)
	{
		IOHandler* handler = this;
		while (handler->chained)
		{
			handler = handler->chained;
		}
		handler->chained = new IOHandler(chained);
	}

	IOConnector::IOConnector(WORD deviceIOMask) :
		Logger("IO"),
		m_deviceIOMask(deviceIOMask)
	{
	}

	IOConnector::~IOConnector()
	{
	}

	bool IOConnector::Connect(WORD address, READFunction readFunc)
	{
		LogPrintf(LOG_INFO, "Connect IOREAD address 0x%04X", address);

		if (m_readMap.find(address) != m_readMap.end())
		{
			LogPrintf(LOG_ERROR, "Address already exists");
			return false;
		}

		m_readMap.emplace(address, IOHandler(this, readFunc));

		return true;
	}

	bool IOConnector::Connect(WORD address, WRITEFunction writeFunc, bool share)
	{
		LogPrintf(LOG_INFO, "Connect IOWRITE address 0x%04X", address);

		IOWriteMap::iterator it = m_writeMap.find(address);
		if (it != m_writeMap.end())
		{
			if (share)
			{
				LogPrintf(LOG_INFO, "Chaining IOWRITE address 0x%04X", address);
				it->second.Chain(IOHandler(this, writeFunc));
			}
			else
			{
				LogPrintf(LOG_ERROR, "Address already mapped");
				return false;
			}
		}

		m_writeMap.emplace(address, IOHandler(this, writeFunc));

		return true;
	}

	void IOConnector::Attach(IOConnector& other)
	{
		// Does *not* handle chaining / already defined
		m_readMap.merge(other.m_readMap);
		m_writeMap.merge(other.m_writeMap);
	}

	bool IOConnector::IORead(WORD address, BYTE& value)
	{
		auto it = m_readMap.find(address);
		if (it == m_readMap.end())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "IOConnector::Read: IOREAD address 0x%04X not allocated", address);
#endif
			value = 0xFF;
			return false;
		}

		value = it->second.Read();
		return true;
	}

	bool IOConnector::IOWrite(WORD address, BYTE value)
	{
		auto it = m_writeMap.find(address);
		if (it == m_writeMap.end())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "IOConnector::Write(0x%04X, 0x%02X): address not allocated", address, value);
#endif
			return false;
		}

		it->second.Write(value);
		return true;
	}
}