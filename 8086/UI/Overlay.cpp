#include "Overlay.h"

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

#include <filesystem>
namespace fs = std::filesystem;

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

		m_floppy0 = toolbar->AddToolbarItem("floppy0", m_floppyInactive, "A:");
		m_eject0 = toolbar->AddToolbarItem("eject0", RES().FindImage("toolbar", 7));

		toolbar->AddSeparator();

		m_floppy1 = toolbar->AddToolbarItem("floppy1", m_floppyInactive, "B:");
		m_eject1 = toolbar->AddToolbarItem("eject1", RES().FindImage("toolbar", 7));

		toolbar->AddSeparator();

		m_hdd0 = toolbar->AddToolbarItem("hdd0", m_hddInactive, "C:");
		m_hdd1 = toolbar->AddToolbarItem("hdd1", m_hddInactive, "D:");

		toolbar->AddSeparator();

		m_speed = toolbar->AddToolbarItem("speed", RES().FindImage("toolbar", 6), " 0.00 MHz");

		toolbar->AddSeparator();

		m_snapshot = toolbar->AddToolbarItem("saveSnapshot", RES().FindImage("toolbar", 8));
		toolbar->AddToolbarItem("loadSnapshot", RES().FindImage("toolbar", 9));

		m_mainWnd->SetToolbar(toolbar);
		m_mainWnd->SetText(m_pc->GetName());
		UpdateSpeed();

		return true;
	}

	void Overlay::UpdateSpeed()
	{
		static char speedStr[32];
		emul::CPUSpeed speed = m_pc->GetCPUSpeed();
		sprintf(speedStr, "%5.2fMHz", speed.GetSpeed() / 1000000.0f);
		m_speed->SetText(speedStr);
	}

	bool Overlay::Update()
	{
		m_floppy0->SetImage(m_pc->GetFloppy().IsActive(0) ? m_floppyActive : m_floppyInactive);
		m_floppy1->SetImage(m_pc->GetFloppy().IsActive(1) ? m_floppyActive : m_floppyInactive);
		return true;
	}

	// video::Renderer
	void Overlay::Render()
	{
		WINMGR().Draw();
	}

	void Overlay::LoadDiskImage(BYTE drive, ToolbarItemPtr toolbarItem, const char* str, bool eject)
	{
		fs::path diskImage;

		if (eject)
		{
			m_pc->GetFloppy().ClearDiskImage(drive);
			toolbarItem->SetText(str);
		}
		else if (SelectFile(diskImage, GetHWND(m_window)))
		{
			std::string label = str;
			label += " ";
			label += diskImage.filename().string();
			m_pc->GetFloppy().LoadDiskImage(drive, diskImage.string().c_str());
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