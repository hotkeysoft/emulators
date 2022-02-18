#pragma once
#include "Memory.h"
#include "MemoryMap.h"
#include "../Common.h"
#include <Logger.h>

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
		CPU(size_t addressBits, Memory& memory, MemoryMap& mmap);
		virtual ~CPU();

		virtual size_t GetAddressBits() const = 0;
		virtual ADDRESS GetCurrentAddress() const = 0;
		virtual void Exec(BYTE opcode) = 0;

		virtual void Reset();
		void Run();
		virtual bool Step();

		// Watches
		void AddWatch(ADDRESS address, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
		void AddWatch(const char* label, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
		void RemoveWatch(ADDRESS address);
		void RemoveWatch(const char* label);

		uint32_t GetInstructionTicks() { return m_opTicks; }

	protected:
		enum class CPUState { STOP, RUN, STEP, HALT };

		CPUState m_state;
		Memory& m_memory;
		MemoryMap& m_mmap;

		uint32_t m_opTicks = 0;
		inline void TICK(uint32_t count) { m_opTicks += count; }

		bool IsParityOdd(BYTE b);
		bool IsParityEven(BYTE b) { return !IsParityOdd(b); };

		void OnCall(ADDRESS caller, ADDRESS target);
		void OnReturn(ADDRESS address);

		void UnknownOpcode(BYTE opcode);

	private:
		typedef std::map<ADDRESS, WatchItem > WatchList;
		WatchList m_callWatches;
		WatchList m_returnWatches;
	};
}