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
//#define NO_CONSOLE
//#define CPU_TEST

FILE* logFile = nullptr;

enum class Mode { MONITOR = 0, CONSOLE = 1, LOG = 2};
Mode mode = Mode::LOG;

Console console;
emul::Monitor monitor(console);

void DumpScreen(cga::DeviceCGA& screen);

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

	if (mode == Mode::LOG)
	{
		fprintf(logFile ? logFile : stderr, str);
	}
}

void ToggleMode(cga::DeviceCGA& screen)
{
	switch (mode)
	{
	case Mode::MONITOR: mode = Mode::CONSOLE; break;
	case Mode::CONSOLE: mode = Mode::LOG; break;
	case Mode::LOG: mode = Mode::MONITOR; break;
	}

	switch (mode)
	{
	case Mode::MONITOR:
		monitor.Show();
		break;
	case Mode::CONSOLE:
		DumpScreen(screen);
		break;
	case Mode::LOG:
		console.Clear();
		DumpBackLog(24);
		break;
	}
}

void DumpScreen(cga::DeviceCGA& screen)
{
	for (WORD offset = 0; offset < 4000; offset += 2)
	{
		BYTE val = screen.GetVideoRAM().read(emul::S2A(0xB800, offset));
		BYTE attr = screen.GetVideoRAM().read(emul::S2A(0xB800, offset+1));

		// TODO: 40 cols
		short y = offset / (CONSOLE_COLS * 2);
		short x = (offset/2) % CONSOLE_COLS;

		console.WriteAt(x+1, y+1, val, attr);
	}
}

int main(void)
{
	//	logFile = fopen("./dump.log", "w");

#ifndef NO_CONSOLE
	console.Init(CONSOLE_COLS, CONSOLE_FONT_SIZE);
#endif

	Logger::RegisterLogCallback(LogCallback);

	emul::Computer pc;

	pc.Init();
	pc.Reset();
	pc.EnableLog(true, Logger::LOG_ERROR);

	monitor.Init(pc, pc.GetMemory());

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	time_t startTime, stopTime;
	time(&startTime);

	try
	{
		while (pc.Step())
		{ 
			if (((mode == Mode::MONITOR) || (pc.GetTicks() % 10000 == 0)) && _kbhit())
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
						ToggleMode(pc.GetCGA());
						break;
					}
				}
				else if (ch == 0)
				{
					if (mode == Mode::MONITOR)
					{
						monitor.SendKey(_getch());
					}
					else switch (ch = _getch())
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

			if (mode == Mode::CONSOLE && (pc.GetTicks() % 100000 == 0))
			{
				DumpScreen(pc.GetCGA());
			}
			else if (mode == Mode::MONITOR)
			{
				monitor.Update();
				monitor.Step(); // Waits for key if in STEP mode
			}
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	time(&stopTime);

	pc.Dump();

	DumpBackLog();

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


