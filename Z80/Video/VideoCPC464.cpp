#include "stdafx.h"
#include "VideoCPC464.h"

using emul::GetBit;
using emul::SetBit;

using crtc_6845::CRTCConfig;
using crtc_6845::CRTCData;

namespace video
{
	VideoCPC464::VideoCPC464() : Video(), Logger("vidCPC464")
	{
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
	}

	SDL_Rect VideoCPC464::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
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
			}
			else
			{
				LogPrintf(LOG_INFO, "Select pen: [%d]", (value & 15));
			}
			break;
		case 1:
			LogPrintf(LOG_INFO, "Select color: [%d]", (value & 31));
			break;
		case 2:
			LogPrintf(LOG_INFO, "[%cINT] [%cHROM] [%cLROM] [MODE%d]",
				(GetBit(value, 4) ? ' ' : '/'),
				(GetBit(value, 3) ? '/' : ' '),
				(GetBit(value, 2) ? '/' : ' '),
				(value & 3));
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
			//Draw();
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
	}

	void VideoCPC464::Serialize(json& to)
	{
		Video::Serialize(to);
		m_crtc.Serialize(to["crtc"]);
	}

	void VideoCPC464::Deserialize(const json& from)
	{
		Video::Deserialize(from);
		m_crtc.Deserialize(from["crtc"]);
	}
}