#pragma once

#include "CPU.h"
#include "PortConnector.h"
#include "PortAggregator.h"

namespace emul
{

	class CPU8086 : public CPU
	{
	public:
		CPU8086(Memory& memory, MemoryMap& mmap);
		virtual ~CPU8086();

		void AddDevice(PortConnector& ports);

		void Dump();

		virtual void Reset();

	protected:
		PortAggregator m_ports;

		// Helper functions

		// Opcodes
	};
}
