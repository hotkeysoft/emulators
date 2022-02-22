#include "VideoHGC.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;
using emul::ADDRESS;

using crtc::CRTCConfig;
using crtc::CRTCData;

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
	}

	SDL_Rect VideoHGC::GetDisplayRect(BYTE border) const
	{
		uint16_t xMultiplier = m_modeHGC.graphics ? 2 : 1;

		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hSyncMin - border - 1) * xMultiplier);
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

		return rect;
	}

	BYTE VideoHGC::ReadStatusRegister()
	{
		// Only difference is bit 7 == vsync
		BYTE status = VideoMDA::ReadStatusRegister();
		SetBit(status, 7, m_crtc.IsVSync());

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

		m_crtc.SetCharWidth(m_modeHGC.graphics ? 8 : 9);

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
			LogPrintf(LOG_INFO, "OnChangeMode: Draw720x348");
			m_drawFunc = (DrawFunc)&VideoHGC::Draw720x348;
		}
		else
		{
			VideoMDA::OnChangeMode();
		}
	}

	void VideoHGC::Draw720x348()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels, but since crtc is 45 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels

		uint32_t fg = GetMonitorPalette()[15];
		uint32_t bg = GetMonitorPalette()[0];

		if (m_crtc.IsDisplayArea() && m_mode.enableVideo)
		{
			ADDRESS base = (m_modeHGC.displayPage * 0x8000) + (data.rowAddress * 0x2000) + (m_crtc.GetMemoryAddress12() * 2u);
			base &= 0xFFFF;

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_screenB000.read(base++);

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
			DrawBackground(16);
		}
	}

	void VideoHGC::Serialize(json& to)
	{
		VideoMDA::Serialize(to);
		to["id.ext"] = "hgc";

		json mode;
		mode["graphics"] = m_modeHGC.graphics;
		mode["displayPage"] = m_modeHGC.displayPage;

		to["modeHGC"] = mode;
	}

	void VideoHGC::Deserialize(json& from)
	{
		if (from["id.ext"] != "hgc")
		{
			throw emul::SerializableException("VideoHGC: Incompatible mode");
		}

		const json& mode = from["modeHGC"];
		m_modeHGC.graphics = mode["graphics"];
		m_modeHGC.displayPage = mode["displayPage"];

		VideoMDA::Deserialize(from);
	}
}
