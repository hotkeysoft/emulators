#include "Video6845.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{

	Video6845::Video6845(WORD baseAddress, BYTE charWidth) :
		Logger("Video6845"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress, charWidth)
	{
		Reset();
	}

	void Video6845::Reset()
	{
		m_crtc.Reset();
	}

	void Video6845::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void Video6845::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		assert(charROM);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		Video::Init(memory, charROM, forceMono);
	}	
	
	bool Video6845::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
	}

	SDL_Rect Video6845::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hSyncMin - border - 1) * xMultiplier);
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

		return rect;
	}

	void Video6845::OnRenderFrame()
	{
		Video::RenderFrame();
	}

	void Video6845::Tick()
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

	void Video6845::OnNewFrame()
	{
		BeginFrame();
	}
	
	void Video6845::OnEndOfRow()
	{
		NewLine();
	}

	bool Video6845::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();
		const struct CRTCData& data = m_crtc.GetData();

		return (data.memoryAddress == config.cursorAddress) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void Video6845::Draw320x200x4()
	{
		const struct CRTCData& data = GetCRTC().GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels
		if (GetCRTC().IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_memory->Read8(base++);

				DrawPixel(GetColor((ch & 0b11000000) >> 6));
				DrawPixel(GetColor((ch & 0b00110000) >> 4));
				DrawPixel(GetColor((ch & 0b00001100) >> 2));
				DrawPixel(GetColor((ch & 0b00000011) >> 0));
			}
		}
		else
		{
			DrawBackground(8);
		}
	}

	void Video6845::Draw640x200x2()
	{
		const struct CRTCData& data = GetCRTC().GetData();

		// Called every 8 horizontal pixels, but since crtc is 40 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels

		uint32_t fg = GetColor(15);
		uint32_t bg = GetColor(0);

		if (GetCRTC().IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_memory->Read8(base++);

				DrawPixel((ch & 0b10000000) ? fg : bg);
				DrawPixel((ch & 0b01000000) ? fg : bg);
				DrawPixel((ch & 0b00100000) ? fg : bg);
				DrawPixel((ch & 0b00010000) ? fg : bg);
				DrawPixel((ch & 0b00001000) ? fg : bg);
				DrawPixel((ch & 0b00000100) ? fg : bg);
				DrawPixel((ch & 0b00000010) ? fg : bg);
				DrawPixel((ch & 0b00000001) ? fg : bg);
			}
		}
		else
		{
			DrawBackground(16, bg);
		}
	}

	void Video6845::Draw200x16()
	{
		// Called every 4 horizontal pixels
		// In this mode 1 byte = 2 pixels
		if (GetCRTC().IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_memory->Read8(base++);

				DrawPixel(GetColor((ch & 0b11110000) >> 4));
				DrawPixel(GetColor((ch & 0b00001111) >> 0));
			}
		}
		else
		{
			DrawBackground(4);
		}
	}

	void Video6845::Draw640x200x4()
	{
		// Called every 8 horizontal pixels
		// In this mode 2 bytes = 8 pixels
		if (GetCRTC().IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			BYTE chEven = m_memory->Read8(base);
			BYTE chOdd = m_memory->Read8(base + 1);

			for (int x = 0; x < 8; ++x)
			{
				BYTE val =
					((chEven & (1 << (7 - x))) ? 1 : 0) |
					((chOdd & (1 << (7 - x))) ? 2 : 0);
				DrawPixel(GetColor(val));
			}
		}
		else
		{
			DrawBackground(8);
		}
	}

	void Video6845::Serialize(json& to)
	{
		m_crtc.Serialize(to["crtc"]);
	}

	void Video6845::Deserialize(json& from)
	{
		m_crtc.Deserialize(from["crtc"]);
	}
}
