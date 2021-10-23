#include "Video.h"

#include <SDL.h>

#include <assert.h>

namespace video
{
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

	void Video::Init(BYTE border)
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
