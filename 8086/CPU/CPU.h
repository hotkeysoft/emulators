#pragma once
#include "Memory.h"

namespace emul
{
	class CPU;
	typedef void(*CPUCallbackFunc)(CPU* cpu, ADDRESS addr);

	struct WatchItem
	{
		ADDRESS addr;
		CPUCallbackFunc onCall;
		CPUCallbackFunc onRet;
	};

	class CPU : virtual public Logger
	{
	public:
		CPU(size_t addressBits, Memory& memory);
		virtual ~CPU();

		virtual const std::string GetID() const = 0;
		virtual size_t GetAddressBits() const = 0;
		virtual ADDRESS GetCurrentAddress() const = 0;
		virtual void Exec(BYTE opcode) = 0;

		virtual void Reset();
		void Run();
		virtual bool Step();

		uint32_t GetInstructionTicks() const { return m_opTicks; }

	protected:
		enum class CPUState { STOP, RUN, STEP, HALT };

		CPUState m_state;
		Memory& m_memory;

		uint32_t m_opTicks = 0;
		inline void TICK(uint32_t count) { m_opTicks += count; }
	};
}
