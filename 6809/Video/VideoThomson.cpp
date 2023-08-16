#include "stdafx.h"
#include "VideoThomson.h"

using emul::GetBit;
using emul::SetBit;
using emul::Thomson::ModelToString;
using Model = emul::Thomson::Model;

namespace video
{
	static const uint32_t s_palette[16] = {
		0xFF000000, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00,
		0xFF0000FF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF,
		0xFFBBBBBB, 0xFFDD7777, 0xFF77DD77, 0xFFDDDD77,
		0xFF7777DD, 0xFFDD77EE, 0xFFBBFFFF, 0xFFEEBB00,
	};

	static AttributeColors GetAttributeColorsMO5(BYTE attr)
	{
		return {
			s_palette[(attr >> 4)],
			s_palette[(attr & 15)]
		};
	}

	static AttributeColors GetAttributeColorsTO7(BYTE attr)
	{
		return {
			s_palette[(attr >> 3) & 7],
			s_palette[(attr & 7)]
		};
	}

	VideoThomson::VideoThomson() :
		Logger("vidThomson"),
		IOConnector(0x03) // Addresses 0-3 are decoded by device
	{
		// These shouldn't change
		static_assert(H_TOTAL == 64);
		static_assert(V_TOTAL == 312);

		static_assert(H_DISPLAY + H_BLANK + H_SYNC == H_TOTAL);
		static_assert(V_DISPLAY + V_BLANK + V_SYNC == V_TOTAL);
	}

	void VideoThomson::Init(Model model, emul::MemoryBlock* pixelRAM, emul::MemoryBlock* attributeRAM)
	{
		assert(pixelRAM);
		assert(attributeRAM);
		m_pixelRAM = pixelRAM;
		m_attributeRAM = attributeRAM;

		Video::Init(nullptr, nullptr);
		InitFrameBuffer(H_TOTAL_PX, V_TOTAL);

		IOConnector::Connect(0, static_cast<IOConnector::READFunction>(&VideoThomson::ReadSCRCLKhigh));
		IOConnector::Connect(1, static_cast<IOConnector::READFunction>(&VideoThomson::ReadSCRCLKlow));
		IOConnector::Connect(2, static_cast<IOConnector::READFunction>(&VideoThomson::ReadLineCounter));
		IOConnector::Connect(3, static_cast<IOConnector::READFunction>(&VideoThomson::ReadINITN));

		switch (model)
		{
		case Model::MO5: GetAttributeColors = GetAttributeColorsMO5; break;
		case Model::TO7: GetAttributeColors = GetAttributeColorsTO7; break;
		default:
			LogPrintf(LOG_ERROR, "Unknown model [%s], using MO5", ModelToString(model).c_str());
			GetAttributeColors = GetAttributeColorsMO5;
			break;
		}
	}

	void VideoThomson::Tick()
	{
		++m_currX;
		++m_currChar;

		if (m_currX == LEFT_BORDER)
		{
			m_currChar = H_DISPLAY * (m_currY - TOP_BORDER);
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

		if (m_lightPenValid &&
			(m_currX == m_lightPenX) &&
			(m_currY >= m_lightPenY - 1) &&
			(m_currY <= m_lightPenY + 1))
		{
			LatchLightpenCounters();
			m_lightPen = true; // Get pixel color
		}
		else
		{
			m_lightPen = false;
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

	SDL_Rect VideoThomson::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
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

	void VideoThomson::SetBorderColor(BYTE borderRGBP)
	{
		m_borderColor = s_palette[borderRGBP & 15];
	}

	BYTE VideoThomson::ReadSCRCLKhigh()
	{
		return emul::GetHByte(m_scrClk);
	}

	BYTE VideoThomson::ReadSCRCLKlow()
	{
		return emul::GetLByte(m_scrClk);
	}

	BYTE VideoThomson::ReadLineCounter()
	{
		BYTE value = 0xFF;

		SetBit(value, 6, m_INILN);
		SetBit(value, 7, m_LT3);

		return value;
	}

	// bit 7: INITN: 0 when the GA is drawing horizontal borders
	//        above and below the screen (during vblank)
	BYTE VideoThomson::ReadINITN()
	{
		return IsVBlank() ? 0b01111111 : 0b11111111;
	}

	// LT3  : No documention found on this except a mention about
	//        distinguishing between left and right borders
	// INILN: 0 when the GA is drawing vertical borders
	//        on the left and right of the screen (during hblank)
	void VideoThomson::LatchLightpenCounters()
	{
		m_scrClk = (m_currChar * CHAR_WIDTH) + m_lightPenXOffsetPx;

		m_INILN = IsHSync();
		m_LT3 = m_currX >= RIGHT_BORDER;
	}

	void VideoThomson::SetLightPenPos(int x, int y)
	{
		x += LEFT_BORDER_PX + LIGHTPEN_OFFSET;
		y += TOP_BORDER;

		x = std::clamp(x, (int)LEFT_BORDER_PX, (int)(RIGHT_BORDER_PX + LIGHTPEN_OFFSET));

		if ((y < V_LIGHTPEN_MARGIN) ||
			(y >= V_TOTAL - V_LIGHTPEN_MARGIN))
		{
			m_lightPenX = 0;
			m_lightPenY = 0;
			m_lightPenValid = false;
			LogPrintf(LOG_DEBUG, "LightPen Pos: [out of bounds]");
		}
		else
		{
			m_lightPenX = x / CHAR_WIDTH;
			m_lightPenXOffsetPx = x % CHAR_WIDTH;
			m_lightPenY = y;
			m_lightPenValid = true;
			LogPrintf(LOG_DEBUG, "LightPen Pos: [(%d*8) + %d, %d]",
				m_lightPenX,
				m_lightPenXOffsetPx,
				m_lightPenY);
		}
	}

	void VideoThomson::Draw()
	{
		const BYTE pixels = m_pixelRAM->read(m_currChar);
		const BYTE attr = m_attributeRAM->read(m_currChar);

		AttributeColors colors = GetAttributeColors(attr);
		const uint32_t fg = std::get<0>(colors);
		const uint32_t bg = std::get<1>(colors);

		for (int i = 0; i < 8; ++i)
		{
			DrawPixel(GetBit(pixels, 7 - i) ? fg : bg);
		}
	}
}