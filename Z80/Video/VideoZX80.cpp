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

			uint32_t color = 0xFF000000;
			color |= (m_rowCounter << 13);
			DrawBackground(32, color);
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

	void VideoZX80::LatchCurrentChar(BYTE ch)
	{
		if (ch == 0)
		{
			return;
		}

		bool invert = GetBit(ch, 7);
		SetBit(ch, 7, false);

		ADDRESS charROM = 0x0E00 + (ch * 8) + m_rowCounter;
		// TODO: This is a big shortcut
		m_currentChar = m_memory->Read8(charROM);

		for (int i = 7; i >= 0; --i)
		{
			bool set = GetBit(m_currentChar, i) ^ invert;
			uint32_t color = set ? 0xFFFFFFFF : 0;
			DrawPixel(color);
		}
	}
}