// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Memory.h"
#include "MemoryBlock.h"
#include "MemoryMap.h"
#include "CPU8086.h"
#include <conio.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <inttypes.h>

#include <Windows.h>

void LogCallback(const char *str)
{
	fprintf(stderr, str);
}

unsigned long elapsed;

void onCall(emul::CPU* cpu, emul::WORD addr)
{
	elapsed = cpu->getTime();
}

void onRet(emul::CPU* cpu, emul::WORD addr)
{
	fprintf(stderr, "\tELAPSED: %ul\n", cpu->getTime()-elapsed);
}

inline emul::ADDRESS S2A(WORD segment, WORD offset = 0) 
{
	return (segment << 4) + offset;
}

int main(void)
{
	Logger::RegisterLogCallback(LogCallback);

	emul::Memory memory(emul::CPU8086_ADDRESS_BITS);
	memory.EnableLog(true);
	emul::MemoryMap mmap;
	mmap.EnableLog(false);

	fprintf(stderr, "\nMax address: 0x%" PRIx64"\n", emul::GetMaxAddress(emul::CPU8086_ADDRESS_BITS));

	emul::MemoryBlock biosF000(S2A(0xF000), 0x8000, emul::MemoryType::ROM);
	biosF000.LoadBinary("data/BIOS_5160_V3_F000.BIN");
	memory.Allocate(&biosF000);

	emul::MemoryBlock biosF800(S2A(0xF800), 0x8000, emul::MemoryType::ROM);
	biosF800.LoadBinary("data/BIOS_5160_V3_F800.BIN");
	memory.Allocate(&biosF800);

	emul::MemoryBlock buffer_memory(0x8000, 0x8000, emul::MemoryType::RAM);
	memory.Allocate(&buffer_memory);

	emul::CPU8086 cpu(memory, mmap);

//	cpu.AddWatch("EXECUTE", onCall, onRet);

	cpu.Reset();

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	time_t startTime, stopTime;
	time(&startTime);

	try
	{
		while (cpu.Step()) {};
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu\n");
	}

	time(&stopTime);

	fprintf(stderr, "Time elapsed: %I64u\n", stopTime-startTime);
	cpu.getTime();
	fprintf(stderr, "CPU ticks: %u\n", cpu.getTime());
	if (stopTime - startTime > 1)
	{
		fprintf(stderr, "Avg speed: %I64u ticks/s\n", cpu.getTime() / (stopTime - startTime));
	}

	return 0;
}


