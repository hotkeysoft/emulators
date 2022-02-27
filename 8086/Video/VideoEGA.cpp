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

namespace video
{
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
		SetMode("text");

		MapMemory();
	}	

	void VideoEGA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		m_egaRAM.EnableLog(LOG_INFO);
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
		Connect(base + 0xA, static_cast<PortConnector::INFunction>(&VideoEGA::ReadFeatureControlRegister));

		// Input Status Register 1
		Connect(base + 0x2, static_cast<PortConnector::INFunction>(&VideoEGA::ReadStatusRegister1));
	}

	void VideoEGA::WriteMiscRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteMiscRegister, value=%02Xh", value);

		bool oldColor = m_misc.color;

		m_misc.color = GetBit(value, 0);
		m_misc.enableRAM = GetBit(value, 1);
		m_misc.disableVideo = GetBit(value, 4);
		m_misc.pageHigh = GetBit(value, 5);

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

		LogPrintf(Logger::LOG_INFO, "WriteMiscRegister [%cCOLOR %cRAM %cDISABLE %cPAGEBIT CLK[%s]]",			
			m_misc.color ? ' ' : '/',
			m_misc.enableRAM ? ' ' : '/',
			m_misc.disableVideo ? ' ' : '/',
			m_misc.pageHigh ? ' ' : '/',
			clockSelStr);

		if (oldColor != m_misc.color)
		{
			DisconnectRelocatablePorts(oldColor ? m_baseAddressColor : m_baseAddressMono);
			ConnectRelocatablePorts(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
			m_crtc.SetBasePort(m_misc.color ? m_baseAddressColor : m_baseAddressMono);
		}
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
		// Bit0: 1:Display cpu access enabled (vsync | hsync)
		// 
		// Light pen, not implemented
		// Bit1: 1:Light pen trigger set
		// Bit2: 1:Light pen switch off, 0: switch on
		//
		// Bit3 1:Vertical retrace active
		//
		// Bit4&5:Pixel data, selected by Color Plane Register

		// TODO
		BYTE diagnostic = 0;

		BYTE status =
			((!IsDisplayArea()) << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(IsVSync() << 3) |
			(diagnostic << 4);

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
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerMapMask, value=%02Xh", value);
		m_mapMask = value & 0x0F;
	}
	void VideoEGA::WriteSequencerCharMapSelect(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerCharMapSelect, value=%02Xh", value);

		m_charMapSelectB = value & 3;
		m_charMapSelectA = (value >> 2) & 3;

		LogPrintf(Logger::LOG_INFO, "WriteSequencerCharMapSelect, charMapA[%d] charMapB[%d]", m_charMapSelectA, m_charMapSelectB);
	}
	void VideoEGA::WriteSequencerMemoryMode(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteSequencerMemoryMode, value=%02Xh", value);

		m_memoryMode.alpha = GetBit(value, 0);
		m_memoryMode.extMemory = GetBit(value, 1);
		m_memoryMode.sequential = GetBit(value, 2);

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
			// TODO, read mode = compare
			if (m_graphController.colorCompare)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Data Rotate not implemented");
			}
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
			// TODO
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Read Map Select %d", value);
			m_graphController.mapSelect = value & 7;
			if (m_graphController.mapSelect > 3)
			{
				LogPrintf(Logger::LOG_ERROR, "WriteGraphicsValue, Invalid Map: ", m_graphController.mapSelect);
			}
			break;
		case GraphControllerAddress::GRAPH_MODE:
			// TODO
			LogPrintf(Logger::LOG_DEBUG, "WriteGraphicsValue, Mode %d", value);
			m_graphController.writeMode = value % 3;
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
			// TODO
			m_graphController.colorDontCare = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Color Don't Care[%x]", value);
			break;
		case GraphControllerAddress::GRAPH_BIT_MASK:
			// TODO
			m_graphController.bitMask = value;
			LogPrintf(Logger::LOG_INFO, "WriteGraphicsValue, Bit Mask[%02x]", m_graphController.bitMask);
			break;
		default:
			// TODO
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

	void VideoEGA::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();

		uint32_t fg = GetColor(15);
		uint32_t bg = GetColor(0);
		
		if (IsDisplayArea() && IsEnabled())
		{
			ADDRESS base = m_crtc.GetMemoryAddress(); // TODO temp
			//LogPrintf(LOG_INFO, "Base=%d, rowScan=%d", base, m_crtc.GetData().rowAddress);

			for (int i = 0; i < 8; ++i)
			{
				DrawPixel(IsCursor() ? fg : (0xFF000000 | (base<<4)));
			}
		}
		else
		{
			DrawBackground(8, bg);
		}

	}

	void VideoEGA::Serialize(json& to)
	{
		Video::Serialize(to);
		to["baseAddress"] = m_baseAddress;		
	}

	void VideoEGA::Deserialize(json& from)
	{
		Video::Deserialize(from);
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoEGA: Incompatible baseAddress");
		}

		//OnChangeMode();
		//OnNewFrame();
	}
}