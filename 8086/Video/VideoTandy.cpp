#include "VideoTandy.h"
#include <assert.h>

using emul::Memory;
using emul::MemoryBlock;
using emul::GetBit;
using emul::SetBit;

using crtc_6845::CRTCConfig;
using crtc_6845::CRTCData;

namespace video
{
	VideoTandy::VideoTandy(WORD baseAddress) :
		Logger("vidTandy"),
		Video6845(baseAddress),
		m_baseAddress(baseAddress),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
	}

	void VideoTandy::Init(emul::Memory* memory, const char* charROM, bool)
	{
		assert(memory);
		m_memory = memory;

		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = 4096 + 2048;

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

		Video6845::Init(memory, charROM);

		// Normally max y is 262 but leave some room for custom crtc programming
		Video::InitFrameBuffer(2048, 300);

		AddMode("text", (DrawFunc)&VideoTandy::DrawTextMode, (AddressFunc)&VideoTandy::GetBaseAddressText, (ColorFunc)&VideoTandy::GetIndexedColor16);
		AddMode("320x200x4", (DrawFunc)&VideoTandy::Draw320x200x4, (AddressFunc)&VideoTandy::GetBaseAddressGraph, (ColorFunc)&VideoTandy::GetIndexedColor4);
		AddMode("640x200x2", (DrawFunc)&VideoTandy::Draw640x200x2, (AddressFunc)&VideoTandy::GetBaseAddressGraph, (ColorFunc)&VideoTandy::GetIndexedColor16);
		AddMode("640x200x4", (DrawFunc)&VideoTandy::Draw640x200x4, (AddressFunc)&VideoTandy::GetBaseAddressGraph, (ColorFunc)&VideoTandy::GetIndexedColor16);
		AddMode("x200x16", (DrawFunc)&VideoTandy::Draw200x16, (AddressFunc)&VideoTandy::GetBaseAddressGraph, (ColorFunc)&VideoTandy::GetIndexedColor16);

		SetMode("text");
	}

	SDL_Rect VideoTandy::GetDisplayRect(BYTE border, WORD) const
	{
		uint16_t xMultiplier = (m_mode.graphics && m_mode.hiResolution && !m_mode.graph640x200x4) ? 2 : 1;

		return Video6845::GetDisplayRect(border, xMultiplier);
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

		OnChangeMode();
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

			OnChangeMode();
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
			(!(IsDisplayArea()) << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(GetCRTC().IsVSync() << 3);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoTandy::OnChangeMode()
	{
		GetCRTC().SetCharWidth(m_mode.graph16Colors ? 4 : 8);

		//// Select draw function
		if (!m_mode.graphics)
		{
			SetMode("text");
		}
		else if (m_mode.hiResolution)
		{
			if (m_mode.graph640x200x4)
			{
				SetMode("640x200x4");
			}
			else
			{
				SetMode("640x200x2");
			}
		}
		else if (m_mode.graph16Colors)
		{
			SetMode("x200x16");
		}
		else
		{
			SetMode("320x200x4");
		}
	}

	void VideoTandy::Tick()
	{
		if (!m_mode.hiDotClock)
		{
			static bool div2 = false;
			div2 = !div2;
			if (div2)
			{
				return;
			}
		}

		Video6845::Tick();
	}

	void VideoTandy::OnEndOfRow()
	{
		Video6845::OnEndOfRow();
		m_currGraphPalette[0] = m_mode.borderEnable ? m_mode.borderColor : m_color.color;
		for (int i = 1; i < 4; ++i)
		{
			m_currGraphPalette[i] =
				(m_color.palIntense << 3) | // Intensity
				(i << 1) |
				(m_color.palSelect && !m_mode.monochrome) | // Palette shift for non mono modes
				(m_mode.monochrome & (i & 1)); // Palette shift for mono modes
		}
	}

	void VideoTandy::DrawTextMode()
	{
		const struct CRTCData& data = GetCRTC().GetData();
		const struct CRTCConfig& config = GetCRTC().GetConfig();

		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			bool isCursorChar = IsCursor();
			BYTE ch = m_memory->Read8(base);
			BYTE attr = m_memory->Read8(base + 1);

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
			// 225 lines char mode: Extend line 8 to 9 for some characters, need info (assuming B0-DF)
			WORD rowAddress = data.rowAddress;
			if (rowAddress > 7)
			{
				rowAddress = 7;
				if ((ch < 0xB0) || (ch >= 0xE0))
				{
					ch = 0;
				}
			}
			BYTE currChar = m_charROM.read(m_charROMStart + ((size_t)ch * 8) + rowAddress);
			bool draw = !charBlink || (charBlink && GetCRTC().IsBlink16());

			bool cursorLine = isCursorChar && (data.rowAddress >= config.cursorStart) && (data.rowAddress <= config.cursorEnd);
			for (int x = 0; x < 8; ++x)
			{
				bool set = draw && GetBit(currChar, 7 - x);
				DrawPixel(GetColor((set || cursorLine) ? fg : bg));
			}
		}
		else
		{
			DrawBackground(8);
		}
	}

	void VideoTandy::Serialize(json& to)
	{
		Video6845::Serialize(to);
		to["baseAddress"] = m_baseAddress;

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
	}

	void VideoTandy::Deserialize(json& from)
	{
		Video6845::Deserialize(from);

		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoTandy: Incompatible baseAddress");
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

		UpdatePageRegisters();
		OnChangeMode();
		OnNewFrame();
	}
}
