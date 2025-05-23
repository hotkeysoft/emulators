#include "stdafx.h"

#include <Config.h>
#include <FileUtil.h>
#include <UI/MainWindow.h>
#include <UI/Overlay.h>
#include <Sound/Sound.h>

#include <CPU/Memory.h>
#include <CPU/MemoryBlock.h>

#include "IO/Console.h"
#include "IO/MonitorBase.h"
#include "IO/Monitor6800.h"
#include "IO/Monitor6809.h"

#include <Computer/ComputerBase.h>
#include "Computer6800.h"
#include "Computer6803.h"
#include "Computer6809.h"
#include "ComputerThomson.h"
#include "ComputerMC10.h"

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

namespace fs = std::filesystem;

using cfg::CONFIG;
using emul::ComputerBase;
using sound::SOUND;
using ui::MAINWND;
using ui::Overlay;

size_t emul::g_ticks = 0;

hscommon::fileUtil::File logFile;

enum class Mode { MONITOR = 0, LOG = 2};
Mode mode = Mode::LOG;
emul::ADDRESS customMemoryView = 0;

Console console;
emul::MonitorBase* monitor = nullptr;

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

bool consoleInit = false;
void ShowMonitor()
{
	if (!consoleInit)
	{
		consoleInit = true;
		console.Init(CONSOLE_COLS, CONSOLE_FONT_SIZE);
	}
	monitor->SetCustomMemoryView(customMemoryView);

	if (mode != Mode::MONITOR)
	{
		mode = Mode::MONITOR;
		monitor->Show();
	}
	else
	{
		monitor->SetStepMode();
	}
}

void ToggleMode()
{
	switch (mode)
	{
	case Mode::LOG:
		ShowMonitor();
		mode = Mode::MONITOR;
		break;
	case Mode::MONITOR:
		mode = Mode::LOG;
		console.Clear();
		DumpBackLog(24);
		break;
	}
}

void InitSound()
{
	SOUND().Init();
	SOUND().EnableLog(CONFIG().GetLogLevel("sound"));
	SOUND().SetMute(CONFIG().GetValueBool("sound", "mute"));
	SOUND().SetMasterVolume(CONFIG().GetValueInt32("sound", "volume", 128));

	std::string audioStream = CONFIG().GetValueStr("sound", "raw");
	if (audioStream.size())
	{
		SOUND().StreamToFile(true, audioStream.c_str());
	}
}

ComputerBase* CreateComputer(std::string arch)
{
	ComputerBase* computer = nullptr;

	if (arch == "6800")
	{
		computer = new emul::Computer6800();
	}
	else if (arch == "6803")
	{
		computer = new emul::Computer6803();
	}
	else if (arch == "6809")
	{
		computer = new emul::Computer6809();
	}
	else if (arch == "thomson")
	{
		computer = new emul::ComputerThomson();
	}
	else if (arch == "trs80")
	{
		computer = new emul::ComputerMC10();
	}

	return computer;
}

void InitPC(ComputerBase* pc, Overlay& overlay, bool reset = true)
{
	pc->EnableLog(CONFIG().GetLogLevel("computer"));
	if (reset)
	{
		pc->Reset();
	}

	delete monitor;

	std::string cpuID = pc->GetCPU()->GetID();
	if (cpuID == "6800" || cpuID == "6803")
	{
		monitor = new emul::Monitor6800(console);
	}
	else if (cpuID == "6809")
	{
		monitor = new emul::Monitor6809(console);
	}
	else
	{
		throw std::exception("Unknown cpuID");
	}

	monitor->Init(pc->GetCPU(), pc->GetMemory());

	overlay.SetPC(pc);

	pc->GetVideo().AddRenderer(&overlay);
	pc->GetInputs().AddEventHandler(&overlay);
	pc->GetInputs().AddEventHandler(&(pc->GetVideo())); // Window resize events
}

void InitLeakCheck()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
}

bool restoreSnapshot = false;
json snapshotData;
fs::path snapshotDir;

// Here we only save some info to be retreived in the main loop.
// We cannot restore the actual pc here becore we're deep in the
// computer message handlers. So we set a flag
// and to the actual restore and new pc creation from the main loop.
void NewComputerCallback(fs::path dir, json& j)
{
	if (restoreSnapshot)
	{
		fprintf(stderr, "Loaded snapshot already pending, bailing out\n");
		return;
	}

	restoreSnapshot = true;
	snapshotData = j;
	snapshotDir = dir;
}

// Restoring a snapshot with a new computer architecture is a bit tricky
//
// To deserialize a pc from json data, you need to have it created
// and initialized.
//
// However as soon as you create a new Computer object,
// you reset some internal static things to point to the new computer object.
//
// Same things happens with port connections, which are global and reset
// when you initialize the new Computer.
//
// This means that as soon as you call CreateComputer(), you can *not* use the
// old Computer object anymore for anything.
//
// This also means that if the deseriaization fails with the new Computer, you can
// not fall back to the previous one. You have to carry on with the new Computer.
ComputerBase* RestoreNewComputerFromSnapshot()
{
	fprintf(stderr, "Restore new computer from snapshot\n");
	ComputerBase* newPC = nullptr;

	// Until we have created the new pc object, we can bail out and
	// continue with the old PC.
	bool failureIsFatal = false;

	// First, load the config file so we have the proper hardware
	fs::path config = snapshotDir;
	config.append("config.ini");
	if (!CONFIG().LoadConfigFile(config.string().c_str()))
	{
		fprintf(stderr, "Unable to read config file %s, aborting\n", config.string().c_str());
	}

	std::string arch = CONFIG().GetValueStr("core", "arch");
	fprintf(stderr, "NewComputerCallback: Create new PC [%s]\n", arch.c_str());

	try
	{
		newPC = CreateComputer(arch);
		if (newPC)
		{
			// Starting here, anything failing is non-recoverable
			failureIsFatal = true;

			int32_t baseRAM = CONFIG().GetValueInt32("core", "baseram");
			newPC->Init(baseRAM);

			newPC->SetSerializationDir(snapshotDir);
			newPC->Deserialize(snapshotData);
		}
		else
		{
			fprintf(stderr, "Unknown architecture\n");
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Unable to restore PC: %s\n", e.what());
		delete newPC;
		newPC = nullptr;
	}

	restoreSnapshot = false;
	snapshotData.clear();
	snapshotDir.clear();

	if (!newPC && failureIsFatal)
	{
		// Bail out for now
		// TODO: Before restoring a snapshot, we should save state.
		// If restore fails, re-create the original pc and restore the saved snapshot
		fprintf(stderr, "Unrecoverable error while restoring snapshot\n");
		exit(3);
	}

	return newPC;
}

int main(int argc, char* args[])
{
	InitLeakCheck();

	Logger::RegisterLogCallback(LogCallback);

	if (!CONFIG().LoadConfigFile("config/config.ini"))
	{
		return 1;
	}

	std::string logFileName = CONFIG().GetValueStr("debug", "logfile");
	if (logFileName.size())
	{
		Logger::EnableColors(false);

		fprintf(stdout, "Logging to output file: %s\n", logFileName.c_str());

		bool flush = CONFIG().GetValueBool("debug", "logfile.flush");

		logFile.Open(logFileName.c_str(), flush ? "wc" : "w");
		if (!logFile)
		{
			fprintf(stderr, "Error opening log file\n");
		}
	}

	bool breakpointEnabled = false;
	emul::ADDRESS breakpoint = CONFIG().GetValueDWORD("monitor", "breakpoint", 0xFFFFFFFF);
	if (breakpoint <= 0xFFFF)
	{
		breakpointEnabled = true;
		fprintf(stderr, "Set Breakpoint to [0x%04X]\n", breakpoint);
	}

	customMemoryView = CONFIG().GetValueWORD("monitor", "custommem", 0);
	fprintf(stderr, "Set Monitor Custom Memory View to [0x%04X]\n", customMemoryView);

	MAINWND().Init("hotkey6800emu", Overlay::GetOverlayHeight());
	InitSound();

	// TODO: switchable overlay
	Overlay overlay;
	overlay.Init();
	overlay.SetNewComputerCallback(NewComputerCallback);
	bool showOverlay = true;

	std::string arch = CONFIG().GetValueStr("core", "arch");
	ComputerBase* pc = CreateComputer(arch);
	if (!pc)
	{
		fprintf(stderr, "Unknown architecture: [core].arch=[%s]", arch.c_str());
		return 2;
	}

	int32_t baseRAM = CONFIG().GetValueInt32("core", "baseram");
	pc->Init(baseRAM);
	InitPC(pc, overlay);

	if (breakpointEnabled)
	{
		monitor->SetBreakpoint(breakpoint);
	}
	else
	{
		monitor->ClearBreakpoint();
	}

	if (mode == Mode::MONITOR)
	{
		ShowMonitor();
	}

	const int cooldownFactor = 10000;

	try
	{
		std::string snapshotDir;
		bool run = true;
		int cooldown = cooldownFactor;

		while (run)
		{
			if (monitor->IsBreakpoint())
			{
				ShowMonitor();
				mode = Mode::MONITOR;
			}

			if (mode == Mode::MONITOR)
			{
				switch (monitor->Run())
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

			// Some things don't need to be updated every cpu tick.
			// Do them every n steps.
			if (--cooldown == 0)
			{
				cooldown = cooldownFactor;

				// Overlay update
				//
				// For now this only updates the fdd/hdd LEDs.
				// The actual GUI is refreshed in a callback above
				if (showOverlay && !overlay.Update())
				{
					break;
				}

				if (mode != Mode::MONITOR && _kbhit())
				{
					BYTE keyCode;
					bool shift = false;
					bool ctrl = false;
					bool alt = false;
					bool newKeycode = false;
					int ch = _getch();

					if (ch == 224)
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
							showOverlay = !showOverlay;
							fprintf(stderr, "%s Overlay\n", showOverlay ? "Show" : "Hide");
							overlay.Show(showOverlay);
							break;
						case FKEY + 2:
						case FKEY + 3:
						case FKEY + 4:
							break;
						case FKEY + 5:
						{
							char buf[128];
							sprintf(buf, "dump/RAM_%zu.bin", time(nullptr));
							fprintf(stderr, "Dump RAM to %s\n", buf);
							pc->GetMemory().Dump(0x4000, 0x1000, buf);
							break;
						}
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
							SOUND().SetMasterVolume(SOUND().GetMasterVolume() - 1);
							break;
						case '+':
							SOUND().SetMasterVolume(SOUND().GetMasterVolume() + 1);
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
						//kbd::DeviceKeyboard& kbd = pc->GetKeyboard();

						//if (shift) kbd.InputKey(0x2A);
						//if (alt) kbd.InputKey(0x38);
						//if (ctrl) kbd.InputKey(0x1D);

						//kbd.InputKey(keyCode);
						//kbd.InputKey(keyCode | 0x80);

						//if (shift) kbd.InputKey(0x2A | 0x80);
						//if (alt) kbd.InputKey(0x38 | 0x80);
						//if (ctrl) kbd.InputKey(0x1D | 0x80);
					}
				}

				if (restoreSnapshot)
				{
					ComputerBase* newPC = RestoreNewComputerFromSnapshot();
					if (newPC)
					{
						delete pc;
						pc = newPC;
						InitPC(pc, overlay, false);
					}
				}
			}
		}
	}
	catch (std::exception e)
	{
		fprintf(stderr, "Error while running cpu [%s]\n", e.what());
	}

	// Cleanup
	{
		delete pc;
		pc = nullptr;

		delete monitor;
		monitor = nullptr;


		SOUND().Cleanup();

		fprintf(stderr, "Shutdown SDL Subsystems\n");
		SDL_Quit();
	}

	return 0;
}
