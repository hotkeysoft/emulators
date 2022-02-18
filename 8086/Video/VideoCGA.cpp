#include "VideoCGA.h"
#include "../CPU/PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	const float VSCALE = 2.4f;

	const uint32_t Composite640Palette[16] =
	{
		0x00000000, 0x00006E31, 0x003109FF, 0x00008AFF, 0x00A70031, 0x00767676, 0x00EC11FF, 0x00BB92FF,
		0x00315A00, 0x0000DB00, 0x00767676, 0x0045F7BB, 0x00EC6300, 0x00BBE400, 0x00FF7FBB, 0x00FFFFFF
	};

	const uint32_t TempGray[] =
	{
		0xFF000000, 0xFF3F3F3F, 0xFF7F7F7F, 0xFFFFFFFF
	};

	VideoCGA::VideoCGA(WORD baseAddress) :
		Logger("CGA"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress),
		m_screenB800("CGA", 16384, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM)
	{
		Reset();
	}

	void VideoCGA::Reset()
	{
		m_crtc.Reset();
	}

	void VideoCGA::EnableLog(SEVERITY minSev)
	{
		m_crtc.EnableLog(minSev);
		Video::EnableLog(minSev);
	}

	void VideoCGA::Init(emul::Memory* memory, const char* charROM, bool)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr(4096 + 2048);

		m_crtc.Init();
		m_crtc.SetEventHandler(this);

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteModeControlRegister));

		// Color Select Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteColorSelectRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoCGA::ReadStatusRegister));

		memory->Allocate(&GetVideoRAM(), emul::S2A(0xB800));
		memory->Allocate(&GetVideoRAM(), emul::S2A(0xBC00));

		Video::Init(memory, charROM);
	}	
	
	bool VideoCGA::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
	}

	void VideoCGA::OnChangeMode()
	{
		// Select draw function
		if (!m_mode.graphics)
		{
			LogPrintf(LOG_INFO, "OnChangeMode: DrawTextMode");
			m_drawFunc = &VideoCGA::DrawTextMode;
		}
		else if (m_mode.hiResolution)
		{
			LogPrintf(LOG_INFO, "OnChangeMode: Draw640x200");
			m_drawFunc = &VideoCGA::Draw640x200;
		}
		else
		{
			LogPrintf(LOG_INFO, "OnChangeMode: Draw320x200");
			m_drawFunc = &VideoCGA::Draw320x200;
		}

		uint16_t xMultiplier = m_mode.hiResolution ? 2 : 1;

		Video::InitFrameBuffer(m_crtc.GetData().hTotal * xMultiplier, m_crtc.GetData().vTotal);
	}

	void VideoCGA::OnRenderFrame()
	{
		uint32_t borderRGB = GetMonitorPalette()[m_color.color];
		Video::RenderFrame(borderRGB);
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
			((!m_crtc.IsDisplayArea()) << 0) |
			(0 << 1) | // Light Pen Trigger
			(1 << 2) | // Light Pen switch
			(m_crtc.IsVSync() << 3);

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
		if (!m_crtc.IsInit())
		{
			return;
		}

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

		if (m_mode.enableVideo)
		{
			(this->*m_drawFunc)();
		}

		m_crtc.Tick();
	}

	void VideoCGA::OnNewFrame()
	{
	}
	
	void VideoCGA::OnEndOfRow()
	{
		m_currGraphPalette[0] = GetMonitorPalette()[m_color.color];
		for (int i = 1; i < 4; ++i)
		{
			m_currGraphPalette[i] = GetMonitorPalette()[
				(m_color.palIntense << 3) // Intensity
					| (i << 1)
					| (m_color.palSelect && !m_mode.monochrome) // Palette shift for non mono modes
					| (m_mode.monochrome & (i & 1)) // Palette shift for mono modes
			];
		}
	}

	bool VideoCGA::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();
		const struct CRTCData& data = m_crtc.GetData();

		return (data.memoryAddress == config.cursorAddress) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoCGA::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (m_crtc.IsDisplayArea() && ((data.vPos % data.vCharHeight) == 0))
		{
			ADDRESS base = data.memoryAddress * 2u;

			bool isCursorChar = IsCursor();
			BYTE ch = m_screenB800.read(base);
			BYTE attr = m_screenB800.read(base + 1);

			BYTE bg = attr >> 4;
			BYTE fg = attr & 0x0F;
			bool charBlink = false;

			// Background
			if (m_mode.blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = GetBit(bg, 3);
				SetBit(bg, 3, false);
			}

			uint32_t fgRGB = GetMonitorPalette()[fg];
			uint32_t bgRGB = GetMonitorPalette()[bg];

			// Draw character
			BYTE* currCharPos = m_charROMStart + ((size_t)ch * 8) + (data.vPos % data.vCharHeight);
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());

			for (int y = 0; y < data.vCharHeight; ++y)
			{
				uint32_t offset = m_fbWidth * (uint32_t)(data.vPos + y) + data.hPos;
				bool cursorLine = isCursorChar && (y >= config.cursorStart) && (y <= config.cursorEnd);
				for (int x = 0; x < 8; ++x)
				{
					bool set = draw && ((*(currCharPos + y)) & (1 << (7 - x)));
					m_fb[offset + x] = (set || cursorLine) ? fgRGB : bgRGB;
				}
			}
		}
	}
	void VideoCGA::Draw320x200()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels
		if (m_crtc.IsDisplayArea())
		{
			ADDRESS base = (data.rowAddress * 0x2000) + (data.memoryAddress * 2u);
			base &= 0x3FFF;

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_screenB800.read(base++);
				for (int x = 0; x < 4; ++x)
				{
					BYTE val = ch & 3;
					ch >>= 2;

					m_fb[m_fbWidth * data.vPos + data.hPos + (w * 4) + (3 - x)] = m_currGraphPalette[val];
				}
			}
		}
	}

	void VideoCGA::Draw640x200()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels, but since crtc is 40 cols we have to process 2 characters = 16 pixels
		// In this mode 1 byte = 8 pixels

		if (m_crtc.IsDisplayArea())
		{
			ADDRESS base = (data.rowAddress * 0x2000) + (data.memoryAddress * 2u);

			uint32_t fg = GetMonitorPalette()[m_color.color];
			uint32_t bg = GetMonitorPalette()[0];

			uint32_t baseX = (m_fbWidth * data.vPos) + (data.hPos * 2);

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = m_screenB800.read(base++);

				m_fb[baseX++] = (ch & 0b10000000) ? fg : bg;
				m_fb[baseX++] = (ch & 0b01000000) ? fg : bg;
				m_fb[baseX++] = (ch & 0b00100000) ? fg : bg;
				m_fb[baseX++] = (ch & 0b00010000) ? fg : bg;
				m_fb[baseX++] = (ch & 0b00001000) ? fg : bg;
				m_fb[baseX++] = (ch & 0b00000100) ? fg : bg;
				m_fb[baseX++] = (ch & 0b00000010) ? fg : bg;
				m_fb[baseX++] = (ch & 0b00000001) ? fg : bg;
			}
		}
	}

	void VideoCGA::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "cga";

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

		m_crtc.Serialize(to["crtc"]);
	}

	void VideoCGA::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("VideoCGA: Incompatible baseAddress");
		}

		if (from["id"] != "cga")
		{
			throw emul::SerializableException("VideoCGA: Incompatible mode");
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

		m_crtc.Deserialize(from["crtc"]);

		OnChangeMode();
		OnNewFrame();
	}
}
