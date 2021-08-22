// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Memory.h"
#include "MemoryBlock.h"
#include "MemoryMap.h"
#include "CPU8086.h"
#include "Device8254.h"
#include "Device8255.h"
#include "Device8237.h"
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

int main(void)
{
	Logger::RegisterLogCallback(LogCallback);

	emul::Memory memory(emul::CPU8086_ADDRESS_BITS);
	memory.EnableLog(true, Logger::LOG_ERROR);
	emul::MemoryMap mmap;
	mmap.EnableLog(true, Logger::LOG_ERROR);

	fprintf(stderr, "\nMax address: 0x%" PRIx64"\n", (uint64_t)emul::GetMaxAddress(emul::CPU8086_ADDRESS_BITS));

	emul::MemoryBlock biosF000(emul::S2A(0xF000), 0x8000, emul::MemoryType::ROM);
	biosF000.LoadBinary("data/BIOS_5160_V3_F000.BIN");
	memory.Allocate(&biosF000);

	emul::MemoryBlock biosF800(emul::S2A(0xF800), 0x8000, emul::MemoryType::ROM);
	biosF800.LoadBinary("data/BIOS_5160_V3_F800.BIN");
	memory.Allocate(&biosF800);

	emul::MemoryBlock base64K(0, 0x10000, emul::MemoryType::RAM);
	base64K.Clear(0xA5);
	memory.Allocate(&base64K);

	//emul::MemoryBlock extraRam(0x10000, 0x40000, emul::MemoryType::RAM);
	//memory.Allocate(&extraRam);

	pit::Device8254 pit(0x40);
	pit.Init();
	pit.EnableLog(true, Logger::LOG_INFO);

	ppi::Device8255 ppi(0x60);
	ppi.Init();
	ppi.EnableLog(true, Logger::LOG_DEBUG);

	dma::Device8237 dma(0x00);
	dma.Init();
	dma.EnableLog(false);

	emul::CPU8086 cpu(memory, mmap);

//	cpu.AddWatch("EXECUTE", onCall, onRet);

	cpu.AddDevice(pit);
	cpu.AddDevice(ppi);
	cpu.AddDevice(dma);
	cpu.Reset();
	cpu.EnableLog(false);  // Enabled internally

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	time_t startTime, stopTime;
	time(&startTime);

	try
	{
		while (cpu.Step())
		{
			pit.Tick();
		};
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	time(&stopTime);

	cpu.Dump();
	fprintf(stderr, "\n");
	for (emul::ADDRESS a = 0x400; a < 0x400 + 32; ++a)
	{
		BYTE val;
		memory.Read(a, val);
		fprintf(stderr, "%02X ", val);
	}
	fprintf(stderr, "\n");
	

	fprintf(stderr, "Time elapsed: %I64u\n", stopTime-startTime);
	cpu.getTime();
	fprintf(stderr, "CPU ticks: %u\n", cpu.getTime());
	if (stopTime - startTime > 1)
	{
		fprintf(stderr, "Avg speed: %I64u ticks/s\n", cpu.getTime() / (stopTime - startTime));
	}

	return 0;
}


