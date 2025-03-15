#include "stdafx.h"
#include "VideoMC10.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	//static const uint32_t s_palette[16] = {
	//	0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00,
	//	0xFF0000FF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF,
	//	0xFFBBBBBB, 0xFFDD7777, 0xFF77DD77, 0xFFDDDD77,
	//	0xFF7777DD, 0xFFDD77EE, 0xFFBBFFFF, 0xFFEEBB00,
	//};

	VideoMC10::VideoMC10() :
		Logger("vidMC10"),
		m_charROM("char", 64*7, emul::MemoryType::ROM)
	{
		// These shouldn't change
		static_assert(H_TOTAL == 48);
		static_assert(V_TOTAL == 262);

		static_assert(H_DISPLAY + H_BLANK + H_SYNC == H_TOTAL);
		static_assert(V_DISPLAY + V_BLANK + V_SYNC == V_TOTAL);
	}

	void VideoMC10::Init(emul::Memory* memory, const char* charROM)
	{
		assert(charROM);
		Video::Init(memory, charROM);

		m_charROM.LoadFromFile(charROM);

		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);
	}

	void VideoMC10::Tick()
	{
		++m_currX;
		++m_currChar;

		if (m_currX == LEFT_BORDER)
		{
			m_currChar = H_DISPLAY * ((m_currY - TOP_BORDER) / 12);
		}
		else if (m_currX == H_TOTAL)
		{
			NewLine();

			++m_currY;
			m_currX = 0;
		}

		if (m_currY == V_TOTAL)
		{
			RenderFrame();
			BeginFrame();

			m_currY = 0;
			m_currX = 0;
		}

		if (IsDisplayArea())
		{
			Draw();
//			DrawBackground(8, 0xFF000080);
		}
		else
		{
			if (!IsHSync())
			{
				//DrawBackground(8, m_borderColor);
				DrawBackground(8, 0xFF008000);
			}
		}
	}

	SDL_Rect VideoMC10::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		border = 16;
		return SDL_Rect {
			(int)(LEFT_BORDER_PX - border),
			(int)(TOP_BORDER - border),
			(int)(H_DISPLAY_PX + (2 * border)),
			(int)(V_DISPLAY + (2 * border))
		};

		//return SDL_Rect {
		//	0, 0, H_TOTAL_PX, V_TOTAL
		//};
	}

	void VideoMC10::Draw()
	{
		// TEMP
		const int currRow = ((m_currY - TOP_BORDER) % 12) - 3;
		const BYTE currChar = m_memory->Read8(0x4000 + m_currChar) & 0b111111;

		//AttributeColors colors = GetAttributeColors(attr);
		const uint32_t fg = 0xFF00FF00;
		const uint32_t bg = 0xFF000000;

		if (currRow < 0 || currRow > 6)
		{
			DrawBackground(8);
		}
		else
		{
			const BYTE pixels = m_charROM.read(currChar * 7 + currRow);

			DrawBackground(2);
			for (int i = 0; i < 5; ++i)
			{
				DrawPixel(GetBit(pixels, i) ? fg : bg);
			}
			DrawBackground(1);
		}
	}
}