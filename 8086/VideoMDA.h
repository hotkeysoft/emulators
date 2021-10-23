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
	class VideoMDA : public crtc::Device6845
	{
	public:
		VideoMDA(WORD baseAddress);
		virtual ~VideoMDA();

		VideoMDA() = delete;
		VideoMDA(const VideoMDA&) = delete;
		VideoMDA& operator=(const VideoMDA&) = delete;
		VideoMDA(VideoMDA&&) = delete;
		VideoMDA& operator=(VideoMDA&&) = delete;

		void Init(const char* charROM, BYTE border = 10);
		virtual void Reset() override;

		virtual void Tick() override;

		emul::MemoryBlock& GetVideoRAM() { return m_screenB000; }

	protected:
		bool IsCursor() const;

		// Mode Control Register
		struct MODEControl
		{
			bool enableVideo = false;
			bool hiResolution = false;
			bool blink = false;
		} m_mode;
		void WriteModeControlRegister(BYTE value);

		typedef void(VideoMDA::* DrawFunc)();
		DrawFunc m_drawFunc = &VideoMDA::DrawTextMode;
		void DrawTextMode();

		// Status Register
		BYTE ReadStatusRegister();

		// 4K screen buffer
		emul::MemoryBlock m_screenB000;

		// Text mode pointers
		BYTE* m_cursorPos = nullptr;
		BYTE* m_currChar = nullptr;

		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		const uint32_t* m_alphaPalette = nullptr;

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;
		SDL_Texture* m_sdlTexture = nullptr;

		BYTE m_sdlBorderPixels;
		BYTE m_sdlHBorder;
		BYTE m_sdlVBorder;

		uint32_t* m_frameBuffer;

		// dot information (status register)
		bool m_lastDot = 0;

		virtual void RenderFrame() override;
		virtual void NewFrame() override;
	};
}
