#include "stdafx.h"
#include "VideoPET2001.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	const uint32_t vSyncHeight = 64; // Lines
	const uint32_t vDisplayed = 192; // Lines

	const uint32_t hSyncWidth = 96; // Ticks
	const uint32_t hDisplayed = 128; // Ticks

	VideoPET2001::VideoPET2001() :
		Logger("vidPET2001"),
		m_charROM("char", 0x0800, emul::MemoryType::ROM)
	{

	}

	void VideoPET2001::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		assert(charROM);

		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(640, 300);

		m_charROM.LoadFromFile(charROM);
	}

	void VideoPET2001::Tick()
	{
		++m_currX;
		if (m_currX == H_TOTAL)
		{
			NewLine();
			++m_currY;
			m_currX = 0;
			m_currChar = CHAR_BASE + (H_DISPLAY * (m_currY / 8));
			m_currRow = m_currY % 8;
		}

		if (m_currY == V_TOTAL)
		{
			RenderFrame();
			BeginFrame();

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
			DrawBackground(8, 0xFF123456);
		}
	}

	SDL_Rect VideoPET2001::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		// TODO
		return SDL_Rect{ 0, 0, 512, 262 };
	}

	void VideoPET2001::DrawChar()
	{
		BYTE ch = m_memory->Read8(m_currChar);
		bool reverse = GetBit(ch, 7);
		SetBit(ch, 7, 0);

		BYTE pixels = m_charROM.read((ch * 8) + m_currRow);

		for (int i = 0; i < 8; ++i)
		{
			DrawPixel((GetBit(pixels, 7-i) ^ reverse )? 0xFFFFFFFF : 0xFF000000);
		}
	}
}