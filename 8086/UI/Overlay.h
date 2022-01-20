#pragma once

#include "../Common/Logger.h"
#include <Common.h>

namespace ui
{
	class Overlay : public Logger
	{
	public:
		Overlay();

		bool Init(CoreUI::MainWindowRef win, CoreUI::RendererRef ren);
		bool Update();
	private:
		CoreUI::RendererRef m_renderer = nullptr;
		CoreUI::MainWindowRef m_window = nullptr;
	};

}