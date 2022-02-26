#include "VideoMDA.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

using crtc_6845::CRTCConfig;
using crtc_6845::CRTCData;

namespace video
{
	const BYTE CHAR_WIDTH = 9;
	
	VideoMDA::VideoMDA(WORD baseAddress) :
		Logger("MDA"),
		Video6845(baseAddress, CHAR_WIDTH),
		m_screenB000("MDA", emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
	}
	
	void VideoMDA::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = 0;

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

		Video6845::Init(memory, charROM, forceMono);

		// Normally max y is 370 but leave some room for custom crtc programming
		Video::InitFrameBuffer(2048, 400);

		AddMode("text", (DrawFunc)&VideoMDA::DrawTextMode, (AddressFunc)&VideoMDA::GetBaseAddressText, (ColorFunc)&VideoMDA::GetIndexedColor);
		SetMode("text");
	}

	void VideoMDA::OnChangeMode()
	{
		SetMode("text");
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
		bool dot = GetLastDot();

		BYTE status =
			(GetCRTC().IsHSync() << 0) |
			(0 << 1) |
			(0 << 2) |
			(dot << 3) |
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

	void VideoMDA::DrawTextMode()
	{
		const struct CRTCData& data = GetCRTC().GetData();
		const struct CRTCConfig& config = GetCRTC().GetConfig();

		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			bool isCursorChar = IsCursor();
			BYTE ch = m_memory->Read8(base);
			BYTE attr = m_memory->Read8(base + 1);

			bool blinkBit = GetBit(attr, 7);
			bool charBlink = m_mode.blink && blinkBit;
			bool charUnderline = ((attr & 0b111) == 1); // Underline when attr[2:0] == b001

			const BYTE bgFgMask = 0b01110111; // fg and bg without intensity and blink bits
			bool charReverse = ((attr & bgFgMask) == 0b01110000);

			BYTE fg = (attr & bgFgMask) ? (0b111 | (attr & 0b1000)) : 0b000; // Anything that's not black is on
			fg ^= charReverse ? 0b111 : 0b000; // Flip for reverse video

			BYTE bg = (charReverse || (!m_mode.blink && blinkBit)) ? 0xF : 0;

			uint32_t fgRGB = GetColor(fg);
			uint32_t bgRGB = GetColor(bg);

			// Draw character
			ADDRESS currCharPos = m_charROMStart + ((size_t)ch * 8) + (data.rowAddress & 7);
			bool draw = !charBlink || (charBlink && GetCRTC().IsBlink16());

			// Lower part of character is at A10=1 in ROM
			if (data.rowAddress >= 8)
			{
				currCharPos += (1 << 11);
			}
			BYTE currChar = m_charROM.read(currCharPos);

			bool underline = draw && (charUnderline & (data.rowAddress == config.maxScanlineAddress));

			bool cursorLine = isCursorChar && (data.rowAddress >= config.cursorStart) && (data.rowAddress <= config.cursorEnd);
			bool lastDot = false;
			for (int x = 0; x < 9; ++x)
			{
				if (x < 8)
				{
					lastDot = draw && GetBit(currChar, 7 - x);
				}
				// Characters C0h - DFh: 9th pixel == 8th pixel, otherwise blank
				else if ((ch < 0xC0) || (ch > 0xDF))
				{
					lastDot = 0;
				}

				DrawPixel((lastDot || cursorLine || underline) ? fgRGB : bgRGB);
			}
		}
		else
		{
			DrawBackground(9);
		}
	}

	void VideoMDA::Serialize(json& to)
	{
		Video6845::Serialize(to);
		to["baseAddress"] = m_baseAddress;

		json mode;
		mode["enableVideo"] = m_mode.enableVideo;
		mode["hiResolution"] = m_mode.hiResolution;
		mode["blink"] = m_mode.blink;
		to["mode"] = mode;
	}

	void VideoMDA::Deserialize(json& from)
	{
		Video6845::Deserialize(from);

		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoMDA: Incompatible baseAddress");
		}

		const json& mode = from["mode"];
		m_mode.enableVideo = mode["enableVideo"];
		m_mode.hiResolution = mode["hiResolution"];
		m_mode.blink = mode["blink"];

		OnChangeMode();
		OnNewFrame();
	}
}
