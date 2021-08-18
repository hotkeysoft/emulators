#pragma once
#include "Memory.h"
#include "MemoryMap.h"
#include "Common.h"
#include "Logger.h"

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

		virtual const size_t GetAddressBits() const = 0;
		virtual const ADDRESS GetResetAddress() const = 0;

		virtual void Reset();
		void Run();
		virtual bool Step();

		unsigned long getTime() { return m_timeTicks; };

		void DumpUnassignedOpcodes();

		// Watches
		void AddWatch(ADDRESS address, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
		void AddWatch(const char* label, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
		void RemoveWatch(ADDRESS address);
		void RemoveWatch(const char* label);

	protected:
		typedef void (CPU::* OPCodeFunction)(BYTE);

		void AddOpcode(BYTE, OPCodeFunction);

		enum class CPUState { STOP, RUN, STEP };

		CPUState m_state;
		Memory& m_memory;
		MemoryMap& m_mmap;

		unsigned long m_timeTicks;
		ADDRESS m_programCounter;

		// Helper functions
		BYTE getLByte(WORD w) { return BYTE(w & 0x00FF); };
		BYTE getHByte(WORD w) { return BYTE((w >> 8) & 0x00FF); };
		WORD getWord(BYTE h, BYTE l) { return (((WORD)h) << 8) + l; };

		bool isParityOdd(BYTE b);
		bool isParityEven(BYTE b) { return !isParityOdd(b); };

		void OnCall(ADDRESS caller, ADDRESS target);
		void OnReturn(ADDRESS address);

	private:
		typedef std::map<ADDRESS, WatchItem > WatchList;
		WatchList m_callWatches;
		WatchList m_returnWatches;

		OPCodeFunction m_opcodesTable[256];
		void UnknownOpcode(BYTE);
	};
}