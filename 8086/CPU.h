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

		virtual size_t GetAddressBits() const = 0;
		virtual ADDRESS GetCurrentAddress() const = 0;
		virtual void Exec(BYTE opcode) = 0;

		virtual void Reset();
		void Run();
		virtual bool Step();

		unsigned long getTime() { return m_timeTicks; };

		// Watches
		void AddWatch(ADDRESS address, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
		void AddWatch(const char* label, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
		void RemoveWatch(ADDRESS address);
		void RemoveWatch(const char* label);

	protected:
		enum class CPUState { STOP, RUN, STEP };

		CPUState m_state;
		Memory& m_memory;
		MemoryMap& m_mmap;

		unsigned long m_timeTicks;

		// Helper functions
		BYTE getLByte(WORD w) { return BYTE(w & 0x00FF); };
		BYTE getHByte(WORD w) { return BYTE((w >> 8) & 0x00FF); };
		WORD getWord(BYTE h, BYTE l) { return (((WORD)h) << 8) + l; };
		bool getLSB(BYTE b) { return b & 1; }
		bool getLSB(WORD b) { return b & 1; }
		bool getMSB(WORD b) { return b & 32768; }
		bool getMSB(BYTE b) { return b & 128; }

		bool IsParityOdd(BYTE b);
		bool IsParityEven(BYTE b) { return !IsParityOdd(b); };
		bool IsParityOdd(WORD w);
		bool IsParityEven(WORD w) { return !IsParityOdd(w); };


		void OnCall(ADDRESS caller, ADDRESS target);
		void OnReturn(ADDRESS address);

		void UnknownOpcode(BYTE opcode);

	private:
		typedef std::map<ADDRESS, WatchItem > WatchList;
		WatchList m_callWatches;
		WatchList m_returnWatches;
	};
}