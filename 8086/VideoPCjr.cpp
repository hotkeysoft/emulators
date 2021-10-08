#include "VideoPCjr.h"

#include <SDL.h>

#include <assert.h>

using emul::Memory;
using emul::MemoryBlock;
using emul::S2A;

namespace video
{
	const float VSCALE = 2.4f;

	VideoPCjr::VideoPCjr(WORD baseAddress) :
		Logger("vidPCjr"),
		Device6845(baseAddress)
	{
		Reset();
		m_frameBuffer = new uint32_t[640 * 200];
	}

	VideoPCjr::~VideoPCjr()
	{
		delete[] m_frameBuffer;
	}

	void VideoPCjr::Reset()
	{
		Device6845::Reset();
	}

	void VideoPCjr::Init(Memory* memory, const char* charROM, BYTE border)
	{
		assert(memory);
		m_memory = memory;
		//assert(charROM);
		//LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		//m_charROM.LoadBinary(charROM);
		//m_charROMStart = m_charROM.getPtr8(4096 + 2048);

		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = (BYTE)(border / VSCALE);

		Device6845::Init();

		// Registers
		// CRT, Processor Page Register
		Connect(m_baseAddress + 0xF, static_cast<PortConnector::OUTFunction>(&VideoPCjr::WritePageRegister));


		if (SDL_WasInit(SDL_INIT_VIDEO) == 0)
		{
			SDL_InitSubSystem(SDL_INIT_VIDEO);
		}

		SDL_CreateWindowAndRenderer(640 + (2 * border), 480 + (2 * border), 0, &m_sdlWindow, &m_sdlRenderer);

		m_sdlTexture = SDL_CreateTexture(m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 640, 200);

		SDL_RenderSetScale(m_sdlRenderer, 1.0f, VSCALE);
	}

	void VideoPCjr::WritePageRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WritePageRegister, value=%02Xh", value);

		m_pageRegister.crtPage = value & 7;
		m_pageRegister.cpuPage = (value >> 3) & 7;
		m_pageRegister.videoAddressMode = (value >> 6) & 3;

		const char* modeStr;
		switch (m_pageRegister.videoAddressMode)
		{
		case 0:
			m_pageRegister.addressMode = PageRegister::ADDRESSMODE::ALPHA;
			modeStr = "ALPHA";
			break;
		case 1:
			m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_LOW;
			modeStr = "GRAPH LOW";
			break;
		case 2:
			m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_HI;
			modeStr = "GRAPH HI";
			break;
		default:
			LogPrintf(LOG_ERROR, "WritePageRegister: Reserved mode");
			throw std::exception("WritePageRegister: Reserved mode");
		}

		// Pages are 16K (0x4000)
		m_pageRegister.crtBaseAddress = m_pageRegister.crtPage * 0x4000;
		m_pageRegister.cpuBaseAddress = m_pageRegister.cpuPage * 0x4000;

		LogPrintf(Logger::LOG_INFO, "Set Page Register: cpuPage[%d][%05Xh] crtPage=[%d][%05Xh] videoAddressMode=[%d][%s]", 
			m_pageRegister.crtPage, m_pageRegister.crtBaseAddress,
			m_pageRegister.cpuPage, m_pageRegister.cpuBaseAddress,
			m_pageRegister.videoAddressMode, modeStr);

		MapB800Window();
	}

	// This maps the zone of memory pointed to by the cpu page register to a 16k window at B800:0000
	void VideoPCjr::MapB800Window()
	{
		m_memory->MapWindow(m_pageRegister.cpuBaseAddress, 0xB8000, 0x4000);
	}

	void VideoPCjr::RenderFrame()
	{
		static size_t frames = 0;

		// TODO: don't recompute every time
		SDL_Rect srcRect = { 0, 0, (/*m_mode.text80Columns || m_mode.hiResolution*/true) ? 640 : 320, 200 };
		SDL_Rect destRect = { m_sdlHBorder, m_sdlVBorder, 640, 200 };

		SDL_UpdateTexture(m_sdlTexture, NULL, m_frameBuffer, 640 * sizeof(uint32_t));

		SDL_RenderCopy(m_sdlRenderer, m_sdlTexture, &srcRect, &destRect);
		SDL_RenderPresent(m_sdlRenderer);

		Uint32 borderRGB = 0xFFFFFF;// = m_alphaPalette[m_color.color];

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

	void VideoPCjr::Tick()
	{
		if (/*!m_mode.enableVideo || */!IsInit())
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

		//(this->*m_drawFunc)();

		Device6845::Tick();
	}

	void VideoPCjr::NewFrame()
	{
		// Pointers for alpha mode
		//m_currChar = m_screenB800.getPtr8(m_config.startAddress * 2u);
		//if (m_config.cursorAddress * 2u >= m_screenB800.GetSize())
		//{
		//	m_cursorPos = nullptr;
		//}
		//else
		//{
		//	m_cursorPos = m_screenB800.getPtr8(m_config.cursorAddress * 2u);
		//}

		//// Pointers for graphics mode
		//m_bank0 = m_screenB800.getPtr8(0x0000);
		//m_bank1 = m_screenB800.getPtr8(0x2000);

		//// Select draw function
		//m_drawFunc = &VideoPCjr::DrawTextMode;
		//if (m_mode.graphics) m_drawFunc = &VideoPCjr::Draw320x200;
		//if (m_mode.hiResolution) m_drawFunc = &VideoPCjr::Draw640x200;

		//// TODO: Do this for each line instead of each frame
		//m_alphaPalette = (m_mode.monochrome && m_composite) ? AlphaMonoGreyPalette : AlphaColorPalette;
		//m_currGraphPalette[0] = m_alphaPalette[m_color.color];
		//for (int i = 1; i < 4; ++i)
		//{
		//	m_currGraphPalette[i] = m_alphaPalette[
		//		(m_color.palIntense << 3) // Intensity
		//		| (i << 1)
		//		| (m_color.palSelect && !m_mode.monochrome) // Palette shift for non mono modes
		//		| (m_mode.monochrome & (i & 1)) // Palette shift for mono modes
		//	];
		//}

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

	void VideoPCjr::EndOfRow()
	{
		//if (!m_mode.hiResolution || !m_composite || m_data.vPos >= m_data.vTotalDisp)
		//	return;

		//uint32_t baseX = (640 * m_data.vPos);

		//memset(tempRowAlpha, 0, sizeof(tempRowAlpha));
		//for (int offset = 0; offset < 4; ++offset)
		//{
		//	for (int x = 0; x < 640; ++x)
		//	{
		//		//tempRowAlpha[x + offset] += (tempRow[x] & 16) ? 0x3F000000 : 0;
		//		tempRowAlpha[x + offset] += (tempRow[x] & 16) ? 1 : 0;
		//	}
		//}

		//for (int i = 0; i < 640; ++i)
		//{
		//	m_frameBuffer[baseX + i] = Composite640Palette[tempRow[i] & 15] | ((i%256) << 24);// tempRowAlpha[i + 2];
		//	//m_frameBuffer[baseX + i] = TempGray[tempRowAlpha[i+2]];
		//}

		// Clear temp row

	}

	bool VideoPCjr::IsCursor() const
	{
		return false;
		//return (m_currChar == m_cursorPos) &&
		//	(m_config.cursor != CRTCConfig::CURSOR_NONE) &&
		//	((m_config.cursor == CRTCConfig::CURSOR_BLINK32 && IsBlink32()) || IsBlink16());
	}

}
