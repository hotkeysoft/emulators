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

		m_mainWnd = WINMGR().AddWindow("status", { 0, 700, 800, 64 }, WIN_ACTIVE | WIN_CANMOVE | WIN_NOSCROLL);

		RES().LoadImageMap("toolbar", "./res/toolbar.png", 16, 16);

		ToolbarPtr toolbar = Toolbar::CreateAutoSize(m_renderer, "toolbar");
		toolbar->SetBackgroundColor(Color::C_MED_GREY);

		m_floppyInactive = RES().FindImage("toolbar", 0);
		m_floppyActive = RES().FindImage("toolbar", 4);
		m_hddInactive = RES().FindImage("toolbar", 1);
		m_hddActive = RES().FindImage("toolbar", 5);

		m_floppy0 = toolbar->AddToolbarItem("floppy0", m_floppyInactive, "A:");
		m_floppy1 = toolbar->AddToolbarItem("floppy1", m_floppyInactive, "B:");
		toolbar->AddSeparator();
		m_hdd0 = toolbar->AddToolbarItem("hdd0", m_hddInactive, "C:");
		m_hdd1 = toolbar->AddToolbarItem("hdd1", m_hddInactive, "D:");
		toolbar->AddSeparator();
		m_speed = toolbar->AddToolbarItem("speed", RES().FindImage("toolbar", 6), " 0.00 MHz");
		
		//toolbar->AddToolbarItem("gamepad", RES().FindImage("toolbar", 2));
		//toolbar->AddSeparator();
		//toolbar->AddToolbarItem("speaker", RES().FindImage("toolbar", 3));

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