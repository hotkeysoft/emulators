#include "PortAggregator.h"

void PortAggregator::Connect(PortConnector &ports)
{
	InputPortMap &inputs = ports.GetInputPorts();

	for (InputPortMap::iterator it = inputs.begin(); it != inputs.end(); ++it)
	{
		if (m_inputPorts.find(it->first) != m_inputPorts.end())
		{
			LogPrintf(LOG_ERROR, "Input Port 0x%02X already exists\n", it->first);
		}

		m_inputPorts[it->first] = it->second;
	}

	OutputPortMap &outputs = ports.GetOutputPorts();

	for (OutputPortMap::iterator it = outputs.begin(); it != outputs.end(); ++it)
	{
		if (m_outputPorts.find(it->first) != m_outputPorts.end())
		{
			LogPrintf(LOG_ERROR, "Output Port 0x%02X already exists\n", it->first);
		}

		m_outputPorts[it->first] = it->second;
	}
}
