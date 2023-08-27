#include "stdafx.h"
#include "VideoMac.h"

using emul::GetBit;
using emul::SetBit;

namespace video::mac
{
	static video::mac::EventHandler s_defaultHandler;

	VideoMac::VideoMac() :
		Logger("vidMac"),
		m_events(&s_defaultHandler)
	{
		// These shouldn't change
		static_assert(H_TOTAL_PX == 704);
		static_assert(V_TOTAL == 370);

		static_assert(H_DISPLAY_PX + H_BLANK_PX == H_TOTAL_PX);
		static_assert(V_DISPLAY + V_BLANK == V_TOTAL);
	}

	void VideoMac::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, nullptr);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);
	}

	void VideoMac::Tick()
	{
		// Video draws two pixels for each tick
		m_currX += 2;

		if (m_currX == LEFT_BORDER_PX)
		{
			m_currAddress = (H_DISPLAY_PX / 8) * (m_currY - TOP_BORDER);
		}
		else if (m_currX == RIGHT_BORDER_PX)
		{
			m_events->OnHBlankStart();
		}
		else if (m_currX == H_TOTAL_PX)
		{
			NewLine();

			++m_currY;
			m_currX = 0;

			if (m_currY == BOTTOM_BORDER)
			{
				m_events->OnVBlankStart();
			}
		}

		if (m_currY == V_TOTAL)
		{
			RenderFrame();
			BeginFrame();

			m_currY = 0;
			m_currX = 0;
		}

		// Read a word (and draw it) every 16 pixels
		if (m_currX % 16 == 0)
		{
			m_currAddress += 2;

			if (IsDisplayArea())
			{
				Draw();
			}
			else
			{
				DrawBackground(16, 0xFF000000);
			}
		}
	}

	SDL_Rect VideoMac::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		// TODO:Temp
		border = 16;

		return SDL_Rect {
			(int)(LEFT_BORDER_PX - border),
			(int)(TOP_BORDER - border),
			(int)(H_DISPLAY_PX + (2 * border)),
			(int)(V_DISPLAY + (2 * border))
		};

		//return SDL_Rect{
		//	0, 0, H_TOTAL_PX, V_TOTAL
		//};
	}

	void VideoMac::Draw()
	{
		const emul::DWORD pixels = m_memory->Read16be(m_baseAddress + m_currAddress);

		for (int i = 0; i < 16; ++i)
		{
			DrawPixel(GetBit(pixels, 15 - i) ? 0xFF0F0F0F : 0xCCCCCC);
		}
	}
}