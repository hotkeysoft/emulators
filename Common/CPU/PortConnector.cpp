#include "stdafx.h"

#include "PortConnector.h"
#include <Config.h>

namespace emul
{
	PortConnector::OutputPortMap PortConnector::m_outputPorts;
	PortConnector::InputPortMap PortConnector::m_inputPorts;
	WORD PortConnector::m_currentPort = 0;
	PortConnectorMode PortConnector::m_portConnectorMode = PortConnectorMode::UNDEFINED;

	GetPortFunc PortConnector::m_getPortFunc = nullptr;

	WORD GetPortByteHi(WORD port) { return port >> 8; }
	WORD GetPortByteLow(WORD port) { return port & 0xFF; }
	WORD GetPortWord(WORD port) { return port; }

	PortConnector::PortHandler::~PortHandler()
	{
		PortHandler* handler = this->chained;
		while (handler)
		{
			PortHandler* toDelete = handler;
			handler = handler->chained;
			delete (toDelete);
		}
	}

	void PortConnector::PortHandler::Chain(PortHandler chained)
	{
		PortHandler* handler = this;
		while (handler->chained)
		{
			handler = handler->chained;
		}
		handler->chained = new PortHandler(chained);
	}

	PortConnector::PortConnector() : Logger("PORT")
	{
	}

	PortConnector::~PortConnector()
	{
	}

	void PortConnector::Init(PortConnectorMode mode)
	{
		m_portConnectorMode = mode;

		switch (mode)
		{
		case PortConnectorMode::BYTE_HI:
			m_getPortFunc = GetPortByteHi;
			m_inputPorts.resize(256);
			m_outputPorts.resize(256);
			break;

		case PortConnectorMode::BYTE_LOW:
			m_getPortFunc = GetPortByteLow;
			m_inputPorts.resize(256);
			m_outputPorts.resize(256);
			break;

		case PortConnectorMode::WORD:
			m_getPortFunc = GetPortWord;
			m_inputPorts.resize(65536);
			m_outputPorts.resize(65536);
			break;

		default:
			throw std::exception("PortConnector::Init(): Invalid mode");
		}

		Clear();
	}

	void PortConnector::Clear()
	{
		std::fill(m_inputPorts.begin(), m_inputPorts.end(), PortHandler());
		std::fill(m_outputPorts.begin(), m_outputPorts.end(), PortHandler());
	}

	bool PortConnector::Connect(WORD port, INFunction inFunc, bool replace)
	{
		if (!IsInit())
		{
			LogPrintf(LOG_ERROR, "PortConnector: Not Initialized");
			throw std::exception("PortConnector: Not Initialized");
		}

		LogPrintf(LOG_INFO, "Connect input port 0x%04X", port);

		PortHandler& inPort = GetInputPortDirect(port);

		if (!replace && inPort.IsSet())
		{
			LogPrintf(LOG_ERROR, "Port already exists");
			return false;
		}

		inPort = PortHandler(this, inFunc);

		return true;
	}

	bool PortConnector::Connect(WORD port, OUTFunction outFunc, bool share)
	{
		if (!IsInit())
		{
			LogPrintf(LOG_ERROR, "PortConnector: Not Initialized");
			throw std::exception("PortConnector: Not Initialized");
		}

		LogPrintf(LOG_INFO, "Connect output port 0x%04X", port);

		PortHandler& outPort = GetOutputPortDirect(port);

		if (outPort.IsSet())
		{
			if (share)
			{
				LogPrintf(LOG_INFO, "Chaining output port 0x%04X", port);
				outPort.Chain(PortHandler(this, outFunc));
				return true;
			}
			else
			{
				LogPrintf(LOG_ERROR, "Port already exists");
				return false;
			}
		}

		outPort = PortHandler(this, outFunc);

		return true;
	}

	bool PortConnector::Connect(BitMaskB portMask, INFunction inFunc, bool replace)
	{
		if (!IsInit())
		{
			LogPrintf(LOG_ERROR, "PortConnector: Not Initialized");
			throw std::exception("PortConnector: Not Initialized");
		}

		LogPrintf(LOG_INFO, "Connect input ports, mask=[%s]", portMask.ToString().c_str());

		bool ok = true;
		for (WORD i = 0; i < 256; ++i)
		{
			if (portMask.IsMatch((BYTE)i) && !Connect(i, inFunc, replace))
			{
				ok = false;
			}
		}

		return ok;
	}

	bool PortConnector::Connect(BitMaskB portMask, OUTFunction outFunc, bool share)
	{
		if (!IsInit())
		{
			LogPrintf(LOG_ERROR, "PortConnector: Not Initialized");
			throw std::exception("PortConnector: Not Initialized");
		}

		LogPrintf(LOG_INFO, "Connect output ports, mask=[%s]", portMask.ToString().c_str());

		bool ok = true;
		for (WORD i = 0; i < 256; ++i)
		{
			if (portMask.IsMatch((BYTE)i) && !Connect(i, outFunc, share))
			{
				ok = false;
			}
		}

		return ok;
	}

	bool PortConnector::DisconnectInput(WORD port)
	{
		PortHandler& inPort = GetInputPortDirect(port);

		if (inPort.IsSet())
		{
			inPort = PortHandler();
			LogPrintf(LOG_INFO, "Disconnect input port 0x%04X", port);
			return true;
		}
		else
		{
			LogPrintf(LOG_INFO, "Disconnect input port 0x%04X (not connected)", port);
			return false;
		}
	}

	bool PortConnector::DisconnectOutput(WORD port)
	{
		PortHandler& outPort = GetOutputPortDirect(port);

		if (outPort.IsSet())
		{
			outPort = PortHandler();
			LogPrintf(LOG_INFO, "Disconnect output port 0x%04X", port);
			return true;
		}
		else
		{
			LogPrintf(LOG_INFO, "Disconnect output port 0x%04X (not connected)", port);
			return false;
		}
	}

	bool PortConnector::In(WORD port, BYTE& value)
	{
		m_currentPort = port;
		PortHandler& inPort = GetInputPort(port);

		if (!inPort.IsSet())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "PortConnector::In: port 0x%04X not allocated", port);
#endif
			value = 0xFF;
			return false;
		}

		value = inPort.In();
		return true;
	}

	bool PortConnector::Out(WORD port, BYTE value)
	{
		m_currentPort = port;
		PortHandler& outPort = GetOutputPort(port);

		if (!outPort.IsSet())
		{
#ifdef _DEBUG
			LogPrintf(LOG_WARNING, "PortConnector::Out(0x%04X, 0x%02X): port not allocated", port, value);
#endif
			return false;
		}

		outPort.Out(value);
		return true;
	}
}