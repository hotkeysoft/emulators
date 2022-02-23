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
			(this->*m_drawFunc)();
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

	void Video6845::Serialize(json& to)
	{
		m_crtc.Serialize(to["crtc"]);
	}

	void Video6845::Deserialize(json& from)
	{
		m_crtc.Deserialize(from["crtc"]);
	}
}
