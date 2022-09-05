#include "stdafx.h"
#include "VideoZXSpectrum.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	const uint32_t vSyncHeight = 64; // Lines
	const uint32_t vDisplayed = 192; // Lines

	const uint32_t hSyncWidth = 96; // Ticks
	const uint32_t hDisplayed = 128; // Ticks

	VideoZXSpectrum::VideoZXSpectrum() : Video(), Logger("vidZXSpectrum")
	{

	}

	void VideoZXSpectrum::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(720, 640);
	}

	void VideoZXSpectrum::Tick()
	{
		++m_pixelClock;
		if ((m_pixelClock % 224) == 0)
		{
			NewLine();
			DrawBackground(32, 0x00000000);

			m_hSync = false;
			++m_currY;
			UpdateBaseAddress();
			m_currX = 0;
		}

		if (m_currX == 256)
		{
			m_hSync = true;
		}

		if (m_currY == 32)
		{
			m_vSync = false;
		}
		else if (m_currY == 224)
		{
			m_vSync = true;
		}

		if (IsDisplayArea())
		{
			DrawChar();
		}
		else
		{
			DrawBackground(2, 0x00000000);
		}

		m_currX += 2;
	}

	void VideoZXSpectrum::VSync()
	{
		RenderFrame();
		BeginFrame();

		m_currY = 0;
		UpdateBaseAddress();
		m_pixelClock = 0;
		m_currX = 0;
		//LogPrintf(LOG_DEBUG, "VSync");
	}

	SDL_Rect VideoZXSpectrum::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		return SDL_Rect{ 0, 0, 320, 240 };
	}

	void VideoZXSpectrum::DrawChar()
	{
		int xPos = m_currX >> 3;

		ADDRESS pixAddress = m_pixelAddress;
		pixAddress |= xPos;

		BYTE pixels = m_memory->Read8(pixAddress);
		int offset = (m_currX & 0b111);
		pixels <<= offset;

		if (offset == 0)
		{
			BYTE attr = m_memory->Read8(m_attrAddress + xPos);
			bool bright = attr & 0b01000000;
			const uint32_t* palette = bright ? m_palette1 : m_palette0;
			m_currFG = palette[attr & 7];
			m_currBG = palette[(attr >> 3) & 7];
		}

		DrawPixel((pixels & 0b10000000) ? m_currFG : m_currBG);
		DrawPixel((pixels & 0b01000000) ? m_currFG : m_currBG);
	}
}