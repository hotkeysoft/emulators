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
	class VideoCGA : public crtc::Device6845
	{
	public:
		VideoCGA(WORD baseAddress);
		virtual ~VideoCGA();

		VideoCGA() = delete;
		VideoCGA(const VideoCGA&) = delete;
		VideoCGA& operator=(const VideoCGA&) = delete;
		VideoCGA(VideoCGA&&) = delete;
		VideoCGA& operator=(VideoCGA&&) = delete;

		void Init(const char* charROM, BYTE border = 10);
		virtual void Reset() override;

		virtual void Tick() override;

		void SetComposite(bool composite) { m_composite = composite; }

		emul::MemoryBlock& GetVideoRAM() { return m_screenB800; }

	protected:
		bool IsCursor() const;

		// Mode Control Register
		struct MODEControl
		{
			bool text80Columns = false;
			bool graphics = false;
			bool monochrome = false;
			bool enableVideo = false;
			bool hiResolution = false;
			bool blink = false;
		} m_mode;
		void WriteModeControlRegister(BYTE value);

		typedef void(VideoCGA::* DrawFunc)();
		DrawFunc m_drawFunc = &VideoCGA::DrawTextMode;
		void DrawTextMode();
		void Draw320x200();
		void Draw640x200();

		// Color Select Register
		struct COLORSelect
		{
			BYTE color = 0; // bits 0-3: bgri border (alpha) / border+bg (320x200) / fg (640x200)

			bool palIntense = false; // bit 4 intensity palette (320x200)
			bool palSelect = false; // bit 5 graph palette (320x200)
		} m_color;
		void WriteColorSelectRegister(BYTE value);

		// Status Register
		BYTE ReadStatusRegister();

		// 16K screen buffer
		emul::MemoryBlock m_screenB800;

		bool m_composite = false; // Composite artifact colors

		// Text mode pointers
		BYTE* m_cursorPos = nullptr;
		BYTE* m_currChar = nullptr;

		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		// Graph mode banks
		BYTE* m_bank0 = nullptr;
		BYTE* m_bank1 = nullptr;

		const uint32_t* m_alphaPalette = nullptr;
		uint32_t m_currGraphPalette[4];

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
