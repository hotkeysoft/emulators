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

	void HardDriveLED::Update(bool active)
	{
		m_active = active;
		if (active)
		{
			m_cooldown = HardDriveLED::CooldownTime;
			m_lastActive = true;
		}
		else if (m_lastActive) // 1->0
		{
			m_active = true;
			--m_cooldown;
			if (m_cooldown == 0)
			{
				m_active = false;
				m_lastActive = false;
			}
		}
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
			m_floppyButton[0] = toolbar->AddToolbarItem("floppy0", m_floppyInactive, "A:");
			m_ejectButton[0] = toolbar->AddToolbarItem("eject0", RES().FindImage("toolbar", 7));
			UpdateFloppy(0, "A:");
			toolbar->AddSeparator();

			m_floppyButton[1] = toolbar->AddToolbarItem("floppy1", m_floppyInactive, "B:");
			m_ejectButton[1] = toolbar->AddToolbarItem("eject1", RES().FindImage("toolbar", 7));
			UpdateFloppy(1, "B:");
			toolbar->AddSeparator();

			toolbar->AddSeparator();
		}

		if (m_pc->GetHardDrive())
		{
			m_hddButton[0] = toolbar->AddToolbarItem("hdd0", m_hddInactive, "C:");
			UpdateHardDisk(0, "C:");

			m_hddButton[1] = toolbar->AddToolbarItem("hdd1", m_hddInactive, "D:");
			UpdateHardDisk(1, "D:");

			toolbar->AddSeparator();
			toolbar->AddSeparator();
		}

		m_speedButton = toolbar->AddToolbarItem("speed", RES().FindImage("toolbar", 6), " 0.00 MHz");
		UpdateSpeed();

		m_turboButton = toolbar->AddToolbarItem("turbo", RES().FindImage("toolbar", 10));
		UpdateTurbo();

		toolbar->AddSeparator();
		toolbar->AddSeparator();

		toolbar->AddToolbarItem("saveSnapshot", RES().FindImage("toolbar", 8));
		m_snapshot = toolbar->AddToolbarItem("loadSnapshot", RES().FindImage("toolbar", 9));

		toolbar->AddSeparator();
		toolbar->AddSeparator();

		toolbar->AddToolbarItem("reboot", RES().FindImage("toolbar", 11));

		m_mainWnd->SetToolbar(toolbar);
		m_mainWnd->SetText(m_pc->GetName());

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
		m_speedButton->SetText(speedStr);
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

	void Overlay::UpdateFloppy(BYTE drive, const char* path)
	{
		const auto& image = m_pc->GetFloppy()->GetImageInfo(drive);

		std::string label = "[Empty]";
		if (image.loaded)
		{
			std::ostringstream os(path);
			os << image.path.stem().string()
				<< " ["
				<< image.geometry.name
				<< "]";

			label = os.str();
		}

		m_floppyButton[drive]->SetText(label.c_str());
	}

	void Overlay::UpdateHardDisk(BYTE drive, const char* path)
	{
		const auto& image = m_pc->GetHardDrive()->GetImageInfo(drive);

		std::string label = "[Empty]";
		if (image.loaded)
		{
			std::ostringstream os(path);
			os << image.path.stem().string()
				<< " ["
				<< std::fixed << std::setprecision(1) << (image.geometry.GetImageSize() / 1048576.0)
				<< "MB]";

			label = os.str();
		}

		m_hddButton[drive]->SetText(label.c_str());
	}

	void Overlay::UpdateTurbo()
	{
		m_turboButton->SetBackgroundColor(m_turbo ? Color(174, 7, 0) : Color::C_LIGHT_GREY);
	}

	bool Overlay::Update()
	{
		if (m_pc->GetFloppy())
		{
			for (int i = 0; i < 2; ++i)
			{
				m_floppyButton[i]->SetImage(m_pc->GetFloppy()->IsActive(i) ? m_floppyActive : m_floppyInactive);
			}
		}

		if (m_pc->GetHardDrive())
		{
			for (int i = 0; i < 2; ++i)
			{
				m_hardDriveLEDs[i].Update(m_pc->GetHardDrive()->IsActive(i));
				m_hddButton[i]->SetImage(m_hardDriveLEDs[i].GetStatus() ? m_hddActive : m_hddInactive);
			}
		}

		return true;
	}

	// video::Renderer
	void Overlay::Render()
	{
		WINMGR().Draw();
	}

	void Overlay::LoadFloppyDiskImage(BYTE drive, const char* str, bool eject)
	{
		if (!m_pc->GetFloppy())
		{
			return;
		}

		fs::path diskImage;

		if (eject)
		{
			m_pc->GetFloppy()->ClearDiskImage(drive);
		}
		else if (SelectFile(diskImage, GetHWND(m_window)))
		{
			m_pc->GetFloppy()->LoadDiskImage(drive, diskImage.string().c_str());
		}
		UpdateFloppy(drive, str);
	}

	void Overlay::LoadHardDiskImage(BYTE drive, const char* str)
	{
		if (!m_pc->GetHardDrive())
		{
			return;
		}

		fs::path diskImage;
		if (SelectFile(diskImage, GetHWND(m_window)))
		{
			BYTE currImageType = m_pc->GetHardDrive()->GetImageInfo(drive).type;

			// Assume same hdd type for now to simplify things.
			// Incompatible images will be rejected
			m_pc->GetHardDrive()->LoadDiskImage(drive, currImageType, diskImage.string().c_str());
		}
		UpdateHardDisk(drive, str);
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

	void Overlay::ToggleTurbo()
	{
		m_turbo = !m_turbo;
		LogPrintf(LOG_INFO, "Turbo [%s]\n", m_turbo ? "ON" : "OFF");
		m_pc->SetTurbo(m_turbo);
		UpdateTurbo();
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
			LoadFloppyDiskImage(0, "A:");
		}
		else if (widget->GetId() == "floppy1")
		{
			LoadFloppyDiskImage(1, "B:");
		}
		if (widget->GetId() == "eject0")
		{
			LoadFloppyDiskImage(0, "A:", true);
		}
		else if (widget->GetId() == "eject1")
		{
			LoadFloppyDiskImage(1, "B:", true);
		}
		if (widget->GetId() == "hdd0")
		{
			LoadHardDiskImage(0, "C:");
		}
		else if (widget->GetId() == "hdd1")
		{
			LoadHardDiskImage(1, "D:");
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
		else if (widget->GetId() == "reboot")
		{
			// Shift+click = hard reboot
			m_pc->Reboot(SDL_GetModState() & KMOD_SHIFT);
		}
		else if (widget->GetId() == "turbo")
		{
			ToggleTurbo();
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