#include "VideoMDA.h"
#include "PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	const float VSCALE = 1.6f; // TODO
	const BYTE CHAR_WIDTH = 9;
	
	static void OnRenderFrame(crtc::Device6845* crtc, void* data)
	{
		Video* mda = reinterpret_cast<Video*>(data);
		mda->RenderFrame(720, 350);
	}

	static void OnNewFrame(crtc::Device6845* crtc, void* data)
	{
		VideoMDA* mda = reinterpret_cast<VideoMDA*>(data);
		mda->NewFrame();
	}

	VideoMDA::VideoMDA(WORD baseAddress) :
		Video(720, 350, VSCALE),
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
	
	void VideoMDA::Init(emul::Memory* memory, const char* charROM, BYTE border, bool)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(0);

		m_crtc.Init();
		m_crtc.SetRenderFrameCallback(OnRenderFrame, this);
		m_crtc.SetNewFrameCallback(OnNewFrame, this);

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

		Video::Init(memory, charROM, border, true);
	}

	bool VideoMDA::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
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

		if (m_mode.enableVideo)
		{
			(this->*m_drawFunc)();
		}

		m_crtc.Tick();
	}

	void VideoMDA::NewFrame()
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		// Pointers for alpha mode
		m_currChar = m_screenB000.getPtr(config.startAddress * 2u);
		if (config.cursorAddress * 2u >= m_screenB000.GetSize())
		{
			m_cursorPos = nullptr;
		}
		else
		{
			m_cursorPos = m_screenB000.getPtr(config.cursorAddress * 2u);
		}

		// Select draw function
		m_drawFunc = &VideoMDA::DrawTextMode;
	}

	bool VideoMDA::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		return (m_currChar == m_cursorPos) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoMDA::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (m_currChar && m_crtc.IsDisplayArea() && ((data.vPos % data.vCharHeight) == 0))
		{
			bool isCursorChar = IsCursor();

			BYTE ch = *(m_currChar++);
			BYTE attr = *(m_currChar++);

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
			BYTE* currCharPos = m_charROMStart + ((size_t)ch * 8);
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());
			for (int y = 0; y < data.vCharHeight; ++y)
			{
				// Lower part of character is at A10=1 in ROM
				if (y == 8)
				{
					currCharPos += (1 << 11);
				}
				bool underline = draw && (charUnderline & (y == data.vCharHeight - 1));

				uint32_t offset = 720 * (uint32_t)(data.vPos + y) + data.hPos;
				bool cursorLine = isCursorChar && (y >= config.cursorStart) && (y <= config.cursorEnd);
				for (int x = 0; x < 9; ++x)
				{
					if (x < 8)
					{
						m_lastDot = draw && ((*(currCharPos + (y & 7))) & (1 << (7 - x)));
					}
					// Characters C0h - DFh: 9th pixel == 8th pixel, otherwise blank
					else if ((ch < 0xC0) || (ch > 0xDF))
					{
						m_lastDot = 0;
					}

					m_frameBuffer[offset + x] = (m_lastDot || cursorLine || underline) ? fgRGB : bgRGB;
				}
			}
		}
	}
}
