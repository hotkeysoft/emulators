#include "VideoTandy.h"
#include "PortAggregator.h"
#include <assert.h>

using emul::Memory;
using emul::MemoryBlock;
using emul::S2A;
using emul::GetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	const float VSCALE = 2.4f;

	static void OnRenderFrame(crtc::Device6845* crtc, void* data)
	{
		VideoTandy* video = reinterpret_cast<VideoTandy*>(data);
		video->RenderFrame();
	}

	static void OnNewFrame(crtc::Device6845* crtc, void* data)
	{
		VideoTandy* video = reinterpret_cast<VideoTandy*>(data);
		video->NewFrame();
	}

	VideoTandy::VideoTandy(WORD baseAddress) :
		Video(640, 225, VSCALE),
		Logger("vidTandy"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
		m_frameBuffer = new uint32_t[640 * 225];
	}

	void VideoTandy::Reset()
	{
		m_crtc.Reset();
	}

	void VideoTandy::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoTandy::Init(emul::Memory* memory, const char* charROM, BYTE border, bool)
	{
		assert(memory);
		m_memory = memory;

		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr8(4096 + 2048);

		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = (BYTE)(border / VSCALE);

		m_crtc.Init();
		m_crtc.SetRenderFrameCallback(OnRenderFrame, this);
		m_crtc.SetNewFrameCallback(OnNewFrame, this);

		// Registers

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoTandy::WriteModeControlRegister));

		// Color Select Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&VideoTandy::WriteColorSelectRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoTandy::ReadStatusRegister));


		// Video Array Address
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::OUTFunction>(&VideoTandy::WriteVideoArrayAddress));
		// Video Array Data
		Connect(m_baseAddress + 0xE, static_cast<PortConnector::OUTFunction>(&VideoTandy::WriteVideoArrayData));
		
		// CRT, Processor Page Register
		Connect(m_baseAddress + 0xF, static_cast<PortConnector::OUTFunction>(&VideoTandy::WritePageRegister));

		Video::Init(memory, charROM, border);
	}

	bool VideoTandy::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
	}

	void VideoTandy::WriteModeControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteModeControlRegister, value=%02Xh", value);

		m_mode.hiDotClock = GetBit(value, 0);
		m_mode.graphics = GetBit(value, 1);
		m_mode.monochrome = GetBit(value, 2);
		m_mode.enableVideo = GetBit(value, 3);
		m_mode.hiResolution = GetBit(value, 4);
		m_mode.blink = GetBit(value, 5);

		LogPrintf(Logger::LOG_INFO, "WriteModeControlRegister [%c80HIDOTCLK %cGRAPH %cMONO %cVIDEO %cHIRES %cBLINK]",
			m_mode.hiDotClock ? ' ' : '/',
			m_mode.graphics ? ' ' : '/',
			m_mode.monochrome ? ' ' : '/',
			m_mode.enableVideo ? ' ' : '/',
			m_mode.hiResolution ? ' ' : '/',
			m_mode.blink ? ' ' : '/');
	}

	void VideoTandy::WriteColorSelectRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteColorSelectRegister, value=%02Xh", value);

		m_color.color = (value & 15);
		m_color.palIntense = GetBit(value, 4);
		m_color.palSelect = GetBit(value, 5);

		LogPrintf(Logger::LOG_INFO, "WriteColorSelectRegister color=%d, palette %d, intense %d",
			m_color.color,
			m_color.palSelect,
			m_color.palIntense);
	}

	void VideoTandy::WritePageRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WritePageRegister, value=%02Xh", value);

		m_pageRegister.crtPage = value & 7;
		m_pageRegister.cpuPage = (value >> 3) & 7;
		m_pageRegister.videoAddressMode = (value >> 6) & 3;

		// For hi bw graphics modes the low order bit
		// is cleared to align page on 32kb boundary
		if (m_mode.graph640x200x4 || (false)/*TODO*/)
		{
			emul::SetBit(m_pageRegister.crtPage, 0, false);
		}

		// TODO? cpuPage: If an odd page number is selected (1, 3, 5) the window is reduced to 16K

		switch (m_pageRegister.videoAddressMode)
		{
		case 0: m_pageRegister.addressMode = PageRegister::ADDRESSMODE::ALPHA; break;
		case 1: m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_LOW; break;
		case 2: m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_HI; break;
		default:
			LogPrintf(LOG_WARNING, "WritePageRegister: Unused/Reserved mode");
		}

		UpdatePageRegisters();
	}

	void VideoTandy::UpdatePageRegisters()
	{
		// Pages are 16K (0x4000)
		m_pageRegister.crtBaseAddress = m_ramBase + (m_pageRegister.crtPage * 0x4000);
		m_pageRegister.cpuBaseAddress = m_ramBase + (m_pageRegister.cpuPage * 0x4000);

		LogPrintf(Logger::LOG_INFO, "Set Page Register: cpuPage[%d][%05Xh] crtPage=[%d][%05Xh]",
			m_pageRegister.crtPage, m_pageRegister.crtBaseAddress,
			m_pageRegister.cpuPage, m_pageRegister.cpuBaseAddress);

		// TODO: Need to 'unmap'?
		if (m_ramBase != 0xA0000)
		{
			MapB800Window();
		}
	}

	// This maps the zone of memory pointed to by the cpu page register to a 16/32K window at B800:0000
	void VideoTandy::MapB800Window()
	{
		// Window size is 16K for odd page numbers TODO: Validate?
		WORD windowSize = (m_pageRegister.cpuPage & 1) ? 0x4000 : 0x8000;

		m_memory->MapWindow(m_pageRegister.cpuBaseAddress, 0xB8000, windowSize);
	}

	void VideoTandy::SetRAMBase(ADDRESS base)
	{ 
		m_ramBase = base;
		UpdatePageRegisters();
	}

	void VideoTandy::WriteVideoArrayAddress(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteVideoArrayAddress, value=%02Xh", value);
		m_videoArrayRegisterAddress = (VideoArrayAddress)value;
	}
	void VideoTandy::WriteVideoArrayData(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteVideoArrayData, data=%02Xh", value);
		if (m_videoArrayRegisterAddress & GA_PALETTE)
		{
			// TODO: When loading the palette, the video is 'disabled' and the color viewed on the screen
			// is the data contained in the register being addressed by the processor.
			//
			// Video returns to normal after register address is changed to < 10h
			value &= 0b1111;
			BYTE reg = m_videoArrayRegisterAddress - GA_PALETTE;
			LogPrintf(Logger::LOG_INFO, "WriteVideoArrayData: Palette Register[%d] = %02Xh", reg, value);
			m_mode.paletteRegister[reg] = value;
		}
		else switch (m_videoArrayRegisterAddress)
		{
		case VA_PALETTE_MASK:
			value &= 0b1111;
			LogPrintf(Logger::LOG_INFO, "WriteVideoArrayData: Set Palette Mask = %02Xh", value);
			m_mode.paletteMask = value;
			break;
		case VA_BORDER_COLOR:
			value &= 0b1111;
			LogPrintf(Logger::LOG_INFO, "WriteVideoArrayData: Set Border Color = % 02Xh", value);
			m_mode.borderColor = value;
			break;
		case VA_MODE_CTRL:
			LogPrintf(Logger::LOG_DEBUG, "WriteVideoArrayData: Set Mode Control = %02Xh", value);

			m_mode.borderEnable = GetBit(value, 2);
			m_mode.graph640x200x4 = GetBit(value, 3);
			m_mode.graph16Colors = GetBit(value, 4);

			LogPrintf(Logger::LOG_INFO, "WriteVideoArrayData SetMode [%cBORDER_EN %c640x400x4 %cGRAPH16]",
				m_mode.borderEnable ? ' ' : '/',
				m_mode.graph640x200x4 ? ' ' : '/',
				m_mode.graph16Colors ? ' ' : '/');
			break;
		default:
			LogPrintf(Logger::LOG_WARNING, "WriteVideoArrayData: Invalid register address = %02Xh", m_videoArrayRegisterAddress);
			break;
		}

	}

	BYTE VideoTandy::ReadStatusRegister()
	{
		// Bit0: 1:Display enabled
		// 
		// Light pen, not implemented
		// Bit1: 1:Light pen trigger set
		// Bit2: 1:Light pen switch off, 0: switch on
		//
		// Bit3 1:Vertical retrace active
		// 
		// Bit4 1:Video dot

		BYTE status =
			(m_crtc.IsDisplayArea() << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(m_crtc.IsVSync() << 3);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoTandy::RenderFrame()
	{
		// TODO: don't recompute every time
		int w = (m_crtc.GetData().hTotalDisp * 2) / m_xAxisDivider;

		uint32_t borderRGB = GetMonitorPalette()[m_mode.borderEnable ? m_mode.borderColor: m_color.color];
		Video::RenderFrame(w, 225, borderRGB);
	}

	void VideoTandy::Tick()
	{
		if (!m_crtc.IsInit())
		{
			return;
		}

		if (m_mode.enableVideo)
		{
			(this->*m_drawFunc)();
		}
		else
		{
			// TODO: Check if correct
			uint32_t borderRGB = GetMonitorPalette()[m_mode.borderColor];
			std::fill(m_frameBuffer + 0, m_frameBuffer + 640*225, borderRGB);
		}

		m_crtc.Tick();
	}

	void VideoTandy::NewFrame()
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		// Pointers for alpha mode
		m_currChar = m_memory->GetPtr8(m_pageRegister.crtBaseAddress + (config.startAddress * 2u));
		if (config.cursorAddress >= (config.startAddress + 0x1000))
		{
			m_cursorPos = nullptr;
		}
		else
		{
			m_cursorPos = m_memory->GetPtr8(m_pageRegister.crtBaseAddress + (config.cursorAddress * 2u));
		}

		//// Pointers for graphics mode
		m_banks[0] = m_memory->GetPtr8(m_pageRegister.crtBaseAddress + 0x0000);
		m_banks[1] = m_memory->GetPtr8(m_pageRegister.crtBaseAddress + 0x2000);

		bool graph32K = (m_mode.hiDotClock && m_mode.graphics);
		m_banks[2] = graph32K ? m_memory->GetPtr8(m_pageRegister.crtBaseAddress + 0x4000) : nullptr;
		m_banks[3] = graph32K ? m_memory->GetPtr8(m_pageRegister.crtBaseAddress + 0x6000) : nullptr;

		//// Select draw function
		m_xAxisDivider = 2;
		if (!m_mode.graphics)
		{
			m_drawFunc = &VideoTandy::DrawTextMode;
		}
		else if (m_mode.hiResolution)
		{
			m_drawFunc = m_mode.graph640x200x4 ? &VideoTandy::Draw640x200x4 : &VideoTandy::Draw640x200x2;
			m_xAxisDivider = 1;
		}
		else if (m_mode.graph16Colors)
		{
			m_drawFunc = m_mode.hiDotClock ? m_drawFunc = &VideoTandy::Draw320x200x16 : &VideoTandy::Draw160x200x16;
			m_xAxisDivider = 4;
		}
		else
		{
			m_drawFunc = &VideoTandy::Draw320x200x4;
			m_xAxisDivider = 2;
		}

		m_currGraphPalette[0] = m_color.color;
		for (int i = 1; i < 4; ++i)
		{
			m_currGraphPalette[i] =
				(m_color.palIntense << 3) | // Intensity
				(i << 1) |
				(m_color.palSelect && !m_mode.monochrome) | // Palette shift for non mono modes
				(m_mode.monochrome & (i & 1)); // Palette shift for mono modes
		}
	}

	bool VideoTandy::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		return (m_currChar == m_cursorPos) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoTandy::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (m_currChar && m_crtc.IsDisplayArea() && ((data.vPos % data.vCharHeight) == 0))
		{
			BYTE* ch = m_currChar;
			BYTE* attr = ch + 1;
			BYTE bg = (*attr) >> 4;
			BYTE fg = (*attr) & 0x0F;
			bool charBlink = false;

			// Background
			if (m_mode.blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = bg & 8;
				bg = bg & 7;
			}

			bool isCursorChar = IsCursor();

			// Draw character
			BYTE* currCharPos = m_charROMStart + ((size_t)(*ch) * 8) + (data.vPos % data.vCharHeight);
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			// TODO: 225 lines char mode: Extend line 8 to 9 for some characters, need info.
			// For now leave lines above 8 blank
			for (int y = 0; y < data.vCharHeight; ++y)
			{
				uint32_t offset = 640 * (uint32_t)(data.vPos + y) + data.hPos;
				bool cursorLine = isCursorChar && (y >= config.cursorStart) && (y <= config.cursorEnd);
				for (int x = 0; x < 8; ++x)
				{
					bool set = cursorLine || (draw && (y < 8) && ((*(currCharPos + y)) & (1 << (7 - x))));
					assert(offset + x < (640 * 225));
					m_frameBuffer[offset + x] = GetColor(set ? fg : bg);
				}
			}

			m_currChar += 2;
		}
	}

	void VideoTandy::Draw160x200x16()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 2 pixels
		BYTE*& currChar = m_banks[data.vPos & 1];
		if (m_crtc.IsDisplayArea() && data.hPos < 160)
		{
			for (int w = 0; w < 4; ++w)
			{
				BYTE ch = *currChar;
				for (int x = 0; x < 2; ++x)
				{
					BYTE val = ch & 15;
					ch >>= 4;

					m_frameBuffer[640 * data.vPos + data.hPos + (w * 2) + (1 - x)] = GetColor(val);
				}

				++currChar;
			}
		}
	}

	void VideoTandy::Draw320x200x4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels
		BYTE*& currChar = m_banks[data.vPos & 1];
		if (m_crtc.IsDisplayArea())
		{
			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;
				for (int x = 0; x < 4; ++x)
				{
					BYTE val = ch & 3;
					ch >>= 2;

					m_frameBuffer[640 * data.vPos + data.hPos + (w * 4) + (3 - x)] = GetColor(m_currGraphPalette[val]);
				}

				++currChar;
			}
		}
	}

	void VideoTandy::Draw320x200x16()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 2 pixels
		if ((data.vPos & 3) > 3)
			return;
		BYTE*& currChar = m_banks[data.vPos & 3];
		if (m_crtc.IsDisplayArea() && data.hPos < 320)
		{
			for (int w = 0; w < 4; ++w)
			{
				BYTE ch = *currChar;
				for (int x = 0; x < 2; ++x)
				{
					BYTE val = ch & 15;
					ch >>= 4;

					m_frameBuffer[640 * data.vPos + data.hPos + (w * 2) + (1 - x)] = GetColor(val);
				}

				++currChar;
			}
		}
	}
	void VideoTandy::Draw640x200x2()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels, but since crtc is 40 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels

		if (m_crtc.IsDisplayArea())
		{
			BYTE*& currChar = m_banks[data.vPos & 1];

			uint32_t baseX = (640 * data.vPos) + (data.hPos * 2);

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;

				for (int x = 0; x < 8; ++x)
				{
					bool val = (ch & (1 << (7-x)));
					m_frameBuffer[baseX++] = GetColor(val ? 0xF : 0);
				}
				++currChar;
			}
		}
	}
	void VideoTandy::Draw640x200x4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 2 bytes = 8 pixels
		BYTE*& currChar = m_banks[data.vPos & 3];
		if (m_crtc.IsDisplayArea())
		{
			uint32_t baseX = (640 * data.vPos) + data.hPos;

			BYTE chEven = *currChar++;
			BYTE chOdd = *currChar++;

			for (int x = 0; x < 8; ++x)
			{
				BYTE val = 
					((chEven & (1 << (7 - x))) ? 1 : 0) | 
					((chOdd  & (1 << (7 - x))) ? 2 : 0);
				m_frameBuffer[baseX++] = GetColor(val);
			}
		}
	}


}
