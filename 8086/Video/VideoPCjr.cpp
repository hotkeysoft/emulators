#include "VideoPCjr.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::Memory;
using emul::MemoryBlock;
using emul::S2A;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	const float VSCALE = 2.4f;

	VideoPCjr::VideoPCjr(WORD baseAddress) :
		Video(640, 200, VSCALE),
		Logger("vidPCjr"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
		m_frameBuffer = new uint32_t[640 * 200];
	}

	void VideoPCjr::Reset()
	{
		m_crtc.Reset();
	}

	void VideoPCjr::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoPCjr::Init(emul::Memory* memory, const char* charROM, BYTE border, bool)
	{
		assert(memory);
		m_memory = memory;

		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(4096 + 2048);

		m_sdlBorderPixels = border;
		m_sdlHBorder = border;
		m_sdlVBorder = (BYTE)(border / VSCALE);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		// Registers
		// 
		// CRT, Processor Page Register
		Connect(m_baseAddress + 0xF, static_cast<PortConnector::OUTFunction>(&VideoPCjr::WritePageRegister));
		//
		// Gate Array Register: Address/Data
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::OUTFunction>(&VideoPCjr::WriteGateArrayRegister));
		//
		// Gate Array Register: Status
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoPCjr::ReadStatusRegister));

		Video::Init(memory, charROM, border);
	}

	bool VideoPCjr::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
	}

	void VideoPCjr::WritePageRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WritePageRegister, value=%02Xh", value);

		m_pageRegister.crtPage = value & 7;
		m_pageRegister.cpuPage = (value >> 3) & 7;
		m_pageRegister.videoAddressMode = (value >> 6) & 3;

		// For hi bw graphics modes the low order bit
		// is cleared to align page on 32kb boundary
		if (m_mode.hiBandwidth && m_mode.graphics)
		{
			emul::SetBit(m_pageRegister.crtPage, 0, false);
		}

		const char* modeStr;
		switch (m_pageRegister.videoAddressMode)
		{
		case 0:
			m_pageRegister.addressMode = PageRegister::ADDRESSMODE::ALPHA;
			modeStr = "ALPHA";
			break;
		case 1:
			m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_LOW;
			modeStr = "GRAPH LOW";
			break;
		case 2:
			m_pageRegister.addressMode = PageRegister::ADDRESSMODE::GRAPH_HI;
			modeStr = "GRAPH HI";
			break;
		default:
			modeStr = "UNUSED";
			LogPrintf(LOG_WARNING, "WritePageRegister: Unused/Reserved mode");
		}

		// Pages are 16K (0x4000)
		m_pageRegister.crtBaseAddress = m_pageRegister.crtPage * 0x4000;
		m_pageRegister.cpuBaseAddress = m_pageRegister.cpuPage * 0x4000;

		LogPrintf(Logger::LOG_INFO, "Set Page Register: cpuPage[%d][%05Xh] crtPage=[%d][%05Xh] videoAddressMode=[%d][%s]", 
			m_pageRegister.crtPage, m_pageRegister.crtBaseAddress,
			m_pageRegister.cpuPage, m_pageRegister.cpuBaseAddress,
			m_pageRegister.videoAddressMode, modeStr);

		MapB800Window();
	}

	// This maps the zone of memory pointed to by the cpu page register to a 16k window at B800:0000
	void VideoPCjr::MapB800Window()
	{
		m_memory->MapWindow(m_pageRegister.cpuBaseAddress, 0xB8000, 0x4000);
	}

	void VideoPCjr::WriteGateArrayRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteGateArrayRegister, value=%02Xh", value);

		// Set Register Address
		if (m_mode.addressDataFlipFlop == false)
		{
			value &= GA_MASK;
			m_mode.currRegister = (GateArrayAddress)value;
			LogPrintf(Logger::LOG_DEBUG, "WriteGateArrayRegister, set address=%02Xh", value);
		}
		else // Set Register value
		{
			LogPrintf(Logger::LOG_DEBUG, "WriteGateArrayRegister, data=%02Xh", value);
			if (m_mode.currRegister & GA_PALETTE)
			{
				// TODO: When loading the palette, the video is 'disabled' and the color viewed on the screen
				// is the data contained in the register being addressed by the processor.
				//
				// Video returns to normal after register address is changed to < 10h
				value &= 0b1111;
				BYTE reg = m_mode.currRegister - GA_PALETTE;
				LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister: Palette Register[%d] = %02Xh", reg, value);
				m_mode.paletteRegister[reg] = value;
			}
			else switch (m_mode.currRegister)
			{
			case GA_MODE_CTRL_1:
				LogPrintf(Logger::LOG_DEBUG, "WriteGateArrayRegister: Set Mode Control 1 = %02Xh", value);
				m_mode.hiBandwidth =   value & 1;
				m_mode.graphics =      value & 2;
				m_mode.monochrome =    value & 4;
				m_mode.enableVideo =   value & 8;
				m_mode.graph16Colors = value & 16;

				LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister SetMode1 [%cHIBW %cGRAPH %cMONO %cVIDEO %cGRAPH16COLORS]",
					m_mode.hiBandwidth ? ' ' : '/',
					m_mode.graphics ? ' ' : '/',
					m_mode.monochrome ? ' ' : '/',
					m_mode.enableVideo ? ' ' : '/',
					m_mode.graph16Colors ? ' ' : '/');
				break;
			case GA_PALETTE_MASK:
				value &= 0b1111;
				LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister: Set Palette Mask = %02Xh", value);
				m_mode.paletteMask = value;
				break;
			case GA_BORDER_COLOR:
				value &= 0b1111;
				LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister: Set Border Color = % 02Xh", value);
				m_mode.borderColor = value;
				break;
			case GA_MODE_CTRL_2:
				LogPrintf(Logger::LOG_DEBUG, "WriteGateArrayRegister: Set Mode Control 2 = %02Xh", value);
				m_mode.blink = value & 2;
				m_mode.graph2Colors = value & 8;

				LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister SetMode2 [%cBLINK %cGRAPH2COLORS]",
					m_mode.blink ? ' ' : '/',
					m_mode.graph2Colors ? ' ' : '/');
				break;
			case GA_RESET:
				value &= 0b11;
				LogPrintf(Logger::LOG_DEBUG, "WriteGateArrayRegister: Reset, value = %02Xh", value);
				// We don't really have to do anything special for the resets
				if (!value)
				{
					LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister: Reset Completed");
				}
				if (value & 1)
				{
					LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister: Asynchronous Reset");
				}
				else if (value & 2)
				{
					LogPrintf(Logger::LOG_INFO, "WriteGateArrayRegister: Synchronous Reset");
				}
				break;
			default:
				LogPrintf(Logger::LOG_WARNING, "WriteGateArrayRegister: Invalid register address = %02Xh", m_mode.currRegister);
				break;
			}
		}

		m_mode.addressDataFlipFlop = !m_mode.addressDataFlipFlop;
	}

	BYTE VideoPCjr::ReadStatusRegister()
	{
		// Reading the status register resets the GateArrayRegister address/data flip-flop
		m_mode.addressDataFlipFlop = false;

		// Bit0: 1:Display enabled
		// 
		// Light pen, not implemented
		// Bit1: 1:Light pen trigger set
		// Bit2: 1:Light pen switch off, 0: switch on
		//
		// Bit3 1:Vertical retrace active
		// 
		// Bit4 1:Video dot

		// register "address" selects the video dot to inspect (0-3: [B,G,R,I])
		bool dot = (m_lastDot & (1 << (m_mode.currRegister & 3)));

		BYTE status =
			(m_crtc.IsDisplayArea() << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(m_crtc.IsVSync() << 3) |
			(dot << 4); // Video dots

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoPCjr::OnRenderFrame()
	{
		// TODO: don't recompute every time
		int w = (m_crtc.GetData().hTotalDisp * 2) / m_xAxisDivider;

		uint32_t borderRGB = GetMonitorPalette()[m_mode.borderColor];
		Video::RenderFrame(w, 200, borderRGB);
	}

	void VideoPCjr::Tick()
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

	void VideoPCjr::OnNewFrame()
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
		m_banks[2] = m_mode.hiBandwidth ? m_memory->GetPtr8(m_pageRegister.crtBaseAddress + 0x4000) : nullptr;
		m_banks[3] = m_mode.hiBandwidth ? m_memory->GetPtr8(m_pageRegister.crtBaseAddress + 0x6000) : nullptr;

		//// Select draw function
		m_xAxisDivider = 2;
		if (!m_mode.graphics)
		{
			m_drawFunc = &VideoPCjr::DrawTextMode;
		}
		else if (m_mode.graph2Colors)
		{
			m_drawFunc = &VideoPCjr::Draw640x200x2;
			m_xAxisDivider = 1;
		}
		else if (m_mode.graph16Colors)
		{
			if (m_mode.hiBandwidth)
			{
				m_drawFunc = &VideoPCjr::Draw320x200x16;
				m_xAxisDivider = 4;
			} 
			else
			{
				m_drawFunc = &VideoPCjr::Draw160x200x16;
				m_xAxisDivider = 4;
			}
		}
		else if (m_mode.hiBandwidth)
		{
			m_drawFunc = &VideoPCjr::Draw640x200x4;
		}
		else
		{
			m_drawFunc = &VideoPCjr::Draw320x200x4;
			m_xAxisDivider = 2;
		}
	}

	bool VideoPCjr::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		return (m_currChar == m_cursorPos) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoPCjr::DrawTextMode()
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
			for (int y = 0; y < data.vCharHeight; ++y)
			{
				uint32_t offset = 640 * (uint32_t)(data.vPos + y) + data.hPos;
				bool cursorLine = isCursorChar && (y >= config.cursorStart) && (y <= config.cursorEnd);
				for (int x = 0; x < 8; ++x)
				{
					bool set = cursorLine || (draw && ((*(currCharPos + y)) & (1 << (7 - x))));
					m_lastDot = set ? fg : bg;
					m_frameBuffer[offset + x] = GetColor(m_lastDot);
				}
			}

			m_currChar += 2;
		}
	}

	void VideoPCjr::Draw160x200x16()
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

	void VideoPCjr::Draw320x200x4()
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

					m_frameBuffer[640 * data.vPos + data.hPos + (w * 4) + (3 - x)] = GetColor(val);
				}

				++currChar;
			}
		}
	}

	void VideoPCjr::Draw320x200x16()
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
	void VideoPCjr::Draw640x200x2()
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
					m_frameBuffer[baseX++] = GetColor(val);
				}
				++currChar;
			}
		}
	}
	void VideoPCjr::Draw640x200x4()
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

	void VideoPCjr::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "pcjr";

		json pageReg;
		pageReg["crtPage"] = m_pageRegister.crtPage;
		pageReg["cpuPage"] = m_pageRegister.cpuPage;
		pageReg["videoAddressMode"] = m_pageRegister.videoAddressMode;
		pageReg["addressMode"] = m_pageRegister.addressMode;
		pageReg["crtBaseAddress"] = m_pageRegister.crtBaseAddress;
		pageReg["cpuBaseAddress"] = m_pageRegister.cpuBaseAddress;
		to["pageRegister"] = pageReg;

		json mode;
		mode["currRegister"] = m_mode.currRegister;
		mode["paletteRegister"] = m_mode.paletteRegister;
		mode["borderColor"] = m_mode.borderColor;
		mode["paletteMask"] = m_mode.paletteMask;
		mode["hiBandwidth"] = m_mode.hiBandwidth;
		mode["graphics"] = m_mode.graphics;
		mode["monochrome"] = m_mode.monochrome;
		mode["enableVideo"] = m_mode.enableVideo;
		mode["graph16Colors"] = m_mode.graph16Colors;
		mode["blink"] = m_mode.blink;
		mode["graph2Colors"] = m_mode.graph2Colors;
		mode["addressDataFlipFlop"] = m_mode.addressDataFlipFlop;
		to["mode"] = mode;

		to["xAxisDivider"] = m_xAxisDivider;
		to["lastDot"] = m_lastDot;

		m_crtc.Serialize(to["crtc"]);
	}

	void VideoPCjr::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoPCjr: Incompatible baseAddress");
		}

		if (from["id"] != "pcjr")
		{
			throw emul::SerializableException("VideoPCjr: Incompatible mode");
		}

		const json& pageReg = from["pageRegister"];
		m_pageRegister.crtPage = pageReg["crtPage"];
		m_pageRegister.cpuPage = pageReg["cpuPage"];
		m_pageRegister.videoAddressMode = pageReg["videoAddressMode"];
		m_pageRegister.addressMode = pageReg["addressMode"];
		m_pageRegister.crtBaseAddress = pageReg["crtBaseAddress"];
		m_pageRegister.cpuBaseAddress = pageReg["cpuBaseAddress"];

		const json& mode = from["mode"];
		m_mode.currRegister = mode["currRegister"];
		m_mode.paletteRegister = mode["paletteRegister"];
		m_mode.borderColor = mode["borderColor"];
		m_mode.paletteMask = mode["paletteMask"];
		m_mode.hiBandwidth = mode["hiBandwidth"];
		m_mode.graphics = mode["graphics"];
		m_mode.monochrome = mode["monochrome"];
		m_mode.enableVideo = mode["enableVideo"];
		m_mode.graph16Colors = mode["graph16Colors"];
		m_mode.blink = mode["blink"];
		m_mode.graph2Colors = mode["graph2Colors"];
		m_mode.addressDataFlipFlop = mode["addressDataFlipFlop"];

		m_xAxisDivider = from["xAxisDivider"];
		m_lastDot = from["lastDot"];

		m_crtc.Deserialize(from["crtc"]);

		MapB800Window();
		OnNewFrame();
	}
}
