// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Memory.h"
#include "MemoryBlock.h"
#include "MemoryMap.h"
#include "Computer.h"
#include "Console.h"
#include <conio.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <Windows.h>

//#define NO_CONSOLE
//#define CPU_TEST

FILE* logFile = nullptr;

Console console;

void DumpScreen(cga::DeviceCGA& screen);
void DumpBackLog();

bool showScreen = false;
void ToggleScreen(cga::DeviceCGA& screen)
{
	showScreen = !showScreen;
	if (showScreen)
	{
		DumpScreen(screen);
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

void DumpScreen(cga::DeviceCGA& screen)
{
	for (WORD offset = 0; offset < 4000; offset += 2)
	{
		BYTE val = screen.GetVideoRAM().read(emul::S2A(0xB800, offset));
		BYTE attr = screen.GetVideoRAM().read(emul::S2A(0xB800, offset+1));

		// TODO: 40 cols
		short y = offset / (80 * 2);
		short x = (offset/2) % 80;

		console.WriteAt(x, y, val, attr);
	}
}

int main(void)
{
	//	logFile = fopen("./dump.log", "w");

#ifndef NO_CONSOLE
	console.Init(80);
#endif

	Logger::RegisterLogCallback(LogCallback);

	emul::Computer pc;

	pc.Init();
	pc.Reset();
	pc.EnableLog(true, Logger::LOG_ERROR);

	fprintf(stderr, "Press any key to continue\n");
	_getch();

	time_t startTime, stopTime;
	time(&startTime);

	try
	{
		while (pc.Step())
		{ 
			if (showScreen && (pc.GetTicks() % 100000 == 0))
			{
				DumpScreen(pc.GetCGA());
			}

			if ((pc.GetTicks() % 10000 == 0) && _kbhit())
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
						ToggleScreen(pc.GetCGA());
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
						pc.GetMemory().Dump(0, 65536, buf);
						break;
					}
					case 68: // F10
						pc.LoadBinary("data/BASIC_F600.BIN", 0x1000);
						pc.Reset(0x0100, 0x0000);
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


