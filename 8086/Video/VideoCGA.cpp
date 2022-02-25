#include "VideoCGA.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	VideoCGA::VideoCGA(WORD baseAddress) :
		Logger("CGA"),
		Video6845(baseAddress),
		m_screenB800("CGA", 16384, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
	}

	void VideoCGA::Init(emul::Memory* memory, const char* charROM, bool forceMono)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(4096 + 2048);

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteModeControlRegister));

		// Color Select Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteColorSelectRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoCGA::ReadStatusRegister));

		memory->Allocate(&GetVideoRAM(), emul::S2A(0xB800));
		memory->Allocate(&GetVideoRAM(), emul::S2A(0xBC00));

		Video6845::Init(memory, charROM, forceMono);

		// Normally max y is 262 but leave some room for custom crtc programming
		InitFrameBuffer(2048, 300);

		AddMode("text", (DrawFunc)&VideoCGA::DrawTextMode, (AddressFunc)&VideoCGA::GetBaseAddressText, (ColorFunc)&VideoCGA::GetIndexedColor16);
		AddMode("320x200x4", (DrawFunc)&VideoCGA::Draw320x200x4, (AddressFunc)&VideoCGA::GetBaseAddressGraph, (ColorFunc)&VideoCGA::GetIndexedColor4);
		AddMode("640x200x2", (DrawFunc)&VideoCGA::Draw640x200x2, (AddressFunc)&VideoCGA::GetBaseAddressGraph, (ColorFunc)&VideoCGA::GetIndexedColor2);
		SetMode("text");
	}	

	SDL_Rect VideoCGA::GetDisplayRect(BYTE border, WORD) const
	{
		uint16_t xMultiplier = m_mode.hiResolution ? 2 : 1;

		return Video6845::GetDisplayRect(border, xMultiplier);
	}

	void VideoCGA::OnChangeMode()
	{
		if (!m_mode.graphics)
		{
			SetMode("text");
		}
		else if (m_mode.hiResolution)
		{
			SetMode("640x200x2");
		}
		else
		{
			SetMode("320x200x4");
		}
	}

	BYTE VideoCGA::ReadStatusRegister()
	{
		// Bit0: 1:Display cpu access enabled (vsync | hsync)
		// 
		// Light pen, not implemented
		// Bit1: 1:Light pen trigger set
		// Bit2: 1:Light pen switch off, 0: switch on
		//
		// Bit3 1:Vertical retrace active

		BYTE status =
			((!GetCRTC().IsDisplayArea()) << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(GetCRTC().IsVSync() << 3);

		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, value=%02Xh", status);

		return status;
	}

	void VideoCGA::WriteModeControlRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteModeControlRegister, value=%02Xh", value);

		m_mode.text80Columns = GetBit(value, 0);
		m_mode.graphics = GetBit(value, 1);
		m_mode.monochrome = GetBit(value, 2);
		m_mode.enableVideo = GetBit(value, 3);
		m_mode.hiResolution = GetBit(value, 4);
		m_mode.blink = GetBit(value, 5);

		LogPrintf(Logger::LOG_INFO, "WriteModeControlRegister [%c80COLUMNS %cGRAPH %cMONO %cVIDEO %cHIRES %cBLINK]",
			m_mode.text80Columns ? ' ' : '/',
			m_mode.graphics ? ' ' : '/',
			m_mode.monochrome ? ' ' : '/',
			m_mode.enableVideo ? ' ' : '/',
			m_mode.hiResolution ? ' ' : '/',
			m_mode.blink ? ' ' : '/');

		OnChangeMode();
	}

	void VideoCGA::WriteColorSelectRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "WriteColorSelectRegister, value=%02Xh", value);

		m_color.color = (value & 15);
		m_color.palIntense = value & 16;
		m_color.palSelect = value & 32;

		LogPrintf(Logger::LOG_INFO, "WriteColorSelectRegister color=%d, palette %d, intense %d", 
			m_color.color, 
			m_color.palSelect,
			m_color.palIntense);
	}

	void VideoCGA::Tick()
	{
		// Halve the clock for all modes except 80 cols
		if (!m_mode.text80Columns)
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

	void VideoCGA::OnEndOfRow()
	{
		Video6845::OnEndOfRow();
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

	void VideoCGA::DrawTextMode()
	{
		const struct CRTCData& data = GetCRTC().GetData();
		const struct CRTCConfig& config = GetCRTC().GetConfig();

		if (GetCRTC().IsDisplayArea() && IsEnabled())
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

	void VideoCGA::Serialize(json& to)
	{
		Video6845::Serialize(to);
		to["baseAddress"] = m_baseAddress;

		json mode;
		mode["text80Columns"] = m_mode.text80Columns;
		mode["graphics"] = m_mode.graphics;
		mode["monochrome"] = m_mode.monochrome;
		mode["enableVideo"] = m_mode.enableVideo;
		mode["hiResolution"] = m_mode.hiResolution;
		mode["blink"] = m_mode.blink;
		to["mode"] = mode;

		json color;
		color["color"] = m_color.color;
		color["palIntense"] = m_color.palIntense;
		color["palSelect"] = m_color.palSelect;
		to["color"] = color;

		GetCRTC().Serialize(to["crtc"]);
	}

	void VideoCGA::Deserialize(json& from)
	{
		Video6845::Deserialize(from);

		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoCGA: Incompatible baseAddress");
		}

		const json& mode = from["mode"];
		m_mode.text80Columns = mode["text80Columns"];
		m_mode.graphics = mode["graphics"];
		m_mode.monochrome = mode["monochrome"];
		m_mode.enableVideo = mode["enableVideo"];
		m_mode.hiResolution = mode["hiResolution"];
		m_mode.blink = mode["blink"];

		const json& color = from["color"];
		m_color.color = color["color"];
		m_color.palIntense = color["palIntense"];
		m_color.palSelect = color["palSelect"];

		OnChangeMode();
		OnNewFrame();
	}
}
