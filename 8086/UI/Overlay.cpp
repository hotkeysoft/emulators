#include "Overlay.h"
#include "../Config.h"
#include "../Storage/DeviceFloppy.h"
#include "../Storage/DeviceHardDrive.h"

#include <fstream>
#include <filesystem>

#pragma warning(disable:4251)

#include <Common.h>
#include <Core/Window.h>
#include <Core/WindowManager.h>
#include <Core/ResourceManager.h>
#include <Core/Widget.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarItem.h>
#include <Widgets/Button.h>
#include <Widgets/Label.h>
#include <Widgets/TextBox.h>

#include <commdlg.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>

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

		SDL_InitSubSystem(SDL_INIT_TIMER);
		m_renderer = pc->GetVideo().GetRenderer();
		m_window = pc->GetVideo().GetWindow();

		RES().Init(m_renderer);
		WINMGR().Init(m_window, m_renderer);

		Rect windowSize = WINMGR().GetWindowSize();
		Rect toolbarRect = windowSize;
		toolbarRect.h = GetOverlayHeight();
		toolbarRect.y = windowSize.h - toolbarRect.h;

		m_mainWnd = WINMGR().AddWindow("status", toolbarRect, WIN_ACTIVE | WIN_CANMOVE | WIN_CANRESIZE | WIN_NOSCROLL);
		m_mainWnd->SetMinSize((uint8_t)GetOverlayHeight());

		RES().LoadImageMap("toolbar", "./res/toolbar.png", 16, 16);

		ToolbarPtr toolbar = Toolbar::CreateAutoSize(m_renderer, "toolbar");
		toolbar->SetBackgroundColor(Color::C_MED_GREY);

		m_mainWnd->SetToolbar(toolbar);
		UpdateTitle();

		m_floppyInactive = RES().FindImage("toolbar", 0);
		m_floppyActive = RES().FindImage("toolbar", 4);
		m_hddInactive = RES().FindImage("toolbar", 1);
		m_hddActive = RES().FindImage("toolbar", 5);
		m_turboOff = RES().FindImage("toolbar", 10);
		m_turboOn = RES().FindImage("toolbar", 12);

		// Toolbar section: Reboot
		ToolbarItemPtr rebootButton = toolbar->AddToolbarItem("reboot", RES().FindImage("toolbar", 11));
		rebootButton->SetTooltip("Click for Soft Reboot (CTRL+ATL+DEL)\nShift-Click for Hard Reboot");

		toolbar->AddSeparator();

		// Toolbar section: Floppy drives
		if (m_pc->GetFloppy())
		{
			m_floppyButton[0] = toolbar->AddToolbarItem("floppy0", m_floppyInactive, "A:");
			m_ejectButton[0] = toolbar->AddToolbarItem("eject0", RES().FindImage("toolbar", 7));
			m_ejectButton[0]->SetTooltip("Eject A:");
			UpdateFloppy(0);
			m_floppyButton[1] = toolbar->AddToolbarItem("floppy1", m_floppyInactive, "B:");
			m_ejectButton[1] = toolbar->AddToolbarItem("eject1", RES().FindImage("toolbar", 7));
			m_ejectButton[1]->SetTooltip("Eject B:");
			UpdateFloppy(1);

			toolbar->AddSeparator();
		}

		// Toolbar section: Hard disks
		if (m_pc->GetHardDrive())
		{
			m_hddButton[0] = toolbar->AddToolbarItem("hdd0", m_hddInactive, "C:");
			UpdateHardDisk(0);

			m_hddButton[1] = toolbar->AddToolbarItem("hdd1", m_hddInactive, "D:");
			UpdateHardDisk(1);

			toolbar->AddSeparator();
		}

		// Toolbar section: Speed
		m_speedButton = toolbar->AddToolbarItem("speed", RES().FindImage("toolbar", 6), " 0.00 MHz");
		m_speedButton->SetTooltip("Toggle CPU Speed");
		UpdateSpeed();

		m_turboButton = toolbar->AddToolbarItem("turbo", m_turboOff);
		m_turboButton->SetTooltip("Toggle Warp Speed");
		UpdateTurbo();

		toolbar->AddSeparator();

		// Toolbar section: Snapshots
		ToolbarItemPtr saveSnapshotButton = toolbar->AddToolbarItem("saveSnapshot", RES().FindImage("toolbar", 8));
		saveSnapshotButton->SetTooltip("Save Computer state to disk");
		m_loadSnapshotButton = toolbar->AddToolbarItem("loadSnapshot", RES().FindImage("toolbar", 9));
		m_loadSnapshotButton->SetTooltip("Restore last saved state from disk\nShift-click for more options");

		GetSnapshotBaseDirectory(m_snapshotBaseDirectory);
		LogPrintf(LOG_INFO, "Snapshot base directory is [%s]", m_snapshotBaseDirectory.string().c_str());
		if (!GetLastSnapshotDirectory(m_lastSnapshotDir))
		{
			m_lastSnapshotDir.clear();
		}
		UpdateSnapshot();

		toolbar->AddSeparator();

		// Toolbar section: Joystick
		if (m_pc->GetInputs().GetJoystick())
		{
			m_joystickButton = toolbar->AddToolbarItem("joystick", RES().FindImage("toolbar", 2));
			m_joystickButton->SetTooltip("Joystick Configuration");
		}

		return true;
	}

	void Overlay::OnClick(WidgetRef widget)
	{
		if (widget->GetId() == "floppy0")
		{
			LoadFloppyDiskImage(0);
		}
		else if (widget->GetId() == "floppy1")
		{
			LoadFloppyDiskImage(1);
		}
		if (widget->GetId() == "eject0")
		{
			LoadFloppyDiskImage(0, true);
		}
		else if (widget->GetId() == "eject1")
		{
			LoadFloppyDiskImage(1, true);
		}
		if (widget->GetId() == "hdd0")
		{
			LoadHardDiskImage(0);
		}
		else if (widget->GetId() == "hdd1")
		{
			LoadHardDiskImage(1);
		}
		else if (widget->GetId() == "speed")
		{
			ToggleCPUSpeed();
		}
		else if (widget->GetId() == "saveSnapshot")
		{
			RemoveSnapshotWindow();
			fs::path snapshotDir;
			if (MakeSnapshotDirectory(snapshotDir))
			{
				SaveSnapshot(snapshotDir);
			}
		}
		else if (widget->GetId() == "loadSnapshot")
		{
			if (SDL_GetModState() & KMOD_SHIFT)
			{
				ShowSnapshotWindow();
			}
			else
			{
				RemoveSnapshotWindow();
				LoadLastSnapshot();
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
		else if (widget->GetId() == "joystick")
		{
			JoystickConfig();
		}
		else if (widget->GetId() == "trimX-")
		{
			--m_trim.x;
			SetTrim();
		}
		else if (widget->GetId() == "trimX--")
		{
			m_trim.x -= 5;
			SetTrim();
		}
		else if (widget->GetId() == "trimX+")
		{
			++m_trim.x;
			SetTrim();
		}
		else if (widget->GetId() == "trimX++")
		{
			m_trim.x += 5;
			SetTrim();
		}
		else if (widget->GetId() == "trimY-")
		{
			--m_trim.y;
			SetTrim();
		}
		else if (widget->GetId() == "trimY--")
		{
			m_trim.y -= 5;
			SetTrim();
		}
		else if (widget->GetId() == "trimY+")
		{
			++m_trim.y;
			SetTrim();
		}
		else if (widget->GetId() == "trimY++")
		{
			m_trim.y += 5;
			SetTrim();
		}
		else if (widget->GetId() == "resetX")
		{
			m_trim.x = 0;
			SetTrim();
		}
		else if (widget->GetId() == "resetY")
		{
			m_trim.y = 0;
			SetTrim();
		}
		else if (widget->GetId() == "close")
		{
			m_joystickConfigWnd->Show(false);
			m_joystickButton->SetPushed(false);
		}
	}

	void Overlay::UpdateSpeed()
	{
		static char speedStr[32];
		emul::CPUSpeed speed = m_pc->GetCPUSpeed();
		sprintf(speedStr, "%.2fMHz", speed.GetSpeed() / 1000000.0f);
		m_speedButton->SetText(speedStr);
	}

	void Overlay::UpdateSnapshot()
	{
		if (m_lastSnapshotDir.empty())
		{
			m_loadSnapshotButton->SetText("");
		}
		else
		{
			std::string name = GetSnapshotName(m_lastSnapshotDir);
			m_loadSnapshotButton->SetText(name.c_str());
		}
	}

	void Overlay::UpdateFloppy(BYTE drive)
	{
		const auto& image = m_pc->GetFloppy()->GetImageInfo(drive);

		std::ostringstream os;
		os << (char)('A'+drive) << ':';
		if (image.loaded)
		{
			m_floppyButton[drive]->SetTooltip(image.path.stem().string().c_str());

			os << " ["
				<< image.geometry.name
				<< "]";
		}
		else
		{
			m_floppyButton[drive]->SetTooltip("Load floppy image");
		}

		m_floppyButton[drive]->SetText(os.str().c_str());
	}

	void Overlay::UpdateHardDisk(BYTE drive)
	{
		const auto& image = m_pc->GetHardDrive()->GetImageInfo(drive);

		std::ostringstream os;
		os << (char)('C' + drive) << ':';
		std::string label = "[Empty]";
		if (image.loaded)
		{
			m_hddButton[drive]->SetTooltip(image.path.stem().string().c_str());

			os << " ["
				<< std::fixed << std::setprecision(1) << (image.geometry.GetImageSize() / 1048576.0)
				<< "MB]";
		}
		else
		{
			m_hddButton[drive]->SetTooltip("Load hard disk image");
		}

		m_hddButton[drive]->SetText(os.str().c_str());
	}

	void Overlay::UpdateTurbo()
	{
		m_turboButton->SetPushed(m_turbo);
		m_turboButton->SetImage(m_turbo ? m_turboOn : m_turboOff);
	}

	void Overlay::UpdateTrim()
	{
		m_trim = m_pc->GetInputs().GetJoystick()->GetAxisTrim();
		{
			std::ostringstream os;
			os << m_trim.x;
			m_trimX->SetText(os.str().c_str());
		}

		{
			std::ostringstream os;
			os << m_trim.y;
			m_trimY->SetText(os.str().c_str());
		}
	}

	void Overlay::SetTrim()
	{
		m_pc->GetInputs().GetJoystick()->SetAxisTrim({ m_trim.x, m_trim.y });
		UpdateTrim();
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

	void Overlay::UpdateTitle(float fps)
	{
		std::ostringstream os;
		os << m_pc->GetName();

		// Display video mode only if non-trivial (e.g skip pcjr)
		const auto& videoModes = m_pc->GetVideoModes();
		if (videoModes.size() > 1)
		{
			os << " [" << m_pc->GetVideo().GetDisplayName() << "]";
		}

		if (fps > 0.)
		{
			os << " [" << std::fixed << std::setprecision(1) << fps << " fps]";
		}

		m_mainWnd->SetText(os.str().c_str());
	}

	// video::Renderer
	void Overlay::Render()
	{
		const size_t frames = 240;
		static size_t frameCounter = 0;
		static auto frameTime = std::chrono::high_resolution_clock::now();
		float fps = 0.;

		if (++frameCounter == 240)
		{
			auto now = std::chrono::high_resolution_clock::now();
			auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameTime).count();
			frameTime = now;
			fps = frames * 1000.f / delta;
			frameCounter = 0;
		}

		if (fps > 0.)
		{
			if (m_show)
			{
				UpdateTitle(fps);
			}
			else
			{
				LogPrintf(LOG_INFO, "FPS: %.2f", fps);
			}
		}

		if (m_show)
		{
			WINMGR().Draw();
		}
	}

	void Overlay::LoadFloppyDiskImage(BYTE drive, bool eject)
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
		UpdateFloppy(drive);
	}

	void Overlay::LoadHardDiskImage(BYTE drive)
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
		UpdateHardDisk(drive);
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

		LogPrintf(LOG_INFO, "SaveSnapshot: Saved to [%s]", outFile.string().c_str());

		m_lastSnapshotDir = snapshotDir;
		UpdateSnapshot();
	}

	void Overlay::RestoreSnapshot(const fs::path& snapshotDir)
	{
		fs::path inFile = snapshotDir;
		inFile.append("computer.json");
		std::ifstream inStream(inFile);

		LogPrintf(LOG_INFO, "RestoreSnapshot: Read from [%s]", inFile.string().c_str());

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

		try
		{
			m_pc->SetSerializationDir(snapshotDir);
			m_pc->Deserialize(j);
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "RestoreSnapshot: Error deserializing snapshot: %s\n", e.what());
		}

		UpdateFloppy(0);
		UpdateFloppy(1);
		UpdateHardDisk(0);
		UpdateHardDisk(1);
		UpdateSpeed();
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
		sprintf(buf, "snap-%llu", time(nullptr));
		path.append(buf);

		if (!fs::create_directories(path))
		{
			LogPrintf(LOG_ERROR, "MakeSnapshotDirectory: Unable to create directory [%s] in snapshot folder", buf);
			return false;
		}

		dir = fs::absolute(path);
		return true;
	}

	void Overlay::LoadSnapshotList()
	{
		fs::path path = m_snapshotBaseDirectory;
		if (path.empty())
		{
			LogPrintf(LOG_ERROR, "GetLastSnapshotDirectory: Base directory not set");
		}
	
		for (auto const& entry : std::filesystem::directory_iterator(path))
		{
			if (entry.is_directory())
			{
				m_snapshots.insert(entry.path());
			}
		}
	}

	bool Overlay::GetLastSnapshotDirectory(fs::path& snapshotDir)
	{
		snapshotDir.clear();

		LoadSnapshotList();

		if (!m_snapshots.size())
		{
			return false;
		}

		// Last one = more recent
		snapshotDir = *(m_snapshots.rbegin());
		return true;
	}

	std::string Overlay::GetSnapshotName(const fs::path& snapshotDir)
	{
		std::string name = snapshotDir.filename().string();

		time_t value;
		if (sscanf(name.c_str(), "snap-%llu", &value) == 1)
		{
			char buf[128];
			struct tm* local = localtime(&value);
			strftime(buf, sizeof(buf), "%c", local);
			name = buf;
		}

		return name;
	}

	void Overlay::LoadLastSnapshot()
	{
		fs::path snapshotDir;
		GetLastSnapshotDirectory(snapshotDir);

		if (!snapshotDir.empty())
		{
			RestoreSnapshot(snapshotDir);
		}
	}

	void Overlay::RemoveSnapshotWindow()
	{
		if (m_snapshotWnd)
		{
			WINMGR().RemoveWindow("snapshots");
			m_snapshotWnd = nullptr;
		}
	}

	void Overlay::ShowSnapshotWindow()
	{	
		RemoveSnapshotWindow();

		const int nbItems = 16;
		const int hItem = 24;
		const int width = 400;
		const int height = hItem * nbItems + 34;

		Rect r = WINMGR().GetWindowSize();
		r.x = (r.w - width) / 2;
		r.y = r.h - (GetOverlayHeight() + height);
		r.w = width;
		r.h = height;

		m_snapshotWnd = WINMGR().AddWindow("snapshots", r, WIN_CANMOVE);
		m_snapshotWnd->SetText("Snapshots");
		m_snapshotWnd->SetActive();

		LoadSnapshotList();

		int index = 0;
		size_t count = m_snapshots.size();
		for (fs::path snapshot : m_snapshots)
		{
			AddSnapshotItem(snapshot, index++, width - ((count <= nbItems) ? 8 : 26), hItem);
		}

		m_snapshotWnd->GetScrollBars()->RefreshScrollBarStatus();
		m_snapshotWnd->GetScrollBars()->ScrollTo(&Point(0, hItem * index));
	}

	void Overlay::AddSnapshotItem(const std::filesystem::path& path, int index, int w, int h)
	{
		std::string name = GetSnapshotName(path);
		Rect pos(0, index * h, w-24, h);
		Rect posDelete(w-24, index * h, 24, h);

		char id[64];
		sprintf(id, "snapshot%d", index);

		m_snapshotWnd->AddControl(CoreUI::Button::Create(id, m_renderer, pos, name.c_str()));
		strcat(id, "x");
		m_snapshotWnd->AddControl(CoreUI::Button::Create(id, m_renderer, posDelete, "X"));
	}

	void Overlay::JoystickConfig()
	{
		if (m_joystickConfigWnd)
		{
			bool isVisible = m_joystickConfigWnd->GetShowState() & WindowState::WST_VISIBLE;

			m_joystickConfigWnd->Show(isVisible ? false : true);
			m_joystickButton->SetPushed(!isVisible);
			m_joystickConfigWnd->SetActive();
			return;
		}

		m_joystickButton->SetPushed(true);

		const int width = 278;
		const int height = 120;
		const int line0y = 4;
		const int line1y = 28;
		const int line2y = 56;
		const int lineh = 24;

		Rect r = WINMGR().GetWindowSize();
		r.x = r.w - width;
		r.y = r.h - (GetOverlayHeight() + height);
		r.w = width;
		r.h = height;
		m_joystickConfigWnd = WINMGR().AddWindow("joystick", r, WIN_DIALOG | WIN_CANMOVE | WIN_NOSCROLL);
		m_joystickConfigWnd->SetText("Joystick");
		m_joystickConfigWnd->SetActive();

		m_joystickConfigWnd->AddControl(CoreUI::Label::CreateSingle("lx", m_renderer, Rect(0, line0y, 50, lineh), "Trim X"));
		m_joystickConfigWnd->AddControl(CoreUI::Label::CreateSingle("ly", m_renderer, Rect(0, line1y, 50, lineh), "Trim Y"));

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX--", m_renderer, Rect(50, line0y, 28, lineh), "<<"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY--", m_renderer, Rect(50, line1y, 28, lineh), "<<"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX-", m_renderer, Rect(76, line0y, 24, lineh), "<"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY-", m_renderer, Rect(76, line1y, 24, lineh), "<"));

		m_trimX = CoreUI::Label::CreateSingle("trimX", m_renderer, Rect(110, line0y, 28, lineh), "0", nullptr, CoreUI::Label::TEXT_H_CENTER);
		m_joystickConfigWnd->AddControl(m_trimX);

		m_trimY = CoreUI::Label::CreateSingle("trimY", m_renderer, Rect(110, line1y, 28, lineh), "0", nullptr, CoreUI::Label::TEXT_H_CENTER);
		m_joystickConfigWnd->AddControl(m_trimY);

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX+", m_renderer, Rect(146, line0y, 24, lineh), ">"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY+", m_renderer, Rect(146, line1y, 24, lineh), ">"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimX++", m_renderer, Rect(170, line0y, 28, lineh), ">>"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("trimY++", m_renderer, Rect(170, line1y, 28, lineh), ">>"));

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("resetX", m_renderer, Rect(206, line0y, 60, lineh), "Reset"));
		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("resetY", m_renderer, Rect(206, line1y, 60, lineh), "Reset"));

		m_joystickConfigWnd->AddControl(CoreUI::Button::Create("close", m_renderer, Rect(206, line2y, 60, lineh), "Close"));

		UpdateTrim();
	}

	// events::EventHandler
	bool Overlay::HandleEvent(SDL_Event& e)
	{
		static Uint32 toolbarEvent = WINMGR().GetEventType(ToolbarItem::EventClassName());
		static Uint32 buttonEvent = WINMGR().GetEventType(Button::EventClassName());
		static Uint32 timerEvent = WINMGR().GetEventType(Timer::EventClassName());
		const static Uint32 timerEventID = WINMGR().GetEventType("timer");

		bool handled = false;
		bool redraw = false;

		if (e.type == SDL_QUIT)
		{
			// Do nothing
		}
		else if (e.type == toolbarEvent || e.type == buttonEvent)
		{
			OnClick((WidgetRef)e.user.data1);
		}
		else if (e.type == timerEvent)
		{
			if (WINMGR().IsTimerValid(e.user.code))
			{
				Timer* timer = (Timer*)e.user.data1;
				Widget* owner = timer->GetWidget();
				if (owner)
				{
					owner->HandleEvent(&e);
					handled = true;
				}
			}
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
		else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED)
		{
			int w = e.window.data1;
			int h = e.window.data2;

			Rect r{ 0, h - GetOverlayHeight(), w, GetOverlayHeight() };
			m_mainWnd->MoveRect(&r);
		}
		else // Pass to active window
		{
			handled = WINMGR().GetActive()->HandleEvent(&e);
		}

		return handled;
	}
}