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

	typedef void(*OpcodeFunc)(uint8_t);

	typedef void(*KCallbackFunc)();
	typedef void(*OCallbackFunc)(uint8_t);
	typedef void(*RCallbackFunc)(uint8_t, bool);

	struct CPUState {
		// Registers
		uint8_t A;
		uint8_t X;
		uint8_t Y;

		// Status
		bool S;
		bool SL;

		// ROM Addressing
		uint8_t PA;
		uint8_t PB;
		uint8_t PC;
		uint8_t SR;
		bool CL;

		// IO
		uint8_t K;
		uint8_t O;

		bool R[RWidth];

		// TMS1100/TMS1300 (1 bit) / TMS1400 (2 bits)
		uint8_t CA; // Chapter Address Latch/Buffer
		uint8_t CB; // Chapter Buffer latch/Buffer
		uint8_t CS; // Chapter Subroutine latch/buffer 

		// TMS1400 - Three level stack
		uint32_t SR1400; // PC Stack: 3 * 6 bits = 18
		uint16_t PSR1400; // Page Stack: 3 * 4 bits = 12
		uint8_t CSR1400; // Chapter stack: 3 * 2 bits = 6	
		uint8_t CL1400; // Call latch stack: 3 bits
	};

	struct Memory {
		uint8_t* RAM = nullptr;
		const uint8_t* ROM = nullptr;

		int ramSize = 0;
		int romSize = 0;
	};

	extern CPUState g_cpu;
	extern Memory g_memory;

	void Init(TMS1000Family model, uint16_t romSize, uint16_t ramSize);

	void Reset();
	void Step();

	void Exec(uint8_t opcode);
	long GetTicks();

	uint8_t GetB(uint8_t opcode);
	uint8_t GetC(uint8_t opcode);
	uint8_t GetF(uint8_t opcode);
	uint8_t GetW(uint8_t opcode);

	uint8_t GetM();
	uint8_t GetM(uint8_t X, uint8_t Y);
	uint16_t GetROMAddress();
	uint8_t GetROMData();
	uint8_t GetRAM(uint8_t addr = GetM());
	void PutRAM(uint8_t value, uint8_t addr = GetM());

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