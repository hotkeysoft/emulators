#include "VideoMDA.h"

#include <SDL.h>

#include <assert.h>

namespace video
{
	const float VSCALE = 1.6f; // TODO
	const BYTE CHAR_WIDTH = 9;
	
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

	const uint32_t Composite640Palette[16] =
	{
		0x00000000, 0x00006E31, 0x003109FF, 0x00008AFF, 0x00A70031, 0x00767676, 0x00EC11FF, 0x00BB92FF,
		0x00315A00, 0x0000DB00, 0x00767676, 0x0045F7BB, 0x00EC6300, 0x00BBE400, 0x00FF7FBB, 0x00FFFFFF
	};

	VideoMDA::VideoMDA(WORD baseAddress) :
		Logger("MDA"),
		Device6845(baseAddress, CHAR_WIDTH),
		m_screenB000("MDA", 4096, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM),
		m_alphaPalette(AlphaMonoGreenPalette)
	{
		Reset();
		m_frameBuffer = new uint32_t[720 * 350];
	}

	VideoMDA::~VideoMDA()
	{
		delete[] m_frameBuffer;
	}

	void VideoMDA::Reset()
	{
		Device6845::Reset();
	}

	void VideoMDA::Init(const char* charROM, BYTE border)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr8(0);

		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = (BYTE)(border / VSCALE);

		Device6845::Init();

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoMDA::WriteModeControlRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoMDA::ReadStatusRegister));

		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			SDL_InitSubSystem(SDL_INIT_VIDEO);
		}



		SDL_CreateWindowAndRenderer(720 + (2 * border), (350 * VSCALE) + (2 * border), 0, &m_sdlWindow, &m_sdlRenderer);

		m_sdlTexture = SDL_CreateTexture(m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 720, 350);

		SDL_RenderSetScale(m_sdlRenderer, 1.0f, VSCALE);
	}

	void VideoMDA::RenderFrame()
	{
		static size_t frames = 0;

		// TODO: don't recompute every time
		SDL_Rect srcRect = { 0, 0, 720, 350 };
		SDL_Rect destRect = { m_sdlHBorder, m_sdlVBorder, 720, 350 };

		SDL_UpdateTexture(m_sdlTexture, NULL, m_frameBuffer, 720 * sizeof(uint32_t));

		SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, &srcRect, &destRect);
		SDL_RenderPresent(m_sdlRenderer);

		Uint32 borderRGB = m_alphaPalette[/*m_color.color*/0];

		Uint8 r = Uint8(borderRGB >> 16);
		Uint8 g = Uint8(borderRGB >> 8);
		Uint8 b = Uint8(borderRGB);

		SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);
		SDL_RenderClear(m_sdlRenderer);

		if (++frames == 60)
		{
			LogPrintf(Logger::LOG_ERROR, "60 frames");
			frames = 0;
		}
	}

	BYTE VideoMDA::ReadStatusRegister()
	{
		// Bit0: Horizontal Retrace: 1:Hsync active
		// 
		// Bit1: Nothing, always 0
		// Bit2: Nothing, always 0
		//
		// Bit3: Video dot: 1:Green or Bright Green pixel drawn at the moment
		// 
		// Bit4-7: Nothing, always 1

		// register "address" selects the video dot to inspect (0-3: [B,G,R,I])
		bool dot = m_lastDot;

		BYTE status =
			(IsHSync() << 0) |
			(0 << 1) |
			(0 << 2) |
			(m_lastDot << 3) |
			(1 << 4) |
			(1 << 5) |
			(1 << 6) |
			(1 << 7);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoMDA::WriteModeControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteModeControlRegister, value=%02Xh", value);

		m_mode.hiResolution = value & 1;
		m_mode.enableVideo = value & 8;
		m_mode.blink = value & 32;

		LogPrintf(Logger::LOG_INFO, "WriteModeControlRegister [%cBLINK %cENABLE %cHIRES]",
			m_mode.blink ? ' ' : '/',
			m_mode.enableVideo ? ' ' : '/',
			m_mode.hiResolution ? ' ' : '/');
	}

	void VideoMDA::Tick()
	{
		if (!m_mode.enableVideo || !IsInit())
		{
			return;
		}

		(this->*m_drawFunc)();

		Device6845::Tick();
	}

	void VideoMDA::NewFrame()
	{
		// Pointers for alpha mode
		m_currChar = m_screenB000.getPtr8(m_config.startAddress * 2u);
		if (m_config.cursorAddress * 2u >= m_screenB000.GetSize())
		{
			m_cursorPos = nullptr;
		}
		else
		{
			m_cursorPos = m_screenB000.getPtr8(m_config.cursorAddress * 2u);
		}

		// Select draw function
		m_drawFunc = &VideoMDA::DrawTextMode;
	}

	bool VideoMDA::IsCursor() const
	{
		return (m_currChar == m_cursorPos) &&
			(m_config.cursor != CRTCConfig::CURSOR_NONE) &&
			((m_config.cursor == CRTCConfig::CURSOR_BLINK32 && IsBlink32()) || IsBlink16());
	}

	void VideoMDA::DrawTextMode()
	{
		if (m_currChar && IsDisplayArea() && ((m_data.vPos % m_data.vCharHeight) == 0))
		{
			BYTE* ch = m_currChar;
			BYTE* attr = ch + 1;
			BYTE bg = (*attr) >> 4;
			BYTE fg = (*attr) & 0x0F;
			bool charBlink = false;

			// Background
			if (m_mode.blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = bg & 8;
				bg = bg & 7;
			}

			uint32_t fgRGB = m_alphaPalette[fg];
			uint32_t bgRGB = m_alphaPalette[bg];

			bool isCursorChar = IsCursor();

			// Draw character
			BYTE* currCharPos = m_charROMStart + ((uint32_t)(*ch) * 8);
			bool draw = !charBlink || (charBlink && IsBlink16());
			for (int y = 0; y < m_data.vCharHeight; ++y)
			{
				// Lower part of character is at A10=1 in ROM
				if (y == 8)
				{
					currCharPos += (1 << 11);
				}

				uint32_t offset = 720 * (uint32_t)(m_data.vPos + y) + m_data.hPos;
				bool cursorLine = isCursorChar && (y >= m_config.cursorStart) && (y <= m_config.cursorEnd);
				for (int x = 0; x < 9; ++x)
				{

					if (x < 8)
					{
						m_lastDot = draw && ((*(currCharPos + (y & 7))) & (1 << (7 - x)));
					}
					// Characters C0h - DFh: 9th pixel == 8th pixel, otherwise blank
					else if ((*ch < 0xC0) || (*ch > 0xDF))
					{
						m_lastDot = 0;
					}

					m_frameBuffer[offset + x] = (m_lastDot || cursorLine) ? fgRGB : bgRGB;
				}
			}

			m_currChar += 2;
		}
	}
}
