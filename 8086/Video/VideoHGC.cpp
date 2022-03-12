#include "stdafx.h"

#include "VideoHGC.h"

using emul::SetBit;
using emul::GetBit;

using crtc_6845::CRTCConfig;
using crtc_6845::CRTCData;

namespace video
{
	VideoHGC::VideoHGC(WORD baseAddress) :
		VideoMDA(baseAddress),
		Logger("HGC")
	{
	}

	void VideoHGC::Init(emul::Memory* memory, const char* charROM, bool)
	{
		VideoMDA::Init(memory, charROM);

		// TODO, show 4kb repeated pages in text mode?

		// 64KB video memory buffer, repeated from B000 to BFFF
		m_screenB000.Alloc(65536);
		memory->Allocate(&GetVideoRAM(), emul::S2A(0xB000));

		AddMode("720x348x2", (DrawFunc)&VideoHGC::Draw640x200x2, (AddressFunc)&VideoHGC::GetBaseAddressGraph, (ColorFunc)&VideoHGC::GetIndexedColor);
	}

	SDL_Rect VideoHGC::GetDisplayRect(BYTE border, WORD) const
	{
		uint16_t xMultiplier = m_modeHGC.graphics ? 2 : 1;

		return VideoMDA::GetDisplayRect(border, xMultiplier);
	}

	BYTE VideoHGC::ReadStatusRegister()
	{
		// Only difference is bit 7 == vsync
		BYTE status = VideoMDA::ReadStatusRegister();
		SetBit(status, 7, GetCRTC().IsVSync());

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister (HGC), value=%02Xh", status);

		return status;
	}

	void VideoHGC::WriteModeControlRegister(BYTE value)
	{
		// Additions are:
		// Bit 1: 1=Graphics Mode
		// Bit 7: 0=Display Page 1, 1=Display Page 2

		m_modeHGC.graphics = GetBit(value, 1);
		m_modeHGC.displayPage = GetBit(value, 7);

		GetCRTC().SetCharWidth(m_modeHGC.graphics ? 8 : 9);

		VideoMDA::WriteModeControlRegister(value);

		LogPrintf(LOG_INFO, "WriteModeControlRegister (HGC): [%s] [%s]",
			m_modeHGC.graphics ? "GRAPHICS" : "TEXT",
			m_modeHGC.displayPage ? "Page 2" : "Page 1");

		OnChangeMode();
	}

	void VideoHGC::OnChangeMode()
	{
		if (m_modeHGC.graphics)
		{
			SetMode("720x348x2");
		}
		else
		{
			SetMode("text");
		}
	}

	void VideoHGC::Serialize(json& to)
	{
		VideoMDA::Serialize(to);

		json mode;
		mode["graphics"] = m_modeHGC.graphics;
		mode["displayPage"] = m_modeHGC.displayPage;

		to["modeHGC"] = mode;
	}

	void VideoHGC::Deserialize(const json& from)
	{
		const json& mode = from["modeHGC"];
		m_modeHGC.graphics = mode["graphics"];
		m_modeHGC.displayPage = mode["displayPage"];

		VideoMDA::Deserialize(from);
	}
}
