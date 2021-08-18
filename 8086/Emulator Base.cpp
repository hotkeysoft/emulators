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

int main(void)
{
	Logger::RegisterLogCallback(LogCallback);

	emul::Memory memory;
	memory.EnableLog(false);
	emul::MemoryMap mmap;
	mmap.EnableLog(false);

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


