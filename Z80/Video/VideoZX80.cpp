#include "stdafx.h"
#include "VideoZX80.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	VideoZX80::VideoZX80() : Video(), Logger("vidZX80")
	{

	}

	void VideoZX80::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(720, 480);
	}

	void VideoZX80::Tick()
	{
		if (m_hSync && (--m_hSyncCounter == 0))
		{
			m_hSync = false;

			DrawBackground(32, m_background);
		}

		if (IsDisplayArea() && (m_fbCurrY > 0))
		{
			//LogPrintf(LOG_ERROR, "DrawChar, x=%d, y=%d", m_fbCurrX, m_fbCurrY);
			DrawChar();
		}
	}

	SDL_Rect VideoZX80::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		return SDL_Rect{ 0, 0, 320, 320 };
	}

	void VideoZX80::HSync()
	{
		NewLine();
		m_hSync = true;
		m_hSyncCounter = 8;

		++m_rowCounter;
		m_rowCounter &= 7;
	}

	void VideoZX80::VSync(bool set)
	{
		m_rowCounter = 7;
		if (set)
		{
			if (!m_vSync)
			{
				RenderFrame();
				BeginFrame();
			}
		}
		m_vSync = set;
	}

	void VideoZX80::DrawChar()
	{
		for (int i = 7; i >= 0; --i)
		{
			bool set = GetBit(m_currentChar, i) ^ m_invertChar;
			uint32_t color = set ? m_foreground : m_background;
			DrawPixel(color);
		}
	}
	void VideoZX80::LatchCurrentChar(BYTE ch)
	{
		m_invertChar = GetBit(ch, 7);
		SetBit(ch, 7, false);

		ADDRESS charROM = 0x0E00 + (ch * 8) + m_rowCounter;
		m_currentChar = m_memory->Read8(charROM);
	}
}