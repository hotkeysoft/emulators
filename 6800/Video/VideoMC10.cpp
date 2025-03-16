#include "stdafx.h"
#include "VideoMC10.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	static constexpr uint32_t s_palette[] = {
		0xFF00FF00,  // Green
		0xFFFFFF00,  // Yellow
		0xFF0000FF,  // Blue
		0xFFFF0000,  // Red
		0xFFEEE4B6,  // Buff
		0xFF00FFFF,  // Cyan
		0xFFFF00FF,  // Magenta
		0xFFFFA500,  // Orange
		//
	    0xFF000000,  // Black (graphics mode)
		0xFF004000,  // Dark Green (alphanumeric mode)
		0xFF804000,  // Dark Orange (alphanumeric mode)
	};

	enum class PaletteMC6847 {
		GREEN, YELLOW, BLUE, RED,
		BUFF, CYAN, MAGENTA, ORANGE,
		BLACK,
		DARK_GREEN,
		DARK_ORANGE,
	};

	constexpr uint32_t GetPalette(int index) { return s_palette[index]; }
	constexpr uint32_t GetPalette(PaletteMC6847 index) { return GetPalette((int)index); }

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
		}
		else
		{
			if (!IsHSync())
			{
				DrawBackground(8, m_borderColor);
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
		const BYTE currChar = m_memory->Read8(0x4000 + m_currChar);

		// TEMP, MC10/Alice specific
		SetInverse(GetBit(currChar, 6));
		SetAlphaSemigraph(GetBit(currChar, 7));

		if (!m_alphaGraph)
		{
			m_alphaSemigraph ? DrawSemigraph4(currChar) : DrawAlpha4(currChar);
		}
		else
		{
			LogPrintf(LOG_ERROR, "Graphics mode not implemented");
		}
	}

	void VideoMC10::DrawAlpha4(BYTE currChar)
	{
		const int currRow = ((m_currY - TOP_BORDER) % 12) - 3;

		uint32_t fg = m_css ? GetPalette(PaletteMC6847::ORANGE) : GetPalette(PaletteMC6847::GREEN);
		uint32_t bg = m_css ? GetPalette(PaletteMC6847::DARK_ORANGE) : GetPalette(PaletteMC6847::DARK_GREEN);

		if (m_inverse)
		{
			emul::Swap(fg, bg);
		}

		if (currRow < 0 || currRow > 6)
		{
			DrawBackground(8, bg);
		}
		else
		{
			const BYTE pixels = m_charROM.read((currChar& 0b111111) * 7 + currRow);

			DrawPixel2(bg);

			for (int i = 0; i < 5; ++i)
			{
				DrawPixel(GetBit(pixels, i) ? fg : bg);
			}
			DrawPixel(bg);
		}
	}

	void VideoMC10::DrawSemigraph4(BYTE currChar)
	{
		// 12 pixel vertical block is divided in two 6 pixel 'rows'
		const int currRow = ((m_currY - TOP_BORDER) / 6) % 2;

		// Extract foreground colour (bits 4-6)
		const int colourIndex = (currChar >> 4) & 7;
		const uint32_t fg = GetPalette(colourIndex);
		const uint32_t bg = m_css ? GetPalette(PaletteMC6847::DARK_ORANGE) : GetPalette(PaletteMC6847::DARK_GREEN);

		// Two by two 'pixels':
		// -----------
		// | D3 | D2 | (currRow == 0)
		// -----------
		// | D1 | D0 | (currRow == 1)
		// -----------
		DrawPixel4(GetBit(currChar, 3 - (2 * currRow)) ? fg : bg);
		DrawPixel4(GetBit(currChar, 2 - (2 * currRow)) ? fg : bg);
	}


}