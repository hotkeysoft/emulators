#include "DeviceCGA.h"
#include <assert.h>

#include "SDL.h"

namespace cga
{
	const uint32_t CGAPalette[16] = {
		0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
		0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
	};

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
		m_charROMStart = m_charROM.getPtr8(4096 + 2048);

		// CRTC Register Select
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&DeviceCGA::SelectCRTCRegister));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&DeviceCGA::SelectCRTCRegister));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&DeviceCGA::SelectCRTCRegister));
		Connect(m_baseAddress + 6, static_cast<PortConnector::OUTFunction>(&DeviceCGA::SelectCRTCRegister));

		// CRTC Register Data
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&DeviceCGA::WriteCRTCData));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&DeviceCGA::WriteCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&DeviceCGA::WriteCRTCData));
		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&DeviceCGA::WriteCRTCData));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadCRTCData));
		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadCRTCData));
		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadCRTCData));

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&DeviceCGA::WriteModeControlRegister));

		// Color Select Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&DeviceCGA::WriteColorSelectRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&DeviceCGA::ReadStatusRegister));

		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(640, 480, 0, &m_sdlWindow, &m_sdlRenderer);
		SDL_RenderSetScale(m_sdlRenderer, 1.0f, 2.4f);
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

	void DeviceCGA::SelectCRTCRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "SelectCRTCRegister, reg=%d", value);
		m_crtc.currRegister = (value > _CRT_MAX_REG) ? CRT_INVALID_REG : (CRTRegister)value;
	}

	BYTE DeviceCGA::ReadCRTCData()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadCRTCData, reg=%d", m_crtc.currRegister);

		switch (m_crtc.currRegister)
		{
		case CRT_CURSOR_ADDR_HI:
			return (m_crtc.cursorAddress >> 8);
		case CRT_CURSOR_ADDR_LO:
			return (BYTE)m_crtc.cursorAddress;

		case CRT_LIGHT_PEN_HI:
		case CRT_LIGHT_PEN_LO:
		default:
			return 0xFF;
		}
	}
	void DeviceCGA::WriteCRTCData(BYTE value)
	{
		switch (m_crtc.currRegister)
		{
		case CRT_H_TOTAL_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             hTotal = %d characters", value);
			m_crtc.hTotal = value;
			break;
		case CRT_H_DISPLAYED_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hDisplayed = %d characters", value);
			m_crtc.hDisplayed = value;
			break;
		case CRT_H_SYNC_POS_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           hSyncPos = %d characters", value);
			m_crtc.hSyncPos = value;
			break;
		case CRT_H_SYNC_WIDTH_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hSyncWidth = %d characters", value);
			m_crtc.hSyncWidth = value;
			break;
		case CRT_V_TOTAL_ROW:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             vTotal = %d rows", value);
			m_crtc.vTotal = value;
			break;
		case CRT_V_TOTAL_ADJ_LINES:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       vTotalAdjust = %d scanlines", value);
			m_crtc.vTotalAdjust = value;
			break;
		case CRT_V_DISPLAYED_ROW:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:    vTotalDisplayed = %d rows", value);
			m_crtc.vTotalDisplayed = value;
			break;
		case CRT_V_SYNC_POS_ROW:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           vSyncPos = %d rows", value);
			m_crtc.vSyncPos = value;
			break;
		case CRT_INTERLACE_MODE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      interlaceMode = %d", value);
			m_crtc.interlaceMode = value;
			break;
		case CRT_MAX_SCANLINE_ADDR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData: maxScanlineAddress = %d scanlines", value);
			m_crtc.maxScanlineAddress = value;
			break;
		case CRT_CURSOR_START_LINE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        cursorStart = %d scanline" , value);
			m_crtc.cursorStart = value;
			break;
		case CRT_CURSOR_END_LINE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          cursorEnd = %d scanline", value);
			m_crtc.cursorEnd = value;
			break;
		case CRT_START_ADDR_HI:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:   startAddress(HI) = %d", value);
			emul::SetHiByte(m_crtc.startAddress, value);
			break;
		case CRT_START_ADDR_LO:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:  startAddress(LOW) = %d", value);
			emul::SetLowByte(m_crtc.startAddress, value);
			break;
		case CRT_CURSOR_ADDR_HI:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:  cursorAddress(HI) = %d", value);
			emul::SetHiByte(m_crtc.cursorAddress, value);
			break;
		case CRT_CURSOR_ADDR_LO:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:  cursorAddress(LO) = %d", value);
			emul::SetLowByte(m_crtc.cursorAddress, value);
			break;
		default:
			LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: Invalid Register, reg=%d", m_crtc.currRegister);
		}
	}

	BYTE DeviceCGA::ReadStatusRegister()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, hSync=%d, vSync=%d", IsHSync(), IsVSync());
		return (IsHSync() ? 1 : 0) | (IsVSync() ? 8 : 0);
	}

	void DeviceCGA::WriteModeControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteModeControlRegister, value=%02Xh", value);
	}

	void DeviceCGA::WriteColorSelectRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteColorSelectRegister, value=%02Xh", value);
	}

	void DeviceCGA::Tick()
	{
		if (m_vPos < 200 && m_hPos < 640 && ((m_vPos % 8) == 0))
		{
			BYTE* ch = m_screenB800.getPtr8(((m_vPos / 8) * 80 + (m_hPos / 8)) * 2);
			BYTE* attr = ch + 1;
			BYTE bg = (*attr) >> 4;
			BYTE fg = (*attr) & 0x0F;

			// Background
			uint32_t bgRGB = CGAPalette[bg & 7];
			Uint8 r = Uint8(bgRGB >> 16);
			Uint8 g = Uint8(bgRGB >> 8);
			Uint8 b = Uint8(bgRGB);

			SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);
			SDL_Rect bgRect;
			bgRect.x = m_hPos;
			bgRect.y = m_vPos;
			bgRect.w = 8;
			bgRect.h = 8;
			SDL_RenderFillRect(m_sdlRenderer, &bgRect);

			// Foreground
			uint32_t fgRGB = CGAPalette[fg];
			r = Uint8(fgRGB >> 16);
			g = Uint8(fgRGB >> 8);
			b = Uint8(fgRGB);

			SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);

			BYTE* currCharPos = m_charROMStart + ((*ch) * 8) + (m_vPos % 8);
			for (int y = 0; y < 8; ++y)
			{
				for (int x = 0; x < 8; ++x)
				{
					if (((*(currCharPos + y)) & (1 << (7 - x))))
					{
						SDL_RenderDrawPoint(m_sdlRenderer, m_hPos + x, (m_vPos + y));
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
