#include "VideoCGA.h"
#include "PortAggregator.h"
#include <assert.h>

using emul::SetBit;
using emul::GetBit;

using crtc::CRTCConfig;
using crtc::CRTCData;

namespace video
{
	const float VSCALE = 2.4f;

	const uint32_t AlphaColorPalette[16] = 
	{
		0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA, 0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
		0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF, 0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
	};

	const uint32_t AlphaMonoGreyPalette[16] =
	{
		0xFF000000, 0xFF0C0C0C, 0xFF7A7A7A, 0xFF868686, 0xFF242424, 0xFF303030, 0xFF616161, 0xFFAAAAAA,
		0xFF555555, 0xFF616161, 0xFFCFCFCF, 0xFFDBDBDB, 0xFF797979, 0xFF858585, 0xFFF3F3F3, 0xFFFFFFFF
	};

	const uint32_t AlphaMonoGreenPalette[16] =
	{
		0xFF000000, 0xFF020A00, 0xFF1D7700, 0xFF218400, 0xFF082300, 0xFF0B2D00, 0xFF186000, 0xFF2AA800,
		0xFF155400, 0xFF186000, 0xFF33CE00, 0xFF36D800, 0xFF1D7700, 0xFF218400, 0xFF3CF200, 0xFF41ff00
	};

	const uint32_t Composite640Palette[16] =
	{
		0x00000000, 0x00006E31, 0x003109FF, 0x00008AFF, 0x00A70031, 0x00767676, 0x00EC11FF, 0x00BB92FF,
		0x00315A00, 0x0000DB00, 0x00767676, 0x0045F7BB, 0x00EC6300, 0x00BBE400, 0x00FF7FBB, 0x00FFFFFF
	};

	const uint32_t TempGray[] =
	{
		0xFF000000, 0xFF3F3F3F, 0xFF7F7F7F, 0xFFFFFFFF
	};

	static void OnRenderFrame(crtc::Device6845* crtc, void* data)
	{
		VideoCGA* cga = reinterpret_cast<VideoCGA*>(data);
		cga->RenderFrame();
	}

	static void OnNewFrame(crtc::Device6845* crtc, void* data)
	{
		VideoCGA* cga = reinterpret_cast<VideoCGA*>(data);
		cga->NewFrame();
	}

	static void OnEndOfRow(crtc::Device6845* crtc, void* data)
	{
		VideoCGA* cga = reinterpret_cast<VideoCGA*>(data);
		cga->EndOfRow();
	}

	VideoCGA::VideoCGA(WORD baseAddress) :
		Video(640, 200, VSCALE),
		Logger("CGA"),
		m_baseAddress(baseAddress),
		m_crtc(baseAddress),
		m_screenB800("CGA", 16384, emul::MemoryType::RAM),
		m_charROM("CHAR", 8192, emul::MemoryType::ROM),
		m_alphaPalette(AlphaColorPalette)
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

	void VideoCGA::Init(emul::Memory& memory, const char* charROM, BYTE border)
	{
		assert(charROM);
		LogPrintf(Logger::LOG_INFO, "Loading char ROM [%s]", charROM);
		m_charROM.LoadFromFile(charROM);
		m_charROMStart = m_charROM.getPtr8(4096 + 2048);

		m_crtc.Init();
		m_crtc.SetRenderFrameCallback(OnRenderFrame, this);
		m_crtc.SetNewFrameCallback(OnNewFrame, this);
		m_crtc.SetEndOfRowCallback(OnEndOfRow, this);

		// Mode Control Register
		Connect(m_baseAddress + 8, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteModeControlRegister));

		// Color Select Register
		Connect(m_baseAddress + 9, static_cast<PortConnector::OUTFunction>(&VideoCGA::WriteColorSelectRegister));

		// Status Register
		Connect(m_baseAddress + 0xA, static_cast<PortConnector::INFunction>(&VideoCGA::ReadStatusRegister));

		memory.Allocate(&GetVideoRAM(), emul::S2A(0xB800));
		memory.Allocate(&GetVideoRAM(), emul::S2A(0xBC00));

		Video::Init(border);
	}	
	
	bool VideoCGA::ConnectTo(emul::PortAggregator& dest)
	{
		// Connect sub devices
		dest.Connect(m_crtc);
		return PortConnector::ConnectTo(dest);
	}

	void VideoCGA::RenderFrame()
	{
		uint32_t borderRGB = m_alphaPalette[m_color.color];
		uint16_t width = (m_mode.text80Columns || m_mode.hiResolution) ? 640 : 320;
		Video::RenderFrame(width, 200, borderRGB);
	}

	BYTE VideoCGA::ReadStatusRegister()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadStatusRegister, hSync=%d, vSync=%d", m_crtc.IsHSync(), m_crtc.IsVSync());
		return (m_crtc.IsDisplayArea() ? 1 : 0) | (m_crtc.IsVSync() ? 8 : 0);
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

		if (m_mode.enableVideo)
		{
			(this->*m_drawFunc)();
		}

		m_crtc.Tick();
	}

	void VideoCGA::NewFrame()
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		// Pointers for alpha mode
		m_currChar = m_screenB800.getPtr8(config.startAddress * 2u);
		if (config.cursorAddress * 2u >= m_screenB800.GetSize())
		{
			m_cursorPos = nullptr;
		}
		else
		{
			m_cursorPos = m_screenB800.getPtr8(config.cursorAddress * 2u);
		}

		// Pointers for graphics mode
		m_bank0 = m_screenB800.getPtr8(0x0000);
		m_bank1 = m_screenB800.getPtr8(0x2000);

		// Select draw function
		m_drawFunc = &VideoCGA::DrawTextMode;
		if (m_mode.graphics) m_drawFunc = &VideoCGA::Draw320x200;
		if (m_mode.hiResolution) m_drawFunc = &VideoCGA::Draw640x200;

		// TODO: Do this for each line instead of each frame
		m_alphaPalette = (m_mode.monochrome && m_composite) ? AlphaMonoGreyPalette : AlphaColorPalette;
		m_currGraphPalette[0] = m_alphaPalette[m_color.color];
		for (int i = 1; i < 4; ++i)
		{
			m_currGraphPalette[i] = m_alphaPalette[
				(m_color.palIntense << 3) // Intensity
				| (i << 1)
				| (m_color.palSelect && !m_mode.monochrome) // Palette shift for non mono modes
				| (m_mode.monochrome & (i & 1)) // Palette shift for mono modes
			];
		}
	}

	static uint32_t tempRowAlpha[640 + 2];
	static uint32_t tempRow[640];

	void VideoCGA::EndOfRow()
	{
		const struct CRTCData& data = m_crtc.GetData();
		if (!m_mode.hiResolution || !m_composite || data.vPos >= data.vTotalDisp)
			return;

		uint32_t baseX = (640 * data.vPos);

		memset(tempRowAlpha, 0, sizeof(tempRowAlpha));
		for (int offset = 0; offset < 4; ++offset)
		{
			for (int x = 0; x < 640; ++x)
			{
				//tempRowAlpha[x + offset] += (tempRow[x] & 16) ? 0x3F000000 : 0;
				tempRowAlpha[x + offset] += (tempRow[x] & 16) ? 1 : 0;
			}
		}

		for (int i = 0; i < 640; ++i)
		{
			m_frameBuffer[baseX + i] = Composite640Palette[tempRow[i] & 15] | ((i%256) << 24);// tempRowAlpha[i + 2];
			//m_frameBuffer[baseX + i] = TempGray[tempRowAlpha[i+2]];
		}

		// Clear temp row

	}

	bool VideoCGA::IsCursor() const
	{
		const struct CRTCConfig& config = m_crtc.GetConfig();

		return (m_currChar == m_cursorPos) &&
			(config.cursor != CRTCConfig::CURSOR_NONE) &&
			((config.cursor == CRTCConfig::CURSOR_BLINK32 && m_crtc.IsBlink32()) || m_crtc.IsBlink16());
	}

	void VideoCGA::DrawTextMode()
	{
		const struct CRTCData& data = m_crtc.GetData();
		const struct CRTCConfig& config = m_crtc.GetConfig();

		if (m_currChar && m_crtc.IsDisplayArea() && ((data.vPos % data.vCharHeight) == 0))
		{
			bool isCursorChar = IsCursor();

			BYTE ch = *(m_currChar++);
			BYTE attr = *(m_currChar++);
			BYTE bg = attr >> 4;
			BYTE fg = attr & 0x0F;
			bool charBlink = false;

			// Background
			if (m_mode.blink) // Hi bit: intense bg vs blink fg
			{
				charBlink = GetBit(bg, 3);
				SetBit(bg, 3, false);
			}

			uint32_t fgRGB = m_alphaPalette[fg];
			uint32_t bgRGB = m_alphaPalette[bg];

			// Draw character
			BYTE* currCharPos = m_charROMStart + ((uint32_t)ch * 8) + (data.vPos % data.vCharHeight);
			bool draw = !charBlink || (charBlink && m_crtc.IsBlink16());
			for (int y = 0; y < data.vCharHeight; ++y)
			{
				uint32_t offset = 640 * (uint32_t)(data.vPos + y) + data.hPos;
				bool cursorLine = isCursorChar && (y >= config.cursorStart) && (y <= config.cursorEnd);
				for (int x = 0; x < 8; ++x)
				{
					bool set = draw && ((*(currCharPos + y)) & (1 << (7 - x)));
					m_frameBuffer[offset + x] = (set || cursorLine) ? fgRGB : bgRGB;
				}
			}
		}
	}
	void VideoCGA::Draw320x200()
	{
		const struct CRTCData& data = m_crtc.GetData();

		// Called every 8 horizontal pixels
		// In this mode 1 byte = 4 pixels

		BYTE* &currChar = (data.vPos & 1) ? m_bank1 : m_bank0;

		if (m_crtc.IsDisplayArea())
		{
			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;
				for (int x = 0; x < 4; ++x)
				{
					BYTE val = ch & 3;
					ch >>= 2;

					m_frameBuffer[640 * data.vPos + data.hPos + (w * 4) + (3 - x)] = m_currGraphPalette[val];
				}

				++currChar;
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
			BYTE*& currChar = (data.vPos & 1) ? m_bank1 : m_bank0;

			uint32_t fg = m_alphaPalette[m_color.color];
			uint32_t bg = m_alphaPalette[0];

			uint32_t baseX = (640 * data.vPos) + (data.hPos * 2);

			uint32_t compositeOffset = data.hPos * 2;

			for (int w = 0; w < 2; ++w)
			{
				BYTE ch = *currChar;

				if (m_composite)
				{
					BYTE colorH = ch >> 4;
					BYTE colorL = ch & 15;

					tempRow[compositeOffset++] = colorH | ((ch & 0b10000000) ? 16 : 0);
					tempRow[compositeOffset++] = colorH | ((ch & 0b01000000) ? 16 : 0);
					tempRow[compositeOffset++] = colorH | ((ch & 0b00100000) ? 16 : 0);
					tempRow[compositeOffset++] = colorH | ((ch & 0b00010000) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00001000) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00000100) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00000010) ? 16 : 0);
					tempRow[compositeOffset++] = colorL | ((ch & 0b00000001) ? 16 : 0);
				}
				else
				{
					m_frameBuffer[baseX++] = (ch & 0b10000000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b01000000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00100000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00010000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00001000) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00000100) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00000010) ? fg : bg;
					m_frameBuffer[baseX++] = (ch & 0b00000001) ? fg : bg;
				}
				++currChar;
			}
		}
	}
}
