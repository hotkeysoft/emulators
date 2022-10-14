#include "stdafx.h"
#include "VideoVIC.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	VideoVIC::VideoVIC() : Logger("vidVIC")
	{
	}

	void VideoVIC::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);

		m_bgColor = GetMonitorPalette()[0];
		m_fgColor = GetMonitorPalette()[15];
	}

	void VideoVIC::Tick()
	{
		++m_currX;

		if (m_currX == RIGHT_BORDER)
		{
			NewLine();
		}
		else if (m_currX == H_TOTAL)
		{
			++m_currY;
			m_currX = 0;
			m_currChar = CHAR_BASE + (H_DISPLAY * (m_currY / CHAR_HEIGHT));
			m_currRow = m_currY % CHAR_HEIGHT;
		}

		if ((m_currY == BOTTOM_BORDER) && (m_currX == 0))
		{
			RenderFrame();
			BeginFrame();
		}
		else if (m_currY == V_TOTAL)
		{
			m_currY = 0;
			m_currX = 0;
			m_currRow = 0;
			m_currChar = CHAR_BASE;
		}

		if (IsDisplayArea())
		{
			DrawChar();
			++m_currChar;
		}
		else
		{
			DrawBackground(8, m_bgColor);
		}
	}

	SDL_Rect VideoVIC::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		// TODO
		const uint32_t tempBorder = 8;
		return SDL_Rect{
			LEFT_BORDER_PX - tempBorder,
			TOP_BORDER - tempBorder,
			H_DISPLAY_PX + (2 * tempBorder),
			V_DISPLAY + (2 * tempBorder)
		};
	}

	void VideoVIC::DrawChar()
	{
		BYTE ch = m_memory->Read8(m_currChar);
		const bool reverse = GetBit(ch, 7);
		SetBit(ch, 7, 0);

		const WORD charROMAddress = CHARROM_BASE + (ch * CHAR_HEIGHT) + m_currRow;
		const BYTE pixels = m_memory->Read8(charROMAddress);

		for (int i = 0; i < CHAR_WIDTH; ++i)
		{
			DrawPixel((GetBit(pixels, 7-i) ^ reverse )? m_fgColor : m_bgColor);
		}
	}
}