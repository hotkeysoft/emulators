#pragma once

#include "../Common/Logger.h"
#include "../Video/Video.h"
#include "../IO/InputEvents.h"
#include "../Computer.h"
#include <Common.h>

namespace ui
{
	class Overlay : public Logger, public video::Renderer, public events::EventHandler
	{
	public:
		Overlay();

		bool Init(emul::Computer* pc);
		bool Update();

		void UpdateSpeed();

		// video::Renderer
		virtual void Render() override;

		// events::EventHangler
		virtual bool HandleEvent(SDL_Event& e) override;

	protected:
		void OnClick(CoreUI::WidgetRef widget);

		void LoadDiskImage(BYTE drive, CoreUI::ToolbarItemPtr toolbarItem, const char* str, bool eject = false);
		void ToggleCPUSpeed();

		std::string m_title = "PC";

		emul::Computer* m_pc = nullptr;

		// UI Elements
		CoreUI::WindowPtr m_mainWnd;

		CoreUI::RendererRef m_renderer = nullptr;
		CoreUI::MainWindowRef m_window = nullptr;

		CoreUI::ToolbarItemPtr m_floppy0;
		CoreUI::ToolbarItemPtr m_floppy1;

		CoreUI::ToolbarItemPtr m_eject0;
		CoreUI::ToolbarItemPtr m_eject1;

		CoreUI::ToolbarItemPtr m_hdd0;
		CoreUI::ToolbarItemPtr m_hdd1;

		CoreUI::ToolbarItemPtr m_speed;
		CoreUI::ToolbarItemPtr m_snapshot;

		CoreUI::ImageRef m_floppyInactive = nullptr;
		CoreUI::ImageRef m_floppyActive = nullptr;

		CoreUI::ImageRef m_hddInactive = nullptr;
		CoreUI::ImageRef m_hddActive = nullptr;
	};

}