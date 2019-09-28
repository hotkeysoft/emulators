#pragma once

namespace TMS1000
{
	const int RWidth = 15;

	enum TMS1000Family { 
		CPU_TMS1000, 
		CPU_TMS1200, 
		CPU_TMS1070, 
		CPU_TMS1270, 
		CPU_TMS1100, 
		CPU_TMS1300, 
		CPU_TMS1400,
		CPU_TMS1700
	};

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

		bool R[RWidth];

		// TMS1100/TMS1300 (1 bit) / TMS1400 (2 bits)
		BYTE CA; // Chapter Address Latch/Buffer
		BYTE CB; // Chapter Buffer latch/Buffer
		BYTE CS; // Chapter Subroutine latch/buffer 

		// TMS1400 - Three level stack
		uint32_t SR1400; // PC Stack: 3 * 6 bits = 18
		uint16_t PSR1400; // Page Stack: 3 * 4 bits = 12
		uint8_t CSR1400; // Chapter stack: 3 * 2 bits = 6	
		uint8_t CL1400; // Call latch stack: 3 bits
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