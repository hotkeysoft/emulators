#include "Overlay.h"
#include "../Config.h"
#include "../Storage/DeviceFloppy.h"
#include "../Storage/DeviceHardDrive.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>

#pragma warning(disable:4251)

#include <Common.h>
#include <Core/Window.h>
#include <Core/WindowManager.h>
#include <Core/ResourceManager.h>
#include <Core/Widget.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarItem.h>
#include <Widgets/Button.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

using namespace cfg;
using namespace CoreUI;

namespace ui
{
	HWND GetHWND(SDL_Window* sdlWindow)
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(sdlWindow, &wmInfo);
		return wmInfo.info.win.window;
	}

	bool SelectFile(fs::path& path, HWND parent)
	{
		OPENFILENAMEA ofn;
		char szFile[1024];

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = parent;
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

	Overlay::Overlay() : Logger("GUI")
	{
	}

	bool Overlay::Init(emul::Computer* pc)
	{
		LogPrintf(LOG_TRACE, "Overlay::Init");

		assert(pc);
		m_pc = pc;

		if (TTF_Init() == -1)
		{
			LogPrintf(LOG_ERROR, "Failed to initialize TTF: %s", SDL_GetError());
			return false;
		}

		m_renderer = pc->GetVideo().GetRenderer();
		m_window = pc->GetVideo().GetWindow();

		RES().Init(m_renderer);
		WINMGR().Init(m_window, m_renderer);

		Rect windowSize = WINMGR().GetWindowSize();
		Rect toolbarRect = windowSize;
		toolbarRect.h = 64;
		toolbarRect.y = windowSize.h - toolbarRect.h;

		m_mainWnd = WINMGR().AddWindow("status", toolbarRect, WIN_ACTIVE | WIN_CANMOVE | WIN_NOSCROLL);

		RES().LoadImageMap("toolbar", "./res/toolbar.png", 16, 16);

		ToolbarPtr toolbar = Toolbar::CreateAutoSize(m_renderer, "toolbar");
		toolbar->SetBackgroundColor(Color::C_MED_GREY);

		m_floppyInactive = RES().FindImage("toolbar", 0);
		m_floppyActive = RES().FindImage("toolbar", 4);
		m_hddInactive = RES().FindImage("toolbar", 1);
		m_hddActive = RES().FindImage("toolbar", 5);

		if (m_pc->GetFloppy())
		{
			m_floppy0 = toolbar->AddToolbarItem("floppy0", m_floppyInactive, "A:");
			m_eject0 = toolbar->AddToolbarItem("eject0", RES().FindImage("toolbar", 7));

			toolbar->AddSeparator();

			m_floppy1 = toolbar->AddToolbarItem("floppy1", m_floppyInactive, "B:");
			m_eject1 = toolbar->AddToolbarItem("eject1", RES().FindImage("toolbar", 7));

			toolbar->AddSeparator();
		}

		if (m_pc->GetHardDrive())
		{
			m_hdd0 = toolbar->AddToolbarItem("hdd0", m_hddInactive, "C:");
			m_hdd1 = toolbar->AddToolbarItem("hdd1", m_hddInactive, "D:");

			toolbar->AddSeparator();
		}

		m_speed = toolbar->AddToolbarItem("speed", RES().FindImage("toolbar", 6), " 0.00 MHz");

		toolbar->AddSeparator();

		m_snapshot = toolbar->AddToolbarItem("saveSnapshot", RES().FindImage("toolbar", 8));
		toolbar->AddToolbarItem("loadSnapshot", RES().FindImage("toolbar", 9));

		m_mainWnd->SetToolbar(toolbar);
		m_mainWnd->SetText(m_pc->GetName());
		UpdateSpeed();

		GetSnapshotBaseDirectory(m_snapshotBaseDirectory);
		LogPrintf(LOG_INFO, "Snapshot base directory is [%s]", m_snapshotBaseDirectory.string().c_str());
		if (!GetLastSnapshotDirectory(m_lastSnapshotDir))
		{
			m_lastSnapshotDir.clear();
		}
		UpdateSnapshot();

		return true;
	}

	void Overlay::UpdateSpeed()
	{
		static char speedStr[32];
		emul::CPUSpeed speed = m_pc->GetCPUSpeed();
		sprintf(speedStr, "%5.2fMHz", speed.GetSpeed() / 1000000.0f);
		m_speed->SetText(speedStr);
	}

	void Overlay::UpdateSnapshot()
	{
		if (m_lastSnapshotDir.empty())
		{
			m_snapshot->SetText("");
		}
		else
		{
			std::string name = GetSnapshotName(m_lastSnapshotDir);
			m_snapshot->SetText(name.c_str());
		}
	}

	bool Overlay::Update()
	{
		if (m_pc->GetFloppy())
		{
			m_floppy0->SetImage(m_pc->GetFloppy()->IsActive(0) ? m_floppyActive : m_floppyInactive);
			m_floppy1->SetImage(m_pc->GetFloppy()->IsActive(1) ? m_floppyActive : m_floppyInactive);
		}

		if (m_pc->GetHardDrive())
		{
			m_hdd0->SetImage(m_pc->GetHardDrive()->IsActive(0) ? m_hddActive : m_hddInactive);
			m_hdd1->SetImage(m_pc->GetHardDrive()->IsActive(1) ? m_hddActive : m_hddInactive);
		}

		return true;
	}

	// video::Renderer
	void Overlay::Render()
	{
		WINMGR().Draw();
	}

	void Overlay::LoadDiskImage(BYTE drive, ToolbarItemPtr toolbarItem, const char* str, bool eject)
	{
		if (!m_pc->GetFloppy())
		{
			return;
		}
		fs::path diskImage;

		if (eject)
		{
			m_pc->GetFloppy()->ClearDiskImage(drive);
			toolbarItem->SetText(str);
		}
		else if (SelectFile(diskImage, GetHWND(m_window)))
		{
			std::string label = str;
			label += " ";
			label += diskImage.filename().string();
			m_pc->GetFloppy()->LoadDiskImage(drive, diskImage.string().c_str());
			toolbarItem->SetText(label.c_str());
		}
	}

	void Overlay::ToggleCPUSpeed()
	{
		const emul::CPUSpeed currSpeed = m_pc->GetCPUSpeed();
		const emul::Computer::CPUSpeeds& speeds = m_pc->GetCPUSpeeds();
		emul::Computer::CPUSpeeds::const_iterator currIt = speeds.find(currSpeed);
		if (currIt == speeds.end())
		{
			currIt = speeds.begin();
		}
		else
		{
			++currIt;
			if (currIt == speeds.end())
			{
				currIt = speeds.begin();
			}		
		}
		m_pc->SetCPUSpeed(*currIt);
		UpdateSpeed();
	}

	void Overlay::SaveSnapshot(const fs::path& snapshotDir)
	{
		json j;
		j["core"]["arch"] = Config::Instance().GetValueStr("core", "arch");
		j["core"]["baseram"] = Config::Instance().GetValueInt32("core", "baseram", 640);

		m_pc->SetSerializationDir(snapshotDir);
		m_pc->Serialize(j);

		fs::path outFile = snapshotDir;
		outFile.append("computer.json");

		std::ofstream outStream(outFile.string());
		outStream << std::setw(4) << j;

		LogPrintf(LOG_INFO, "SaveSnapshot: Saved to [%s]\n", outFile.string().c_str());

		m_lastSnapshotDir = snapshotDir;
		UpdateSnapshot();
	}

	void Overlay::RestoreSnapshot(const fs::path& snapshotDir)
	{
		fs::path inFile = snapshotDir;
		inFile.append("computer.json");
		std::ifstream inStream(inFile);

		LogPrintf(LOG_INFO, "RestoreSnapshot: Read from [%s]\n", inFile.string().c_str());

		if (!inStream)
		{
			LogPrintf(LOG_ERROR, "RestoreSnapshot: Error opening file\n");
			return;
		}

		json j;
		try
		{
			inStream >> j;
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "RestoreSnapshot: Error reading snapshot: %s\n", e.what());
			return;
		}

		std::string archConfig = Config::Instance().GetValueStr("core", "arch");
		std::string archSnapshot = j["core"]["arch"];
		if (archConfig != archSnapshot)
		{
			LogPrintf(LOG_ERROR, "RestoreSnapshot: Snapshot architecture[%s] different from config[%s]\n",
				archSnapshot.c_str(), archConfig.c_str());
			return;
		}

		int baseRAMConfig = Config::Instance().GetValueInt32("core", "baseram", 640);
		int baseRAMSnapshot = j["core"]["baseram"];
		if (baseRAMConfig != baseRAMSnapshot)
		{
			LogPrintf(LOG_ERROR, "RestoreSnapshot: Snapshot base RAM[%d] different from config[%d]\n",
				baseRAMSnapshot, baseRAMConfig);
			return;
		}

		m_pc->SetSerializationDir(snapshotDir);
		m_pc->Deserialize(j);
	}

	bool Overlay::GetSnapshotBaseDirectory(fs::path& baseDir)
	{
		baseDir.clear();
		fs::path path = Config::Instance().GetValueStr("dirs", "snapshot", "./snapshots");

		if (!fs::is_directory(fs::status(path)))
		{
			LogPrintf(LOG_ERROR, "GetSnapshotBaseDirectory: [%s] is not a directory", path.string().c_str());
			return false;
		}

		baseDir = fs::absolute(path);
		return true;
	}

	bool Overlay::MakeSnapshotDirectory(fs::path& dir)
	{
		fs::path path = m_snapshotBaseDirectory;
		if (path.empty())
		{
			LogPrintf(LOG_ERROR, "MakeSnapshotDirectory: Base directory not set");
		}

		char buf[64];
		sprintf(buf, "snap-%zu", time(nullptr));
		path.append(buf);

		if (!fs::create_directories(path))
		{
			LogPrintf(LOG_ERROR, "MakeSnapshotDirectory: Unable to create directory [%s] in snapshot folder", buf);
			return false;
		}

		dir = fs::absolute(path);
		return true;
	}

	bool Overlay::GetLastSnapshotDirectory(fs::path& snapshotDir)
	{
		snapshotDir.clear();

		fs::path path = m_snapshotBaseDirectory;
		if (path.empty())
		{
			LogPrintf(LOG_ERROR, "GetLastSnapshotDirectory: Base directory not set");
		}

		std::set<fs::path> snapshots;
		for (auto const& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				snapshots.insert(entry.path());
			}
		}

		if (!snapshots.size())
		{
			return false;
		}

		// Last one = more recent
		snapshotDir = *(snapshots.rbegin());
		return true;
	}

	std::string Overlay::GetSnapshotName(const fs::path& snapshotDir)
	{
		std::string name = snapshotDir.filename().string();

		time_t value;
		if (sscanf(name.c_str(), "snap-%zu", &value) == 1)
		{
			char buf[128];
			struct tm* local = localtime(&value);
			strftime(buf, sizeof(buf), "%c", local);
			name = buf;
		}

		return name;
	}

	void Overlay::OnClick(WidgetRef widget)
	{
		if (widget->GetId() == "floppy0")
		{
			LoadDiskImage(0, m_floppy0, "A:");
		}
		else if (widget->GetId() == "floppy1")
		{
			LoadDiskImage(1, m_floppy1, "B:");
		}
		if (widget->GetId() == "eject0")
		{
			LoadDiskImage(0, m_floppy0, "A:", true);
		}
		else if (widget->GetId() == "eject1")
		{
			LoadDiskImage(1, m_floppy1, "B:", true);
		}
		else if (widget->GetId() == "speed")
		{
			ToggleCPUSpeed();
		}
		else if (widget->GetId() == "saveSnapshot")
		{
			fs::path snapshotDir;
			if (MakeSnapshotDirectory(snapshotDir))
			{
				SaveSnapshot(snapshotDir);
			}
		}
		else if (widget->GetId() == "loadSnapshot")
		{
			fs::path snapshotDir;
			GetLastSnapshotDirectory(snapshotDir);

			if (!snapshotDir.empty())
			{
				RestoreSnapshot(snapshotDir);
			}
		}
	}

	// events::EventHangler
	bool Overlay::HandleEvent(SDL_Event& e)
	{
		static Uint32 toolbarEvent = WINMGR().GetEventType(ToolbarItem::EventClassName());

		bool handled = false;
		bool redraw = false;

		if (e.type == SDL_QUIT)
		{
			// Do nothing
		}
		else if (e.type == toolbarEvent)
		{
			OnClick((WidgetRef)e.user.data1);
		}
		else if (WINMGR().GetCapture() && WINMGR().GetCapture().Target.target->HandleEvent(&e))
		{
			handled = true;
		} 
		else if (e.type == SDL_MOUSEMOTION)
		{
			Point pt(e.button.x, e.button.y);
			HitResult hit = WINMGR().HitTest(&pt);
			if (hit)
			{
				handled = hit.target->HandleEvent(&e);
			}
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN)
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				Point pt(e.button.x, e.button.y);
				HitResult hit = WINMGR().HitTest(&pt);
				if (hit)
				{
					hit.target->SetActive();
					handled = hit.target->HandleEvent(&e);
				}
			}
		}
		else // Pass to active window
		{
			handled = WINMGR().GetActive()->HandleEvent(&e);
		}

		return handled;
	}
}