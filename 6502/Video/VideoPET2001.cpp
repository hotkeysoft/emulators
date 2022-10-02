#include "stdafx.h"
#include "VideoPET2001.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	VideoPET2001::VideoPET2001() :
		Logger("vidPET2001"),
		m_charROM("char", 0x0800, emul::MemoryType::ROM)
	{
	}

	void VideoPET2001::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		assert(charROM);

		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);

		m_charROM.LoadFromFile(charROM);

		m_bgColor = GetMonitorPalette()[0];
		m_fgColor = GetMonitorPalette()[15];
	}

	void VideoPET2001::Tick()
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

		if (m_currY == BOTTOM_BORDER)
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

	SDL_Rect VideoPET2001::GetDisplayRect(BYTE border, WORD xMultiplier) const
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

	void VideoPET2001::DrawChar()
	{
		BYTE ch = m_memory->Read8(m_currChar);
		bool reverse = GetBit(ch, 7);
		SetBit(ch, 7, 0);

		BYTE pixels = m_charROM.read((ch * CHAR_HEIGHT) + m_currRow);

		for (int i = 0; i < CHAR_WIDTH; ++i)
		{
			DrawPixel((GetBit(pixels, 7-i) ^ reverse )? m_fgColor : m_bgColor);
		}
	}
}