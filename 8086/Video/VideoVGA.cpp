#include "stdafx.h"

#include "VideoVGA.h"
#include <Config.h>

using cfg::CONFIG;

using emul::SetBit;
using emul::GetBit;
using emul::ADDRESS;

using crtc_vga::CRTController;
using crtc_vga::CRTCConfig;
using crtc_vga::CRTCData;

using memory_vga::RAMSIZE;
using memory_vga::MemoryVGA;

using graph_vga::GraphControllerData;
using graph_vga::ALUFunction;

using attr_vga::AttrControllerData;

using seq_vga::SequencerData;

namespace video
{
	VideoVGA::VideoVGA(WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor) :
		Logger("VGA"),
		m_crtc(baseAddressMono),
		m_sequencer(baseAddress),
		m_graphController(baseAddress),
		m_attrController(baseAddress),
		m_dac(baseAddress),
		m_baseAddress(baseAddress),
		m_baseAddressMono(baseAddressMono),
		m_baseAddressColor(baseAddressColor),
		m_vgaROM("VGABIOS", 0x8000, emul::MemoryType::ROM),
		m_vgaRAM()
	{
		EnableLog(LOG_INFO);
	}

	void VideoVGA::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		// charROM not used, part of the BIOS

		m_vgaROM.LoadFromFile("data/XT/VGA_674008222554_C000.BIN");
		memory->Allocate(&m_vgaROM, 0xC0000);

		m_crtc.Init();
		m_crtc.SetVidEventHandler(this);

		Video::Init(memory, charROM, forceMono);

		// Normally max y is nnn but leave some room for custom crtc programming
		InitFrameBuffer(2048, 600);

		AddMode("text", (DrawFunc)&VideoVGA::DrawTextMode, (AddressFunc)&VideoVGA::GetBaseAddress, (ColorFunc)&VideoVGA::GetIndexedColor);
		AddMode("textMDA", (DrawFunc)&VideoVGA::DrawTextModeMDA, (AddressFunc)&VideoVGA::GetBaseAddress, (ColorFunc)&VideoVGA::GetIndexedColor);
		AddMode("graph", (DrawFunc)&VideoVGA::DrawGraphMode, (AddressFunc)&VideoVGA::GetBaseAddress, (ColorFunc)&VideoVGA::GetIndexedColor);
		AddMode("graphCGA4", (DrawFunc)&VideoVGA::DrawGraphModeCGA4, (AddressFunc)&VideoVGA::GetBaseAddress, (ColorFunc)&VideoVGA::GetIndexedColor);
		AddMode("graph256", (DrawFunc)&VideoVGA::DrawGraphMode256, (AddressFunc)&VideoVGA::GetBaseAddress, (ColorFunc)&VideoVGA::GetColor256);
		SetMode("text");

		m_sequencer.Init();
		m_sequencer.SetVidEventHandler(this);

		m_graphController.Init(memory, &m_vgaRAM);
		m_graphController.SetVidEventHandler(this);

		m_attrController.Init();

		m_vgaRAM.SetGraphController(&m_graphController.GetData());
		m_vgaRAM.SetSequencer(&m_sequencer.GetData());

		// The two ports below are always connected. The others are enabled/disabled dynamically
		// Depending on the global enable register
		//
		// (The null odd ports handlers are there because the bios does 16 bit out, this avoids
		// unallocated ports warnings in the logs)

		// Setup Control Register
		Connect(0x46E8, static_cast<PortConnector::OUTFunction>(&VideoVGA::WriteSetupRegister));
		Connect(0x46E9, static_cast<PortConnector::OUTFunction>(&VideoVGA::WriteNull));

		// Global Enable Register
		Connect(0x102, static_cast<PortConnector::OUTFunction>(&VideoVGA::WriteGlobalEnableRegister));
		Connect(0x103, static_cast<PortConnector::OUTFunction>(&VideoVGA::WriteNull));
	}

	void VideoVGA::DisconnectPorts()
	{
		// Misc Register
		DisconnectOutput(m_baseAddress + 0x2);
		DisconnectInput(m_baseAddress + 0xC);

		// Status Register
		DisconnectInput(m_baseAddress + 0x2);

		DisconnectRelocatablePorts(m_baseAddressMono);
		DisconnectRelocatablePorts(m_baseAddressColor);

		m_crtc.DisconnectPorts();
		m_sequencer.DisconnectPorts();
		m_graphController.DisconnectPorts();
		m_attrController.DisconnectPorts();
		m_dac.DisconnectPorts();
	}

	void VideoVGA::ConnectPorts()
	{
		if (m_globalEnable && m_setup.enable)
		{
			LogPrintf(LOG_INFO, "Connecting VGA IO Ports");

			// Misc Register
			Connect(m_baseAddress + 0x2, static_cast<PortConnector::OUTFunction>(&VideoVGA::WriteMiscRegister));
			Connect(m_baseAddress + 0xC, static_cast<PortConnector::INFunction>(&VideoVGA::ReadMiscRegister));

			// Status Register
			Connect(m_baseAddress + 0x2, static_cast<PortConnector::INFunction>(&VideoVGA::ReadStatusRegister0));

			ConnectRelocatablePorts(m_misc.color ? m_baseAddressColor : m_baseAddressMono);

			m_crtc.SetBasePort(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.ConnectPorts();
			m_sequencer.ConnectPorts();
			m_graphController.ConnectPorts();
			m_attrController.ConnectPorts();
			m_dac.ConnectPorts();
		}
	}

	void VideoVGA::DisconnectRelocatablePorts(WORD base)
	{
		LogPrintf(Logger::LOG_DEBUG, "DisconnectRelocatablePorts, base=%04Xh", base);

		DisconnectOutput(base + 0xA);
		DisconnectInput(base + 0xA);
	}

	void VideoVGA::ConnectRelocatablePorts(WORD base)
	{
		LogPrintf(Logger::LOG_DEBUG, "ConnectRelocatablePorts, base=%04Xh", base);

		Connect(base + 0xA, static_cast<PortConnector::OUTFunction>(&VideoVGA::WriteFeatureControlRegister));
		Connect(base + 0xA, static_cast<PortConnector::INFunction>(&VideoVGA::ReadStatusRegister1));
	}

	void VideoVGA::Reset()
	{
		Video::Reset();

		m_framePelPanning = 0;
		m_setup.enable = false;
		m_setup.setupMode = false;
		m_globalEnable = false;

		m_misc.color = false;
		m_misc.enableRAM = false;
		m_misc.clockSel = MISCRegister::ClockSelect::CLK_25;
		m_misc.pageHigh = false;
		m_misc.hSyncPolarity = false;
		m_misc.vSyncPolarity = false;

		m_crtc.Reset();
		m_crtc.SetBasePort(m_baseAddressMono);
		m_sequencer.Reset();
		m_graphController.Reset();
		m_attrController.Reset();
		m_dac.Reset();

		DisconnectPorts();
		SetMode("text");
	}

	void VideoVGA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		m_sequencer.EnableLog(minSev);
		m_graphController.EnableLog(minSev);
		m_attrController.EnableLog(minSev);
		m_dac.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	SDL_Rect VideoVGA::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		const struct CRTCData& crtcData = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (crtcData.hTotal - crtcData.hBlankMax - border - 1) * xMultiplier);
		rect.y = std::max(0, (crtcData.vTotal - crtcData.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (crtcData.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (crtcData.vTotalDisp + (2u * border)));

		return rect;
	}

	void VideoVGA::OnChangeMode()
	{
		m_crtc.SetCharWidth(m_sequencer.GetData().clockingMode.charWidth);

		if (!m_attrController.GetData().graphics)
		{
			SetMode(m_attrController.GetData().monochrome ? "textMDA" : "text");
		}
		else if (m_graphController.GetData().color256)
		{
			SetMode("graph256");
			m_crtc.SetCharWidth(4);
		}
		else
		{

			SetMode(m_graphController.GetData().shiftRegister ? "graphCGA4" : "graph");
		}
	}

	void VideoVGA::OnRenderFrame()
	{
		Video::RenderFrame();
	}

	void VideoVGA::Tick()
	{
		if (!m_crtc.IsInit())
		{
			return;
		}

		if (m_misc.clockSel == MISCRegister::ClockSelect::CLK_25)
		{
			// 25.175MHz clock
			// Double ticks, skip one out of four
			// Gives 25.056MHz (0.5% off)
			// TODO: Improve this
			static uint32_t tick4 = 0;
			if (++tick4 == 4)
			{
				tick4 = 0;
			}
			else
			{
				InternalTick();
			}
		}
		else if (m_misc.clockSel == MISCRegister::ClockSelect::CLK_28)
		{
			// 28.322MHz clock
			// Double ticks
			// Gives 28.636MHz (1% off)
			// TODO: Improve this
			InternalTick();
		}

		InternalTick();
	}

	void VideoVGA::InternalTick()
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

	void VideoVGA::OnNewFrame()
	{
		BeginFrame();
		const struct SequencerData& seq = m_sequencer.GetData();
		m_vgaRAM.SelectCharMaps(seq.charMapSelectA, seq.charMapSelectA);

		m_framePelPanning = AdjustPelPanning(m_attrController.GetData().hPelPanning);
	}

	void VideoVGA::OnEndOfRow()
	{
		NewLine();
	}

	BYTE VideoVGA::AdjustPelPanning(BYTE pelPan) const
	{
		const auto& graphData = m_graphController.GetData();
		const auto& seqClocking = m_sequencer.GetData().clockingMode;

		if (graphData.color256)
		{
			return  (pelPan & 7) / 2; // 0..3
		}
		else if (!graphData.graphics && seqClocking.charWidth == 9)
		{
			return (pelPan >= 8) ? 0 : pelPan + 1;
		}
		else
		{
			return pelPan & 7;
		}
	}

	bool VideoVGA::IsCursor() const
	{
		const struct CRTCConfig& crtcConfig = m_crtc.GetConfig();
		const struct CRTCData& crtcData = m_crtc.GetData();

		return (crtcData.memoryAddress == crtcConfig.cursorAddress) && m_crtc.IsBlink8();
	}

	void VideoVGA::WriteSetupRegister(BYTE value)
	{
		bool oldEnable = m_setup.enable;

		m_setup.enable = GetBit(value, 3);
		m_setup.setupMode = GetBit(value, 4);
		LogPrintf(LOG_WARNING, "WriteSetupRegister [%cSETUP %cENABLE]",
			m_setup.setupMode ? ' ' : '/',
			m_setup.enable ? ' ' : '/');

		if (oldEnable && !m_setup.enable)
		{
			DisconnectPorts();
		}
		else if (!oldEnable && m_setup.enable)
		{
			ConnectPorts();
		}
	}

	void VideoVGA::WriteGlobalEnableRegister(BYTE value)
	{
		// Only answers in setup mode
		if (!m_setup.setupMode)
			return;

		bool oldEnable = m_globalEnable;
		m_globalEnable = GetBit(value, 0);
		LogPrintf(LOG_INFO, "WriteGlobalEnableRegister [%s]", m_globalEnable ? "ENABLE":"DISABLE");

		if (oldEnable && !m_globalEnable)
		{
			DisconnectPorts();
		}
		else if (!oldEnable && m_globalEnable)
		{
			ConnectPorts();
		}
	}

	BYTE VideoVGA::ReadMiscRegister()
	{
		BYTE value =
			(m_misc.color << 0) |
			(m_misc.enableRAM << 1) |
			((BYTE)(m_misc.clockSel) << 2) |
			(m_misc.pageHigh << 5) |
			(m_misc.hSyncPolarity << 6) |
			(m_misc.vSyncPolarity << 7);

		LogPrintf(Logger::LOG_DEBUG, "ReadMiscRegister, value=%02Xh", value);
		return value;
	}

	void VideoVGA::WriteMiscRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteMiscRegister, value=%02Xh", value);

		bool oldColor = m_misc.color;

		m_misc.color = GetBit(value, 0);
		m_misc.enableRAM = GetBit(value, 1);
		m_misc.pageHigh = GetBit(value, 5);
		m_misc.hSyncPolarity = !GetBit(value, 6);
		m_misc.vSyncPolarity = !GetBit(value, 7);

		// TODO: Still needed in VGA?
		// m_attrController.SetColorMode(m_misc.vSyncPolarity ? ColorMode::RGB4 : ColorMode::RGB6);

		const char* clockSelStr = "";
		switch ((value >> 2) & 3)
		{
		case 0:
			m_misc.clockSel = MISCRegister::ClockSelect::CLK_25;
			clockSelStr = "25MHz";
			break;
		case 1:
			m_misc.clockSel = MISCRegister::ClockSelect::CLK_28;
			clockSelStr = "28MHz";
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

		LogPrintf(Logger::LOG_INFO, "WriteMiscRegister [%cCOLOR %cRAM %cPAGEBIT %cHSYNCPOL %cVSYNCPOL CLK[%s]]",
			m_misc.color ? ' ' : '/',
			m_misc.enableRAM ? ' ' : '/',
			m_misc.pageHigh ? ' ' : '/',
			m_misc.hSyncPolarity ? '+' : '-',
			m_misc.vSyncPolarity ? '+' : '-',
			clockSelStr);

		if (oldColor != m_misc.color)
		{
			DisconnectRelocatablePorts(oldColor ? m_baseAddressColor : m_baseAddressMono);
			ConnectRelocatablePorts(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.SetBasePort(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.ConnectPorts();
		}

		m_vgaRAM.Enable(m_misc.enableRAM);
	}

	void VideoVGA::WriteFeatureControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteFeatureControlRegister, value=%02Xh", value);
		// Unused
	}

	BYTE VideoVGA::ReadStatusRegister0()
	{
		BYTE value = 0xFF;

		// Sense looks at the output of the DAC
		// Allows detection of mono/color monitor
		// For now, false if any channel is larger than a threshold
		bool sense = !(GetLastDot() & 0x00808080);

		value =
			(sense << 4) |
			(1 << 5) |
			(1 << 6) |
			(m_crtc.IsInterruptPending() << 7);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister0, value=%02Xh", value);

		return value;
	}

	BYTE VideoVGA::ReadStatusRegister1()
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
		// Bit4&5:Attribute controller pixel data, selected by Color Plane Register

		bool diag4 = false;
		bool diag5 = false;

		BYTE lastAttribute = m_attrController.GetLastDot();
		switch (m_attrController.GetData().videoStatusMux)
		{
		case 0:
			diag5 = GetBit(lastAttribute, 2);
			diag4 = GetBit(lastAttribute, 0);
			break;
		case 1:
			diag5 = GetBit(lastAttribute, 5);
			diag4 = GetBit(lastAttribute, 4);
			break;
		case 2:
			diag5 = GetBit(lastAttribute, 3);
			diag4 = GetBit(lastAttribute, 1);
			break;
		case 3:
			diag5 = GetBit(lastAttribute, 7);
			diag4 = GetBit(lastAttribute, 6);
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

	void VideoVGA::DrawTextMode()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct CRTCConfig& crtcConfig = m_crtc.GetConfig();
		const struct AttrControllerData attrData = m_attrController.GetData();
		const int charWidth = crtcData.charWidth;

		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			bool isCursorChar = IsCursor();
			BYTE ch = m_vgaRAM.readRaw(0, base);
			BYTE attr = m_vgaRAM.readRaw(1, base);

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
			BYTE currChar = m_vgaRAM.GetCharMapA()[((size_t)ch * 0x20) + crtcData.rowAddress];
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			// TODO: This is not the correct behavior
			bool cursorLine = isCursorChar && (crtcData.rowAddress > crtcConfig.cursorStart);
			if (crtcConfig.cursorEnd && (crtcData.rowAddress > crtcConfig.cursorEnd))
			{
				// TODO: handles (wrongly) cursorEnd == 0
				cursorLine = false;
			}

			int startBit = charWidth - 1;
			int endBit = 0;
			const int extraBits = charWidth - 8;
			if (crtcData.hPos == 0)
			{
				startBit -= m_framePelPanning;
			}
			else if (crtcData.hPos == crtcData.hTotalDisp)
			{
				endBit = charWidth - m_framePelPanning;
			}

			bool lastDot = false;
			for (int x = startBit; x >= endBit; --x)
			{
				const int currBit = x - extraBits;
				if (currBit >= 0)
				{
					lastDot = draw && GetBit(currChar, currBit);
				}
				// Characters C0h - DFh: 9th pixel == 8th pixel, otherwise blank
				else if ((ch < 0xC0) || (ch > 0xDF))
				{
					lastDot = 0;
				}

				DrawPixel(GetColor((lastDot || cursorLine) ? fg : bg));
			}
		}
		else
		{
			DrawBackground(charWidth);
		}
	}

	void VideoVGA::DrawTextModeMDA()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct CRTCConfig& crtcConfig = m_crtc.GetConfig();
		const struct AttrControllerData attrData = m_attrController.GetData();
		const struct SequencerData seqData = m_sequencer.GetData();

		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = GetAddress();

			bool isCursorChar = IsCursor();
			BYTE ch = m_vgaRAM.readRaw(0, base);
			BYTE attr = m_vgaRAM.readRaw(1, base);

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
			BYTE currChar = m_vgaRAM.GetCharMapA()[((size_t)ch * 0x20) + crtcData.rowAddress];
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

	void VideoVGA::DrawGraphModeCGA4()
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
				pixData[i] = GetBit(attrData.colorPlaneEnable, i) ? m_vgaRAM.readRaw(i, base) : 0;
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

	void VideoVGA::DrawGraphMode()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();
		const struct AttrControllerData attrData = m_attrController.GetData();

		// Called every 8 horizontal pixels
		if (IsEnabled() && IsDisplayArea() && !m_crtc.IsVBlank())
		{
			ADDRESS base = GetAddress();
			BYTE pixData[4];
			for (int i = 0; i < 4; ++i)
			{
				pixData[i] = GetBit(attrData.colorPlaneEnable, i) ?  m_vgaRAM.readRaw(i, base) : 0;

				if (crtcData.hPos == crtcData.hTotalDisp)
				{
					pixData[i] &= (0xFF << (8 - m_framePelPanning));
				}
			}

			int startBit = 7;
			if (crtcData.hPos == 0)
			{
				startBit -= m_framePelPanning;
			}

			for (int i = startBit; i >= 0; --i)
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

	void VideoVGA::DrawGraphMode256()
	{
		const struct CRTCData& crtcData = m_crtc.GetData();

		// Called every 4 horizontal pixel
		if (IsEnabled() && IsDisplayArea() && !m_crtc.IsBlank())
		{
			ADDRESS base = GetAddress();

			bool blankRight = false;
			int startBit = 3;
			int endBit = 0;
			if (crtcData.hPos == 0)
			{
				startBit -= m_framePelPanning;
			}
			else if (crtcData.hPos == crtcData.hTotalDisp)
			{
				endBit = 4 - m_framePelPanning;
				blankRight = true;
			}

			for (int i = startBit; i >= endBit; --i)
			{
				DrawPixel(GetColor(m_vgaRAM.readRaw(3 - i, base)));
			}
			if (blankRight)
			{
				DrawBackground(endBit);
			}
		}
		else
		{
			DrawBackground(4);
		}
	}

	void VideoVGA::Serialize(json& to)
	{
		Video::Serialize(to);
		to["baseAddress"] = m_baseAddress;

		json misc;
		misc["color"] = m_misc.color;
		misc["enableRAM"] = m_misc.enableRAM;
		misc["clockSel"] = m_misc.clockSel;
		misc["pageHigh"] = m_misc.pageHigh;
		misc["hSyncPolarity"] = m_misc.hSyncPolarity;
		misc["vSyncPolarity"] = m_misc.vSyncPolarity;
		to["misc"] = misc;

		m_sequencer.Serialize(to["sequencer"]);
		m_graphController.Serialize(to["graphController"]);
		m_attrController.Serialize(to["attrController"]);
		m_crtc.Serialize(to["crtc"]);
		m_dac.Serialize(to["digitalAnalogConverter"]);
		m_vgaRAM.Serialize(to["vgaRAM"]);
	}

	void VideoVGA::Deserialize(const json& from)
	{
		Video::Deserialize(from);
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoVGA: Incompatible baseAddress");
		}

		bool oldColor = m_misc.color;

		const json& misc = from["misc"];
		m_misc.color = misc["color"];
		m_misc.enableRAM = misc["enableRAM"];
		m_misc.clockSel = misc["clockSel"];
		m_misc.pageHigh = misc["pageHigh"];
		m_misc.hSyncPolarity = misc["hSyncPolarity"];
		m_misc.vSyncPolarity = misc["vSyncPolarity"];

		m_sequencer.Deserialize(from["sequencer"]);
		m_graphController.Deserialize(from["graphController"]);
		m_attrController.Deserialize(from["attrController"]);
		m_dac.Deserialize(from["digitalAnalogConverter"]);
		m_vgaRAM.Deserialize(from["vgaRAM"]);

		if (oldColor != m_misc.color)
		{
			DisconnectRelocatablePorts(oldColor ? m_baseAddressColor : m_baseAddressMono);
			ConnectRelocatablePorts(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.SetBasePort(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.ConnectPorts();
		}
		m_crtc.Deserialize(from["crtc"]);

		OnChangeMode();
		OnNewFrame();
	}
}
