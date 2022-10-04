#include "stdafx.h"
#include <UI/MainWindow.h>
#include <Config.h>
#include <SDL.h>

using cfg::CONFIG;

namespace ui
{
	MainWindow::MainWindow() : Logger("mainWnd")
	{

	}

	MainWindow& MainWindow::Get()
	{
		static MainWindow wnd;
		return wnd;
	}

	bool MainWindow::Init(WORD overlayHeight)
	{
		EnableLog(CONFIG().GetLogLevel("mainwindow"));

		// TODO
		m_size.w = 800;
		m_size.h = 600;

		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			LogPrintf(LOG_INFO, "SDL Init Subsystem [Video]");
			if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
			{
				LogPrintf(LOG_ERROR, "Error initializing video subsystem: %s", SDL_GetError());
				return false;
			}
		}

		bool fullScreen = CONFIG().GetValueBool("video", "fullscreen");
		LogPrintf(LOG_INFO, "Full screen: %d", fullScreen);

		SDL_CreateWindowAndRenderer(
			(int)(m_size.w),
			(int)(m_size.h) + overlayHeight,
			fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE,
			&m_sdlWindow,
			&m_sdlRenderer);

		SDL_SetWindowTitle(m_sdlWindow, "hotkey86");

		SDL_GetWindowSize(m_sdlWindow, &m_size.w, &m_size.h);
		LogPrintf(LOG_INFO, "Window Size: %dx%d", m_size.w, m_size.h);

		std::string filtering = CONFIG().GetValueStr("video", "filtering", "0");
		if (filtering.empty())
		{
			filtering = "0";
		}

		LogPrintf(LOG_INFO, "Render Scale Quality: %s", filtering.c_str());
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filtering.c_str());

		return true;
	}
}
