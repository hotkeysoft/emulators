#include "stdafx.h"
#include "VideoCPC464.h"

using emul::GetBit;
using emul::SetBit;

using crtc_6845::CRTCConfig;
using crtc_6845::CRTCData;

namespace video
{
	VideoCPC464::VideoCPC464(emul::MemoryBlock* ram) : Video(), Logger("vidCPC464"), m_ram(ram)
	{
		assert(ram);
		Reset();
	}

	void VideoCPC464::Reset()
	{
		m_crtc.Reset();
	}

	void VideoCPC464::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoCPC464::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		Video::Init(memory, charROM, forceMono);
		InitFrameBuffer(512, 262);

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
		border = 8;
		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hSyncMin - border - 1) * xMultiplier);
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

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
				m_currPen = 16;
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
			LogPrintf(LOG_INFO, "[%cINT] [%cHROM] [%cLROM] [MODE%d]",
				(GetBit(value, 4) ? ' ' : '/'),
				(GetBit(value, 3) ? '/' : ' '),
				(GetBit(value, 2) ? '/' : ' '),
				(value & 3));

			m_mode = (value & 3);
			break;
		case 3:
			LogPrintf(LOG_INFO, "(Function 3, not present on CPC464)");
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

		if (!m_crtc.IsVSync())
		{
			Draw();
		}

		m_crtc.Tick();
	}

	void VideoCPC464::OnNewFrame()
	{
		BeginFrame();
	}

	void VideoCPC464::OnEndOfRow()
	{
		NewLine();
		UpdateBaseAddress();
		UpdateMode();
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

		LogPrintf(LOG_INFO, "Set Mode [%s]", m_modes[m_mode]);
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

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels
		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_ram->read(base++);

				for (int i = 0; i < 4; ++i)
				{
					const BYTE index =
						(GetBit(ch, 7) << 0) |
						(GetBit(ch, 3) << 1);

					DrawPixel(GetColor(index));
					ch <<= 1;
				}
			}
		}
		else
		{
			DrawBackground(8);
		}
	}

	void VideoCPC464::Serialize(json& to)
	{
		Video::Serialize(to);
		m_crtc.Serialize(to["crtc"]);
		to["mode"] = m_mode;
		to["pens"] = m_pens;
	}

	void VideoCPC464::Deserialize(const json& from)
	{
		Video::Deserialize(from);
		m_crtc.Deserialize(from["crtc"]);
		m_mode = from["mode"];
		m_pens = from["pens"];

		UpdateMode();
		UpdateBaseAddress();
	}
}