#include "DeviceCGA.h"
#include <assert.h>

#include "SDL.h"

namespace cga
{
	DeviceCGA::DeviceCGA(WORD baseAddress) :
		Logger("CGA"),
		m_baseAddress(baseAddress),
		m_screenB800("CGA", 16384, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
	}

	void DeviceCGA::Reset()
	{
		m_hPos = 0;
		m_vPos = 0;
	}

	void DeviceCGA::Init(const char* charROM)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadBinary(charROM);
		m_charROMStart = m_charROM.getPtr8(4096+2048);

		// Register Select
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		// Register Value
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));


		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		Connect(m_baseAddress + 8, static_cast<PortConnector::INFunction>(&DeviceCGA::IN));

		// Color Control Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&DeviceCGA::OUT));
		Connect(m_baseAddress + 9, static_cast<PortConnector::INFunction>(&DeviceCGA::IN));

		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadStatus));

		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(640, 480, 0, &m_sdlWindow, &m_sdlRenderer);
	}

	void DeviceCGA::Render()
	{
		static size_t frames = 0;

		SDL_RenderPresent(m_sdlRenderer);

		SDL_SetRenderDrawColor(m_sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(m_sdlRenderer);

		if (++frames == 60)
		{
			LogPrintf(Logger::LOG_ERROR, "60 frames");
			frames = 0;
		}
	}

	BYTE DeviceCGA::IN()
	{
		LogPrintf(Logger::LOG_DEBUG, "IN");
		return 0;
	}
	void DeviceCGA::OUT(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "OUT");
	}

	BYTE DeviceCGA::ReadStatus()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatus, hSync=%d, vSync=%d", IsHSync(), IsVSync());
		return (IsHSync() ? 1 : 0) | (IsVSync() ? 8 : 0);
	}

	void DeviceCGA::Tick()
	{
		if (m_vPos < 200 && m_hPos < 640 && ((m_vPos % 8) == 0))
		{
			BYTE* ch = m_screenB800.getPtr8(((m_vPos/8) * 80 + (m_hPos/8)) * 2);
			BYTE* attr = ch + 1;

			BYTE* currCharPos = m_charROMStart + ((*ch) * 8) + (m_vPos % 8);
			for (int y = 0; y < 8; ++y)
			{
				for (int x = 0; x < 8; ++x)
				{
					Uint8 color = ((*(currCharPos+y)) & (1 << (7 - x))) ? 255 : 0;
					if (color)
					{
						SDL_SetRenderDrawColor(m_sdlRenderer, color, color, color, 255);

						SDL_RenderDrawPoint(m_sdlRenderer, m_hPos + x, (m_vPos + y) * 2);
						SDL_RenderDrawPoint(m_sdlRenderer, m_hPos + x, (m_vPos + y) * 2 + 1);
					}
				}
			}

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{
					SDL_Quit();
				}
			}
		}

		m_hPos += 8;
		if (m_hPos > 448 * 2)
		{
			m_hPos = 0;
			++m_vPos;
		}
		if (m_vPos > 256)
		{
			m_vPos = 0;
			Render();
		}

	}
}
