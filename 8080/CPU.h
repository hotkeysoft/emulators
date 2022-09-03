#pragma once
#include <CPU/Memory.h>

namespace emul
{
	class CPU;

	class CPU : virtual public Logger
	{
	public:
		CPU(Memory& memory);
		virtual ~CPU();

		virtual void Reset();
		void Run();
		virtual bool Step();

		unsigned long getTime() { return m_timeTicks; };

		void DumpUnassignedOpcodes();

	protected:
		typedef void (CPU::* OPCodeFunction)(BYTE);

		void AddOpcode(BYTE, OPCodeFunction);

		enum CPUState { STOP, RUN, STEP };

		CPUState m_state;
		Memory& m_memory;

		unsigned long m_timeTicks;
		unsigned int m_programCounter;

		// Helper functions
		BYTE getLByte(WORD w) { return BYTE(w & 0x00FF); };
		BYTE getHByte(WORD w) { return BYTE((w >> 8) & 0x00FF); };
		WORD getWord(BYTE h, BYTE l) { return (((WORD)h) << 8) + l; };

		bool isParityOdd(BYTE b);
		bool isParityEven(BYTE b) { return !isParityOdd(b); };

	private:
		OPCodeFunction m_opcodesTable[256];
		void UnknownOpcode(BYTE);
	};
}
