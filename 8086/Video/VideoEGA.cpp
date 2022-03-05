#include "VideoEGA.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;
using emul::ADDRESS;

using crtc_ega::CRTController;
using crtc_ega::CRTCConfig;
using crtc_ega::CRTCData;

using memory_ega::RAMSIZE;
using memory_ega::MemoryEGA;

using graph_ega::GraphControllerData;
using graph_ega::ALUFunction;

using attr_ega::AttrController;
using attr_ega::AttrControllerAddress;
using attr_ega::RegisterMode;
using attr_ega::PaletteSource;

using seq_ega::SequencerData;

namespace video
{
	BYTE MakeColor8(bool h, bool l)
	{
		BYTE b = (h * 2) + l;
		return b * 85;
	}

	uint32_t RGB6toARGB32(BYTE value)
	{
		BYTE a = 0xFF;
		BYTE r = MakeColor8(GetBit(value, 2), GetBit(value, 5));
		BYTE g = MakeColor8(GetBit(value, 1), GetBit(value, 4));
		BYTE b = MakeColor8(GetBit(value, 0), GetBit(value, 3));

		return (a << 24) | (r << 16) | (g << 8) | b;
	}

	uint32_t RGB4toARGB32(BYTE value)
	{
		// Extend green secondary (=Intensity in 4 bit color mode) to all secondary bits
		if (value & 0x10)
		{
			value |= 0x38;
		} 
		else if (value == 6)
		{
			// Yellow adjust
			value = 20;
		}

		return RGB6toARGB32(value);
	}

	VideoEGA::VideoEGA(RAMSIZE ramsize, WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor) :
		Logger("EGA"),
		m_crtc(baseAddressMono),
		m_sequencer(baseAddress),
		m_graphController(baseAddress),
		m_ramSize(ramsize),
		m_baseAddress(baseAddress),
		m_baseAddressMono(baseAddressMono),
		m_baseAddressColor(baseAddressColor),
		m_egaROM("EGABIOS", 16384, emul::MemoryType::ROM),
		m_egaRAM(ramsize)
	{
	}

	void VideoEGA::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		// charROM not used, part of the BIOS

		m_egaROM.LoadFromFile("data/XT/EGA_6277356_C0000.BIN");
		memory->Allocate(&m_egaROM, 0xC0000);

		// TODO: Check if other ports don't have A0 decoded. 
		// AttributeController (0x3C0) is confirmed because BIOS POST writes in it

		Connect(m_baseAddress + 0x0, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteAttributeController));
		Connect(m_baseAddress + 0x1, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteAttributeController));

		Connect(m_baseAddress + 0x2, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteMiscRegister));
		Connect(m_baseAddress + 0x2, static_cast<PortConnector::INFunction>(&VideoEGA::ReadStatusRegister0));

		ConnectRelocatablePorts(m_baseAddressMono);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		Video::Init(memory, charROM, forceMono);

		// Normally max y is nnn but leave some room for custom crtc programming
		InitFrameBuffer(2048, 600);

		AddMode("text", (DrawFunc)&VideoEGA::DrawTextMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		AddMode("graph", (DrawFunc)&VideoEGA::DrawGraphMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		AddMode("graphCGA4", (DrawFunc)&VideoEGA::DrawGraphModeCGA4, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		SetMode("text");

		m_sequencer.Init();
		m_graphController.Init(memory, &m_egaRAM);

		m_egaRAM.SetGraphController(&m_graphController.GetData());
		m_egaRAM.SetSequencer(&m_sequencer.GetData());
	}	

	void VideoEGA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		m_sequencer.EnableLog(minSev);
		m_graphController.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	SDL_Rect VideoEGA::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hBlankMax - border - 1) * xMultiplier);
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

		return rect;
	}

	void VideoEGA::OnChangeMode()
	{
		if (m_sequencer.GetData().memoryMode.alpha)
		{
			SetMode("text");
		}
		else
		{			
			SetMode(m_graphController.GetData().shiftRegister ? "graphCGA4" : "graph");
		}
	}

	void VideoEGA::OnRenderFrame()
	{
		Video::RenderFrame();
	}

	void VideoEGA::Tick()
	{
		if (!m_crtc.IsInit())
		{
			return;
		}

		if (m_misc.clockSel == MISCRegister::ClockSelect::CLK_16)
		{
			static uint32_t tick16 = 0;
			// Add two ticks every 15 ticks to approximate 16 mhz clock
			// Gives 16.227MHz instead of 16.257Mhz (0.2% off)
			if (++tick16 == 15)
			{
				tick16 = 0;
				InternalTick();
				InternalTick();
			}
		}

		InternalTick();
	}

	void VideoEGA::InternalTick()
	{
		if (m_sequencer.GetData().clockingMode.halfDotClock)
		{
			static bool div2 = false;
			div2 = !div2;
			if (div2)
			{
				return;
			}
		}

		Draw();

		m_crtc.Tick();
	}

	void VideoEGA::OnNewFrame()
	{
		BeginFrame();
		const struct SequencerData& seq = m_sequencer.GetData();
		m_egaRAM.SelectCharMaps(seq.charMapSelectA, seq.charMapSelectA);
	}

	void VideoEGA::OnEndOfRow()
	{
		NewLine();
	}

	bool VideoEGA::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();
		const struct CRTCData& data = m_crtc.GetData();

		return (data.memoryAddress == config.cursorAddress) && m_crtc.IsBlink8();
	}

	void VideoEGA::DisconnectRelocatablePorts(WORD base)
	{
		LogPrintf(Logger::LOG_INFO, "DisconnectRelocatablePorts, base=%04Xh", base);

		DisconnectOutput(base + 0xA);
		DisconnectInput(base + 0xA);
	}

	void VideoEGA::ConnectRelocatablePorts(WORD base)
	{
		LogPrintf(Logger::LOG_INFO, "ConnectRelocatablePorts, base=%04Xh", base);

		Connect(base + 0xA, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteFeatureControlRegister));
		Connect(base + 0xA, static_cast<PortConnector::INFunction>(&VideoEGA::ReadStatusRegister1));
	}

	void VideoEGA::WriteMiscRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteMiscRegister, value=%02Xh", value);

		bool oldColor = m_misc.color;

		m_misc.color = GetBit(value, 0);
		m_misc.enableRAM = GetBit(value, 1);
		m_misc.disableVideo = GetBit(value, 4);
		m_misc.pageHigh = GetBit(value, 5);
		m_misc.hSyncPolarity = !GetBit(value, 6);
		m_misc.vSyncPolarity = !GetBit(value, 7);

		const char* clockSelStr = "";
		switch ((value >> 2) & 3)
		{
		case 0:
			m_misc.clockSel = MISCRegister::ClockSelect::CLK_14;
			clockSelStr = "14MHz";
			break;
		case 1:
			m_misc.clockSel = MISCRegister::ClockSelect::CLK_16;
			clockSelStr = "16MHz";
			break;
		case 2:
			m_misc.clockSel = MISCRegister::ClockSelect::CLK_EXT;
			clockSelStr = "External";
			break;
		case 3:
			m_misc.clockSel = MISCRegister::ClockSelect::CLK_UNUSED;
			clockSelStr = "Unused";
			break;
		}

		LogPrintf(Logger::LOG_INFO, "WriteMiscRegister [%cCOLOR %cRAM %cVIDEO %cPAGEBIT %cHSYNCPOL %cVSYNCPOL CLK[%s]]",			
			m_misc.color ? ' ' : '/',
			m_misc.enableRAM ? ' ' : '/',
			!m_misc.disableVideo ? ' ' : '/',
			m_misc.pageHigh ? ' ' : '/',
			m_misc.hSyncPolarity ? '+' : '-',
			m_misc.vSyncPolarity ? '+' : '-',
			clockSelStr);

		if (oldColor != m_misc.color)
		{
			DisconnectRelocatablePorts(oldColor ? m_baseAddressColor : m_baseAddressMono);
			ConnectRelocatablePorts(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.SetBasePort(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
		}

		m_egaRAM.Enable(m_misc.enableRAM);
	}

	void VideoEGA::WriteFeatureControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteFeatureControlRegister, value=%02Xh", value);
		// Unused
	}

	BYTE VideoEGA::ReadStatusRegister0()
	{
		BYTE value = 0xFF;

		// Bit4: Dip switch sense. Use clksel to determine switch to read
		// Bit7: CRT Interrupt. Not clear on 0/1 (docs contradict) but vsync
		// interrupt is not really used in the real world

		// CLKSEL is used to determine which swich to read;
		// Need to invert logic, off = opened swich = 5V = logical 1
		bool switchSense = !m_dipSwitches[(int)m_misc.clockSel];

		value =
			(switchSense << 4) |
			(m_crtc.IsInterruptPending() << 7);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister0, value=%02Xh", value);

		return value;
	}

	BYTE VideoEGA::ReadStatusRegister1() 
	{
		m_attr.ResetMode();

		// Bit0: 1:Display cpu access enabled (vsync | hsync)
		// 
		// Light pen, not implemented
		// Bit1: 1:Light pen trigger set
		// Bit2: 1:Light pen switch off, 0: switch on
		//
		// Bit3 1:Vertical retrace active
		//
		// Bit4&5:Pixel data, selected by Color Plane Register

		bool diag4 = false;
		bool diag5 = false;
		uint32_t lasDot = GetLastDot();
		switch (m_attr.videoStatusMux)
		{
		case 0:
			diag5 = GetBit(lasDot, 23); // REDh
			diag4 = GetBit(lasDot, 7); // BLUEh
			break;
		case 1:
			diag5 = GetBit(lasDot, 6); // BLUEl
			diag4 = GetBit(lasDot, 15); // GREENh
			break;
		case 2:
			diag5 = GetBit(lasDot, 22); // REDl
			diag4 = GetBit(lasDot, 14); // GREENl
			break;
		}

		BYTE status =
			((!IsDisplayArea()) << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(IsVSync() << 3) |
			(diag4 << 4) |
			(diag5 << 5);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister1, value=%02Xh", status);

		return status;
	}

	void VideoEGA::WriteAttributeController(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteAttributeController, value=%02x", value);
		if (m_attr.currMode == RegisterMode::ADDRESS)
		{
			//ATTR_PALETTE_MAX
			value &= 31;
			m_attr.currRegister = ((AttrControllerAddress)value > AttrControllerAddress::_ATTR_MAX) ? AttrControllerAddress::ATTR_INVALID : (AttrControllerAddress)value;

			m_attr.currMode = RegisterMode::DATA;

			m_attr.paletteSource = GetBit(value, 5) ? PaletteSource::VIDEO : PaletteSource::CPU;

		}
		else // DATA
		{
			if (m_attr.currRegister <= AttrControllerAddress::ATTR_PALETTE_MAX)
			{
				if (m_attr.paletteSource == PaletteSource::CPU)
				{
					BYTE index = (BYTE)m_attr.currRegister;
					LogPrintf(LOG_INFO, "WriteAttributeController, Palette[%d]=%d", index, value);
					m_attr.palette[index] = (GetColorMode() == ColorMode::RGB4) ? RGB4toARGB32(value) : RGB6toARGB32(value);
				}
				else
				{
					LogPrintf(LOG_WARNING, "WriteAttributeController, Trying to set palette with source=VIDEO");
				}
			}
			else switch (m_attr.currRegister)
			{
			case AttrControllerAddress::ATTR_MODE_CONTROL:
				LogPrintf(LOG_DEBUG, "WriteAttributeController, Mode Control %d", value);	
				
				// TODO
				m_attr.graphics = GetBit(value, 0);
				m_attr.monochrome = GetBit(value, 1);
				m_attr.extend8to9 = GetBit(value, 2);
				m_attr.blink = GetBit(value, 3);

				LogPrintf(Logger::LOG_INFO, "WriteAttributeController Mode control [%cGRAPH %cMONO %cEXTEND8TO9, %cBLINK]",
					m_attr.graphics ? ' ' : '/',
					m_attr.monochrome ? ' ' : '/',
					m_attr.extend8to9 ? ' ' : '/',
					m_attr.blink ? ' ' : '/');
				break;
			case AttrControllerAddress::ATTR_OVERSCAN_COLOR:
				LogPrintf(LOG_DEBUG, "WriteAttributeController, Overscan Color %d", value);
				m_attr.overscanColor = (GetColorMode() == ColorMode::RGB4) ? RGB4toARGB32(value) : RGB6toARGB32(value);
				break;
			case AttrControllerAddress::ATTR_COLOR_PLANE_EN:
				LogPrintf(LOG_DEBUG, "WriteAttributeController, Color Plane Enable %d", value);

				m_attr.colorPlaneEnable = value & 15;
				LogPrintf(LOG_INFO, "WriteAttributeController, Color Plane Enable %02x", m_attr.colorPlaneEnable);

				m_attr.videoStatusMux = (value >> 4) & 3;
				LogPrintf(LOG_INFO, "WriteAttributeController, Video Status Mux %02x", m_attr.videoStatusMux);
				break;
			case AttrControllerAddress::ATTR_H_PEL_PANNING:
				m_attr.hPelPanning = value & 15;
				LogPrintf(LOG_INFO, "WriteAttributeController, Horizontal Pel Panning %d", m_attr.hPelPanning);
				break;
			default:
				LogPrintf(LOG_ERROR, "WriteAttributeController, Invalid Address %d", m_attr.currRegister);
			}

			m_attr.currMode = RegisterMode::ADDRESS;
		}
	}

	void VideoEGA::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			bool isCursorChar = IsCursor();
			BYTE ch = m_egaRAM.readRaw(0, base);
			BYTE attr = m_egaRAM.readRaw(1, base);

			BYTE bg = attr >> 4;
			BYTE fg = attr & 0x0F;
			bool charBlink = false;

			// Background
			if (m_attr.blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = GetBit(bg, 3);
				SetBit(bg, 3, false);
			}

			// Draw character
			BYTE currChar = m_egaRAM.GetCharMapA()[((size_t)ch * 0x20) + data.rowAddress];
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			// TODO: This is not the correct behavior
			bool cursorLine = isCursorChar && (data.rowAddress > config.cursorStart);
			if (config.cursorEnd && (data.rowAddress > config.cursorEnd))
			{
				// TODO: handles (wrongly) cursorEnd == 0
				cursorLine = false;
			}

			// TODO: >8px width
			int startBit = 7;
			int endBit = 0;
			if (data.hPos == 0)
			{
				startBit -= m_attr.hPelPanning;
			}
			else if (data.hPos == data.hTotalDisp)
			{
				endBit = 8 - m_attr.hPelPanning;
			}

			for (int i = startBit; i >= endBit; --i)
			{
				bool set = draw && GetBit(currChar, i);
				DrawPixel(GetColor((set || cursorLine) ? fg : bg));
			}
		}
		else
		{
			DrawBackground(8);
		}

	}

	void VideoEGA::DrawGraphModeCGA4()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels

		// TODO: Pel panning is ignored here
		if (IsDisplayArea() && IsEnabled() && (data.hPos < data.hTotalDisp))
		{
			ADDRESS base = GetAddress();
			BYTE pixData[4];
			for (int i = 0; i < 4; ++i)
			{
				pixData[i] = GetBit(m_attr.colorPlaneEnable, i) ? m_egaRAM.readRaw(i, base) : 0;
			}

			for (int i = 0; i < 2; ++i)
			{
				for (int j = 3; j >= 0; --j)
				{
					BYTE color =
						(GetBit(pixData[i],     (j * 2))     << 0) |
						(GetBit(pixData[i],     (j * 2) + 1) << 1) |
						(GetBit(pixData[i + 2], (j * 2))     << 2) |
						(GetBit(pixData[i + 2], (j * 2) + 1) << 3);
					DrawPixel(GetColor(color));
				}
			}
		}
		else
		{
			DrawBackground(8);
		}
	}

	void VideoEGA::DrawGraphMode()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();
			BYTE pixData[4];
			for (int i = 0; i < 4; ++i)
			{
				pixData[i] = GetBit(m_attr.colorPlaneEnable, i) ?  m_egaRAM.readRaw(i, base) : 0;
			}

			int startBit = 7;
			int endBit = 0;
			if (data.hPos == 0)
			{
				startBit -= m_attr.hPelPanning;
			}
			else if (data.hPos == data.hTotalDisp)
			{
				endBit = 8 - m_attr.hPelPanning;
			}

			for (int i = startBit; i >= endBit; --i)
			{
				BYTE color = 
					(GetBit(pixData[0], i) << 0) |
					(GetBit(pixData[1], i) << 1) |
					(GetBit(pixData[2], i) << 2) |
					(GetBit(pixData[3], i) << 3);

				// For graphics mode blink, flip ATR3 every 16 frames
				if (m_attr.blink && m_crtc.IsBlink16())
				{
					color ^= 0b1000;
				}
				DrawPixel(GetColor(color));
			}
		}
		else
		{
			DrawBackground(8);
		}
	}

	void VideoEGA::Serialize(json& to)
	{
		Video::Serialize(to);
		to["baseAddress"] = m_baseAddress;
		// TODO ram size
		// TODO color mono ports

		json misc;
		misc["color"] = m_misc.color;
		misc["enableRAM"] = m_misc.enableRAM;
		misc["clockSel"] = m_misc.clockSel;
		misc["disableVideo"] = m_misc.disableVideo;
		misc["pageHigh"] = m_misc.pageHigh;
		misc["hSyncPolarity"] = m_misc.hSyncPolarity;
		misc["vSyncPolarity"] = m_misc.vSyncPolarity;
		to["misc"] = misc;

		m_sequencer.Serialize(to["sequencer"]);
		m_graphController.Serialize(to["graphController"]);
		m_attr.Serialize(to["attrController"]);
		m_crtc.Serialize(to["crtc"]);
		m_egaRAM.Serialize(to["egaRAM"]);

		// TODO dip switches
	}

	void VideoEGA::Deserialize(json& from)
	{
		Video::Deserialize(from);
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoEGA: Incompatible baseAddress");
		}

		const json& misc = from["misc"];
		m_misc.color = misc["color"];
		m_misc.enableRAM = misc["enableRAM"];
		m_misc.clockSel = misc["clockSel"];
		m_misc.disableVideo = misc["disableVideo"];
		m_misc.pageHigh = misc["pageHigh"];
		m_misc.hSyncPolarity = misc["hSyncPolarity"];
		m_misc.vSyncPolarity = misc["vSyncPolarity"];

		m_sequencer.Deserialize(from["sequencer"]);
		m_graphController.Deserialize(from["graphController"]);
		m_attr.Deserialize(from["attrController"]);
		m_crtc.Deserialize(from["crtc"]);
		m_egaRAM.Deserialize(from["egaRAM"]);

		OnChangeMode();
		OnNewFrame();
	}
}
