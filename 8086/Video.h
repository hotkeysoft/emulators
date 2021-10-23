#pragma once

#include "Common.h"
#include "PortConnector.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace video
{
	class Video : public PortConnector
	{
	public:
		Video(uint16_t width, uint16_t height, float vScale = 1.0f);
		virtual ~Video();

		Video() = delete;
		Video(const Video&) = delete;
		Video& operator=(const Video&) = delete;
		Video(Video&&) = delete;
		Video& operator=(Video&&) = delete;

		virtual void Reset() {};
		void Init(BYTE border = 10);

		virtual void Tick() = 0;

		void RenderFrame(uint16_t w, uint16_t h, uint32_t borderRGB = 0xFF000000);

	protected:

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;
		SDL_Texture* m_sdlTexture = nullptr;

		uint16_t m_sdlWidth = 0;
		uint16_t m_sdlHeight = 0;

		BYTE m_sdlBorderPixels;
		BYTE m_sdlHBorder;
		BYTE m_sdlVBorder;

		float m_vScale = 1.0f;

		uint32_t* m_frameBuffer;
	};
}
