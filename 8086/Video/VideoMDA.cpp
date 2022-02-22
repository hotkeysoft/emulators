#include "VideoMDA.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;
using emul::ADDRESS;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	const float VSCALE = 1.6f; // TODO
	const BYTE CHAR_WIDTH = 9;
	
	VideoMDA::VideoMDA(WORD baseAddress) :
		Logger("MDA"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress, CHAR_WIDTH),
		m_screenB000("MDA", emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
	}

	void VideoMDA::Reset()
	{
		m_crtc.Reset();
	}

	void VideoMDA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}
	
	void VideoMDA::Init(emul::Memory* memory, const char* charROM, bool)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(0);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoMDA::WriteModeControlRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoMDA::ReadStatusRegister));

		// 4KB video memory buffer, repeated from B000 to B7FF
		m_screenB000.Alloc(4096);
		for (int i = 0; i < 8; ++i)
		{
			memory->Allocate(&GetVideoRAM(), emul::S2A(0xB000 + (i * 0x100)));
		}

		Video::Init(memory, charROM, true);

		// Normally max y is 370 but leave some room for custom crtc programming
		Video::InitFrameBuffer(2048, 400);
	}

	bool VideoMDA::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
	}

	SDL_Rect VideoMDA::GetDisplayRect(BYTE border) const
	{
		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hSyncMin - border - 1));
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)));
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

		return rect;
	}

	void VideoMDA::OnChangeMode()
	{
		// Select draw function
		LogPrintf(LOG_INFO, "OnChangeMode: DrawTextMode");
		m_drawFunc = &VideoMDA::DrawTextMode;
	}

	void VideoMDA::OnRenderFrame()
	{
		Video::RenderFrame();
	}

	BYTE VideoMDA::ReadStatusRegister()
	{
		// Bit0: Horizontal Retrace: 1:Hsync active
		// 
		// Bit1: Nothing, always 0
		// Bit2: Nothing, always 0
		//
		// Bit3: Video dot: 1:Green or Bright Green pixel drawn at the moment
		// 
		// Bit4-7: Nothing, always 1

		// register "address" selects the video dot to inspect (0-3: [B,G,R,I])
		bool dot = m_lastDot;

		BYTE status =
			(m_crtc.IsHSync() << 0) |
			(0 << 1) |
			(0 << 2) |
			(m_lastDot << 3) |
			(1 << 4) |
			(1 << 5) |
			(1 << 6) |
			(1 << 7);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoMDA::WriteModeControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteModeControlRegister, value=%02Xh", value);

		m_mode.hiResolution = GetBit(value, 0);
		m_mode.enableVideo = GetBit(value, 3);
		m_mode.blink = GetBit(value, 5);

		LogPrintf(Logger::LOG_INFO, "WriteModeControlRegister [%cBLINK %cENABLE %cHIRES]",
			m_mode.blink ? ' ' : '/',
			m_mode.enableVideo ? ' ' : '/',
			m_mode.hiResolution ? ' ' : '/');
	}

	void VideoMDA::Tick()
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

	void VideoMDA::OnNewFrame()
	{
		BeginFrame();
	}

	void VideoMDA::OnEndOfRow()
	{
		NewLine();
	}

	bool VideoMDA::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();
		const struct CRTCData& data = m_crtc.GetData();

		return (data.memoryAddress == config.cursorAddress) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoMDA::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (m_crtc.IsDisplayArea() && m_mode.enableVideo)
		{
			ADDRESS base = m_crtc.GetMemoryAddress13() * 2u;

			bool isCursorChar = IsCursor();
			BYTE ch = m_screenB000.read(base);
			BYTE attr = m_screenB000.read(base + 1);

			bool blinkBit = GetBit(attr, 7);
			bool charBlink = m_mode.blink && blinkBit;
			bool charUnderline = ((attr & 0b111) == 1); // Underline when attr[2:0] == b001

			const BYTE bgFgMask = 0b01110111; // fg and bg without intensity and blink bits
			bool charReverse = ((attr & bgFgMask) == 0b01110000);

			BYTE fg = (attr & bgFgMask) ? (0b111 | (attr & 0b1000)) : 0b000; // Anything that's not black is on
			fg ^= charReverse ? 0b111 : 0b000; // Flip for reverse video

			BYTE bg = (charReverse || (!m_mode.blink && blinkBit)) ? 0xF : 0;

			uint32_t fgRGB = GetMonitorPalette()[fg];
			uint32_t bgRGB = GetMonitorPalette()[bg];

			// Draw character
			BYTE* currCharPos = m_charROMStart + ((size_t)ch * 8) + (data.rowAddress & 7);
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			// Lower part of character is at A10=1 in ROM
			if (data.rowAddress >= 8)
			{
				currCharPos += (1 << 11);
			}
			bool underline = draw && (charUnderline & (data.rowAddress == config.maxScanlineAddress));

			bool cursorLine = isCursorChar && (data.rowAddress >= config.cursorStart) && (data.rowAddress <= config.cursorEnd);
			for (int x = 0; x < 9; ++x)
			{
				if (x < 8)
				{
					m_lastDot = draw && GetBit(*(currCharPos), 7 - x);
				}
				// Characters C0h - DFh: 9th pixel == 8th pixel, otherwise blank
				else if ((ch < 0xC0) || (ch > 0xDF))
				{
					m_lastDot = 0;
				}

				DrawPixel((m_lastDot || cursorLine || underline) ? fgRGB : bgRGB);
			}
		}
		else
		{
			DrawBackground(9);
		}
	}

	void VideoMDA::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "mda";

		json mode;
		mode["enableVideo"] = m_mode.enableVideo;
		mode["hiResolution"] = m_mode.hiResolution;
		mode["blink"] = m_mode.blink;
		to["mode"] = mode;

		to["lastDot"] = m_lastDot;

		m_crtc.Serialize(to["crtc"]);
	}

	void VideoMDA::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoMDA: Incompatible baseAddress");
		}

		if (from["id"] != "mda")
		{
			throw emul::SerializableException("VideoMDA: Incompatible mode");
		}

		const json& mode = from["mode"];
		m_mode.enableVideo = mode["enableVideo"];
		m_mode.hiResolution = mode["hiResolution"];
		m_mode.blink = mode["blink"];

		m_lastDot = from["lastDot"];

		m_crtc.Deserialize(from["crtc"]);

		OnChangeMode();
		OnNewFrame();
	}
}
