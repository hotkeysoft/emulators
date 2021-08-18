#pragma once

#include "CPU.h"
#include "PortConnector.h"
#include "PortAggregator.h"

namespace emul
{
	static const size_t CPU8086_ADDRESS_BITS = 20;

	class CPU8086 : public CPU
	{
	public:
		CPU8086(Memory& memory, MemoryMap& mmap);
		virtual ~CPU8086();

		virtual const size_t GetAddressBits() const { return CPU8086_ADDRESS_BITS; }
		virtual const ADDRESS GetResetAddress() const { return 0xffff0; }

		void AddDevice(PortConnector& ports);

		void Dump();

		virtual void Reset();

	protected:
		PortAggregator m_ports;

		// Helper functions

		// Opcodes
	};
}
