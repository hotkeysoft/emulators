#include "VideoHGC.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

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
	}

	void VideoHGC::OnChangeMode()
	{
		if (m_modeHGC.graphics)
		{
			LogPrintf(LOG_INFO, "OnChangeMode: Draw720x348");
			m_drawFunc = (DrawFunc)&VideoHGC::Draw720x348;
			Video::InitFrameBuffer(m_crtc.GetData().hTotal * 2, m_crtc.GetData().vTotal);
		}
		else
		{
			VideoMDA::OnChangeMode();
		}
	}

	void VideoHGC::OnNewFrame()
	{
		if (m_modeHGC.graphics)
		{
			// Pointers for graphics mode
			WORD page = m_modeHGC.displayPage ? 0x8000 : 0;

			m_banks[0] = m_screenB000.getPtr(page + 0x0000);
			m_banks[1] = m_screenB000.getPtr(page + 0x2000);
			m_banks[2] = m_screenB000.getPtr(page + 0x4000);
			m_banks[3] = m_screenB000.getPtr(page + 0x6000);
		}
		else
		{
			VideoMDA::OnNewFrame();
		}
	}

	void VideoHGC::Draw720x348()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels, but since crtc is 45 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels

		if (m_crtc.IsDisplayArea())
		{
			BYTE*& currChar = m_banks[data.vPos & 3];

			uint32_t baseX = (m_fbWidth * data.vPos) + (data.hPos * 2);

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;

				for (int x = 0; x < 8; ++x)
				{
					bool val = (ch & (1 << (7 - x)));
					m_fb[baseX++] = GetMonitorPalette()[val ? 0x0F : 0];
				}
				++currChar;
			}
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
