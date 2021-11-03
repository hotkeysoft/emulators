// Emulator Base.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Memory.h"
#include "MemoryBlock.h"
#include "MemoryMap.h"
#include "Console.h"
#include "Monitor.h"

#include "ComputerXT.h"
#include "ComputerPCjr.h"
#include "ComputerTandy.h"

#include "Config.h"

#include <conio.h>
#include <vector>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <Windows.h>
#include <commdlg.h>

const short CONSOLE_FONT_SIZE = 22;
const short CONSOLE_COLS = 80;
#define NO_CONSOLE
//#define CPU_TEST

#ifdef CPU_TEST
#include "CPU8086Test.h"
#endif

using cfg::Config;

size_t emul::g_ticks = 0;

FILE* logFile = nullptr;

enum class Mode { MONITOR = 0, LOG = 2};
Mode mode = Mode::LOG;

Console console;
emul::Monitor monitor(console);

const size_t BACKLOG_MAX = 1000;
std::string backLog[BACKLOG_MAX];
size_t backLogPtr = 0;

bool SelectFile(std::string& path)
{
	OPENFILENAMEA ofn;
	char szFile[1024];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Floppy Image\0*.IMG\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.

	if (!GetOpenFileNameA(&ofn))
	{
		return false;
	}
	path = szFile;
	return true;
}

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
		if (logFile)
		{
			fflush(logFile);
		}
	}
}

void ToggleMode()
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

bool ToggleTurbo()
{
	static bool turbo = false;

	turbo = !turbo;
	fprintf(stderr, "Turbo [%s]\n", turbo ? "ON" : "OFF");
	return turbo;
}

int main(int argc, char* args[])
{
	//logFile = fopen("./dump/dump.log", "w");

#ifndef NO_CONSOLE
	console.Init(CONSOLE_COLS, CONSOLE_FONT_SIZE);
#endif

	Logger::RegisterLogCallback(LogCallback);

	if (!Config::Instance().LoadConfigFile("config/config.ini"))
	{
		return 1;
	}

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

	emul::Computer* pc = nullptr;

	std::string arch = Config::Instance().GetValueStr("core", "arch");
	if (arch == "xt")
	{
		pc = new emul::ComputerXT();
	}
	else if (arch == "pcjr")
	{
		pc = new emul::ComputerPCjr();
		
	}
	else if (arch == "tandy")
	{
		pc = new emul::ComputerTandy();
	}
	else
	{
		fprintf(stderr, "Unknown architecture: [core].arch=[%s]", arch.c_str());
		return 2;
	}

	pc->Init();
	pc->Reset();

#if 0
	emul::MemoryBlock testROMF000("TEST", 0x10000, emul::MemoryType::ROM);
	testROMF000.LoadFromFile(R"(C:\Users\hotkey\Actual Documents\electro\PC\80186_tests\fail\segpr.bin)");
	pc->GetMemory().Allocate(&testROMF000, emul::S2A(0xF000));
	pc->Reset(0xF000, 0);
#endif

	pc->EnableLog(true, Logger::LOG_INFO);
	//pc->EnableLog(true, Logger::LOG_DEBUG);

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
		int cooldown = 10000;

		while (run)
		{ 
			//if ((pc.GetCurrentAddress() == emul::S2A(0xF000, 0x0450))/* &&
			//	(pc.GetCurrentAddress() < emul::S2A(0xF000, 0x0E24F))*/)
			//{
			//	monitor.SetCustomMemoryView(0x40, 0x80);
			//	monitor.Show();
			//	mode = Mode::MONITOR;
			//}

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
					run = pc->Step();
					break;
				case emul::MonitorState::SWITCH_MODE:
					ToggleMode();
					break;
				}
			}
			else
			{
				run = pc->Step();
			}

			// TODO: Keyboard input is now handled in InputEvents.
			// We can focus here on console-specific commands/hotkeys
			if (cooldown == 0)
			{
				cooldown = 10000;

				if (mode != Mode::MONITOR && _kbhit())
				{
					cooldown = 10000;
					BYTE keyCode;
					bool shift = false;
					bool ctrl = false;
					bool alt = false;
					bool newKeycode = false;
					int ch = _getch();

					/*if (ch == 27)
					{
						run = false;
					}
					else */if (ch == 224)
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
							ToggleMode();
							break;
						}
					}
					else if (ch == 0)
					{
						std::string diskImage;
						switch (ch = _getch())
						{
						case 59: // F1
							if (SelectFile(diskImage))
							{
								pc->GetFloppy().LoadDiskImage(0, diskImage.c_str());
							}
							break;
						case 60: // F2
							if (SelectFile(diskImage))
							{
								pc->GetFloppy().LoadDiskImage(1, diskImage.c_str());
							}
							break;
						case 61: // F3
							pc->SetTurbo(ToggleTurbo());
							break;
						case 62: // F4
							break;
						case 63: // F5
						{
							char buf[128];
							sprintf(buf, "dump/memdump_%zu.bin", time(nullptr));
							pc->GetMemory().Dump(0, 65536 * 2, buf);
							break;
						}
						case 64: // F6
						case 65: // F7
						case 66: // F8
						case 67: // F9
							break;
						case 68: // F10
							pc->Reboot();
							break;
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
						shift = GetAsyncKeyState(VK_SHIFT);
						ctrl = GetAsyncKeyState(VK_CONTROL);
						alt = GetAsyncKeyState(VK_MENU);
						keyCode = MapVirtualKeyA(LOBYTE(vkey), 0);
						newKeycode = true;
					}

					if (newKeycode)
					{
						kbd::DeviceKeyboard& kbd = pc->GetKeyboard();


						if (shift) kbd.InputKey(0x2A);
						if (alt) kbd.InputKey(0x38);
						if (ctrl) kbd.InputKey(0x1D);

						kbd.InputKey(keyCode);
						kbd.InputKey(keyCode | 0x80);

						if (shift) kbd.InputKey(0x2A | 0x80);
						if (alt) kbd.InputKey(0x38 | 0x80);
						if (ctrl) kbd.InputKey(0x1D | 0x80);
					}
				}
			}
			else
			{
				--cooldown;
			}
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	time(&stopTime);

	pc->Dump();

	pc->GetMemory().Dump(0, 65536, "dump/memdump.bin");
	//pc->GetFloppy().SaveDiskImage(0, "dump/floppy0.img");

	//DumpBackLog();

	delete pc;
	pc = nullptr;

	if (logFile)
	{
		fclose(logFile);
	}

	fprintf(stderr, "Time elapsed: %I64u\n", stopTime-startTime);

	return 0;
}
