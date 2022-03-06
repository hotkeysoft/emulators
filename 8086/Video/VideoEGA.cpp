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

using attr_ega::AttrControllerData;
using attr_ega::ColorMode;

using seq_ega::SequencerData;

namespace video
{
	VideoEGA::VideoEGA(const char* mode, RAMSIZE ramsize, WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor) :
		Logger("EGA"),
		m_crtc(baseAddressMono),
		m_sequencer(baseAddress),
		m_graphController(baseAddress),
		m_attrController(baseAddress),
		m_ramSize(ramsize),
		m_baseAddress(baseAddress),
		m_baseAddressMono(baseAddressMono),
		m_baseAddressColor(baseAddressColor),
		m_egaROM("EGABIOS", 16384, emul::MemoryType::ROM),
		m_egaRAM(ramsize)
	{
		assert(mode);

		LogPrintf(LOG_INFO, "VideoEGA, RAM Size = [%dK]", (int)ramsize / 1024);

		const auto& it = m_videoModes.find(mode);
		if (it == m_videoModes.end())
		{
			LogPrintf(LOG_ERROR, "VideoEGA, mode not found [%s], fallback to default mode [ega]", mode);
			m_videoMode = m_videoModes.at("ega");
		}
		else
		{
			m_videoMode = it->second;
			LogPrintf(LOG_INFO, "VideoEGA, mode=[%s]: %s", mode, m_videoMode.description);
		}
	}

	void VideoEGA::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		// charROM not used, part of the BIOS

		m_egaROM.LoadFromFile("data/XT/EGA_6277356_C0000.BIN");
		memory->Allocate(&m_egaROM, 0xC0000);

		Connect(m_baseAddress + 0x2, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteMiscRegister));
		Connect(m_baseAddress + 0x2, static_cast<PortConnector::INFunction>(&VideoEGA::ReadStatusRegister0));

		ConnectRelocatablePorts(m_baseAddressMono);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		Video::Init(memory, charROM, forceMono);

		// Normally max y is nnn but leave some room for custom crtc programming
		InitFrameBuffer(2048, 600);

		AddMode("text", (DrawFunc)&VideoEGA::DrawTextMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		AddMode("textMDA", (DrawFunc)&VideoEGA::DrawTextModeMDA, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		AddMode("graph", (DrawFunc)&VideoEGA::DrawGraphMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		AddMode("graphCGA4", (DrawFunc)&VideoEGA::DrawGraphModeCGA4, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		SetMode("text");

		m_sequencer.Init();
		m_graphController.Init(memory, &m_egaRAM);
		m_attrController.Init();

		m_egaRAM.SetGraphController(&m_graphController.GetData());
		m_egaRAM.SetSequencer(&m_sequencer.GetData());
	}	

	void VideoEGA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		m_sequencer.EnableLog(minSev);
		m_graphController.EnableLog(minSev);
		m_attrController.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	SDL_Rect VideoEGA::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		const struct CRTCData& crtcData = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (crtcData.hTotal - crtcData.hBlankMax - border - 1) * xMultiplier);
		rect.y = std::max(0, (crtcData.vTotal - crtcData.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (crtcData.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (crtcData.vTotalDisp + (2u * border)));

		return rect;
	}

	void VideoEGA::OnChangeMode()
	{	
		m_crtc.SetCharWidth(m_sequencer.GetData().clockingMode.charWidth);

		if (m_sequencer.GetData().memoryMode.alpha)
		{			
			SetMode(m_attrController.GetData().monochrome ? "textMDA" : "text");
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
		const struct CRTCConfig& crtcConfig = m_crtc.GetConfig();
		const struct CRTCData& crtcData = m_crtc.GetData();

		return (crtcData.memoryAddress == crtcConfig.cursorAddress) && m_crtc.IsBlink8();
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

		m_attrController.SetColorMode(m_misc.vSyncPolarity ? ColorMode::RGB4 : ColorMode::RGB6);

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
		// Need to invert logic, off = opened switch = 5V = logical 1
		// Neet to reverse switches order
		bool switchSense = !m_videoMode.dipSwitches[3-(int)m_misc.clockSel];

		value =
			(switchSense << 4) |
			(m_crtc.IsInterruptPending() << 7);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister0, value=%02Xh", value);

		return value;
	}

	BYTE VideoEGA::ReadStatusRegister1() 
	{
		m_attrController.ResetMode();

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
		switch (m_attrController.GetData().videoStatusMux)
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

	void VideoEGA::DrawTextMode()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct CRTCConfig& crtcConfig = m_crtc.GetConfig();
		const struct AttrControllerData attrData = m_attrController.GetData();

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
			if (m_attrController.GetData().blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = GetBit(bg, 3);
				SetBit(bg, 3, false);
			}

			// Draw character
			BYTE currChar = m_egaRAM.GetCharMapA()[((size_t)ch * 0x20) + crtcData.rowAddress];
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			// TODO: This is not the correct behavior
			bool cursorLine = isCursorChar && (crtcData.rowAddress > crtcConfig.cursorStart);
			if (crtcConfig.cursorEnd && (crtcData.rowAddress > crtcConfig.cursorEnd))
			{
				// TODO: handles (wrongly) cursorEnd == 0
				cursorLine = false;
			}

			int startBit = 7;
			int endBit = 0;
			if (crtcData.hPos == 0)
			{
				startBit -= attrData.hPelPanning;
			}
			else if (crtcData.hPos == crtcData.hTotalDisp)
			{
				endBit = 8 - attrData.hPelPanning;
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

	void VideoEGA::DrawTextModeMDA()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct CRTCConfig& crtcConfig = m_crtc.GetConfig();
		const struct AttrControllerData attrData = m_attrController.GetData();
		const struct SequencerData seqData = m_sequencer.GetData();

		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			bool isCursorChar = IsCursor();
			BYTE ch = m_egaRAM.readRaw(0, base);
			BYTE attr = m_egaRAM.readRaw(1, base);

			bool blinkBit = GetBit(attr, 7);
			bool charBlink = attrData.blink && blinkBit;
			bool charUnderline = ((attr & 0b111) == 1); // Underline when attr[2:0] == b001

			const BYTE bgFgMask = 0b01110111; // fg and bg without intensity and blink bits
			bool charReverse = ((attr & bgFgMask) == 0b01110000);

			BYTE fg = (attr & bgFgMask) ? (0b111 | (attr & 0b1000)) : 0b000; // Anything that's not black is on
			fg ^= charReverse ? 0b111 : 0b000; // Flip for reverse video

			BYTE bg = (charReverse || (!attrData.blink && blinkBit)) ? 0xF : 0;

			uint32_t fgRGB = GetColor(fg);
			uint32_t bgRGB = GetColor(bg);

			// Draw character
			BYTE currChar = m_egaRAM.GetCharMapA()[((size_t)ch * 0x20) + crtcData.rowAddress];
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			bool underline = draw && (charUnderline & (crtcData.rowAddress == crtcConfig.underlineLocation));

			// TODO: This is not the correct behavior
			bool cursorLine = isCursorChar && (crtcData.rowAddress > crtcConfig.cursorStart);
			if (crtcConfig.cursorEnd && (crtcData.rowAddress > crtcConfig.cursorEnd))
			{
				// TODO: handles (wrongly) cursorEnd == 0
				cursorLine = false;
			}

			// TODO: Pel panning
			if (crtcData.hPos == crtcData.hTotalDisp)
			{
				DrawBackground(seqData.clockingMode.charWidth);
				return;
			}

			bool lastDot = false;
			for (int x = 0; x < seqData.clockingMode.charWidth; ++x)
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
			DrawBackground(seqData.clockingMode.charWidth);
		}
	}

	void VideoEGA::DrawGraphModeCGA4()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct AttrControllerData attrData = m_attrController.GetData();

		// Called every 8 horizontal pixels

		// TODO: Pel panning is ignored here
		if (IsDisplayArea() && IsEnabled() && (crtcData.hPos < crtcData.hTotalDisp))
		{
			ADDRESS base = GetAddress();
			BYTE pixData[4];
			for (int i = 0; i < 4; ++i)
			{
				pixData[i] = GetBit(attrData.colorPlaneEnable, i) ? m_egaRAM.readRaw(i, base) : 0;
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
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct AttrControllerData attrData = m_attrController.GetData();

		// Called every 8 horizontal pixels
		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();
			BYTE pixData[4];
			for (int i = 0; i < 4; ++i)
			{
				pixData[i] = GetBit(attrData.colorPlaneEnable, i) ?  m_egaRAM.readRaw(i, base) : 0;
			}

			int startBit = 7;
			int endBit = 0;
			if (crtcData.hPos == 0)
			{
				startBit -= attrData.hPelPanning;
			}
			else if (crtcData.hPos == crtcData.hTotalDisp)
			{
				endBit = 8 - attrData.hPelPanning;
			}

			for (int i = startBit; i >= endBit; --i)
			{
				BYTE color = 
					(GetBit(pixData[0], i) << 0) |
					(GetBit(pixData[1], i) << 1) |
					(GetBit(pixData[2], i) << 2) |
					(GetBit(pixData[3], i) << 3);

				// For graphics mode blink, flip ATR3 every 16 frames
				if (attrData.blink && m_crtc.IsBlink16())
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
		m_attrController.Serialize(to["attrController"]);
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
		m_attrController.Deserialize(from["attrController"]);
		m_crtc.Deserialize(from["crtc"]);
		m_egaRAM.Deserialize(from["egaRAM"]);

		OnChangeMode();
		OnNewFrame();
	}
}
