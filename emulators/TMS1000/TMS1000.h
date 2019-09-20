#pragma once
#include "CPUInfo.h"

namespace TMS1000
{
	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef void(*OpcodeFunc)(BYTE);

	typedef void(*IOCallbackFunc)();

	enum OperandFormat {
		Format1, // W = Branch Address = I(2-7)
		Format2, // C = Constant Operand I(7-4)
		Format3, // B = RAM-X OR BIT ADDRESS I(7,6)
		Format4, // No operands
		Format5, // F = File Address I(7-5) (TMS1100 only)
	};

	struct Instruction {
		OpcodeFunc func;
		OperandFormat operandFormat;
		const char* name;
	};

	struct CPUState {
		// Registers
		BYTE A;
		BYTE X;
		BYTE Y;

		// Status
		bool S;
		bool SL;

		// ROM Addressing
		BYTE PA;
		BYTE PB;
		BYTE PC;
		BYTE SR;
		bool CL;

		// IO
		BYTE K;
		BYTE O;
		WORD R;
	};

	struct Memory {
		BYTE* RAM = nullptr;
		BYTE* ROM = nullptr;

		int ramSize = 0;
		int romSize = 0;
	};

	extern CPUState g_cpu;
	extern Memory g_memory;

	void Init(TMS1000Family family, CPUInfo* pCPUInfo);
	void LoadROM(const char* path);
	void Reset();
	void Step();
	void Exec(BYTE opcode);
	long GetTicks();

	BYTE GetB(BYTE opcode);
	BYTE GetC(BYTE opcode);
	BYTE GetF(BYTE opcode);
	BYTE GetW(BYTE opcode);

	BYTE GetM();
	BYTE GetM(BYTE X, BYTE Y);
	BYTE GetROM();
	BYTE GetRAM(BYTE addr = GetM());
	void PutRAM(BYTE value, BYTE addr = GetM());

	void SetInputCallback(IOCallbackFunc);
	void SetOutputCallback(IOCallbackFunc);

	void Disassemble(BYTE opcode, char* line, int lineSize);
}