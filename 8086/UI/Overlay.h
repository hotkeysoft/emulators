#pragma once

#include "../Common/Logger.h"
#include "../Video/Video.h"
#include "../IO/InputEvents.h"
#include <Common.h>

namespace ui
{
	class Overlay : public Logger, public video::Renderer, public events::EventHandler
	{
	public:
		Overlay();

		bool Init(CoreUI::MainWindowRef win, CoreUI::RendererRef ren);
		bool Update();

		// video::Renderer
		virtual void Render() override;

		// events::EventHangler
		virtual bool HandleEvent(SDL_Event& e) override;

	protected:
		void OnClick(CoreUI::WidgetRef widget);

		CoreUI::RendererRef m_renderer = nullptr;
		CoreUI::MainWindowRef m_window = nullptr;
	};

}