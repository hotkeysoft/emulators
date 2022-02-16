#include "VideoTandy.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::Memory;
using emul::MemoryBlock;
using emul::S2A;
using emul::GetBit;
using emul::SetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	VideoTandy::VideoTandy(WORD baseAddress) :
		Logger("vidTandy"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
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

	void VideoTandy::Init(emul::Memory* memory, const char* charROM, bool)
	{
		assert(memory);
		m_memory = memory;

		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(4096 + 2048);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

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

		Video::Init(memory, charROM);
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
		if (m_mode.graphics && m_mode.hiDotClock)
		{
			SetBit(m_pageRegister.crtPage, 0, false);
		}

		switch (m_pageRegister.videoAddressMode)
		{
		case 0: m_pageRegister.addressMode = PageRegister::ADDRESSMODE::ALPHA; break;
		case 1: m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_LOW; break;
		case 3: m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_HI; break;
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

		LogPrintf(Logger::LOG_INFO, "Set Page Register: crtPage[%d][%05Xh] cpuPage=[%d][%05Xh]",
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
		// Bit0: 1:Display cpu access enabled (vsync | hsync)
		// 
		// Light pen, not implemented
		// Bit1: 1:Light pen trigger set
		// Bit2: 1:Light pen switch off, 0: switch on
		//
		// Bit3 1:Vertical retrace active

		BYTE status =
			(!(m_crtc.IsDisplayArea()) << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(m_crtc.IsVSync() << 3);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoTandy::OnChangeMode()
	{
		m_crtc.SetCharWidth(m_mode.graph16Colors ? 4 : 8);

		uint16_t width = m_crtc.GetData().hTotal;

		//// Select draw function
		if (!m_mode.graphics)
		{
			LogPrintf(LOG_INFO, "OnChangeMode: DrawTextMode");
			m_drawFunc = &VideoTandy::DrawTextMode;
		}
		else if (m_mode.hiResolution)
		{
			if (m_mode.graph640x200x4)
			{
				LogPrintf(LOG_INFO, "OnChangeMode: Draw640x200x4");
				m_drawFunc = &VideoTandy::Draw640x200x4;
			}
			else
			{
				LogPrintf(LOG_INFO, "OnChangeMode: Draw640x200x2");
				m_drawFunc = &VideoTandy::Draw640x200x2;
				width *= 2;
			}
		}
		else if (m_mode.graph16Colors)
		{
			LogPrintf(LOG_INFO, "OnChangeMode: Draw16");
			m_drawFunc = &VideoTandy::Draw16;
		}
		else
		{
			LogPrintf(LOG_INFO, "OnChangeMode: Draw320x200x4");
			m_drawFunc = &VideoTandy::Draw320x200x4;
		}
		
		InitFrameBuffer(width, m_crtc.GetData().vTotal);
	}

	void VideoTandy::OnRenderFrame()
	{
		uint32_t borderRGB = GetMonitorPalette()[m_mode.borderEnable ? m_mode.borderColor: m_color.color];
		Video::RenderFrame(borderRGB);
	}

	void VideoTandy::Tick()
	{
		if (!m_crtc.IsInit())
		{
			return;
		}

		if (!m_mode.hiDotClock)
		{
			static bool div2 = false;
			div2 = !div2;
			if (div2)
			{
				return;
			}
		}

		if (m_mode.enableVideo)
		{
			(this->*m_drawFunc)();
		}
		else
		{
			uint32_t borderRGB = GetMonitorPalette()[m_mode.borderEnable ? m_mode.borderColor : m_color.color];
			std::fill(m_fb.begin(), m_fb.end(), borderRGB);
		}

		m_crtc.Tick();
	}

	void VideoTandy::OnNewFrame()
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		unsigned int offset = config.startAddress * 2u;

		//// Pointers for graphics mode
		m_banks[0] = 0x0000 + offset;
		m_banks[1] = 0x2000 + offset;
		m_banks[2] = 0x4000 + offset;
		m_banks[3] = 0x6000 + offset;
	}

	void VideoTandy::OnEndOfRow()
	{
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

		return ((m_banks[0] / 2) == config.cursorAddress) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoTandy::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (m_crtc.IsDisplayArea() && ((data.vPos % data.vCharHeight) == 0))
		{
			ADDRESS& base = m_banks[0];

			bool isCursorChar = IsCursor();
			BYTE ch = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);
			BYTE attr = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);
			base &= 0x7FFF;

			BYTE bg = attr >> 4;
			BYTE fg = attr & 0x0F;
			bool charBlink = false;

			// Background
			if (m_mode.blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = GetBit(bg, 3);
				SetBit(bg, 3, false);
			}

			// Draw character
			BYTE* currCharPos = m_charROMStart + ((size_t)ch * 8) + (data.vPos % data.vCharHeight);
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			// TODO: 225 lines char mode: Extend line 8 to 9 for some characters, need info.
			// For now leave lines above 8 blank
			for (int y = 0; y < data.vCharHeight; ++y)
			{
				uint32_t offset = m_fbWidth * (uint32_t)(data.vPos + y) + data.hPos;
				bool cursorLine = isCursorChar && (y >= config.cursorStart) && (y <= config.cursorEnd);
				for (int x = 0; x < 8; ++x)
				{
					bool set = cursorLine || (draw && (y < 8) && ((*(currCharPos + y)) & (1 << (7 - x))));
					m_fb[offset + x] = GetColor(set ? fg : bg);
				}
			}
		}
	}

	void VideoTandy::Draw16()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 4 horizontal pixels
		// In this mode 1 byte = 2 pixels
		if (m_crtc.IsDisplayArea())
		{
			ADDRESS& base = m_banks[data.rowAddress];

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);
				for (int x = 0; x < 2; ++x)
				{
					BYTE val = ch & 15;
					ch >>= 4;

					m_fb[m_fbWidth * data.vPos + data.hPos + (w * 2) + (1 - x)] = GetColor(val);
				}
			}
		}
	}

	void VideoTandy::Draw320x200x4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels
		if (m_crtc.IsDisplayArea())
		{
			ADDRESS& base = m_banks[data.rowAddress];

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);
				for (int x = 0; x < 4; ++x)
				{
					BYTE val = ch & 3;
					ch >>= 2;

					m_fb[m_fbWidth * data.vPos + data.hPos + (w * 4) + (3 - x)] = GetColor(m_currGraphPalette[val]);
				}
			}
			base &= 0x7FFF;
		}
	}

	void VideoTandy::Draw640x200x2()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels, but since crtc is 40 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels
		if (m_crtc.IsDisplayArea())
		{
			ADDRESS& base = m_banks[data.rowAddress];

			uint32_t baseX = (m_fbWidth * data.vPos) + (data.hPos * 2);

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);

				for (int x = 0; x < 8; ++x)
				{
					bool val = (ch & (1 << (7-x)));
					m_fb[baseX++] = GetColor(val ? 0xF : 0);
				}
			}
			base &= 0x7FFF;
		}
	}
	void VideoTandy::Draw640x200x4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 2 bytes = 8 pixels
		if (m_crtc.IsDisplayArea())
		{
			ADDRESS& base = m_banks[data.rowAddress];

			uint32_t baseX = (m_fbWidth * data.vPos) + data.hPos;

			BYTE chEven = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);
			BYTE chOdd = m_memory->Read8(m_pageRegister.crtBaseAddress + base++);

			for (int x = 0; x < 8; ++x)
			{
				BYTE val = 
					((chEven & (1 << (7 - x))) ? 1 : 0) | 
					((chOdd  & (1 << (7 - x))) ? 2 : 0);
				m_fb[baseX++] = GetColor(val);
			}
			base &= 0x7FFF;
		}
	}

	void VideoTandy::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "tga";

		json pageReg;
		pageReg["crtPage"] = m_pageRegister.crtPage;
		pageReg["cpuPage"] = m_pageRegister.cpuPage;
		pageReg["videoAddressMode"] = m_pageRegister.videoAddressMode;
		pageReg["addressMode"] = m_pageRegister.addressMode;
		pageReg["crtBaseAddress"] = m_pageRegister.crtBaseAddress;
		pageReg["cpuBaseAddress"] = m_pageRegister.cpuBaseAddress;
		to["pageRegister"] = pageReg;

		json color;
		color["color"] = m_color.color;
		color["palIntense"] = m_color.palIntense;
		color["palSelect"] = m_color.palSelect;
		to["color"] = color;

		json mode;
		mode["hiDotClock"] = m_mode.hiDotClock;
		mode["graphics"] = m_mode.graphics;
		mode["monochrome"] = m_mode.monochrome;
		mode["enableVideo"] = m_mode.enableVideo;
		mode["hiResolution"] = m_mode.hiResolution;
		mode["blink"] = m_mode.blink;
		mode["paletteRegister"] = m_mode.paletteRegister;
		mode["paletteMask"] = m_mode.paletteMask;
		mode["borderEnable"] = m_mode.borderEnable;
		mode["borderColor"] = m_mode.borderColor;
		mode["graph640x200x4"] = m_mode.graph640x200x4;
		mode["graph16Colors"] = m_mode.graph16Colors;
		to["mode"] = mode;

		to["videoArrayRegisterAddress"] = m_videoArrayRegisterAddress;

		m_crtc.Serialize(to["crtc"]);
	}

	void VideoTandy::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoTandy: Incompatible baseAddress");
		}

		if (from["id"] != "tga")
		{
			throw emul::SerializableException("VideoTandy: Incompatible mode");
		}

		const json& pageReg = from["pageRegister"];
		m_pageRegister.crtPage = pageReg["crtPage"];
		m_pageRegister.cpuPage = pageReg["cpuPage"];
		m_pageRegister.videoAddressMode = pageReg["videoAddressMode"];
		m_pageRegister.addressMode = pageReg["addressMode"];
		m_pageRegister.crtBaseAddress = pageReg["crtBaseAddress"];
		m_pageRegister.cpuBaseAddress = pageReg["cpuBaseAddress"];

		const json& color = from["color"];
		m_color.color = color["color"];
		m_color.palIntense = color["palIntense"];
		m_color.palSelect = color["palSelect"];

		const json& mode = from["mode"];
		m_mode.hiDotClock = mode["hiDotClock"];
		m_mode.graphics = mode["graphics"];
		m_mode.monochrome = mode["monochrome"];
		m_mode.enableVideo = mode["enableVideo"];
		m_mode.hiResolution = mode["hiResolution"];
		m_mode.blink = mode["blink"];
		m_mode.paletteRegister = mode["paletteRegister"];
		m_mode.paletteMask = mode["paletteMask"];
		m_mode.borderEnable = mode["borderEnable"];
		m_mode.borderColor = mode["borderColor"];
		m_mode.graph640x200x4 = mode["graph640x200x4"];
		m_mode.graph16Colors = mode["graph16Colors"];

		m_videoArrayRegisterAddress = from["videoArrayRegisterAddress"];

		m_crtc.Deserialize(from["crtc"]);

		UpdatePageRegisters();
		OnChangeMode();
		OnNewFrame();
	}
}
