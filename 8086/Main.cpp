#include "CPU/Memory.h"
#include "CPU/MemoryBlock.h"
#include "CPU/MemoryMap.h"
#include "IO/Console.h"
#include "IO/Monitor.h"

#include "UI/Overlay.h"

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
#include <filesystem>

const short CONSOLE_FONT_SIZE = 22;
const short CONSOLE_COLS = 80;
#define NO_CONSOLE
//#define CPU_TEST

#ifdef CPU_TEST
#include "CPU8086Test.h"
#include "Main.h"
#endif

namespace fs = std::filesystem;
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
	ui::Overlay overlay;

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

	int masterVolume = Config::Instance().GetValueInt32("sound", "volume", 128);
	pc->GetSound().SetMasterVolume(masterVolume);

	int32_t baseRAM = Config::Instance().GetValueInt32("core", "baseram", 640);

	pc->EnableLog(Config::Instance().GetLogLevel("pc"));
	pc->Init(baseRAM);
	pc->Reset();

	emul::Computer::CPUSpeeds speedList = pc->GetCPUSpeeds();
	bool turbo = Config::Instance().GetValueBool("core", "turbo");

	emul::Computer::CPUSpeeds::const_iterator currSpeed = speedList.begin();
	if (turbo)
	{
		currSpeed = --speedList.end();
	}

	pc->SetCPUSpeed(*currSpeed);

#if 0
	emul::MemoryBlock testROMF000("TEST", 0x10000, emul::MemoryType::ROM);
	
	testROMF000.LoadFromFile(R"(C:\Users\hotkey\Actual Documents\electro\PC\80186_tests\pass\add.bin)");
	pc->GetMemory().Allocate(&testROMF000, emul::S2A(0xF000));
	pc->Reset(0xF000, 0);
#endif

#ifndef NO_CONSOLE
	monitor.Init(*pc, pc->GetMemory());
#endif

	//fprintf(stderr, "Press any key to continue\n");
	//_getch();

	if (mode == Mode::MONITOR)
		monitor.Show();

	time_t startTime, stopTime;
	time(&startTime);

	// TODO: sdl window is created in Video class, not ideal
	overlay.Init(pc);

	pc->GetVideo().AddRenderer(&overlay);
	pc->GetInputs().AddEventHandler(&overlay);
	pc->GetInputs().AddEventHandler(&(pc->GetVideo())); // Window resize events

	try
	{
		std::string snapshotDir;
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

			if (!overlay.Update())
			{
				break;
			}

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
						const int FKEY = 58;
						const int SHIFT = 25;
						const int CTRL = 35;
						const int ALT = 45;

						switch (ch = _getch())
						{
						case FKEY + 1:
						case FKEY + 2:
						case FKEY + 3:
						case FKEY + 4:
						case FKEY + 5:
						case FKEY + 6:
						case FKEY + 7:
						case FKEY + 8:
						case FKEY + 9:
						case FKEY + 10:
							break;
						default:
							fprintf(stderr, "Unknown extended keycode: [0][%d]\n", ch);
						}
					}
					else
					{
						switch (ch)
						{
						case '-':
							pc->GetSound().SetMasterVolume(pc->GetSound().GetMasterVolume() - 1);
							break;
						case '+':
							pc->GetSound().SetMasterVolume(pc->GetSound().GetMasterVolume() + 1);
							break;
						default:
							{
								SHORT vkey = VkKeyScanA(ch);
								shift = GetAsyncKeyState(VK_SHIFT);
								ctrl = GetAsyncKeyState(VK_CONTROL);
								alt = GetAsyncKeyState(VK_MENU);
								keyCode = MapVirtualKeyA(LOBYTE(vkey), 0);
								newKeycode = true;
							}
						}
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
