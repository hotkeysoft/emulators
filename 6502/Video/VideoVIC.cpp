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

	void VideoVIC::Reset()
	{
		Video::Reset();
		m_rawVICRegisters.fill(0);
		UpdateBaseAddress();
		m_currX = 0;
		m_currY = 0;
		m_currRow = 0;
	}

	// CR0
	BYTE VideoVIC::ReadScreenOriginX()
	{
		BYTE value = ReadVICRegister(VICRegister::ORIGIN_X);
		LogPrintf(LOG_DEBUG, "ReadScreenOriginX, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteScreenOriginX(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteScreenOriginX value=%02X, not implemented", value);
	}

	// CR1
	BYTE VideoVIC::ReadScreenOriginY()
	{
		BYTE value = ReadVICRegister(VICRegister::ORIGIN_Y);
		LogPrintf(LOG_DEBUG, "ReadScreenOriginY, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteScreenOriginY(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteScreenOriginY, value=%02X, not implemented", value);
	}

	// CR2
	BYTE VideoVIC::ReadColumns()
	{
		BYTE value = ReadVICRegister(VICRegister::COLUMNS);
		LogPrintf(LOG_DEBUG, "ReadColumns, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteColumns(BYTE value)
	{
		WriteVICRegister(VICRegister::COLUMNS, value);
		LogPrintf(LOG_WARNING, "WriteColumns, value=%02X", value);

		// High bit contains video memory offset
		UpdateBaseAddress();
	}

	// CR3
	BYTE VideoVIC::ReadRows()
	{
		BYTE value = ReadVICRegister(VICRegister::ROWS);
		LogPrintf(LOG_DEBUG, "ReadRows, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteRows(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteRows, value=%02X, not implemented", value);
	}

	// CR4
	BYTE VideoVIC::ReadRaster()
	{
		BYTE value = ReadVICRegister(VICRegister::RASTER);
		LogPrintf(LOG_DEBUG, "ReadRaster, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteRaster(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteRaster, value=%02X, not implemented", value);
	}

	// CR5
	BYTE VideoVIC::ReadBaseAddress()
	{
		BYTE value = ReadVICRegister(VICRegister::BASE_ADDRESS);
		LogPrintf(LOG_DEBUG, "ReadBaseAddress, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteBaseAddress(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteBaseAddress, value=%02X", value);
		WriteVICRegister(VICRegister::BASE_ADDRESS, value);
		UpdateBaseAddress();
	}

	// CR6
	BYTE VideoVIC::ReadLightPenX()
	{
		BYTE value = ReadVICRegister(VICRegister::LIGHTPEN_X);
		LogPrintf(LOG_DEBUG, "ReadLightPenX, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteLightPenX(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteLightPenX, value=%02X, not implemented", value);
	}

	// CR7
	BYTE VideoVIC::ReadLightPenY()
	{
		BYTE value = ReadVICRegister(VICRegister::LIGHTPEN_Y);
		LogPrintf(LOG_DEBUG, "ReadLightPenY, return=%02X", value);
		return value;
	}
	void VideoVIC::WriteLightPenY(BYTE value)
	{
		LogPrintf(LOG_WARNING, "WriteLightPenY, value=%02X, not implemented", value);
	}

	// CR8
	BYTE VideoVIC::ReadPotX()
	{
		BYTE value = ReadVICRegister(VICRegister::POT_X);
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
		BYTE value = ReadVICRegister(VICRegister::POT_Y);
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
		BYTE value = ReadVICRegister(VICRegister::AUDIO_FREQ1);
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
		BYTE value = ReadVICRegister(VICRegister::AUDIO_FREQ2);
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
		BYTE value = ReadVICRegister(VICRegister::AUDIO_FREQ3);
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
		BYTE value = ReadVICRegister(VICRegister::AUDIO_FREQ4);
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
		BYTE value = ReadVICRegister(VICRegister::AUDIO_AMPLITUDE);
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
		BYTE value = ReadVICRegister(VICRegister::COLOR_CONTROL);
		LogPrintf(LOG_DEBUG, "ReadColorControl, return=%02X", value);
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
			m_currChar = m_matrixBaseAddress + (H_DISPLAY * (m_currY / CHAR_HEIGHT));
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
		BYTE reg = ReadVICRegister(VICRegister::BASE_ADDRESS);

		// Bits 3-0: start address of character cell space (A13-A10)
		m_charBaseAddress = (reg & 0x0F) << 10;
		AdjustA13(m_charBaseAddress);
		LogPrintf(LOG_WARNING, "CHAR BASE:   %04X", m_charBaseAddress);

		// Hi bit of CR2 contains an offset for the matrix and color base addresses
		bool offset = emul::GetMSB(ReadVICRegister(VICRegister::COLUMNS));

		// Bits 7-4: start address of char matrix space (A13-A10)
		m_matrixBaseAddress = (reg & 0xF0) << (10 - 4);
		AdjustA13(m_matrixBaseAddress);
		// offset in A9
		SetBit(m_matrixBaseAddress, 9, offset);
		LogPrintf(LOG_WARNING, "MATRIX BASE: %04X", m_matrixBaseAddress);

		// Color RAM @ 0x9400
		m_colorBaseAddress = 0x9400;
		SetBit(m_colorBaseAddress, 9, offset);
		LogPrintf(LOG_WARNING, "COLOR BASE:  %04X", m_colorBaseAddress);
	}

	void VideoVIC::DrawChar()
	{
		BYTE ch = m_memory->Read8(m_currChar);
		const bool reverse = GetBit(ch, 7);
		SetBit(ch, 7, 0);

		const WORD charROMAddress = m_charBaseAddress + (ch * CHAR_HEIGHT) + m_currRow;
		const BYTE pixels = m_memory->Read8(charROMAddress);

		for (int i = 0; i < CHAR_WIDTH; ++i)
		{
			DrawPixel((GetBit(pixels, 7-i) ^ reverse )? m_fgColor : m_bgColor);
		}
	}

	// emul::Serializable
	void VideoVIC::Serialize(json& to)
	{
		Video::Serialize(to);

		to["registers"] = m_rawVICRegisters;

		to["currChar"] = m_currChar;
		to["bg"] = m_bgColor;
		to["fg"] = m_fgColor;
		to["currX"] = m_currX;
		to["currY"] = m_currY;
		to["currRow"] = m_currRow;
	}
	void VideoVIC::Deserialize(const json& from)
	{
		Video::Deserialize(from);

		m_rawVICRegisters = from["registers"];
		UpdateBaseAddress();

		m_currChar = from["currChar"];
		m_bgColor = from["bg"];
		m_fgColor = from["fg"];
		m_currX = from["currX"];
		m_currY = from["currY"];
		m_currRow = from["currRow"];
	}
}
