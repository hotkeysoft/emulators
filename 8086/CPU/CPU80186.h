#pragma once

#include "CPU8086.h"

namespace emul
{
	class CPU80186 : public CPU8086
	{
	public:
		CPU80186(Memory& memory);

		virtual void Init() override;

	protected:
		void PUSHA();
		void POPA();

		void SHIFTROT8Imm(BYTE op2);
		void SHIFTROT16Imm(BYTE op2);

		void IMULimm8(BYTE op2);
		void IMULimm16(BYTE op2);

		void _IMUL16imm(SourceDest16& sd, WORD imm);

	};
}
