#pragma once
#include "CPU8080.h"

namespace emul
{
	class CPUZ80 : public CPU8080
	{
	public:
		CPUZ80(Memory& memory, Interrupts& interrupts);

		virtual void Init() override;

		virtual void Reset() override;

	protected:
		CPUZ80(cpuInfo::CPUType type, Memory& memory, Interrupts& interrupts);

		// Helper functions
		void jumpRelIF(bool condition, BYTE offset);

		// Opcodes
		void EXAF();
		void DJNZ();
		void EXX();

		void BITS(BYTE op2);
		void IX(BYTE op2);
		void EXTD(BYTE op2);
		void IY(BYTE op2);
	};
}
