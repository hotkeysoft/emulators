#include "PortAggregator.h"

namespace emul
{
	void PortAggregator::Connect(PortConnector& ports)
	{
		InputPortMap& inputs = ports.GetInputPorts();

		for (const auto& input : inputs)
		{
			if (m_inputPorts.find(input.first) != m_inputPorts.end())
			{
				LogPrintf(LOG_ERROR, "Input Port 0x%04X already exists\n", input.first);
			}

			m_inputPorts[input.first] = input.second;
		}

		OutputPortMap& outputs = ports.GetOutputPorts();

		for (const auto& output : outputs)
		{
			if (m_outputPorts.find(output.first) != m_outputPorts.end())
			{
				LogPrintf(LOG_ERROR, "Output Port 0x%04X already exists\n", output.first);
			}

			m_outputPorts[output.first] = output.second;
		}
	}
}