#pragma once

namespace TMS1000
{
	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef void(*OpcodeFunc)(BYTE);

	typedef void(*IOCallbackFunc)();

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
		const BYTE* ROM = nullptr;

		int ramSize = 0;
		int romSize = 0;
	};

	extern CPUState g_cpu;
	extern Memory g_memory;

	void Init(WORD romSize, WORD ramSize);
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

#ifndef ARDUINO
	void LoadROM(const char* path);
	void SaveROM(const char* path);
#endif
}