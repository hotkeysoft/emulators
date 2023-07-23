#include "stdafx.h"
#include "VideoCPC464.h"

using emul::GetBit;
using emul::SetBit;

using crtc_6845::CRTCConfig;
using crtc_6845::CRTCData;

namespace video::cpc464
{
	static video::cpc464::EventHandler s_defaultHandler;

	VideoCPC464::VideoCPC464(emul::MemoryBlock* ram) :
		Video(),
		Logger("vidCPC464"),
		m_ram(ram),
		m_events(&s_defaultHandler)
	{
		assert(ram);
		Reset();
	}

	void VideoCPC464::Reset()
	{
		Video::Reset();
		m_crtc.Reset();

		m_baseAddress = 0;
		m_currPen = 0;
		m_romHighEnabled = false;
		m_romLowEnabled = true;
		m_interruptCounter = 0;
		m_isInterrupt = false;
		m_mode = 0;

		try
		{
			UpdateMode();
		}
		catch (std::exception ex)
		{
			// Normal here on first reset because modes are not initialized
		}
	}

	void VideoCPC464::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoCPC464::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(1024, 320);

		Connect("01xxxxxx", static_cast<PortConnector::OUTFunction>(&VideoCPC464::Write));

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		AddMode(m_modes[0], (DrawFunc)&VideoCPC464::Draw160x200x16, (AddressFunc)&VideoCPC464::GetBaseAddress, (ColorFunc)&VideoCPC464::GetColor);
		AddMode(m_modes[1], (DrawFunc)&VideoCPC464::Draw320x200x4,  (AddressFunc)&VideoCPC464::GetBaseAddress, (ColorFunc)&VideoCPC464::GetColor);
		AddMode(m_modes[2], (DrawFunc)&VideoCPC464::Draw640x200x2,  (AddressFunc)&VideoCPC464::GetBaseAddress, (ColorFunc)&VideoCPC464::GetColor);
		AddMode(m_modes[3], (DrawFunc)&VideoCPC464::Draw160x200x4,  (AddressFunc)&VideoCPC464::GetBaseAddress, (ColorFunc)&VideoCPC464::GetColor);

		UpdateMode();
	}

	SDL_Rect VideoCPC464::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		xMultiplier = 2;
		border = 8;
		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hSyncMin - border - 1) * xMultiplier);
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

		//rect.x = 0;
		//rect.y = 0;
		//rect.w = 1024;
		//rect.h = 320;

		return rect;
	}

	void VideoCPC464::Write(BYTE value)
	{
		LogPrintf(LOG_TRACE, "Write(%04x, %02x)", GetCurrentPort(), value);

		switch (value >> 6)
		{
		case 0:
			if (GetBit(value, 4))
			{
				LogPrintf(LOG_INFO, "Select pen: [border]");
				m_currPen = PEN_BORDER;
			}
			else
			{
				LogPrintf(LOG_INFO, "Select pen: [%d]", (value & 15));
				m_currPen = (value & 15);
			}
			break;
		case 1:
			LogPrintf(LOG_INFO, "Select color: [%d]", (value & 31));
			m_pens[m_currPen] = (value & 31);
			break;
		case 2:
		{
			const bool interrupt = GetBit(value, 4);
			const bool romL = !GetBit(value, 2);
			const bool romH = !GetBit(value, 3);
			const int mode = value & 3;

			LogPrintf(LOG_DEBUG, "[%cINT_RST] [%cHROM] [%cLROM] [MODE%d]",
				(interrupt ? ' ' : '/'),
				(romH ? ' ' : '/'),
				(romL ? ' ' : '/'),
				(mode));

			m_mode = mode;

			if (interrupt)
			{
				m_interruptCounter = 0;
				m_isInterrupt = 0;
			}

			if (romL != m_romLowEnabled)
			{
				m_romLowEnabled = romL;
				m_events->OnLowROMChange(m_romLowEnabled);
			}

			if (romH != m_romHighEnabled)
			{
				m_romHighEnabled = romH;
				m_events->OnHighROMChange(m_romHighEnabled);
			}
		}
			break;
		case 3:
			LogPrintf(LOG_WARNING, "(Function 3, not present on CPC464)");
			break;
		}
	}

	void VideoCPC464::OnRenderFrame()
	{
		Video::RenderFrame();
	}

	void VideoCPC464::Tick()
	{
		if (!m_crtc.IsInit())
		{
			return;
		}

		// Clock at 1/4 Tick rate
		static int tick4 = 0;
		if (++tick4 == 4)
		{
			tick4 = 0;
		}
		else
		{
			return;
		}

		const auto& crtcData = m_crtc.GetData();
		if (crtcData.hPos == crtcData.hSyncMax)
		{
			UpdateMode();

			// Two lines after start of vsync, interrupt if counter < 32
			// and reset counter

			if (crtcData.vPos == (crtcData.vSyncMin + 2))
			{
				if (!GetBit(m_interruptCounter, 5))
				{
					m_isInterrupt = true;
				}

				m_interruptCounter = 0;
			}
			else if (++m_interruptCounter == 52)
			{
				m_isInterrupt = true;
				m_interruptCounter = 0;
			}
		}

		if (IsDisplayArea() && IsEnabled())
		{
			Draw();
		}
		else
		{
			DrawBackground(16);
		}

		m_crtc.Tick();
	}

	void VideoCPC464::InterruptAcknowledge()
	{
		m_isInterrupt = false;
		SetBit(m_interruptCounter, 5, false);
	}

	void VideoCPC464::OnNewFrame()
	{
		BeginFrame();
	}

	void VideoCPC464::OnEndOfRow()
	{
		NewLine();
		UpdateBaseAddress();
	}

	void VideoCPC464::UpdateMode()
	{
		static int m_lastMode = -1;

		if (m_mode == m_lastMode)
			return;

		if (m_mode < 0 || m_mode > 3)
		{
			LogPrintf(LOG_ERROR, "Invalid Mode: %d", m_mode);
			throw std::exception("Invalid mode");
		}

		LogPrintf(LOG_INFO, "Set Mode [%s] @ line %d", m_modes[m_mode], m_crtc.GetData().vPos);
		SetMode(m_modes[m_mode]);
		m_lastMode = m_mode;
	}

	// Bits MA12-13 of start address sets the 'page' in memory.
	// It's basically shifted to A14-15 to give:
	//
	//   MA13 MA12 | Video Page
	//   -----------------------
	//     0    0  | 0000 - 3FFF
	//     0    1  | 4000 - 7FFF
	//     1    0  | 8000 - BFFF
	//     1    1  | C000 - FFFF
	void VideoCPC464::UpdateBaseAddress()
	{
		m_baseAddress = (m_crtc.GetConfig().startAddress & 0b0011000000000000) << 2;
	}

	// Mode 1
	// 160x200x16 colors (4 bits), 2 pixels / byte
	//
	// High Nibble: pixels[0..1](bit 0, 2)
	// Low Nibble:  pixels[0..1](bit 1, 3)
	//
	// bit:   bit7 bit6 bit5 bit4 | bit3 bit2 bit1 bit0
	// pixel: p0.0 p1.0 p0.2 p1.2 | p0.1 p1.1 p0.3 p1.3
	void VideoCPC464::Draw160x200x16()
	{
		const struct CRTCData& data = m_crtc.GetData();

		ADDRESS base = GetAddress();

		for (int w = 0; w < 2; ++w)
		{
			BYTE ch = m_ram->read(base++);

			for (int i = 0; i < 2; ++i)
			{
				const BYTE index =
					(GetBit(ch, 7) << 0) |
					(GetBit(ch, 5) << 2) |
					(GetBit(ch, 3) << 1) |
					(GetBit(ch, 1) << 3);

				DrawPixel4(GetColor(index));
				ch <<= 1;
			}
		}
	}

	// Mode 1
	// 320x200x4 colors (2 bits), 4 pixels / byte
	//
	// High Nibble: pixels[0..3](l bit)
	// Low Nibble:  pixels[0..3](h bit)
	//
	// bit:   bit7 bit6 bit5 bit4 | bit3 bit2 bit1 bit0
	// pixel: p0.l p1.l p2.l p3.l | p0.h p1.h p2.h p3.h
	void VideoCPC464::Draw320x200x4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		ADDRESS base = GetAddress();

		for (int w = 0; w < 2; ++w)
		{
			BYTE ch = m_ram->read(base++);

			for (int i = 0; i < 4; ++i)
			{
				const BYTE index =
					(GetBit(ch, 7) << 0) |
					(GetBit(ch, 3) << 1);

				DrawPixel2(GetColor(index));
				ch <<= 1;
			}
		}
	}

	// Mode 2
	// 640x200x4 colors (1 bits), 1 pixels / byte
	//
	// bit:   bit7 bit6 bit5 bit4 | bit3 bit2 bit1 bit0
	// pixel: p.7  p.6  p.5  p.4  | p.3  p.2  p.1  p.0
	void VideoCPC464::Draw640x200x2()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 16 horizontal pixels
		// In this mode 1 byte = 8 pixels
		ADDRESS base = GetAddress();

		for (int w = 0; w < 2; ++w)
		{
			BYTE ch = m_ram->read(base++);

			for (int i = 0; i < 8; ++i)
			{
				const BYTE index = emul::GetMSB(ch);
				DrawPixel(GetColor(index));
				ch <<= 1;
			}
		}
	}

	// Mode 3 (unofficial, not supported in BASIC)
	// 160x200x4 colors (2 bits), 2 pixels / byte
	//
	// High Nibble: pixels[0..1](bit 0)
	// Low Nibble:  pixels[0..1](bit 1)
	//
	// bit:   bit7 bit6 bit5 bit4 | bit3 bit2 bit1 bit0
	// pixel: p0.0 p1.0   x    x  | p0.1 p1.1   x    x
	void VideoCPC464::Draw160x200x4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		ADDRESS base = GetAddress();

		for (int w = 0; w < 2; ++w)
		{
			BYTE ch = m_ram->read(base++);

			for (int i = 0; i < 2; ++i)
			{
				const BYTE index =
					(GetBit(ch, 7) << 0) |
					(GetBit(ch, 3) << 1);

				DrawPixel4(GetColor(index));
				ch <<= 1;
			}
		}
	}

	void VideoCPC464::Serialize(json& to)
	{
		Video::Serialize(to);
		m_crtc.Serialize(to["crtc"]);
		to["mode"] = m_mode;
		to["pens"] = m_pens;
		to["romLowEnabled"] = m_romLowEnabled;
		to["romHighEnabled"] = m_romHighEnabled;
		to["interruptCounter"] = m_interruptCounter;
		to["isInterrupt"] = m_isInterrupt;
	}

	void VideoCPC464::Deserialize(const json& from)
	{
		Video::Deserialize(from);
		m_crtc.Deserialize(from["crtc"]);
		m_mode = from["mode"];
		m_pens = from["pens"];
		m_romLowEnabled = from["romLowEnabled"];
		m_romHighEnabled = from["romHighEnabled"];
		m_interruptCounter = from["interruptCounter"];
		m_isInterrupt = from["isInterrupt"];

		UpdateMode();
		UpdateBaseAddress();
		m_events->OnLowROMChange(m_romLowEnabled);
		m_events->OnHighROMChange(m_romHighEnabled);
	}
}