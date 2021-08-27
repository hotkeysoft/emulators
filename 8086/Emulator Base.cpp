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
#include "Device8259.h"
#include "DeviceCGA.h"
#include <conio.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <inttypes.h>

#include <Windows.h>

FILE* logFile = nullptr;

void LogCallback(const char *str)
{
	fprintf(logFile ? logFile : stderr, str);
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

const emul::ADDRESS testROMAddress = 0xB000;
const std::vector<BYTE> testROM = { 0xBA, 0x00, 0xC8, 0x81, 0xEA, 0x00, 0xC8, 0xC4 };

int ReadInput()
{
	int key = _getch();
	// Special Char
	if (key == 0 || key == 0xE0)
	{
		key = _getch();
	}

	return key;
}

void DumpScreen(emul::MemoryBlock& block)
{
	fprintf(stderr, "SCREEN MEMORY DUMP");

	for (WORD offset = 0; offset < 2000; offset += 2)
	{
		if (offset % 80 == 0)
		{
			fprintf(stderr, "\n");
		}
		BYTE val = block.read(emul::S2A(0xB800, offset));
		fprintf(stderr, "%c", val ? val : ' ');
	}
	fprintf(stderr, "\n");
}

class DummyPortSink : public emul::PortConnector
{
public:
	DummyPortSink() : Logger("portSink")
	{
	}

	void AddDummyIN(WORD port)
	{
		Connect(port, static_cast<PortConnector::INFunction>(&DummyPortSink::DUMMY_IN));
	}
	void AddDummyOUT(WORD port)
	{
		Connect(port, static_cast<PortConnector::OUTFunction>(&DummyPortSink::DUMMY_OUT));
	}

	BYTE DUMMY_IN() { return 0; }
	void DUMMY_OUT(BYTE value) {}
};

int main(void)
{
	//	logFile = fopen("./dump.log", "w");

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

	// 16K screen buffer
	emul::MemoryBlock screenB800(emul::S2A(0xB800), 0x4000, emul::MemoryType::RAM);
	memory.Allocate(&screenB800);

	emul::MemoryBlock test(emul::S2A(testROMAddress), testROM, emul::MemoryType::ROM);
	//memory.Allocate(&test);

	emul::MemoryBlock extraRam(0x10000, 0x10000, emul::MemoryType::RAM);
	memory.Allocate(&extraRam);

	pit::Device8254 pit(0x40, 1193182);
	pit.Init();
	pit.EnableLog(true, Logger::LOG_INFO);

	pic::Device8259 pic(0x20);
	pic.Init();
	pic.EnableLog(true, Logger::LOG_INFO);

	ppi::Device8255 ppi(0x60);
	ppi.Init();
	ppi.EnableLog(true, Logger::LOG_INFO);

	// Configuration switches
	{
		ppi.SetPOSTLoop(false);
		ppi.SetMathCoprocessor(false);
		ppi.SetRAMConfig(ppi::RAMSIZE::RAM_128K);
		ppi.SetDisplayConfig(ppi::DISPLAY::COLOR_40x25);
		ppi.SetFloppyCount(1);
	}

	dma::Device8237 dma(0x00);
	dma.Init();
	dma.EnableLog(false);
	dma.EnableLog(true, Logger::LOG_WARNING);

	cga::DeviceCGA cga(0x3D0);
	cga.Init();
	cga.EnableLog(true, Logger::LOG_WARNING);

	DummyPortSink sink;
	sink.AddDummyIN(0x3F4);

	emul::CPU8086 cpu(memory, mmap);

	//	cpu.AddWatch("EXECUTE", onCall, onRet);

	cpu.AddDevice(pic);
	cpu.AddDevice(pit);
	cpu.AddDevice(ppi);
	cpu.AddDevice(dma);
	cpu.AddDevice(cga);
	cpu.AddDevice(sink);
	cpu.Reset();
	cpu.EnableLog(true, Logger::LOG_ERROR);

	//cpu.Reset(emul::S2A(testROMAddress));

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	time_t startTime, stopTime;
	time(&startTime);

	// TODO: Temporary, need dynamic connections
	// Timer 0: Time of day
	// Timer 1: DMA RAM refresh
	// Timer 2: Speaker
	bool timer0Out = false;
	try
	{
		size_t ticks = 0;
		while (cpu.Step())
		{ 
			++ticks;

			if (ticks % 10000)
			{
				if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
				{
					break;
				}
				else if (GetAsyncKeyState(VK_F10) & 0x8000)
				{
					while (GetAsyncKeyState(VK_F10) & 0x8000);
					_getch();
				}
				else if (GetAsyncKeyState(VK_F12) & 0x8000)
				{
					DumpScreen(screenB800);
					while (GetAsyncKeyState(VK_F12) & 0x8000);
				}
				else if (GetAsyncKeyState(VK_F1) & 0x8000)
				{
					static bool release = false;
					fprintf(stderr, "\t* Keyboard interrupt release=%d\n", release);
					fprintf(stderr, "\t* CanInterrupt=%d\n", cpu.CanInterrupt());
					while (!cpu.CanInterrupt())
						cpu.Step();
					cpu.Interrupt(8 + 1); // Hardware interrupt 1: keyboard
					ppi.SetCurrentKeyCode(0x3B, release);
					while (GetAsyncKeyState(VK_F1) & 0x8000);
					release = !release;
				}
			}

			pit.Tick();
			bool out = pit.GetCounter(0).GetOutput();
			if (out != timer0Out)
			{
				timer0Out = out;
				fprintf(stderr,"\t* timer 0 output changed: %d\n", out);
				if (out)
				{
					// TODO: this bypasses a lot of things.
					// Quick and dirty for now: Check mask manually and interrupt cpu
					if (!(pic.Mask_IN() & 0x01))
					{
						cpu.Interrupt(8+0); // Hardware interrupt 0: timer
					}
				}
			}

			dma.Tick();
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	time(&stopTime);

	cpu.Dump();
	fprintf(stderr, "\n");
	for (emul::ADDRESS a = 0x400; a < 0x400 + 64; ++a)
	{
		BYTE val;
		memory.Read(a, val);
		fprintf(stderr, "%02X ", val);
	}
	fprintf(stderr, "\n\n");

	DumpScreen(screenB800);

	if (logFile)
	{
		fclose(logFile);
	}

	fprintf(stderr, "Time elapsed: %I64u\n", stopTime-startTime);
	cpu.getTime();
	fprintf(stderr, "CPU ticks: %u\n", cpu.getTime());
	if (stopTime - startTime > 1)
	{
		fprintf(stderr, "Avg speed: %I64u ticks/s\n", cpu.getTime() / (stopTime - startTime));
	}

	return 0;
}


