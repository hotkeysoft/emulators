#include "stdafx.h"
#include "Device6522Mac.h"

namespace via::mac
{
	static via::mac::EventHandler s_defaultHandler;

	Device6522Mac::Device6522Mac(std::string id) :
		Device6522(id),
		Logger(id.c_str()),
		m_events(&s_defaultHandler)
	{
	}

	void Device6522Mac::Reset()
	{
		Device6522::Reset();
		m_soundReset = false;
		m_videoPageMain = false;
		m_soundBufferMain = false;
		m_headSelect = false;
		m_romOverlayMode = false;
	}

	void Device6522Mac::OnWritePort(VIAPort* src)
	{
		if (src == &m_portB)
		{
			if (bool newReset = IsSoundReset(); newReset != m_soundReset)
			{
				m_events->OnSoundResetChange(newReset);
				m_soundReset = newReset;
			}
		}
		else // Port A
		{
			if (bool newVideoPage = IsMainVideoBuffer(); newVideoPage != m_videoPageMain)
			{
				m_events->OnVideoPageChange(newVideoPage);
				m_videoPageMain = newVideoPage;
			}
			if (bool newHeadSel = GetDiskHeadSelect(); newHeadSel != m_headSelect)
			{
				m_events->OnHeadSelChange(newHeadSel);
				m_headSelect = newHeadSel;
			}
			if (bool newOverlay = IsOverlay(); newOverlay != m_romOverlayMode)
			{
				m_events->OnROMOverlayModeChange(newOverlay);
				m_romOverlayMode = newOverlay;
			}
			if (bool newSoundBuf = IsMainSoundBuffer(); newSoundBuf != m_soundBufferMain)
			{
				m_events->OnSoundBufferChange(newSoundBuf);
				m_soundBufferMain = newSoundBuf;
			}
		}
	}
}