#include "Video.h"
#include "Config.h"

#include <SDL.h>

#include <assert.h>

using cfg::Config;

namespace video
{
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

	Video::Video(uint16_t width, uint16_t height, float vScale) :
		m_sdlWidth(width),
		m_sdlHeight(height),
		m_vScale(vScale)
	{
		assert(width);
		assert(height);
		assert(vScale > 0.0f);

		// TODO: Allow dynamic resizing for different modes
		m_frameBuffer = new uint32_t[width * height];
	}

	Video::~Video()
	{
		delete[] m_frameBuffer;
	}

	void Video::Init(emul::Memory*, const char*, BYTE border, bool forceMono)
	{
		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = (BYTE)(border / m_vScale);

		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			SDL_InitSubSystem(SDL_INIT_VIDEO);
		}

		SDL_CreateWindowAndRenderer(
			m_sdlWidth + (2 * border),
			(int)(m_sdlHeight * m_vScale) + (2 * border),
			0,
			&m_sdlWindow,
			&m_sdlRenderer);

		m_sdlTexture = SDL_CreateTexture(m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_sdlWidth, m_sdlHeight);

		SDL_RenderSetScale(m_sdlRenderer, 1.0f, m_vScale);

		InitMonitor(forceMono);
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

	void Video::RenderFrame(uint16_t w, uint16_t h, uint32_t borderRGB)
	{
		static size_t frames = 0;

		// TODO: don't recompute every time
		SDL_Rect srcRect = { 0, 0, w, h };
		SDL_Rect destRect = { m_sdlHBorder, m_sdlVBorder, m_sdlWidth, m_sdlHeight };

		SDL_UpdateTexture(m_sdlTexture, NULL, m_frameBuffer, m_sdlWidth * sizeof(uint32_t));

		SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, &srcRect, &destRect);
		SDL_RenderPresent(m_sdlRenderer);

		Uint8 r = Uint8(borderRGB >> 16);
		Uint8 g = Uint8(borderRGB >> 8);
		Uint8 b = Uint8(borderRGB);

		SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);
		SDL_RenderClear(m_sdlRenderer);

		if (++frames == 60)
		{
			LogPrintf(Logger::LOG_INFO, "60 frames");
			frames = 0;
		}
	}
}
