#include "stdafx.h"

#include <UI/MainWindow.h>
#include <UI/Overlay.h>
#include <UI/TimeFormatter.h>
#include <UI/SnapshotInfo.h>
#include <UI/SnapshotWidget.h>
#include <Config.h>

#pragma warning(disable:4251)

#include <CPU/CPUCommon.h>
#include <Core/Window.h>
#include <Core/WindowManager.h>
#include <Core/ResourceManager.h>
#include <Core/Widget.h>
#include <Core/Tooltip.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarItem.h>
#include <Widgets/Button.h>
#include <Widgets/Label.h>
#include <Widgets/TextBox.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_syswm.h>

namespace fs = std::filesystem;

using cfg::CONFIG;
using namespace CoreUI;

using emul::SerializableException;
using emul::SerializationError;

namespace ui
{
	Overlay::Overlay() : Logger("GUI")
	{
	}

	bool Overlay::Init()
	{
		LogPrintf(LOG_TRACE, "Overlay::Init");

		if (TTF_Init() == -1)
		{
			LogPrintf(LOG_ERROR, "Failed to initialize TTF: %s", SDL_GetError());
			return false;
		}

		SDL_Renderer* ren = MAINWND().GetRenderer();
		SDL_Window* wnd = MAINWND().GetWindow();

		RES().Init(ren);
		WINMGR().Init(wnd, ren);

		RES().LoadCursor("edit.ibeam", SDL_SYSTEM_CURSOR_IBEAM);
		CursorRef normalCursor = RES().LoadCursor("default", SDL_SYSTEM_CURSOR_ARROW);
		SDL_SetCursor(normalCursor);

		RES().LoadImageMap("overlay16", "./res/overlay16.png", 16, 16);

		m_turboOff = RES().FindImage("overlay16", 10);
		m_turboOn = RES().FindImage("overlay16", 12);

		return true;
	}

	void Overlay::SetPC(emul::Computer* pc)
	{
		if (!pc)
		{
			LogPrintf(LOG_ERROR, "PC Not set");
			return;
		}
		m_pc = pc;

		if (SDL_WasInit(SDL_INIT_TIMER) == 0)
		{
			LogPrintf(LOG_WARNING, "SDL Init Subsystem [Timers]");
			if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
			{
				LogPrintf(LOG_ERROR, "Error initializing timers subsystem: %s", SDL_GetError());
			}
		}
		WINMGR().DeleteAllTimers();

		SDL_Renderer* ren = MAINWND().GetRenderer();

		Rect windowSize = WINMGR().GetWindowSize();
		Rect toolbarRect = windowSize;
		toolbarRect.h = GetOverlayHeight();
		toolbarRect.y = windowSize.h - toolbarRect.h;

		if (m_mainWnd)
		{
			WINMGR().RemoveWindow("status");
			m_mainWnd = nullptr;
		}

		m_mainWnd = WINMGR().AddWindow("status", toolbarRect, WIN_ACTIVE | WIN_CANMOVE | WIN_CANRESIZE | WIN_NOSCROLL);
		m_mainWnd->SetMinSize((uint8_t)GetOverlayHeight());

		m_toolbar = Toolbar::CreateAutoSize(ren, "toolbar");
		m_toolbar->SetBackgroundColor(Color::C_MED_GREY);

		m_mainWnd->SetToolbar(m_toolbar);
		UpdateTitle();

		// Toolbar section: Reboot
		m_rebootButton = m_toolbar->AddToolbarItem("reboot", RES().FindImage("overlay16", 11));

		m_toolbar->AddSeparator();

		m_turboButton = m_toolbar->AddToolbarItem("turbo", m_turboOff, "Turbo");
		m_turboButton->SetTooltip("Toggle Warp Speed");
		UpdateTurbo();

		m_toolbar->AddSeparator();

		// Toolbar section: Snapshots
		ToolbarItemPtr saveSnapshotButton = m_toolbar->AddToolbarItem("saveSnapshot", RES().FindImage("overlay16", 8));
		saveSnapshotButton->SetTooltip("Save Computer state to disk");
		m_loadSnapshotButton = m_toolbar->AddToolbarItem("loadSnapshot", RES().FindImage("overlay16", 9));
		m_loadSnapshotButton->SetTooltip("Restore last saved state from disk\nShift-click for more options");

		GetSnapshotBaseDirectory(m_snapshotBaseDirectory);
		LogPrintf(LOG_DEBUG, "Snapshot base directory is [%s]", m_snapshotBaseDirectory.string().c_str());
		if (!GetLastSnapshotDirectory(m_lastSnapshotDir))
		{
			m_lastSnapshotDir.clear();
		}
		UpdateSnapshot();
	}

	void Overlay::OnClick(WidgetRef widget)
	{
		const std::string& id = widget->GetId();

		if (id == "saveSnapshot")
		{
			RemoveSnapshotWindow();
			fs::path snapshotDir;
			if (MakeSnapshotDirectory(snapshotDir))
			{
				SaveSnapshot(snapshotDir);
			}
		}
		else if (id == "loadSnapshot")
		{
			// Abort if we're currently editing one of the snapshots
			SnapshotWidget::EndEdit();
			if (SDL_GetModState() & KMOD_SHIFT)
			{
				m_loadSnapshotButton->SetPushed(true);
				ShowSnapshotWindow();
			}
			else
			{
				RemoveSnapshotWindow();
				LoadLastSnapshot();
			}
		}
		else if (id == "reboot")
		{
			m_pc->Reboot();
		}
		else if (id == "turbo")
		{
			ToggleTurbo();
		}
		else if (StringUtil::StartsWith(id, "snapshot-"))
		{
			// Abort if we're currently editing one of the snapshots
			SnapshotWidget::EndEdit();

			// Edit/Load/Delete buttons are children of Snapshotwidget, so
			// the tag is in the parent
			std::filesystem::path* path = (std::filesystem::path*)widget->GetParent()->GetTag().o;
			assert(path);
			if (StringUtil::EndsWith(id, "load"))
			{
				m_snapshotWnd->Show(false);
				m_loadSnapshotButton->SetPushed(false);
				RestoreSnapshot(*path);
			}
			else if (StringUtil::EndsWith(id, "edit"))
			{
				SnapshotWidgetRef snapshotWidget = (SnapshotWidgetRef)widget->GetParent();
				snapshotWidget->BeginEdit();
			}
			else if (StringUtil::EndsWith(id, "delete"))
			{
				DeleteSnapshot(*path);
			}
		}
		else
		{
			LogPrintf(LOG_WARNING, "OnClick: Unknown button id [%s]", id.c_str());
		}
	}

	void Overlay::OnClose(WidgetRef widget)
	{
		if (widget->GetId() == "snapshots")
		{
			// Abort if we're currently editing one of the snapshots
			SnapshotWidget::EndEdit();
			RemoveSnapshotWindow();
			m_loadSnapshotButton->SetPushed(false);
		}
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

	void Overlay::UpdateTurbo()
	{
		m_turboButton->SetPushed(m_turbo);
		m_turboButton->SetImage(m_turbo ? m_turboOn : m_turboOff);
	}

	bool Overlay::Update()
	{
		if (!m_pc)
		{
			return false;
		}

		// TODO: The actual refresh interval is proportional
		// to the emulated CPU spees
		const int cooldownTime = 1000;
		static int cooldown = cooldownTime;
		if (--cooldown == 0)
		{
			cooldown = cooldownTime;
			UpdateSnapshot();
		}
		return true;
	}

	void Overlay::UpdateTitle(float fps)
	{
		if (!m_pc)
		{
			return;
		}

		std::ostringstream os;
		os << m_pc->GetName();

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

	void Overlay::ToggleTurbo()
	{
		m_turbo = !m_turbo;
		LogPrintf(LOG_INFO, "Turbo [%s]\n", m_turbo ? "ON" : "OFF");
		m_pc->SetTurbo(m_turbo);
		UpdateTurbo();
	}

	void Overlay::SaveComputerData(const std::filesystem::path& snapshotDir)
	{
		if (!m_pc)
		{
			return;
		}

		json j;
		m_pc->SetSerializationDir(snapshotDir);
		m_pc->Serialize(j);

		fs::path outFile = snapshotDir;
		outFile.append("computer.json");

		std::ofstream outStream(outFile.string());
		outStream << std::setw(4) << j;

		LogPrintf(LOG_INFO, "SaveComputerData: Saved state to [%s]", outFile.string().c_str());
	}

	void Overlay::SaveSnapshotInfo(const fs::path& snapshotDir)
	{
		fs::path outFile = snapshotDir;
		outFile.append("snapshot.json");

		SnapshotInfo info(snapshotDir);
		info.FromPC(m_pc);

		if (!info.ToDisk())
		{
			LogPrintf(LOG_ERROR, "SaveSnapshotInfo: Error saving snapshot info");
		}
	}

	void Overlay::SaveConfigFile(const fs::path& snapshotDir)
	{
		fs::path outFile = snapshotDir;
		outFile.append("config.ini");

		if (!CONFIG().SaveConfigFile(outFile.string().c_str()))
		{
			LogPrintf(LOG_ERROR, "SaveConfigFile: Error saving config file: %s", outFile.string().c_str());
		}
	}

	void Overlay::SaveSnapshot(const fs::path& snapshotDir)
	{
		if (!m_pc)
		{
			return;
		}

		SaveSnapshotInfo(snapshotDir);
		SaveConfigFile(snapshotDir);
		SaveComputerData(snapshotDir);

		m_lastSnapshotDir = snapshotDir;
		UpdateSnapshot();
	}

	void Overlay::DeleteSnapshot(const std::filesystem::path& path)
	{
		SDL_MessageBoxButtonData buttons[2] = {
			{ 0, 1, "Yes" },
			{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT |
			SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "No" }
		};

		SnapshotInfo info(path);
		info.FromDisk();

		std::ostringstream os;
		os << "Do you want to delete the following snapshot?"
			<< std::endl << std::endl
			<< "- " << info.ToString()
			<< std::endl << std::endl
			<< "Saved: " << info.GetTimestamp().ToString(TimeFormat::TF_ABSOLUTE);

		std::string msg = os.str();

		SDL_MessageBoxData data;
		memset(&data, 0, sizeof(data));
		data.flags = SDL_MESSAGEBOX_WARNING | SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
		data.title = "Delete Snapshot";
		data.message = msg.c_str();
		data.window = MAINWND().GetWindow();
		data.buttons = buttons;
		data.numbuttons = SDL_arraysize(buttons);

		int buttonId;
		SDL_ShowMessageBox(&data, &buttonId);

		if (buttonId == 1)
		{
			LogPrintf(LOG_WARNING, "Delete snapshot, path: %s", path.string().c_str());
			std::error_code code;
			fs::remove_all(path, code);
			if (code)
			{
				std::ostringstream os;
				os << "Error deleting snapshot "
					<< path.stem().string()
					<< std::endl
					<< code.message();
				std::string msg = os.str();
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Delete Snapshot", msg.c_str(), MAINWND().GetWindow());
			}

			// In case the most recent was deleted
			if (!GetLastSnapshotDirectory(m_lastSnapshotDir))
			{
				m_lastSnapshotDir.clear();
			}
			// Updates the current snapshot name in the toolbar
			UpdateSnapshot();
			// Reloads the whole list and rebuild the window
			// TODO: Would be easier to just remove the item
			ShowSnapshotWindow();
		}
	}

	void Overlay::RestoreSnapshot(const fs::path& snapshotDir)
	{
		if (!m_pc)
		{
			return;
		}

		// We don't need to read back the config.ini for trivial cases
		// (e.g when the configuration is compatible with the current one)
		// If the deserialization fails with a compatibility error, the
		// full restore will be done in the main loop, including
		// reading back the configuration

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

		bool compatible = true;
		try
		{
			m_pc->SetSerializationDir(snapshotDir);
			m_pc->Deserialize(j);

			SetPC(m_pc);
		}
		catch (SerializableException e)
		{
			// Thrown when hardware configuration is not compatible
			// (different video card, etc.)
			// In that case we need to build a whole new PC with the correct
			// config, but not in here.
			if (e.GetError() == SerializationError::COMPAT)
			{
				compatible = false;
			}
			else
			{
				LogPrintf(LOG_ERROR, "RestoreSnapshot: Fatal Error deserializing snapshot: %s\n", e.what());
				return;
			}
		}
		catch (std::exception e)
		{
			LogPrintf(LOG_ERROR, "RestoreSnapshot: Fatal Error deserializing snapshot: %s\n", e.what());
			return;
		}

		// Snapshot is not compatible with currently loaded PC. Notify someone to take care of it.
		if (!compatible && m_callback)
		{
			(*m_callback)(snapshotDir, j);
		}
	}

	bool Overlay::GetSnapshotBaseDirectory(fs::path& baseDir)
	{
		static fs::path absoluteBaseDir;

		if (absoluteBaseDir.empty())
		{
			fs::path path = CONFIG().GetValueStr("dirs", "snapshot", "./snapshots");

			if (!fs::is_directory(fs::status(path)))
			{
				LogPrintf(LOG_ERROR, "GetSnapshotBaseDirectory: [%s] is not a directory", path.string().c_str());
				return false;
			}
			absoluteBaseDir = fs::absolute(path);
		}

		baseDir = absoluteBaseDir;

		return true;
	}

	bool Overlay::MakeSnapshotDirectory(fs::path& dir)
	{
		fs::path path = m_snapshotBaseDirectory;
		if (path.empty())
		{
			LogPrintf(LOG_ERROR, "MakeSnapshotDirectory: Base directory not set");
			return false;
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
		m_snapshots.clear();

		fs::path path = m_snapshotBaseDirectory;
		if (path.empty())
		{
			LogPrintf(LOG_ERROR, "GetLastSnapshotDirectory: Base directory not set");
			return;
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
			TimeFormatter format(value);
			return format.ToString();
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

		m_snapshotWnd = WINMGR().AddWindow("snapshots", r, WIN_CANMOVE | WIN_CLOSE);
		m_snapshotWnd->SetText("Snapshots");
		m_snapshotWnd->SetActive();

		LoadSnapshotList();

		int index = 0;
		size_t count = m_snapshots.size();
		for (const fs::path& snapshot : m_snapshots)
		{
			AddSnapshotItem(snapshot, index++, width - ((count <= nbItems) ? 8 : 26), hItem);
		}

		m_snapshotWnd->GetScrollBars()->RefreshScrollBarStatus();
		m_snapshotWnd->GetScrollBars()->ScrollTo(&Point(0, hItem * index));
	}

	void Overlay::AddSnapshotItem(const fs::path& path, int index, int w, int h)
	{
		SnapshotInfo info(path);
		info.FromDisk();

		Rect pos(0, index * h, w, h);

		char id[64];
		sprintf(id, "snapshot-%d", index);

		SnapshotWidgetPtr widget = SnapshotWidget::Create(id, MAINWND().GetRenderer(), pos);
		widget->Init(info, {
			RES().FindImage("overlay16", 13),
			RES().FindImage("overlay16", 14),
			RES().FindImage("overlay16", 15)
		});
		widget->SetText(GetSnapshotName(path).c_str());
		widget->SetTag((void*)&path);

		m_snapshotWnd->AddControl(widget);
	}

	// events::EventHandler
	bool Overlay::HandleEvent(SDL_Event& e)
	{
		static Uint32 toolbarEvent = WINMGR().GetEventType(ToolbarItem::EventClassName());
		static Uint32 buttonEvent = WINMGR().GetEventType(Button::EventClassName());
		static Uint32 timerEvent = WINMGR().GetEventType(Timer::EventClassName());
		static Uint32 windowEvent = WINMGR().GetEventType(Window::EventClassName());
		const static Uint32 timerEventID = WINMGR().GetEventType("timer");

		bool handled = false;
		bool redraw = false;

		if (e.type == SDL_QUIT)
		{
			// Do nothing
		}
		else if (e.type == windowEvent)
		{
			if (e.user.code == Window::EVENT_WINDOW_CLOSE)
			{
				WindowRef widget = (WindowRef)e.user.data1;

				widget->Show(false);
				OnClose(widget);
				handled = true;
			}
		}
		else if (e.type == toolbarEvent || e.type == buttonEvent)
		{
			if (m_pc)
			{
				CoreUI::TOOLTIP().Hide();
				OnClick((WidgetRef)e.user.data1);
			}
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
				else
				{
					handled = WINMGR().GetActive()->HandleEvent(&e);
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