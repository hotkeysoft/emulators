// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Memory.h"
#include "MemoryBlock.h"
#include "MemoryMap.h"
#include "Computer.h"
#include "Console.h"
#include "Monitor.h"

#include <conio.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <Windows.h>

const short CONSOLE_FONT_SIZE = 22;
const short CONSOLE_COLS = 80;
#define NO_CONSOLE
//#define CPU_TEST

#ifdef CPU_TEST
#include "CPU8086Test.h"
#endif

FILE* logFile = nullptr;

enum class Mode { MONITOR = 0, LOG = 2};
Mode mode = Mode::LOG;

Console console;
emul::Monitor monitor(console);

const size_t BACKLOG_MAX = 1000;
std::string backLog[BACKLOG_MAX];
size_t backLogPtr = 0;

void DumpBackLog(size_t lastN = BACKLOG_MAX)
{
	size_t ptr = (backLogPtr + (BACKLOG_MAX - lastN)) % BACKLOG_MAX;
	for (size_t pos = 0; pos < lastN; ++pos)
	{
		fprintf(stderr, "%.79s", backLog[(ptr + pos) % BACKLOG_MAX].c_str());
	}
}

void LogCallback(const char* str)
{
	backLogPtr = (backLogPtr + 1) % BACKLOG_MAX;
	backLog[backLogPtr] = str;

	if ((mode == Mode::LOG) || logFile)
	{
		fprintf(logFile ? logFile : stderr, str);
	}
}

void ToggleMode(cga::DeviceCGA& screen)
{
	switch (mode)
	{
	case Mode::MONITOR: mode = Mode::LOG; break;
	case Mode::LOG: mode = Mode::MONITOR; break;
	}

	switch (mode)
	{
	case Mode::MONITOR:
		monitor.Show();
		break;
	case Mode::LOG:
		console.Clear();
		DumpBackLog(24);
		break;
	}
}

int main(void)
{
	//logFile = fopen("./dump.log", "w");

#ifndef NO_CONSOLE
	console.Init(CONSOLE_COLS, CONSOLE_FONT_SIZE);
#endif

	Logger::RegisterLogCallback(LogCallback);

#ifdef CPU_TEST
	{
		emul::Memory mem(20);
		emul::MemoryMap mmap;
		emul::CPU8086Test testCPU(mem, mmap);
		testCPU.Test();

		fprintf(stderr, "Press any key to continue\n");
		_getch();
		return 0;}
#endif

	emul::Computer pc;

	emul::MemoryBlock biosF000("BIOS0", 0x8000, emul::MemoryType::ROM);
	biosF000.LoadBinary("data/BIOS_5160_V3_F000.BIN");
	pc.GetMemory().Allocate(&biosF000, emul::S2A(0xF000));

	emul::MemoryBlock biosF800("BIOS1", 0x8000, emul::MemoryType::ROM);
	biosF800.LoadBinary("data/BIOS_5160_V3_F800.BIN");
	pc.GetMemory().Allocate(&biosF800, emul::S2A(0xF800));

	emul::MemoryBlock testROMF000("TEST", 0x10000, emul::MemoryType::ROM);
	testROMF000.LoadBinary(R"(C:\Users\hotkey\Actual Documents\electro\PC\80186_tests\fail\div.bin)");
	//pc.GetMemory().Allocate(&testROMF000, emul::S2A(0xF000));

	pc.Init();
	pc.Reset();
	//pc.Reset(0xF000, 0);
	pc.EnableLog(true, Logger::LOG_INFO);
	//pc.EnableLog(true, Logger::LOG_DEBUG);

#ifndef NO_CONSOLE
	monitor.Init(pc, pc.GetMemory());
#endif

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	if (mode == Mode::MONITOR)
		monitor.Show();

	time_t startTime, stopTime;
	time(&startTime);

	try
	{
		bool run = true;
		while (run)
		{ 
			if (mode == Mode::MONITOR)
			{
				switch (monitor.Run())
				{
				case emul::MonitorState::EXIT:
					run = false;
					break;
				case emul::MonitorState::WAIT:
					break;
				case emul::MonitorState::RUN:
					run = pc.Step();
					break;
				case emul::MonitorState::SWITCH_MODE:
					ToggleMode(pc.GetCGA());
					break;
				}
			}
			else
			{
				run = pc.Step();
			}

			if (mode != Mode::MONITOR && (pc.GetTicks() % 10000 == 0) && _kbhit())
			{
				BYTE keyCode;
				bool shift = false;
				bool newKeycode = false;
				int ch = _getch();

				if (ch == 27)
				{
					run = false;
				}
				else if (ch == 224)
				{
					switch (ch = _getch())
					{
					case 71: // HOME
					case 72: // UP
					case 73: // PGUP
					case 75: // LEFT
					case 77: // RIGHT
					case 79: // END
					case 80: // DOWN
					case 81: // PGDOWN
					case 82: // INSERT
					case 83: // DELETE
						keyCode = ch;
						newKeycode = true;
						break;

					case 134: // F12
						ToggleMode(pc.GetCGA());
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
					case 67: // F9
					case 68: // F10
					case 71: // HOME
					case 72: // UP
					case 73: // PGUP
					case 74: // -
					case 75: // LEFT
					case 76: // 5
					case 77: // RIGHT
					case 78: // +
					case 79: // END
					case 80: // DOWN
					case 81: // PGDOWN
					case 82: // INSERT
					case 83: // DELETE
						keyCode = ch;
						newKeycode = true;
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
						pc.InputKey(0x2A);
					}
					pc.InputKey(keyCode);
					pc.InputKey(keyCode | 0x80);
					if (shift)
					{
						pc.InputKey(0x2A | 0x80);
					}
				}
			}
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	time(&stopTime);

	pc.Dump();

	pc.GetMemory().Dump(0, 65536, R"(memdump.bin)");

	//DumpBackLog();

	if (logFile)
	{
		fclose(logFile);
	}

	fprintf(stderr, "Time elapsed: %I64u\n", stopTime-startTime);
	pc.getTime();
	fprintf(stderr, "CPU ticks: %zu\n", pc.GetTicks());
	if (stopTime - startTime > 1)
	{
		fprintf(stderr, "Avg speed: %I64u ticks/s\n", pc.GetTicks() / (stopTime - startTime));
	}

	return 0;
}
