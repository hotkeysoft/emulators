#include "Overlay.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#pragma warning(disable:4251)

#include <Common.h>
#include <Core/Window.h>
#include <Core/WindowManager.h>
#include <Core/ResourceManager.h>
#include <Core/Widget.h>
#include <Widgets/Toolbar.h>
#include <Widgets/ToolbarItem.h>
#include <Widgets/Button.h>

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

	// video::Renderer
	void Overlay::Render()
	{
		WINMGR().Draw();
	}

	void Overlay::OnClick(WidgetRef widget)
	{
		if (widget->GetId() == "b2")
		{
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