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

using graph_ega::GraphControllerAddress;
using graph_ega::GraphController;
using graph_ega::MemoryMap;
using graph_ega::RotateFunction;

using attr_ega::AttrController;
using attr_ega::AttrControllerAddress;
using attr_ega::RegisterMode;
using attr_ega::PaletteSource;

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

		Connect(m_baseAddress + 0x4, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteSequencerAddress));
		Connect(m_baseAddress + 0x5, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteSequencerValue));

		Connect(m_baseAddress + 0xC, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteGraphics1Position));
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteGraphics2Position));
		Connect(m_baseAddress + 0xE, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteGraphicsAddress));
		Connect(m_baseAddress + 0xF, static_cast<PortConnector::OUTFunction>(&VideoEGA::WriteGraphicsValue));

		ConnectRelocatablePorts(m_baseAddressMono);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		Video::Init(memory, charROM, forceMono);

		// Normally max y is nnn but leave some room for custom crtc programming
		InitFrameBuffer(2048, 400);

		AddMode("text", (DrawFunc)&VideoEGA::DrawTextMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		AddMode("graph", (DrawFunc)&VideoEGA::DrawGraphMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		SetMode("text");

		m_egaRAM.SetGraphController(&m_graphController);

		MapMemory();
	}	

	void VideoEGA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(LOG_INFO);
		Video::EnableLog(minSev);
	}

	SDL_Rect VideoEGA::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		const struct CRTCData& data = m_crtc.GetData();

		SDL_Rect rect;
		rect.x = std::max(0, (data.hTotal - data.hSyncMin - border - 1) * xMultiplier);
		rect.y = std::max(0, (data.vTotal - data.vSyncMin - border - 1));

		rect.w = std::min(m_fbWidth - rect.x, (data.hTotalDisp + (2u * border)) * xMultiplier);
		rect.h = std::min(m_fbHeight - rect.y, (data.vTotalDisp + (2u * border)));

		return rect;
	}

	void VideoEGA::OnChangeMode()
	{
		if (m_memoryMode.alpha)
		{
			SetMode("text");
		}
		else
		{
			SetMode("graph");
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
		if (m_clockingMode.halfDotClock)
		{
			static bool div2 = false;
			div2 = !div2;
			if (div2)
			{
				return;
			}
		}

		if (!m_crtc.IsVSync())
		{
			Draw();
		}

		m_crtc.Tick();
	}

	void VideoEGA::OnNewFrame()
	{
		BeginFrame();
	}

	void VideoEGA::OnEndOfRow()
	{
		NewLine();
	}

	bool VideoEGA::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();
		const struct CRTCData& data = m_crtc.GetData();

		//TODO
		return (data.memoryAddress == config.cursorAddress)/* &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16())*/;
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

		// CLKSEL is used to determine which swich to read;
		// Need to invert logic, off = opened swich = 5V = logical 1
		bool switchSense = !m_dipSwitches[(int)m_misc.clockSel];

		value =
			(switchSense << 4) |
			(!IsVSync() << 7);

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

	void VideoEGA::WriteSequencerAddress(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerAddress, value=%02Xh", value);

		switch (value & 0b1111)
		{
		case 0: m_seqAddress = SequencerAddress::SEQ_RESET; break;
		case 1: m_seqAddress = SequencerAddress::SEQ_CLOCKING_MODE; break;
		case 2: m_seqAddress = SequencerAddress::SEQ_MAP_MASK; break;
		case 3: m_seqAddress = SequencerAddress::SEQ_CHARMAP_SELECT; break;
		case 4: m_seqAddress = SequencerAddress::SEQ_MEMORY_MODE; break;
		default:
			m_seqAddress = SequencerAddress::SEQ_INVALID; 
			LogPrintf(LOG_WARNING, "Invalid sequencer address %d", value);
			break;
		}
	}

	// Dispatches value to functions below
	void VideoEGA::WriteSequencerValue(BYTE value)
	{
		switch (m_seqAddress)
		{
		case SequencerAddress::SEQ_RESET:          WriteSequencerReset(value);         break;
		case SequencerAddress::SEQ_CLOCKING_MODE:  WriteSequencerClockingMode(value);  break;
		case SequencerAddress::SEQ_MAP_MASK:       WriteSequencerMapMask(value);       break;
		case SequencerAddress::SEQ_CHARMAP_SELECT: WriteSequencerCharMapSelect(value); break;
		case SequencerAddress::SEQ_MEMORY_MODE:    WriteSequencerMemoryMode(value);    break;
		}
	}
	//
	void VideoEGA::WriteSequencerReset(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerReset, value=%02Xh", value);

		if (!GetBit(value, 0))
		{
			LogPrintf(Logger::LOG_INFO, "WriteSequencerReset: ASYNC RESET");
			// TODO: Clear memory
		}
		if (!GetBit(value, 1))
		{
			LogPrintf(Logger::LOG_INFO, "WriteSequencerReset: SYNC RESET");
		}
	}
	void VideoEGA::WriteSequencerClockingMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerClockingMode, value=%02Xh", value);

		m_clockingMode.charWidth = GetBit(value, 0) ? 8 : 9;
		m_clockingMode.lowBandwidth = GetBit(value, 1);
		m_clockingMode.load16 = GetBit(value, 2);
		m_clockingMode.halfDotClock = GetBit(value, 3);

		LogPrintf(Logger::LOG_INFO, "WriteSequencerClockingMode [%cLOWBW %cLOAD16 %cHALFDOT CHARWIDTH[%d]]",
			m_clockingMode.lowBandwidth ? ' ' : '/',
			m_clockingMode.load16 ? ' ' : '/',
			m_clockingMode.halfDotClock ? ' ' : '/',
			m_clockingMode.charWidth);
	}

	void VideoEGA::WriteSequencerMapMask(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteSequencerMapMask, value=%02Xh", value);
		m_planeMask = value & 0x0F;
		m_egaRAM.SetPlaneMask(m_planeMask);

		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerMapMask, Enable planes: [%c0][%c1][%c2][%c3]",
			GetBit(m_planeMask, 0) ? ' ' : '/',
			GetBit(m_planeMask, 1) ? ' ' : '/',
			GetBit(m_planeMask, 2) ? ' ' : '/',
			GetBit(m_planeMask, 3) ? ' ' : '/');

	}
	void VideoEGA::WriteSequencerCharMapSelect(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerCharMapSelect, value=%02Xh", value);

		m_charMapSelectB = value & 3;
		m_charMapSelectA = (value >> 2) & 3;

		LogPrintf(Logger::LOG_INFO, "WriteSequencerCharMapSelect, charMapA[%d] charMapB[%d]", m_charMapSelectA, m_charMapSelectB);

		m_egaRAM.SelectCharMaps(m_charMapSelectA, m_charMapSelectB);
	}
	void VideoEGA::WriteSequencerMemoryMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerMemoryMode, value=%02Xh", value);

		m_memoryMode.alpha = GetBit(value, 0);
		m_memoryMode.extMemory = GetBit(value, 1);
		m_memoryMode.sequential = GetBit(value, 2);

		// TODO: alpha, on changemode
		// TODO: extMemory: disable/enable bits 14/15
		// TODO: sequential

		LogPrintf(Logger::LOG_INFO, "WriteSequencerMemoryMode [%cALPHA %cEXTRAM %cSEQ]",
			m_memoryMode.alpha ? ' ' : '/',
			m_memoryMode.extMemory ? ' ' : '/',
			m_memoryMode.sequential ? ' ' : '/');
	}

	void VideoEGA::WriteGraphics1Position(BYTE value)
	{
		// TODO Not sure how this works
		LogPrintf(Logger::LOG_INFO, "WriteGraphics1Position, value=%02Xh", value);
		if ((value & 3) != 0)
		{
			LogPrintf(Logger::LOG_WARNING, "WriteGraphics1Position, expected value=0");
			throw std::exception("WriteGraphics1Position, expected value=0");
		}
	}
	void VideoEGA::WriteGraphics2Position(BYTE value)
	{
		// TODO Not sure how this works
		LogPrintf(Logger::LOG_INFO, "WriteGraphics2Position, value=%02Xh", value);
		if ((value & 3) != 1)
		{
			LogPrintf(Logger::LOG_WARNING, "WriteGraphics2Position, expected value=1");
			throw std::exception("WriteGraphics2Position, expected value=1");
		}
	}
	void VideoEGA::WriteGraphicsAddress(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteGraphicsAddress, reg=%d", value);
		value &= 15;
		m_graphController.currRegister = (value > (int)GraphControllerAddress::_GRAPH_MAX_REG) ? 
			GraphControllerAddress::GRAPH_INVALID_REG : 
			(GraphControllerAddress)value;
	}
	void VideoEGA::WriteGraphicsValue(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteGraphicsValue, value=%02Xh", value);

		switch (m_graphController.currRegister)
		{
		case GraphControllerAddress::GRAPH_SET_RESET:
			m_graphController.setReset = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Set/Reset %d", m_graphController.setReset);
			// TODO, write mode 0, enabled by next register	
			break;
		case GraphControllerAddress::GRAPH_ENABLE_SET_RESET:
			m_graphController.enableSetReset = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Enable Set/Reset %d", m_graphController.enableSetReset);

			// TODO, write mode 0
			if (m_graphController.enableSetReset)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Enable Set/Reset");
			}
			break;
		case GraphControllerAddress::GRAPH_COLOR_COMPARE:
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Color Compare %d", value);
			m_graphController.colorCompare = value & 15;
			break;
		case GraphControllerAddress::GRAPH_DATA_ROTATE:
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Data Rotate %d", value);
			m_graphController.rotateCount = value & 3;
			m_graphController.rotateFunction = (RotateFunction)((value >> 3) & 3);
			// TODO, write mode 0
			if (m_graphController.rotateCount)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Data Rotate not implemented");
			}
			if (m_graphController.rotateFunction != RotateFunction::NONE)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Data Rotate Function not implemented");
			}
			break;
		case GraphControllerAddress::GRAPH_READ_MAP_SELECT:
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Read Map Select %d", value);
			m_graphController.readPlaneSelect = value & 7;
			if (m_graphController.readPlaneSelect > 3)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Invalid Map: ", m_graphController.readPlaneSelect);
			}
			break;
		case GraphControllerAddress::GRAPH_MODE:
			// TODO
			LogPrintf(Logger::LOG_DEBUG, "WriteGraphicsValue, Mode %d", value);

			m_graphController.writeMode = value & 3;
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, WriteMode[%d]", m_graphController.writeMode);
			if (m_graphController.writeMode == 3)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Illegal Write Mode");
			}

			m_graphController.readModeCompare = GetBit(value, 3);
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, ReadMode[%s]", m_graphController.readModeCompare ? "COMPARE" : "NORMAL");

			m_graphController.oddEven = GetBit(value, 4);
			// Same as MemoryMode.sequential
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Odd/Even addressing mode[%d]", m_graphController.oddEven);

			m_graphController.shiftRegister = GetBit(value, 5);
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Shift register mode[%d]", m_graphController.shiftRegister);
			break;
		case GraphControllerAddress::GRAPH_MISC:
			// TODO
			LogPrintf(Logger::LOG_DEBUG, "WriteGraphicsValue, Miscellaneous %d", value);
			m_graphController.graphics = GetBit(value, 0);
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Graphics[%d]", m_graphController.graphics);
			m_graphController.chainOddEven = GetBit(value, 1);
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Chain Odd/Even[%d]", m_graphController.chainOddEven);
			m_graphController.memoryMap = (MemoryMap)((value >> 2) & 3);
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Memory Map[%x]", m_graphController.memoryMap);
			MapMemory();
			break;
		case GraphControllerAddress::GRAPH_COLOR_DONT_CARE:
			m_graphController.colorDontCare = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Color Don't Care[%x]", value);
			break;
		case GraphControllerAddress::GRAPH_BIT_MASK:
			m_graphController.bitMask = value;
			LogPrintf(Logger::LOG_DEBUG, "WriteGraphicsValue, Bit Mask[%02x]", m_graphController.bitMask);
			break;
		default:
			LogPrintf(Logger::LOG_WARNING, "WriteGraphicsValue, Invalid register");
		}
	}

	void VideoEGA::MapMemory()
	{
		m_memory->Free(&m_egaRAM);

		switch (m_graphController.memoryMap)
		{
		case MemoryMap::A000_128K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [A000][128K]");
			m_memory->Allocate(&m_egaRAM, 0xA0000, 0x20000);
			break;
		case MemoryMap::A000_64K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [A000][64K]");
			m_memory->Allocate(&m_egaRAM, 0xA0000, 0x10000);
			break;
		case MemoryMap::B000_32K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [B000][32K]");
			m_memory->Allocate(&m_egaRAM, 0xB0000, 0x8000);
			break;
		case MemoryMap::B800_32K:
			LogPrintf(Logger::LOG_INFO, "MapMemory: [B800][32K]");
			m_memory->Allocate(&m_egaRAM, 0xB8000, 0x8000);
			break;
		}
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
				break;
			case AttrControllerAddress::ATTR_OVERSCAN_COLOR:
				LogPrintf(LOG_DEBUG, "WriteAttributeController, Overscan Color %d", value);
				m_attr.overscanColor = (GetColorMode() == ColorMode::RGB4) ? RGB4toARGB32(value) : RGB6toARGB32(value);
				break;
			case AttrControllerAddress::ATTR_COLOR_PLANE_EN:
				LogPrintf(LOG_DEBUG, "WriteAttributeController, Color Plane Enable %d", value);

				// TODO
				m_attr.colorPlaneEnable = value & 15;
				LogPrintf(LOG_INFO, "WriteAttributeController, Color Plane Enable %02x", m_attr.colorPlaneEnable);

				m_attr.videoStatusMux = (value >> 4) & 3;
				LogPrintf(LOG_INFO, "WriteAttributeController, Video Status Mux %02x", m_attr.videoStatusMux);
				break;
			case AttrControllerAddress::ATTR_H_PEL_PANNING:
				LogPrintf(LOG_DEBUG, "WriteAttributeController, Horizontal Pel Panning %d", value);
				// TODO
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
			if (/*m_mode.blink*/true) // Hi bit: intense bg vs blink fg
			{
				charBlink = GetBit(bg, 3);
				SetBit(bg, 3, false);
			}

			// Draw character
			BYTE currChar = m_egaRAM.GetCharMapA()[((size_t)ch * 0x20) + data.rowAddress];
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16()); // TODO

			// TODO: This is not the correct behavior
			bool cursorLine = isCursorChar && (data.rowAddress > config.cursorStart);
			if (config.cursorEnd && (data.rowAddress > config.cursorEnd))
			{
				// TODO: handles (wrongly) cursorEnd == 0
				cursorLine = false;
			}

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
				pixData[i] = m_egaRAM.readRaw(i, base);
			}

			for (int i = 7; i >= 0; --i)
			{
				BYTE color = 
					(GetBit(pixData[0], i) << 0) |
					(GetBit(pixData[1], i) << 1) |
					(GetBit(pixData[2], i) << 2) |
					(GetBit(pixData[3], i) << 3);

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

		json seq;
		seq["seqAddress"] = m_seqAddress;
		seq["clk.charWidth"] = m_clockingMode.charWidth;
		seq["clk.lowBandwidth"] = m_clockingMode.lowBandwidth;
		seq["clk.load16"] = m_clockingMode.load16;
		seq["clk.halfDotClock"] = m_clockingMode.halfDotClock;
		seq["planeMask"] = m_planeMask;
		seq["charMapSelectA"] = m_charMapSelectA;
		seq["charMapSelectB"] = m_charMapSelectB;
		seq["mem.alpha"] = m_memoryMode.alpha;
		seq["mem.extMemory"] = m_memoryMode.extMemory;
		seq["mem.sequential"] = m_memoryMode.sequential;
		to["seq"] = seq;

		m_graphController.Serialize(to["graphController"]);
		m_attr.Serialize(to["attrController"]);
		m_crtc.Serialize(to["crtc"]);

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

		const json& seq = from["seq"];
		m_seqAddress = seq["seqAddress"];
		m_clockingMode.charWidth = seq["clk.charWidth"];
		m_clockingMode.lowBandwidth = seq["clk.lowBandwidth"];
		m_clockingMode.load16 = seq["clk.load16"];
		m_clockingMode.halfDotClock = seq["clk.halfDotClock"];
		m_planeMask = seq["planeMask"];
		m_charMapSelectA = seq["charMapSelectA"];
		m_charMapSelectB = seq["charMapSelectB"];
		m_memoryMode.alpha = seq["mem.alpha"];
		m_memoryMode.extMemory = seq["mem.extMemory"];
		m_memoryMode.sequential = seq["mem.sequential"];

		m_graphController.Deserialize(from["graphController"]);
		m_attr.Deserialize(from["attrController"]);
		m_crtc.Deserialize(from["crtc"]);

		m_egaRAM.SelectCharMaps(m_charMapSelectA, m_charMapSelectB);

		MapMemory();
		OnChangeMode();
		OnNewFrame();
	}
}
