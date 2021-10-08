#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"
#include "MemoryBlock.h"
#include "Device6845.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace video
{
	class VideoPCjr : public crtc::Device6845
	{
	public:
		VideoPCjr(WORD baseAddress);
		virtual ~VideoPCjr();

		VideoPCjr() = delete;
		VideoPCjr(const VideoPCjr&) = delete;
		VideoPCjr& operator=(const VideoPCjr&) = delete;
		VideoPCjr(VideoPCjr&&) = delete;
		VideoPCjr& operator=(VideoPCjr&&) = delete;

		void Init(const char* charROM, BYTE border = 10);
		virtual void Reset() override;

		virtual void Tick() override;

	protected:
		bool IsCursor() const;

		typedef void(VideoPCjr::* DrawFunc)();
		DrawFunc m_drawFunc = nullptr; //&VideoPCjr::DrawTextMode;
		//void DrawTextMode();
		//void Draw320x200();
		//void Draw640x200();

		// Text mode pointers
		//BYTE* m_cursorPos = nullptr;
		//BYTE* m_currChar = nullptr;

		//emul::MemoryBlock m_charROM;
		//BYTE* m_charROMStart;

		// Graph mode banks
		//BYTE* m_bank0 = nullptr;
		//BYTE* m_bank1 = nullptr;

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;
		SDL_Texture* m_sdlTexture = nullptr;

		BYTE m_sdlBorderPixels;
		BYTE m_sdlHBorder;
		BYTE m_sdlVBorder;

		uint32_t* m_frameBuffer;

		virtual void RenderFrame() override;
		virtual void NewFrame() override;
		virtual void EndOfRow() override;
	};
}
