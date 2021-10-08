#pragma once

#include "Common.h"
#include "Memory.h"
#include "PortConnector.h"
#include "Logger.h"
#include "Device6845.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

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

		void Init(emul::Memory* memory, const char* charROM, BYTE border = 10);
		virtual void Reset() override;

		virtual void Tick() override;

	protected:
		emul::Memory* m_memory = nullptr;

		bool IsCursor() const;

		struct PageRegister
		{
			BYTE crtPage = 0;
			BYTE cpuPage = 0;
			BYTE videoAddressMode = 0;

			enum class ADDRESSMODE {
				ALPHA = 0,
				GRAPH_LOW = 1,
				GRAPH_HI = 2
			} addressMode;

			ADDRESS crtBaseAddress = 0;
			ADDRESS cpuBaseAddress = 0;

		} m_pageRegister;
		void WritePageRegister(BYTE value);

		void WriteGateArrayRegister(BYTE value);
		BYTE ReadStatusRegister();

		void MapB800Window();

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
