#include "stdafx.h"
#include "VideoVIC.h"

using emul::GetBit;
using emul::GetLSB;
using emul::SetBit;

namespace video
{
	const uint32_t VideoVIC::s_VICPalette[16] = {
		0xFF000000, 0xFFFFFFFF, 0xFF782922, 0xFF87D6DD,
		0xFFAA5FB6, 0xFF55A049, 0xFF40318D, 0xFFBFCE72,
		0xFFAA7449, 0xFFEAB489, 0xFFB86962, 0xFFC7FFFF,
		0xFFEA9FF6, 0xFF94E089, 0xFF8071CC, 0xFFFFFFB2
	};

	VideoVIC::VideoVIC() : Logger("vidVIC"), IOConnector(0x0F)
	{
	}

	void VideoVIC::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Reset();

		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);

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

	void VideoVIC::Reset()
	{
		Video::Reset();
		m_rawVICRegisters.fill(0);
		UpdateBaseAddress();
		UpdateColors();
		UpdateScreenArea();
		m_currX = 0;
		m_currY = 0;
		m_currRow = 0;
	}

	// CR0
	BYTE VideoVIC::ReadScreenOriginX()
	{
		const BYTE value = GetVICRegister(VICRegister::ORIGIN_X);
		LogPrintf(LOG_DEBUG, "ReadScreenOriginX, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteScreenOriginX(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteScreenOriginX value=%02X", value);
		GetVICRegister(VICRegister::ORIGIN_X) = value;
		UpdateScreenArea();
	}

	// CR1
	BYTE VideoVIC::ReadScreenOriginY()
	{
		const BYTE value = GetVICRegister(VICRegister::ORIGIN_Y);
		LogPrintf(LOG_DEBUG, "ReadScreenOriginY, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteScreenOriginY(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteScreenOriginY, value=%02X", value);
		GetVICRegister(VICRegister::ORIGIN_Y) = value;
		UpdateScreenArea();
	}

	// CR2
	BYTE VideoVIC::ReadColumns()
	{
		const BYTE value = GetVICRegister(VICRegister::COLUMNS);
		LogPrintf(LOG_DEBUG, "ReadColumns, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteColumns(BYTE value)
	{
		GetVICRegister(VICRegister::COLUMNS) = value;
		LogPrintf(LOG_DEBUG, "WriteColumns, value=%02X", value);

		UpdateScreenArea();

		// High bit contains video memory offset
		UpdateBaseAddress();
	}

	// CR3
	BYTE VideoVIC::ReadRows()
	{
		// High bit of ROWS rewister contains hi raster counter
		// Actual count is in m_currY, so we need to update the registers before reading
		UpdateVICRaster();
		const BYTE value = GetVICRegister(VICRegister::ROWS);
		LogPrintf(LOG_DEBUG, "ReadRows, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteRows(BYTE value)
	{
		GetVICRegister(VICRegister::ROWS) = value;
		LogPrintf(LOG_DEBUG, "WriteRows, value=%02X", value);

		// D7 is raster.D8
		// D6-D1 is number of rows
		// D0: 0 for Single char height, 1 for double char height
		LogPrintf(LOG_INFO, "WriteRows: Raster.D8[%d] Rows:%d, Double:%d",
			GetVICRaster8(),
			GetVICRows(),
			GetVICDoubleY());

		// Update raster.D8 to set value (actual count is done in m_currY)
		SetBit(m_currY, 8, GetVICRaster8());

		UpdateScreenArea();
	}

	// CR4
	BYTE VideoVIC::ReadRaster()
	{
		// Actual count is in m_currY, so we need to update the registers before reading
		UpdateVICRaster();
		const BYTE value = GetVICRegister(VICRegister::RASTER);
		LogPrintf(LOG_DEBUG, "ReadRaster, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteRaster(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteRaster, value=%02X", value);
		GetVICRegister(VICRegister::RASTER) = value;
		m_currY = GetVICRaster();
	}

	// CR5
	BYTE VideoVIC::ReadBaseAddress()
	{
		const BYTE value = GetVICRegister(VICRegister::BASE_ADDRESS);
		LogPrintf(LOG_DEBUG, "ReadBaseAddress, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteBaseAddress(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteBaseAddress, value=%02X", value);
		GetVICRegister(VICRegister::BASE_ADDRESS) = value;
		UpdateBaseAddress();
	}

	// CR6
	BYTE VideoVIC::ReadLightPenX()
	{
		const BYTE value = GetVICRegister(VICRegister::LIGHTPEN_X);
		LogPrintf(LOG_DEBUG, "ReadLightPenX, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteLightPenX(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteLightPenX, value=%02X", value);
		GetVICRegister(VICRegister::LIGHTPEN_X) = value;
	}

	// CR7
	BYTE VideoVIC::ReadLightPenY()
	{
		const BYTE value = GetVICRegister(VICRegister::LIGHTPEN_Y);
		LogPrintf(LOG_DEBUG, "ReadLightPenY, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteLightPenY(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteLightPenY, value=%02X", value);
		GetVICRegister(VICRegister::LIGHTPEN_Y) = value;
	}

	// CR8
	BYTE VideoVIC::ReadPotX()
	{
		const BYTE value = GetVICRegister(VICRegister::POT_X);
		LogPrintf(LOG_DEBUG, "ReadPotX, return=%02X", value);
		return value;
	}
	void VideoVIC::WritePotX(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WritePotX, value=%02X, not implemented", value);
	}

	// CR9
	BYTE VideoVIC::ReadPotY()
	{
		const BYTE value = GetVICRegister(VICRegister::POT_Y);
		LogPrintf(LOG_DEBUG, "ReadPotY, return=%02X", value);
		return value;
	}
	void VideoVIC::WritePotY(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WritePotY, value=%02X, not implemented", value);
	}

	// CRA
	BYTE VideoVIC::ReadAudioFreq1()
	{
		const BYTE value = GetVICRegister(VICRegister::AUDIO_FREQ1);
		LogPrintf(LOG_DEBUG, "ReadAudioFreq1, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteAudioFreq1(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq1, value=%02X, not implemented", value);
	}

	// CRB
	BYTE VideoVIC::ReadAudioFreq2()
	{
		const BYTE value = GetVICRegister(VICRegister::AUDIO_FREQ2);
		LogPrintf(LOG_DEBUG, "ReadAudioFreq2, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteAudioFreq2(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq2, value=%02X, not implemented", value);
	}

	// CRC
	BYTE VideoVIC::ReadAudioFreq3()
	{
		const BYTE value = GetVICRegister(VICRegister::AUDIO_FREQ3);
		LogPrintf(LOG_DEBUG, "ReadAudioFreq3, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteAudioFreq3(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq3, value=%02X, not implemented", value);
	}

	// CRD
	BYTE VideoVIC::ReadAudioFreq4()
	{
		const BYTE value = GetVICRegister(VICRegister::AUDIO_FREQ4);
		LogPrintf(LOG_DEBUG, "ReadAudioFreq4, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteAudioFreq4(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioFreq4, value=%02X, not implemented", value);
	}

	// CRE
	BYTE VideoVIC::ReadAudioAmplitude()
	{
		const BYTE value = GetVICRegister(VICRegister::AUDIO_AMPLITUDE);
		LogPrintf(LOG_DEBUG, "ReadAudioAmplitude, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteAudioAmplitude(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteAudioAmplitude, value=%02X, not implemented", value);
	}

	// CRF
	BYTE VideoVIC::ReadColorControl()
	{
		const BYTE value = GetVICRegister(VICRegister::COLOR_CONTROL);
		LogPrintf(LOG_DEBUG, "ReadColorControl, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteColorControl(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteColorControl, value=%02X", value);
		GetVICRegister(VICRegister::COLOR_CONTROL) = value;
		UpdateColors();
	}

	void VideoVIC::Tick()
	{
		if (!H_DISPLAY || !V_DISPLAY)
			return;

		++m_currX;

		if (m_currX == RIGHT_BORDER)
		{
			NewLine();
			if (LEFT_BORDER_ODD)
			{
				DrawBackground(4, m_borderColor);
			}
		}
		else if (m_currX == H_TOTAL)
		{
			++m_currY;
			m_currX = 0;
			m_currChar = m_matrixBaseAddress + ((H_DISPLAY/2) * (m_currY / CHAR_HEIGHT));
			m_currColor = m_colorBaseAddress + ((H_DISPLAY/2) * (m_currY / CHAR_HEIGHT));
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
			m_currChar = m_matrixBaseAddress;
			m_currColor = m_colorBaseAddress;
		}

		// Tick every half character (4 pixels)
		// Draw 8 characters, but only on odd or even ticks (depending on the border)
		if (GetLSB(m_currX) ^ LEFT_BORDER_ODD)
		{
			if (IsDisplayArea())
			{
				DrawChar();
				++m_currChar;
				++m_currColor;
			}
			else
			{
				DrawBackground(8, m_borderColor);
			}
		}
	}

	SDL_Rect VideoVIC::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		// TODO, will always center
		const uint32_t tempBorder = 16;

		return SDL_Rect{
			int(LEFT_BORDER_PX - tempBorder),
			int(TOP_BORDER - tempBorder),
			int(H_DISPLAY_PX + (2 * tempBorder)),
			int(V_DISPLAY + (2 * tempBorder))
		};
	}

	// On VIC-20, A13 is mapped to /BLK4 (0x8000)
	void VideoVIC::AdjustA13(ADDRESS& addr) const
	{
		bool A13 = GetBit(addr, 13);
		SetBit(addr, 13, false);
		SetBit(addr, 15, !A13);
	}

	void VideoVIC::UpdateBaseAddress()
	{
		LogPrintf(LOG_INFO, "UpdateBaseAddress");
		BYTE reg = GetVICRegister(VICRegister::BASE_ADDRESS);

		// Bits 3-0: start address of character cell space (A13-A10)
		m_charBaseAddress = (reg & 0x0F) << 10;
		AdjustA13(m_charBaseAddress);
		LogPrintf(LOG_INFO, "  CHAR BASE:   %04X", m_charBaseAddress);

		// Hi bit of CR2 contains an offset for the matrix and color base addresses
		bool offset = GetVICMemOffset();

		// Bits 7-4: start address of char matrix space (A13-A10)
		m_matrixBaseAddress = (reg & 0xF0) << (10 - 4);
		AdjustA13(m_matrixBaseAddress);
		// offset in A9
		SetBit(m_matrixBaseAddress, 9, offset);
		LogPrintf(LOG_INFO, "  MATRIX BASE: %04X", m_matrixBaseAddress);

		// Color RAM @ 0x9400
		m_colorBaseAddress = 0x9400;
		SetBit(m_colorBaseAddress, 9, offset);
		LogPrintf(LOG_INFO, "  COLOR BASE:  %04X", m_colorBaseAddress);
	}

	void VideoVIC::UpdateScreenArea()
	{
		LogPrintf(LOG_INFO, "UpdateScreenArea");

		// Discard hi bit, used for base address offset
		BYTE columns = GetVICColumns();
		BYTE rows = GetVICRows();

		// Unit: half-characters (4 pixels)
		BYTE originX = GetVICOriginX();

		// Unit: 2 pixels
		BYTE originY = GetVICOriginY() * 2;

		H_DISPLAY = columns * 2; // In half-chars
		H_DISPLAY_PX = H_DISPLAY * HALF_CHAR_WIDTH;

		CHAR_HEIGHT = GetVICDoubleY() ? 16 : 8;
		V_DISPLAY = rows * CHAR_HEIGHT;

		LEFT_BORDER = originX;
		LEFT_BORDER_ODD = GetLSB(originX);
		LEFT_BORDER_PX = LEFT_BORDER * HALF_CHAR_WIDTH;
		TOP_BORDER = originY;

		RIGHT_BORDER = (H_TOTAL - LEFT_BORDER);
		BOTTOM_BORDER = (V_TOTAL - TOP_BORDER);

		LogPrintf(LOG_INFO, "  H_DISPLAY: %d half characters (%d pixels)", H_DISPLAY, H_DISPLAY_PX);
		LogPrintf(LOG_INFO, "  H_TOTAL:   %d half characters (%d pixels)", H_TOTAL, H_TOTAL_PX);
		LogPrintf(LOG_INFO, "  L_BORDER:  %d half characters (%d pixels)", LEFT_BORDER, LEFT_BORDER_PX);
		LogPrintf(LOG_INFO, "  R_BORDER:  %d half characters (%d pixels)", RIGHT_BORDER, RIGHT_BORDER * CHAR_WIDTH);

		LogPrintf(LOG_INFO, "  CHAR_H:    %d pixels", CHAR_HEIGHT);
		LogPrintf(LOG_INFO, "  V_DISPLAY: %d characters (%d pixels)", rows, V_DISPLAY);
		LogPrintf(LOG_INFO, "  V_TOTAL:   %d pixels", V_TOTAL);
		LogPrintf(LOG_INFO, "  T_BORDER:  %d pixels", TOP_BORDER);
		LogPrintf(LOG_INFO, "  B_BORDER:  %d pixels", BOTTOM_BORDER);

		if (GetVICInterlace())
		{
			LogPrintf(LOG_WARNING, "UpdateScreenArea: Interlace not supported");
		}
	}

	void VideoVIC::UpdateVICRaster()
	{
		GetVICRegister(VICRegister::RASTER) = m_currY;
		SetVICRaster8(GetBit(m_currY, 8));
	}

	void VideoVIC::UpdateColors()
	{
		LogPrintf(LOG_INFO, "UpdateColors");
		BYTE bg = GetVICBackgroundColor();
		BYTE border = GetVICBorderColor();

		m_backgroundColor = GetVICColor(bg);
		m_borderColor = GetVICColor(border);
		m_invertColors = GetVICInvertColors();

		LogPrintf(LOG_INFO, "  BACKGROUND: %d", bg);
		LogPrintf(LOG_INFO, "  BORDER:     %d", border);
		LogPrintf(LOG_INFO, "  INVERT:     %d", m_invertColors);
	}

	void VideoVIC::DrawChar()
	{
		BYTE ch = m_memory->Read8(m_currChar);
		BYTE color = m_memory->Read8(m_currColor);
		bool multiColor = GetBit(color, 3);
		uint32_t fgColor = GetVICColor(color & 7);

		const WORD charAddress = m_charBaseAddress + (ch * CHAR_HEIGHT) + m_currRow;
		const BYTE pixels = m_memory->Read8(charAddress);

		//if (multiColor)
		//{
		//	for (int i = 0; i < 4; ++i)
		//	{

		//	}
		//}
		//else
		{
			for (int i = 0; i < CHAR_WIDTH; ++i)
			{
				DrawPixel((GetBit(pixels, 7 - i) ^ m_invertColors) ? fgColor : m_backgroundColor);
			}
		}
	}

	// emul::Serializable
	void VideoVIC::Serialize(json& to)
	{
		Video::Serialize(to);

		to["registers"] = m_rawVICRegisters;

		to["currChar"] = m_currChar;
		to["currColor"] = m_currColor;
		to["currX"] = m_currX;
		to["currY"] = m_currY;
		to["currRow"] = m_currRow;
	}
	void VideoVIC::Deserialize(const json& from)
	{
		Video::Deserialize(from);

		m_rawVICRegisters = from["registers"];
		UpdateBaseAddress();
		UpdateScreenArea();
		UpdateColors();

		m_currChar = from["currChar"];
		m_currChar = from["currColor"];
		m_currX = from["currX"];
		m_currY = from["currY"];
		m_currRow = from["currRow"];
	}
}
