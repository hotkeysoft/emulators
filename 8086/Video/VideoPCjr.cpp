#include "VideoPCjr.h"
#include <assert.h>

using emul::Memory;
using emul::MemoryBlock;
using emul::GetBit;
using emul::SetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	VideoPCjr::VideoPCjr(WORD baseAddress) :
		Logger("vidPCjr"),
		Video6845(baseAddress),
		m_baseAddress(baseAddress),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
	}

	void VideoPCjr::Init(emul::Memory* memory, const char* charROM, bool)
	{
		assert(memory);
		m_memory = memory;

		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(4096 + 2048);

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

		Video6845::Init(memory, charROM);

		// Normally max y is 262 but leave some room for custom crtc programming
		Video::InitFrameBuffer(2048, 300);

		AddMode("text", (DrawFunc)&VideoPCjr::DrawTextMode, (AddressFunc)&VideoPCjr::GetBaseAddressText, (ColorFunc)&VideoPCjr::GetIndexedColor16);
		AddMode("320x200x4", (DrawFunc)&VideoPCjr::Draw320x200x4, (AddressFunc)&VideoPCjr::GetBaseAddressGraph, (ColorFunc)&VideoPCjr::GetIndexedColor16);
		AddMode("640x200x2", (DrawFunc)&VideoPCjr::Draw640x200x2, (AddressFunc)&VideoPCjr::GetBaseAddressGraph, (ColorFunc)&VideoPCjr::GetIndexedColor16);
		AddMode("640x200x4", (DrawFunc)&VideoPCjr::Draw640x200x4, (AddressFunc)&VideoPCjr::GetBaseAddressGraph, (ColorFunc)&VideoPCjr::GetIndexedColor16);
		AddMode("x200x16", (DrawFunc)&VideoPCjr::Draw200x16, (AddressFunc)&VideoPCjr::GetBaseAddressGraph, (ColorFunc)&VideoPCjr::GetIndexedColor16);

		SetMode("text");
	}

	SDL_Rect VideoPCjr::GetDisplayRect(BYTE border, WORD) const
	{
		uint16_t xMultiplier = (m_mode.graphics && m_mode.graph2Colors) ? 2 : 1;

		return Video6845::GetDisplayRect(border, xMultiplier);
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
		case 3:
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

				OnChangeMode();
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

				OnChangeMode();
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
		bool dot = (GetLastDot() & (1 << (m_mode.currRegister & 3)));

		BYTE status =
			(IsDisplayArea() << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(IsVSync() << 3) |
			(dot << 4); // Video dots

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoPCjr::OnChangeMode()
	{
		GetCRTC().SetCharWidth(m_mode.graph16Colors ? 4 : 8);

		//// Select draw function
		if (!m_mode.graphics)
		{
			SetMode("text");
		}
		else if (m_mode.graph2Colors)
		{
			SetMode("640x200x2");
		}
		else if (m_mode.graph16Colors)
		{
			SetMode("x200x16");
		}
		else if (m_mode.hiBandwidth)
		{
			SetMode("640x200x4");
		}
		else
		{
			SetMode("320x200x4");
		}
	}

	void VideoPCjr::Tick()
	{
		if (!m_mode.hiBandwidth)
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

	void VideoPCjr::DrawTextMode()
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
			BYTE currChar = m_charROMStart[((size_t)ch * 8) + data.rowAddress];
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

	void VideoPCjr::Serialize(json& to)
	{
		Video6845::Serialize(to);
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
	}

	void VideoPCjr::Deserialize(json& from)
	{
		Video6845::Deserialize(from);

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

		MapB800Window();
		OnChangeMode();
		OnNewFrame();
	}
}
