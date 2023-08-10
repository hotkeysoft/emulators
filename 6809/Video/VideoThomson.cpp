#include "stdafx.h"
#include "VideoThomson.h"
//#include "../Hardware/Device6522PET.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{

	VideoThomson::VideoThomson() : Logger("vidThomson")
	{
		// These shouldn't change
		static_assert(H_TOTAL == 64);
		static_assert(V_TOTAL == 312);

		static_assert(H_DISPLAY + H_BLANK + H_SYNC == H_TOTAL);
		static_assert(V_DISPLAY + V_BLANK + V_SYNC == V_TOTAL);
	}

	void VideoThomson::Init(emul::MemoryBlock* pixelRAM, emul::MemoryBlock* attributeRAM)
	{
		assert(pixelRAM);
		assert(attrRAM);
		m_pixelRAM = pixelRAM;
		m_attributeRAM = attributeRAM;

		Video::Init(nullptr, nullptr);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);
	}

	void VideoThomson::Tick()
	{
		++m_currX;

		if (m_currX == LEFT_BORDER)
		{
			m_currChar = H_DISPLAY * (m_currY - TOP_BORDER);
		}
		else if (m_currX == H_TOTAL)
		{
			NewLine();
			LogPrintf(LOG_WARNING, "[%zu] line", emul::g_ticks);

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
			++m_currChar;
		}
		else
		{
			if (!IsHSync())
			{
				DrawBackground(8, m_borderColor);
			}
		}
	}

	SDL_Rect VideoThomson::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		// TODO
		const uint32_t tempBorder = 16;
		return SDL_Rect{
			LEFT_BORDER_PX - tempBorder,
			TOP_BORDER - tempBorder,
			H_DISPLAY_PX + (2 * tempBorder),
			V_DISPLAY + (2 * tempBorder)
		};

		//return SDL_Rect{
		//	0, 0, H_TOTAL_PX, V_TOTAL
		//};
	}

	void VideoThomson::Draw()
	{
		const BYTE pixels = m_pixelRAM->read(m_currChar);
		const BYTE attr = m_attributeRAM->read(m_currChar);

		const uint32_t fg = m_palette[(attr >> 4)];
		const uint32_t bg = m_palette[(attr & 15)];

		for (int i = 0; i < 8; ++i)
		{
			DrawPixel(GetBit(pixels, 7 - i) ? fg : bg);
		}
	}
}