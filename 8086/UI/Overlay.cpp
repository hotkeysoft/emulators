#include "Overlay.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#pragma warning(disable:4251)

#include <Common.h>
#include <Core/Window.h>
#include <Core/WindowManager.h>
#include <Core/ResourceManager.h>
#include <Widgets/Toolbar.h>

using namespace CoreUI;

namespace ui
{
	Overlay::Overlay() : Logger("GUI")
	{
	}

	bool Overlay::Init(MainWindowRef win, RendererRef ren)
	{
		LogPrintf(LOG_TRACE, "Overlay::Init");

		if (TTF_Init() == -1)
		{
			LogPrintf(LOG_ERROR, "Failed to initialize TTF: %s", SDL_GetError());
			return false;
		}

		RES().Init(ren);
		WINMGR().Init(win, ren);

		m_renderer = ren;
		m_window = win;

		WindowPtr mainWnd = WINMGR().AddWindow("status", { 500, 700, 150, 54 }, WIN_ACTIVE | WIN_CANMOVE | WIN_NOSCROLL);
		mainWnd->SetText("Status");

		RES().LoadImageMap("toolbar", "./res/toolbar.png", 16, 16);

		ToolbarPtr toolbar = Toolbar::CreateAutoSize(ren, "toolbar");
		toolbar->SetBackgroundColor(Color::C_MED_GREY);
		toolbar->AddToolbarItem("image0", RES().FindImage("toolbar", 0));
		mainWnd->SetToolbar(toolbar);

		return true;
	}

	bool Overlay::Update()
	{
		return true;
	}
}