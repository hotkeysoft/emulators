#include "VideoCGA.h"

#include <SDL.h>

#include <assert.h>

namespace video
{
	const float VSCALE = 2.4f;

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

	const uint32_t Composite640Palette[16] =
	{
		0x00000000, 0x00006E31, 0x003109FF, 0x00008AFF, 0x00A70031, 0x00767676, 0x00EC11FF, 0x00BB92FF,
		0x00315A00, 0x0000DB00, 0x00767676, 0x0045F7BB, 0x00EC6300, 0x00BBE400, 0x00FF7FBB, 0x00FFFFFF
	};

	const uint32_t TempGray[] =
	{
		0xFF000000, 0xFF3F3F3F, 0xFF7F7F7F, 0xFFFFFFFF
	};

	VideoCGA::VideoCGA(WORD baseAddress) :
		Logger("CGA"),
		Device6845(baseAddress),
		m_screenB800("CGA", 16384, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM),
		m_alphaPalette(AlphaColorPalette)
	{
		Reset();
		m_frameBuffer = new uint32_t[640 * 200];
	}

	VideoCGA::~VideoCGA()
	{
		delete[] m_frameBuffer;
	}

	void VideoCGA::Reset()
	{
		Device6845::Reset();
	}

	void VideoCGA::Init(const char* charROM, BYTE border)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadBinary(charROM);
		m_charROMStart = m_charROM.getPtr8(4096 + 2048);

		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = (BYTE)(border / VSCALE);

		Device6845::Init();

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteModeControlRegister));

		// Color Select Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteColorSelectRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoCGA::ReadStatusRegister));

		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			SDL_InitSubSystem(SDL_INIT_VIDEO);
		}

		SDL_CreateWindowAndRenderer(640 + (2 * border), 480 + (2 * border), 0, &m_sdlWindow, &m_sdlRenderer);

		m_sdlTexture = SDL_CreateTexture(m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 640, 200);

		SDL_RenderSetScale(m_sdlRenderer, 1.0f, VSCALE);
	}

	void VideoCGA::RenderFrame()
	{
		static size_t frames = 0;

		// TODO: don't recompute every time
		SDL_Rect srcRect = { 0, 0, (m_mode.text80Columns || m_mode.hiResolution) ? 640 : 320, 200 };
		SDL_Rect destRect = { m_sdlHBorder, m_sdlVBorder, 640, 200 };

		SDL_UpdateTexture(m_sdlTexture, NULL, m_frameBuffer, 640 * sizeof(uint32_t));

		SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, &srcRect, &destRect);
		SDL_RenderPresent(m_sdlRenderer);

		Uint32 borderRGB = m_alphaPalette[m_color.color];

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

	BYTE VideoCGA::ReadStatusRegister()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, hSync=%d, vSync=%d", IsHSync(), IsVSync());
		return (IsHSync() ? 1 : 0) | (IsVSync() ? 8 : 0);
	}

	void VideoCGA::WriteModeControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteModeControlRegister, value=%02Xh", value);

		m_mode.text80Columns = value & 1;
		m_mode.graphics = value & 2;
		m_mode.monochrome = value & 4;
		m_mode.enableVideo = value & 8;
		m_mode.hiResolution = value & 16;
		m_mode.blink = value & 32;

		LogPrintf(Logger::LOG_INFO, "WriteModeControlRegister [%c80COLUMNS %cGRAPH %cMONO %cVIDEO %cHIRES %cBLINK]",
			m_mode.text80Columns ? ' ' : '/',
			m_mode.graphics ? ' ' : '/',
			m_mode.monochrome ? ' ' : '/',
			m_mode.enableVideo ? ' ' : '/',
			m_mode.hiResolution ? ' ' : '/',
			m_mode.blink ? ' ' : '/');
	}

	void VideoCGA::WriteColorSelectRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteColorSelectRegister, value=%02Xh", value);

		m_color.color = (value & 15);
		m_color.palIntense = value & 16;
		m_color.palSelect = value & 32;

		LogPrintf(Logger::LOG_INFO, "WriteColorSelectRegister color=%d, palette %d, intense %d", 
			m_color.color, 
			m_color.palSelect,
			m_color.palIntense);
	}

	void VideoCGA::Tick()
	{
		if (!m_mode.enableVideo || !IsInit())
		{
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{
					SDL_Quit();
				}
			}
			return;
		}

		(this->*m_drawFunc)();

		Device6845::Tick();
	}

	void VideoCGA::NewFrame()
	{
		// Pointers for alpha mode
		m_currChar = m_screenB800.getPtr8(m_config.startAddress * 2u);
		if (m_config.cursorAddress * 2u >= m_screenB800.GetSize())
		{
			m_cursorPos = nullptr;
		}
		else
		{
			m_cursorPos = m_screenB800.getPtr8(m_config.cursorAddress * 2u);
		}

		// Pointers for graphics mode
		m_bank0 = m_screenB800.getPtr8(0x0000);
		m_bank1 = m_screenB800.getPtr8(0x2000);

		// Select draw function
		m_drawFunc = &VideoCGA::DrawTextMode;
		if (m_mode.graphics) m_drawFunc = &VideoCGA::Draw320x200;
		if (m_mode.hiResolution) m_drawFunc = &VideoCGA::Draw640x200;

		// TODO: Do this for each line instead of each frame
		m_alphaPalette = (m_mode.monochrome && m_composite) ? AlphaMonoGreyPalette : AlphaColorPalette;
		m_currGraphPalette[0] = m_alphaPalette[m_color.color];
		for (int i = 1; i < 4; ++i)
		{
			m_currGraphPalette[i] = m_alphaPalette[
				(m_color.palIntense << 3) // Intensity
				| (i << 1)
				| (m_color.palSelect && !m_mode.monochrome) // Palette shift for non mono modes
				| (m_mode.monochrome & (i & 1)) // Palette shift for mono modes
			];
		}

		// TODO: Move this elsewhere
		// Process Events
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				SDL_Quit();
			}
		}
	}

	static Uint32 tempRowAlpha[640 + 2];
	static Uint32 tempRow[640];

	void VideoCGA::EndOfRow()
	{
		if (!m_mode.hiResolution || !m_composite || m_data.vPos >= m_data.vTotalDisp)
			return;

		uint32_t baseX = (640 * m_data.vPos);

		memset(tempRowAlpha, 0, sizeof(tempRowAlpha));
		for (int offset = 0; offset < 4; ++offset)
		{
			for (int x = 0; x < 640; ++x)
			{
				//tempRowAlpha[x + offset] += (tempRow[x] & 16) ? 0x3F000000 : 0;
				tempRowAlpha[x + offset] += (tempRow[x] & 16) ? 1 : 0;
			}
		}

		for (int i = 0; i < 640; ++i)
		{
			m_frameBuffer[baseX + i] = Composite640Palette[tempRow[i] & 15] | ((i%256) << 24);// tempRowAlpha[i + 2];
			//m_frameBuffer[baseX + i] = TempGray[tempRowAlpha[i+2]];
		}

		// Clear temp row

	}

	bool VideoCGA::IsCursor() const
	{
		return (m_currChar == m_cursorPos) &&
			(m_config.cursor != CRTCConfig::CURSOR_NONE) &&
			((m_config.cursor == CRTCConfig::CURSOR_BLINK32 && IsBlink32()) || IsBlink16());
	}

	void VideoCGA::DrawTextMode()
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
			BYTE* currCharPos = m_charROMStart + ((uint32_t)(*ch) * 8) + (m_data.vPos % m_data.vCharHeight);
			bool draw = !charBlink || (charBlink && IsBlink16());
			for (int y = 0; y < m_data.vCharHeight; ++y)
			{
				uint32_t offset = 640 * (uint32_t)(m_data.vPos + y) + m_data.hPos;
				bool cursorLine = isCursorChar && (y >= m_config.cursorStart) && (y <= m_config.cursorEnd);
				for (int x = 0; x < 8; ++x)
				{
					bool set = draw && ((*(currCharPos + y)) & (1 << (7 - x)));
					m_frameBuffer[offset + x] = (set || cursorLine) ? fgRGB : bgRGB;
				}
			}

			m_currChar += 2;
		}
	}
	void VideoCGA::Draw320x200()
	{
		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels

		BYTE* &currChar = (m_data.vPos & 1) ? m_bank1 : m_bank0;

		if (IsDisplayArea())
		{
			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;
				for (int x = 0; x < 4; ++x)
				{
					BYTE val = ch & 3;
					ch >>= 2;

					m_frameBuffer[640 * m_data.vPos + m_data.hPos + (w * 4) + (3 - x)] = m_currGraphPalette[val];
				}

				++currChar;
			}
		}
	}

	void VideoCGA::Draw640x200()
	{
		// Called every 8 horizontal pixels, but since crtc is 40 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels

		if (IsDisplayArea())
		{
			BYTE*& currChar = (m_data.vPos & 1) ? m_bank1 : m_bank0;

			Uint32 fg = m_alphaPalette[m_color.color];
			Uint32 bg = m_alphaPalette[0];

			uint32_t baseX = (640 * m_data.vPos) + (m_data.hPos * 2);

			uint32_t compositeOffset = m_data.hPos * 2;

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;

				if (m_composite)
				{
					BYTE colorH = ch >> 4;
					BYTE colorL = ch & 15;

					tempRow[compositeOffset++] = colorH | ((ch & 0b10000000) ? 16 : 0);
					tempRow[compositeOffset++] = colorH | ((ch & 0b01000000) ? 16 : 0);
					tempRow[compositeOffset++] = colorH | ((ch & 0b00100000) ? 16 : 0);
					tempRow[compositeOffset++] = colorH | ((ch & 0b00010000) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00001000) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00000100) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00000010) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00000001) ? 16 : 0);
				}
				else
				{
					m_frameBuffer[baseX++] = (ch & 0b10000000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b01000000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00100000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00010000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00001000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00000100) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00000010) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00000001) ? fg : bg;
				}
				++currChar;
			}
		}
	}
}
