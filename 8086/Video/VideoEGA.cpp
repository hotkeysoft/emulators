#include "VideoEGA.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;
using emul::ADDRESS;

using crtc_ega::DeviceCRTC;

namespace video
{
	VideoEGA::VideoEGA(RAMSIZE ramsize, WORD baseAddress, WORD baseAddressMono, WORD baseAddressColor) :
		Logger("EGA"),
		m_crtc(baseAddressMono),
		m_ramSize(ramsize),
		m_baseAddress(baseAddress),
		m_baseAddressMono(baseAddressMono),
		m_baseAddressColor(baseAddressColor),
		m_egaROM("EGABIOS", 16384, emul::MemoryType::ROM)
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

		ConnectRelocatablePorts(m_baseAddressMono);

		Video::Init(memory, charROM, forceMono);

		// Normally max y is nnn but leave some room for custom crtc programming
		InitFrameBuffer(2048, 400);

		AddMode("text", (DrawFunc)&VideoEGA::DrawTextMode, (AddressFunc)&VideoEGA::GetBaseAddress, (ColorFunc)&VideoEGA::GetIndexedColor);
		SetMode("text");

		m_crtc.Init();
	}	

	SDL_Rect VideoEGA::GetDisplayRect(BYTE border, WORD xMultiplier) const
	{
		return SDL_Rect{ 0, 0, 640, 200 };
	}

	void VideoEGA::Tick()
	{
		m_crtc.Tick();
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

	void VideoEGA::DrawTextMode()
	{
		LogPrintf(LOG_DEBUG, "DrawTextMode");
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
