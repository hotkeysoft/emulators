#pragma once
#include "Memory.h"
#include "MemoryMap.h"
#include "Common.h"
#include "Logger.h"

class CPU;
typedef void(*CPUCallbackFunc)(CPU* cpu, WORD addr);

struct WatchItem
{
	WORD addr;
	CPUCallbackFunc onCall;
	CPUCallbackFunc onRet;
};

class CPU : virtual public Logger
{
public:
	CPU(Memory &memory, MemoryMap &mmap);
	virtual ~CPU();

	virtual void Reset();
	void Run();
	virtual bool Step();

	unsigned long getTime() { return m_timeTicks; };

	void DumpUnassignedOpcodes();

	// Watches
	void AddWatch(WORD address, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
	void AddWatch(const char* label, CPUCallbackFunc onCall, CPUCallbackFunc onRet);
	void RemoveWatch(WORD address);
	void RemoveWatch(const char* label);

protected:
	typedef void (CPU::*OPCodeFunction)(BYTE);

	void AddOpcode(BYTE, OPCodeFunction);

	enum CPUState {STOP, RUN, STEP};

	CPUState m_state;
	Memory &m_memory;
	MemoryMap &m_mmap;

	unsigned long m_timeTicks;
	unsigned int m_programCounter;

	// Helper functions
	BYTE getLByte(WORD w) { return BYTE(w&0x00FF); };
	BYTE getHByte(WORD w) {return BYTE((w>>8)&0x00FF); };
	WORD getWord(BYTE h, BYTE l) { return (((WORD)h)<<8) + l; };

	bool isParityOdd(BYTE b);
	bool isParityEven(BYTE b) { return !isParityOdd(b); };

	void OnCall(WORD caller, WORD target);
	void OnReturn(WORD address);

private:
	typedef std::map<WORD, WatchItem > WatchList;
	WatchList m_callWatches;
	WatchList m_returnWatches;

	OPCodeFunction m_opcodesTable[256];
	void UnknownOpcode(BYTE);
};
