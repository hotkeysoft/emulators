// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Memory.h"
#include "MemoryBlock.h"
#include "MemoryMap.h"
#include "CPU8086.h"
#include "CPU8086Test.h"
#include "Device8254.h"
#include "Device8255.h"
#include "Device8237.h"
#include "Device8259.h"
#include "DeviceCGA.h"
#include "DeviceFloppy.h"
#include "Console.h"
#include <conio.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <inttypes.h>

#include <Windows.h>

//#define NO_CONSOLE
//#define CPU_TEST

FILE* logFile = nullptr;

Console console;

enum SCREENWIDTH { COLS40 = 40, COLS80 = 80 };
const SCREENWIDTH screenWidth = COLS80;

// 16K screen buffer
emul::MemoryBlock screenB800(emul::S2A(0xB800), 0x4000, emul::MemoryType::RAM);

void DumpScreen();
void DumpBackLog();

bool showScreen = false;
void ToggleScreen()
{
	showScreen = !showScreen;
	if (showScreen)
	{
		DumpScreen();
	}
	else
	{
		fprintf(stderr, "Console mode\n");
		DumpBackLog();
	}
}

const size_t BACKLOG_MAX = 1000;
std::deque<std::string> backLog;

void DumpBackLog()
{
	while (backLog.size())
	{
		std::string& entry = backLog.front();
		fprintf(stderr, "%s", entry.c_str());
		backLog.pop_front();
	}
}

void LogCallback(const char *str)
{
	if (showScreen & !logFile)
	{
		backLog.push_back(str);
		while (backLog.size() > BACKLOG_MAX)
		{
			backLog.pop_front();
		}
	}
	else
	{
		fprintf(logFile ? logFile : stderr, str);
	}
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

void DumpScreen()
{
	for (WORD offset = 0; offset < 4000; offset += 2)
	{
		BYTE val = screenB800.read(emul::S2A(0xB800, offset));
		BYTE attr = screenB800.read(emul::S2A(0xB800, offset+1));

		size_t y = offset / (screenWidth * 2);
		size_t x = (offset/2) % screenWidth;

		console.WriteAt(x, y, val, attr);
	}
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

BYTE keyBuf[256];
BYTE keyBufRead = 0;
BYTE keyBufWrite = 0;

int main(void)
{
	//	logFile = fopen("./dump.log", "w");

#ifndef NO_CONSOLE
	console.Init(screenWidth);
#endif

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

	memory.Allocate(&screenB800);

	//emul::MemoryBlock test(emul::S2A(0x8000), 0x10000, emul::MemoryType::RAM);
	//test.LoadBinary(R"(C:\Programs\emu8086\MyBuild\sbb.com)", 0x0100);
	//memory.Allocate(&test);

	//emul::MemoryBlock extraRam(0x10000, 0x10000, emul::MemoryType::RAM);
	//memory.Allocate(&extraRam);

	pit::Device8254 pit(0x40, 1193182);
	pit.Init();
	pit.EnableLog(true, Logger::LOG_WARNING);

	pic::Device8259 pic(0x20);
	pic.Init();
	pic.EnableLog(true, Logger::LOG_WARNING);

	ppi::Device8255 ppi(0x60);
	ppi.Init();
	ppi.EnableLog(true, Logger::LOG_WARNING);

	// Configuration switches
	{
		ppi.SetPOSTLoop(false);
		ppi.SetMathCoprocessor(false);
		ppi.SetRAMConfig(ppi::RAMSIZE::RAM_64K);
		ppi.SetDisplayConfig(screenWidth == COLS80 ? ppi::DISPLAY::COLOR_80x25 : ppi::DISPLAY::COLOR_40x25);
		ppi.SetFloppyCount(1);
	}

	dma::Device8237 dma(0x00, memory);
	dma.Init();
	dma.EnableLog(false);
	dma.EnableLog(true, Logger::LOG_INFO);

	cga::DeviceCGA cga(0x3D0);
	cga.Init();
	cga.EnableLog(true, Logger::LOG_WARNING);

	fdc::DeviceFloppy floppy(0x03F0, 1193182);
	floppy.Init();
	floppy.EnableLog(true, Logger::LOG_INFO);
	floppy.LoadDiskImage("data/PC-DOS-1.00.img");
	//floppy.LoadDiskImage("data/PC-DOS-1.10.img");

	DummyPortSink sink;

	emul::CPU8086 cpu(memory, mmap);

#ifdef CPU_TEST
	{
		emul::CPU8086Test testCPU(memory, mmap);
		testCPU.EnableLog(true, Logger::LOG_DEBUG);
		testCPU.Test();
	}
#endif

	//	cpu.AddWatch("EXECUTE", onCall, onRet);

	cpu.AddDevice(pic);
	cpu.AddDevice(pit);
	cpu.AddDevice(ppi);
	cpu.AddDevice(dma);
	cpu.AddDevice(cga);
	cpu.AddDevice(floppy);
	cpu.AddDevice(sink);
	cpu.Reset();
	cpu.EnableLog(true, Logger::LOG_ERROR);

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

			if (showScreen && (ticks % 100000 == 0))
			{
				DumpScreen();
			}

			if ((ticks % 10000 == 0) && _kbhit())
			{
				BYTE keyCode;
				bool shift = false;
				bool newKeycode = false;
				int ch = _getch();

				if (ch == 27)
				{
					break;
				}
				else if (ch == 224)
				{
					switch (ch = _getch())
					{
					case 134: // F12
						ToggleScreen();
						break;
					}
				}
				else if (ch == 0)
				{
					switch (ch = _getch())
					{
					case 59: // F1
					case 60: // F2
					case 61: // F3
					case 62: // F4
					case 63: // F5
					case 64: // F6
					case 65: // F7
					case 66: // F8
						keyCode = ch;
						newKeycode = true;
						break;
					case 67: // F9
					{
						char buf[256];
						sprintf(buf, "memory.%lld.bin", time(nullptr));
						memory.Dump(0, 65536, buf);
						break;
					}
					case 68: // F10
						base64K.LoadBinary("data/BASIC_F600.BIN", 0x1000);
						cpu.Reset(0x0100, 0x0000);
						break;
					}
				}
				else 
				{
					SHORT vkey = VkKeyScanA(ch);
					shift = HIBYTE(vkey) & 1;
					keyCode = MapVirtualKeyA(LOBYTE(vkey), 0);
					newKeycode = true;
				}

				if (newKeycode)
				{
					if (shift)
					{
						keyBuf[keyBufWrite++] = 0x2A;
					}
					keyBuf[keyBufWrite++] = keyCode;
					keyBuf[keyBufWrite++] = keyCode | 0x80;
					if (shift)
					{
						keyBuf[keyBufWrite++] = 0x2A | 0x80;
					}
				}
			}

			if (ticks % 10000 == 0)
			{
				if (keyBufRead != keyBufWrite && cpu.CanInterrupt())
				{
					cpu.Interrupt(8 + 1); // Hardware interrupt 1: keyboard
					ppi.SetCurrentKeyCode(keyBuf[keyBufRead++]);
				}
			}

			pit.Tick();
			bool out = pit.GetCounter(0).GetOutput();
			if (out != timer0Out)
			{
				timer0Out = out;
				if (out)
				{
					// TODO: this bypasses a lot of things.
					// Quick and dirty for now: Check mask manually and interrupt cpu
					if (!(pic.Mask_IN() & 0x01))
					{
						cpu.Interrupt(8 + 6); // Hardware interrupt 0: timer
					}
				}
			}

			if (floppy.IsInterruptPending() && cpu.CanInterrupt())
			{
				//fprintf(stderr, "Fire Floppy interrupt\n");
				cpu.Interrupt(8 + 6); // Hardware interrupt 6: floppy
				floppy.ClearInterrupt();
			}

			dma.Tick();
			cga.Tick();
			floppy.Tick();

			// TODO: faking it
			if (floppy.IsDMAPending())
			{
				dma.DMARequest(2, true);
				//fprintf(stderr, "floppy DMA pending\n");
			}

			if (dma.DMAAcknowledged(2))
			{
				dma.DMARequest(2, false);

				// Do it manually
				floppy.DMAAcknowledge();
				dma.DMAWrite(2, floppy.ReadDataFIFO());
				if (dma.GetTerminalCount(2))
				{
					floppy.DMATerminalCount();
				}
				//fprintf(stderr, "floppy DMA read\n");
			}
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	time(&stopTime);

	cpu.Dump();

	DumpBackLog();

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


