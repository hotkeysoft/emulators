#include "stdafx.h"
#include "VideoVIC.h"

using emul::GetBit;
using emul::SetBit;

namespace video
{
	VideoVIC::VideoVIC() : Logger("vidVIC"), IOConnector(0x0F)
	{
	}

	void VideoVIC::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Reset();

		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);

		m_bgColor = GetMonitorPalette()[0];
		m_fgColor = GetMonitorPalette()[15];

		IOConnector::Connect(0x0, static_cast<IOConnector::READFunction>(&VideoVIC::ReadScreenOriginX));
		IOConnector::Connect(0x0, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteScreenOriginX));

		IOConnector::Connect(0x1, static_cast<IOConnector::READFunction>(&VideoVIC::ReadScreenOriginY));
		IOConnector::Connect(0x1, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteScreenOriginY));

		IOConnector::Connect(0x2, static_cast<IOConnector::READFunction>(&VideoVIC::ReadColumns));
		IOConnector::Connect(0x2, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteColumns));

		IOConnector::Connect(0x3, static_cast<IOConnector::READFunction>(&VideoVIC::ReadRows));
		IOConnector::Connect(0x3, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteRows));

		IOConnector::Connect(0x4, static_cast<IOConnector::READFunction>(&VideoVIC::ReadRaster));
		IOConnector::Connect(0x4, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteRaster));

		IOConnector::Connect(0x5, static_cast<IOConnector::READFunction>(&VideoVIC::ReadBaseAddress));
		IOConnector::Connect(0x5, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteBaseAddress));

		IOConnector::Connect(0x6, static_cast<IOConnector::READFunction>(&VideoVIC::ReadLightPenX));
		IOConnector::Connect(0x6, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteLightPenX));

		IOConnector::Connect(0x7, static_cast<IOConnector::READFunction>(&VideoVIC::ReadLightPenY));
		IOConnector::Connect(0x7, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteLightPenY));

		IOConnector::Connect(0x8, static_cast<IOConnector::READFunction>(&VideoVIC::ReadPotX));
		IOConnector::Connect(0x8, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WritePotX));

		IOConnector::Connect(0x9, static_cast<IOConnector::READFunction>(&VideoVIC::ReadPotY));
		IOConnector::Connect(0x9, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WritePotY));

		IOConnector::Connect(0xA, static_cast<IOConnector::READFunction>(&VideoVIC::ReadAudioFreq1));
		IOConnector::Connect(0xA, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteAudioFreq1));

		IOConnector::Connect(0xB, static_cast<IOConnector::READFunction>(&VideoVIC::ReadAudioFreq2));
		IOConnector::Connect(0xB, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteAudioFreq2));

		IOConnector::Connect(0xC, static_cast<IOConnector::READFunction>(&VideoVIC::ReadAudioFreq3));
		IOConnector::Connect(0xC, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteAudioFreq3));

		IOConnector::Connect(0xD, static_cast<IOConnector::READFunction>(&VideoVIC::ReadAudioFreq4));
		IOConnector::Connect(0xD, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteAudioFreq4));

		IOConnector::Connect(0xE, static_cast<IOConnector::READFunction>(&VideoVIC::ReadAudioAmplitude));
		IOConnector::Connect(0xE, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteAudioAmplitude));

		IOConnector::Connect(0xF, static_cast<IOConnector::READFunction>(&VideoVIC::ReadColorControl));
		IOConnector::Connect(0xF, static_cast<IOConnector::WRITEFunction>(&VideoVIC::WriteColorControl));
	}

	// CR0
	BYTE VideoVIC::ReadScreenOriginX()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadScreenOriginX, not implemented");
		return value;
	}
	void VideoVIC::WriteScreenOriginX(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteScreenOriginX value=%02X, not implemented", value);
	}

	// CR1
	BYTE VideoVIC::ReadScreenOriginY()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadScreenOriginY, not implemented");
		return value;
	}
	void VideoVIC::WriteScreenOriginY(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteScreenOriginY, value=%02X, not implemented", value);
	}

	// CR2
	BYTE VideoVIC::ReadColumns()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadColumns, not implemented");
		return value;
	}
	void VideoVIC::WriteColumns(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteColumns, value=%02X, not implemented", value);
	}

	// CR3
	BYTE VideoVIC::ReadRows()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadRows, not implemented");
		return value;
	}
	void VideoVIC::WriteRows(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteRows, value=%02X, not implemented", value);
	}

	// CR4
	BYTE VideoVIC::ReadRaster()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadRaster, not implemented");
		return value;
	}
	void VideoVIC::WriteRaster(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteRaster, value=%02X, not implemented", value);
	}

	// CR5
	BYTE VideoVIC::ReadBaseAddress()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadBaseAddress, not implemented");
		return value;
	}
	void VideoVIC::WriteBaseAddress(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteBaseAddress, value=%02X, not implemented", value);
	}

	// CR6
	BYTE VideoVIC::ReadLightPenX()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadLightPenX, not implemented");
		return value;
	}
	void VideoVIC::WriteLightPenX(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteLightPenX, value=%02X, not implemented", value);
	}

	// CR7
	BYTE VideoVIC::ReadLightPenY()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadLightPenY, not implemented");
		return value;
	}
	void VideoVIC::WriteLightPenY(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteLightPenY, value=%02X, not implemented", value);
	}

	// CR8
	BYTE VideoVIC::ReadPotX()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadPotX, not implemented");
		return value;
	}
	void VideoVIC::WritePotX(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WritePotX, value=%02X, not implemented", value);
	}

	// CR9
	BYTE VideoVIC::ReadPotY()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadPotY, not implemented");
		return value;
	}
	void VideoVIC::WritePotY(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WritePotY, value=%02X, not implemented", value);
	}

	// CRA
	BYTE VideoVIC::ReadAudioFreq1()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadAudioFreq1, not implemented");
		return value;
	}
	void VideoVIC::WriteAudioFreq1(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq1, value=%02X, not implemented", value);
	}

	// CRB
	BYTE VideoVIC::ReadAudioFreq2()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadAudioFreq2, not implemented");
		return value;
	}
	void VideoVIC::WriteAudioFreq2(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq2, value=%02X, not implemented", value);
	}

	// CRC
	BYTE VideoVIC::ReadAudioFreq3()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadAudioFreq3, not implemented");
		return value;
	}
	void VideoVIC::WriteAudioFreq3(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq3, value=%02X, not implemented", value);
	}

	// CRD
	BYTE VideoVIC::ReadAudioFreq4()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadAudioFreq4, not implemented");
		return value;
	}
	void VideoVIC::WriteAudioFreq4(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq4, value=%02X, not implemented", value);
	}

	// CRE
	BYTE VideoVIC::ReadAudioAmplitude()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadAudioAmplitude, not implemented");
		return value;
	}
	void VideoVIC::WriteAudioAmplitude(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioAmplitude, value=%02X, not implemented", value);
	}

	// CRF
	BYTE VideoVIC::ReadColorControl()
	{
		BYTE value = 0xFF;
		LogPrintf(LOG_WARNING, "ReadColorControl, not implemented");
		return value;
	}
	void VideoVIC::WriteColorControl(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteColorControl, value=%02X, not implemented", value);
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