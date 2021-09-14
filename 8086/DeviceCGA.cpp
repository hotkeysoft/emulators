#include "DeviceCGA.h"
#include <assert.h>

#include "SDL.h"

namespace cga
{
	const float VSCALE = 2.4f;

	const uint32_t AlphaColorPalette[16] = 
	{
		0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
		0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
	};

	const uint32_t AlphaMonoGreyPalette[16] =
	{
		0x000000, 0x0C0C0C, 0x7A7A7A, 0x868686, 0x242424, 0x303030, 0x616161, 0xAAAAAA,
		0x555555, 0x616161, 0xCFCFCF, 0xDBDBDB, 0x797979, 0x858585, 0xF3F3F3, 0xFFFFFF
	};

	const uint32_t AlphaMonoGreenPalette[16] =
	{
		0x000000, 0x020A00, 0x1D7700, 0x218400, 0x082300, 0x0B2D00, 0x186000, 0x2AA800,
		0x155400, 0x186000, 0x33CE00, 0x36D800, 0x1D7700, 0x218400, 0x3CF200, 0x41ff00
	};

	DeviceCGA::DeviceCGA(WORD baseAddress) :
		Logger("CGA"),
		m_baseAddress(baseAddress),
		m_screenB800("CGA", 16384, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM),
		m_alphaPalette(AlphaColorPalette)
	{
		Reset();
	}

	void DeviceCGA::Reset()
	{
		m_hPos = 0;
		m_vPos = 0;
	}

	void DeviceCGA::Init(const char* charROM, MONITOR monitor, BYTE border)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadBinary(charROM);
		m_charROMStart = m_charROM.getPtr8(4096 + 2048);

		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = border / VSCALE;

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

		switch (monitor)
		{
		case MONITOR::GREEN:
			m_alphaPalette = AlphaMonoGreenPalette;
			break;

		case MONITOR::GREY:
			m_alphaPalette = AlphaMonoGreyPalette;
			break;

		case MONITOR::AMBER: // TODO
		case MONITOR::COLOR:
		default:
			m_alphaPalette = AlphaColorPalette;
		}

		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(640 + (2 * border), 480 + (2 * border), 0, &m_sdlWindow, &m_sdlRenderer);
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
		value &= 31;
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
			UpdateHVTotals();
			break;
		case CRT_H_DISPLAYED_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hDisplayed = %d characters", value);
			m_crtc.hDisplayed = value;
			UpdateHVTotals();
			break;
		case CRT_H_SYNC_POS_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           hSyncPos = %d characters", value);
			m_crtc.hSyncPos = value;
			UpdateHVTotals();
			break;
		case CRT_H_SYNC_WIDTH_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hSyncWidth = %d characters", value);
			m_crtc.hSyncWidth = value & 15;
			UpdateHVTotals();
			break;

		case CRT_V_TOTAL_ROW:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             vTotal = %d rows", value);
			m_crtc.vTotal = value & 127;
			UpdateHVTotals();
			break;
		case CRT_V_TOTAL_ADJ_LINES:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       vTotalAdjust = %d scanlines", value);
			m_crtc.vTotalAdjust = value & 31;
			UpdateHVTotals();
			break;
		case CRT_V_DISPLAYED_ROW:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:    vTotalDisplayed = %d rows", value);
			m_crtc.vTotalDisplayed = value & 127;
			UpdateHVTotals();
			break;
		case CRT_V_SYNC_POS_ROW:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           vSyncPos = %d rows", value);
			m_crtc.vSyncPos = value & 127;
			UpdateHVTotals();
			break;

		case CRT_INTERLACE_MODE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      interlaceMode = %d", value);
			m_crtc.interlaceMode = value;
			break;

		case CRT_MAX_SCANLINE_ADDR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData: maxScanlineAddress = %d scanlines", value);
			m_crtc.maxScanlineAddress = value & 31;
			UpdateHVTotals();
			break;

		case CRT_CURSOR_START_LINE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        cursorStart = %d scanline" , value);
			m_crtc.cursorStart = (value & 31);
			m_crtc.cursor = (CRTCData::CURSOR)((value >> 5) & 3);
			break;
		case CRT_CURSOR_END_LINE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          cursorEnd = %d scanline", value);
			m_crtc.cursorEnd = (value & 31);
			break;

		case CRT_START_ADDR_HI:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:   startAddress(HI) = %d", value);
			emul::SetHiByte(m_crtc.startAddress, value & 63);
			break;
		case CRT_START_ADDR_LO:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:  startAddress(LOW) = %d", value);
			emul::SetLowByte(m_crtc.startAddress, value);
			break;

		case CRT_CURSOR_ADDR_HI:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:  cursorAddress(HI) = %d", value);
			emul::SetHiByte(m_crtc.cursorAddress, value & 63);
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

		// Adjust scaline in 40 column mode
		SDL_RenderSetScale(m_sdlRenderer, m_mode.text80Columns ? 1.0f : 2.0f, VSCALE);
		m_sdlHBorder = m_mode.text80Columns ? m_sdlBorderPixels : m_sdlBorderPixels / 2;
	}

	void DeviceCGA::WriteColorSelectRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteColorSelectRegister, value=%02Xh", value);
	}

	void DeviceCGA::UpdateHVTotals()
	{
		m_hTotalDisp = m_crtc.hDisplayed * 8;
		m_hTotal = m_crtc.hTotal * 8;
		m_hBorder = (m_crtc.hTotal - m_crtc.hSyncPos) * 8;

		m_vCharHeight = (m_crtc.maxScanlineAddress + 1);
		m_vTotalDisp = m_crtc.vTotalDisplayed * m_vCharHeight;
		m_vTotal = m_crtc.vTotal * m_vCharHeight + m_crtc.vTotalAdjust;
		m_hBorder = (m_crtc.vTotal - m_crtc.vSyncPos) * m_vCharHeight;
	}

	void DeviceCGA::Tick()
	{
		if (!m_mode.enableVideo || m_hTotal == 0 || m_vTotal == 0)
		{
			return;
		}

		if (m_currChar && (m_vPos < m_vTotalDisp) && (m_hPos < m_hTotalDisp) && ((m_vPos % 8) == 0))
		{
			BYTE* ch = m_currChar;
			BYTE* attr = ch + 1;
			BYTE bg = (*attr) >> 4;
			BYTE fg = (*attr) & 0x0F;

			// Background
			uint32_t bgRGB = m_alphaPalette[bg & 7];
			Uint8 r = Uint8(bgRGB >> 16);
			Uint8 g = Uint8(bgRGB >> 8);
			Uint8 b = Uint8(bgRGB);

			SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);
			SDL_Rect bgRect;
			bgRect.x = m_hPos + m_sdlHBorder;
			bgRect.y = m_vPos + m_sdlVBorder;
			bgRect.w = 8;
			bgRect.h = m_vCharHeight;
			SDL_RenderFillRect(m_sdlRenderer, &bgRect);

			// Foreground
			uint32_t fgRGB = m_alphaPalette[fg];
			r = Uint8(fgRGB >> 16);
			g = Uint8(fgRGB >> 8);
			b = Uint8(fgRGB);

			SDL_SetRenderDrawColor(m_sdlRenderer, r, g, b, 255);

			BYTE* currCharPos = m_charROMStart + ((*ch) * 8) + (m_vPos % m_vCharHeight);
			for (int y = 0; y < m_vCharHeight; ++y)
			{
				for (int x = 0; x < 8; ++x)
				{
					if (((*(currCharPos + y)) & (1 << (7 - x))))
					{
						SDL_RenderDrawPoint(m_sdlRenderer, m_hPos + x + m_sdlHBorder, (m_vPos + y) + m_sdlVBorder);
					}
				}
			}

			if (m_currChar == m_cursorPos && (m_crtc.cursor != CRTCData::CURSOR_NONE))
			{
				// Validate blink modes/speeds
				bool blink = (m_crtc.cursor == CRTCData::CURSOR_BLINK32 && m_blink32) || (m_blink16);
				
				if (blink)
				{
					SDL_Rect bgRect;
					bgRect.x = m_hPos + m_sdlHBorder;
					bgRect.y = m_vPos + m_sdlVBorder + m_crtc.cursorStart;
					bgRect.w = 8;
					bgRect.h = m_crtc.cursorEnd - m_crtc.cursorStart + 1;
					SDL_RenderFillRect(m_sdlRenderer, &bgRect);
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

			m_currChar += 2;
		}

		m_hPos += 8;

		if (m_hPos > m_hTotal)
		{
			m_hPos = 0;
			++m_vPos;
		}
		if (m_vPos > m_vTotal)
		{
			++m_frame;
			m_vPos = 0;
			Render();
			m_currChar = m_screenB800.getPtr8(m_crtc.startAddress);
			m_cursorPos = m_screenB800.getPtr8(m_crtc.cursorAddress*2);
			
			if ((m_frame % 16) == 0) m_blink16 = !m_blink16;
			if ((m_frame % 32) == 0) m_blink32 = !m_blink32;
		}
	}
}
