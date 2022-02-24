#include "Video.h"
#include "../Config.h"

#include <SDL.h>

#pragma warning(disable:4251)
#include <Core/WindowManager.h>

#include <assert.h>

using cfg::Config;

namespace video
{
	//TODO
	const int overlayHeight = 64;

	const uint32_t AlphaColorPalette[16] =
	{
		0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA, 0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
		0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF, 0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
	};

	const uint32_t AlphaMonoGreyPalette[16] =
	{
		0xFF000000, 0xFF0C0C0C, 0xFF7A7A7A, 0xFF868686, 0xFF242424, 0xFF303030, 0xFF616161, 0xFFAAAAAA,
		0xFF555555, 0xFF616161, 0xFFCFCFCF, 0xFFDBDBDB, 0xFF797979, 0xFF858585, 0xFFF3F3F3, 0xFFFFFFFF
	};

	const uint32_t AlphaMonoGreenPalette[16] =
	{
		0xFF000000, 0xFF020A00, 0xFF1D7700, 0xFF218400, 0xFF082300, 0xFF0B2D00, 0xFF186000, 0xFF2AA800,
		0xFF155400, 0xFF186000, 0xFF33CE00, 0xFF36D800, 0xFF1D7700, 0xFF218400, 0xFF3CF200, 0xFF41ff00
	};

	Video::Video() :
		m_sdlWidth(800),
		m_sdlHeight(600)
	{
	}

	Video::~Video()
	{
		DestroyFrameBuffer();
	}

	void Video::Init(emul::Memory* memory, const char*, bool forceMono)
	{
		assert(memory);
		m_memory = memory;

		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			SDL_InitSubSystem(SDL_INIT_VIDEO);
		}

		float scale = std::max(1.0f, Config::Instance().GetValueFloat("video", "scale", 1.0f));
		bool fullScreen = Config::Instance().GetValueBool("video", "fullscreen");
		LogPrintf(LOG_INFO, "Scale factor: %f", scale);
		LogPrintf(LOG_INFO, "Full screen: %d", fullScreen);

		//TODO: Temporary, while working on rendering
		//SDL_CreateWindowAndRenderer(
		//	(int)((m_sdlWidth + (2 * border)) * scale),
		//	(int)(((m_sdlHeight * m_vScale) + border) * scale + overlayHeight),
		//	fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0,
		//	&m_sdlWindow,
		//	&m_sdlRenderer);

		SDL_CreateWindowAndRenderer(
			(int)(m_sdlWidth),
			(int)(m_sdlHeight) + overlayHeight,
			fullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_RESIZABLE,
			&m_sdlWindow,
			&m_sdlRenderer);

		SDL_SetWindowTitle(m_sdlWindow, "hotkey86");

		int actualW, actualH;
		SDL_GetWindowSize(m_sdlWindow, &actualW, &actualH);
		LogPrintf(LOG_INFO, "Window Size: %dx%d", actualW, actualH);
		m_sdlWidth = actualW;
		m_sdlHeight = actualH;

		std::string filtering = Config::Instance().GetValueStr("video", "filtering", "0");
		if (filtering.empty())
		{
			filtering = "0";
		}
		LogPrintf(LOG_INFO, "Render Scale Quality: %s", filtering.c_str());
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, filtering.c_str());

		//TODO: Temporary, while working on rendering
		//SDL_RenderSetScale(m_sdlRenderer, scale, scale * m_vScale);
		SDL_RenderSetScale(m_sdlRenderer, 1.0f, 1.0f);

		InitMonitor(forceMono);
	}

	void Video::InitFrameBuffer(WORD width, WORD height)
	{
		LogPrintf(LOG_INFO, "InitFrameBuffer: %dx%d", width, height);
		if (m_fbWidth == width && m_fbHeight == height)
		{
			return;
		}

		DestroyFrameBuffer();

		if (width && height)
		{
			m_fbWidth = width;
			m_fbHeight = height;
			m_fb.resize(width * height);
			std::fill(m_fb.begin(), m_fb.end(), GetBackgroundColor());

			m_sdlTexture = SDL_CreateTexture(m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		}
		else
		{
			LogPrintf(LOG_WARNING, "InitFrameBuffer: width or height is zero");
		}
	}

	void Video::DestroyFrameBuffer()
	{
		m_fbWidth = 0;
		m_fbHeight = 0;

		if (m_sdlTexture)
		{
			SDL_DestroyTexture(m_sdlTexture);
			m_sdlTexture = nullptr;
		}
	}

	void Video::InitMonitor(bool forceMono)
	{
		m_monitorPalette = forceMono ? AlphaMonoGreyPalette : AlphaColorPalette;

		std::string monit = Config::Instance().GetValueStr("video", "monitor", "rgb");
		if (monit == "mono" || monit == "monowhite")
		{
			m_monitor = MonitorType::MONO_WHITE;
			m_monitorPalette = AlphaMonoGreyPalette;
			LogPrintf(LOG_INFO, "Monitor: Monochrome (white)");
		}
		else if (monit == "monoamber")
		{
			m_monitor = MonitorType::MONO_AMBER;
			m_monitorPalette = AlphaMonoGreyPalette; // TODO
			LogPrintf(LOG_INFO, "Monitor: Monochrome (amber)");
		}
		else if (monit == "monogreen")
		{
			m_monitor = MonitorType::MONO_GREEN;
			m_monitorPalette = AlphaMonoGreenPalette;
			LogPrintf(LOG_INFO, "Monitor: Monochrome (green)");
		}
		else if (monit == "composite")
		{
			m_monitor = forceMono ? MonitorType::MONO_WHITE : MonitorType::COMPOSITE;
			LogPrintf(LOG_INFO, "Monitor: %s", forceMono ? "Monochrome (white)(forced)" : "Composite");
		}
		else
		{
			m_monitor = forceMono ? MonitorType::MONO_WHITE : MonitorType::RGB;
			LogPrintf(LOG_INFO, "Monitor: %s", forceMono ? "Monochrome (white)(forced)" : "RGB");
		}
	}

	void Video::RenderFrame()
	{
		static size_t frames = 0;

		SDL_Rect srcRect = GetDisplayRect(m_border);
		SDL_Rect destRect = { 0, 0, m_sdlWidth, m_sdlHeight - overlayHeight };

		if (m_sdlTexture)
		{
			SDL_UpdateTexture(m_sdlTexture, nullptr, &m_fb[0], m_fbWidth * sizeof(uint32_t));
			SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, &srcRect, &destRect);
		}

		float scaleX, scaleY;
		SDL_RenderGetScale(m_sdlRenderer, &scaleX, &scaleY);
		SDL_RenderSetScale(m_sdlRenderer, 1.0f, 1.0f);
		for (auto renderer : m_renderers)
		{
			renderer->Render();
		}
		SDL_RenderSetClipRect(m_sdlRenderer, nullptr);
		SDL_RenderSetScale(m_sdlRenderer, scaleX, scaleY);

		SDL_RenderPresent(m_sdlRenderer);

		uint32_t bg = GetBackgroundColor();
		Uint8 r = Uint8(bg >> 16);
		Uint8 g = Uint8(bg >> 8);
		Uint8 b = Uint8(bg);

		SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);
		SDL_RenderClear(m_sdlRenderer);

		if (++frames == 60)
		{
			LogPrintf(Logger::LOG_INFO, "60 frames");
			frames = 0;
		}
	}

	void Video::BeginFrame()
	{
		m_fbCurrPos = 0;
		m_fbCurrY = 0;
	}

	void Video::NewLine()
	{
		m_fbCurrY = std::min(m_fbHeight - 1, m_fbCurrY + 1);
		m_fbCurrPos = m_fbCurrY * m_fbWidth;
	}

	void Video::DrawBackground(BYTE width)
	{
		DrawBackground(width, GetBackgroundColor());
	}

	void Video::DrawBackground(BYTE width, uint32_t color)
	{
		m_lastDot = color;
		if (m_fb.size())
		{
			std::fill_n(m_fb.begin() + m_fbCurrPos, width, color);
			m_fbCurrPos += width;
		}
	}

	void Video::AddMode(const char* id, DrawFunc draw, AddressFunc address, ColorFunc color)
	{
		assert(id);
		assert(draw);
		assert(address);
		assert(color);
		m_modes[id] = { draw, address, color };
	}

	void Video::SetMode(const char* id)
	{
		LogPrintf(LOG_INFO, "Set Mode: %s", id);
		auto it = m_modes.find(id);
		if (it == m_modes.end())
		{
			LogPrintf(LOG_ERROR, "Unknown mode: %s", id);
			throw std::exception("Unknown mode");
		}

		m_currMode = &(it->second);
	}
}
