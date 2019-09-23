#pragma once

namespace TMS1000
{
	enum TMS1000Family { CPU_TMS1000, CPU_TMS1200, CPU_TMS1070, CPU_TMS1270, CPU_TMS1100, CPU_TMS1300 };

	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef void(*OpcodeFunc)(BYTE);

	typedef void(*KCallbackFunc)();
	typedef void(*OCallbackFunc)(BYTE);
	typedef void(*RCallbackFunc)(BYTE, bool);

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

		// TMS1100/TMS1300
		bool CA; // Chapter Address Latch
		bool CB; // Chapter Buffer latch
		bool CS; // Chapter Subroutine latch
	};

	struct Memory {
		BYTE* RAM = nullptr;
		const BYTE* ROM = nullptr;

		int ramSize = 0;
		int romSize = 0;
	};

	extern CPUState g_cpu;
	extern Memory g_memory;

	void Init(TMS1000Family model, WORD romSize, WORD ramSize);

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
	WORD GetROMAddress();
	BYTE GetROMData();
	BYTE GetRAM(BYTE addr = GetM());
	void PutRAM(BYTE value, BYTE addr = GetM());

	void SetInputKCallback(KCallbackFunc);

	// For O Callback, first parameter is 'A' (four bits), bool is SL flag.
	void SetOutputOCallback(OCallbackFunc);

	// R Callback, first parameter is bit to flip, bool is SET(true) / RESET(false)
	void SetOutputRCallback(RCallbackFunc);

#ifndef ARDUINO
	void LoadROM(const char* path);
	void SaveROM(const char* path);
#endif
}