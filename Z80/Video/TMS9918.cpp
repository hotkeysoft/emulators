#include "stdafx.h"
#include "TMS9918.h"

namespace fs = std::filesystem;

using emul::GetBit;
using emul::SetBit;
using emul::MakeWord;

namespace video::vdp
{
	const uint32_t TMS9918::s_palette[16] = {
		0X00000000, 0XFF000000,	0XFF21C842,	0XFF5EDC78,
		0XFF5455ED,	0XFF7D76FC,	0XFFD4524D,	0XFF42EBF5,
		0XFFFC5554,	0XFFFF7978,	0XFFD4C154,	0XFFE6CE80,
		0XFF21B03B,	0XFFC95BBA,	0XFFCCCCCC,	0XFFFFFFFF
	};

	static const char* GetSpriteInfoStr(const Sprite* sprite)
	{
		static char buf[80];
		sprintf(buf, "x=%4d, y=%4d, n=%02X, color=%d",
			sprite->GetX(),
			sprite->GetY(),
			sprite->GetName(),
			sprite->GetColor());
		return buf;
	}

	static const char* GetVideoModeStr(VideoMode mode)
	{
		switch (mode)
		{
		case VideoMode::GRAPH_1: return "GRAPH1";
		case VideoMode::GRAPH_2: return "GRAPH2";
		case VideoMode::MULTICOLOR: return "MULTICOLOR";
		case VideoMode::TEXT: return "TEXT";
		default: throw std::exception("not possible");
		}
	}

	int Sprite::size = 8;
	BYTE Sprite::nameMask = 0xFF;

	void TMS9918::Status::Reset()
	{
		interrupt = false;
		coincidence = false;
		fifthSpriteFlag = false;
		fifthSpriteName = 0;
	}

	inline BYTE TMS9918::Status::Get()
	{
		return (interrupt << 7)
			| (fifthSpriteFlag << 6)
			| (coincidence << 5)
			| (fifthSpriteName & 31);
	}

	// TODO: Different sizes
	TMS9918::TMS9918() : Logger("tms9918"), m_vram("vram", 0x4000)
	{
		Reset();
	}

	void TMS9918::Reset()
	{
		m_dataFlipFlop = false;
		m_tempData = 0;

		m_currWriteAddress = 0;
		m_currReadAddress = 0;

		m_config.vram16k = false;
		m_config.enable = false;
		m_config.interruptEnabled = false;
		m_config.sprites16x16 = false;
		m_config.sprites2x = false;

		Sprite::SetSize(8);
		Sprite::SetNameMask(0xFF);

		m_tables.Reset();

		m_config.m1 = false;
		m_config.m2 = false;
		m_config.m3 = false;
		UpdateMode();
		UpdateSpriteData();

		m_fgColor = 0;
		m_bgColor = 0;

		m_currName = 0;

		m_status.Reset();
	}

	void TMS9918::Init(video::Video* video)
	{
		assert(video);
		m_video = video;
	}

	void TMS9918::Tick()
	{
		++m_currX;

		if (m_currX == RIGHT_BORDER)
		{
			if (IsVDisplay())
			{
				UpdateSpriteDrawList();
				DrawSpriteLine();
				m_video->MergeLine(m_spritePixels[0].data(), H_TOTAL);
			}
			m_video->NewLine();
			++m_currY;
			m_currX = -LEFT_BORDER;

			m_currName = ((m_currY / 8) * 32);
		}

		if ((m_currY == V_DISPLAY) && (m_currX == 0))
		{
			m_status.interrupt = true;
		}

		if (m_currY == BOTTOM_BORDER)
		{
			m_video->RenderFrame();
			m_video->BeginFrame();

			m_currY = -LEFT_BORDER;
			m_currX = -TOP_BORDER;

			m_currName = 0;
		}

		if (IsDisplay() && IsEnabled())
		{
			switch (m_mode)
			{
			case VideoMode::GRAPH_1:
			case VideoMode::GRAPH_2:
				DrawGraph();
				break;
			case VideoMode::MULTICOLOR:
				// TODO
				m_video->DrawBackground(8, m_bgColor);
				break;
			case VideoMode::TEXT:
				// TODO
				m_video->DrawBackground(6, m_bgColor);
				break;
			}
		}
		else
		{
			m_video->DrawPixel(GetColor(m_bgColor));
		}
	}

	BYTE TMS9918::ReadStatus()
	{
		BYTE value = m_status.Get();
		m_status.Reset();

		// Reset data flip flop
		m_dataFlipFlop = false;

		LogPrintf(LOG_DEBUG, "ReadStatus, value=%02X", value);
		return value;
	}

	void TMS9918::Write(BYTE value)
	{
		if (!m_dataFlipFlop)
		{
			LogPrintf(LOG_TRACE, "Write temp data, value = %02X", value);
			m_tempData = value;
		}
		else
		{
			LogPrintf(LOG_TRACE, "Write, value = %02X", value);

			// Two highest bits select operation
			const BYTE op = (value >> 6) & 3;
			value &= 0x3F; // Clear two highest bits for data

			switch (op)
			{
			case 0: // Read address setup
				m_currReadAddress = MakeWord(value, m_tempData);
				LogPrintf(LOG_DEBUG, "Set Read address = %04X", m_currReadAddress);
				break;
			case 1: // Write address setup
				m_currWriteAddress = MakeWord(value, m_tempData);
				LogPrintf(LOG_DEBUG, "Set Write address = %04X", m_currWriteAddress);
				break;
			case 2: // Write register
				WriteRegister(value);
				break;
			case 3:
				LogPrintf(LOG_ERROR, "Invalid operation");
				break;
			default:
				throw std::exception("not possible");
			}
		}

		m_dataFlipFlop = !m_dataFlipFlop;
	}

	void TMS9918::WriteRegister(BYTE reg)
	{
		switch (reg & 7)
		{
		case 0:
			m_config.m3 = GetBit(m_tempData, 1);
			UpdateMode();
			if (GetBit(m_tempData, 0))
			{
				LogPrintf(LOG_WARNING, "R0: External VDP Input not supported");
			}
			break;
		case 1:
			m_config.vram16k = GetBit(m_tempData, 7);
			m_config.enable = GetBit(m_tempData, 6);
			m_config.interruptEnabled = GetBit(m_tempData, 5);
			m_config.m1 = GetBit(m_tempData, 4);
			m_config.m2 = GetBit(m_tempData, 3);
			m_config.sprites16x16 = GetBit(m_tempData, 1);
			m_config.sprites2x = GetBit(m_tempData, 0);

			Sprite::SetSize(8 * ((m_config.sprites16x16) ? 2 : 1) * ((m_config.sprites2x ? 2 : 1)));
			Sprite::SetNameMask(m_config.sprites16x16 ? 0xFC : 0xFF);

			LogPrintf(LOG_DEBUG, "R1: 16K[%d] ENABLE[%d], INT_EN[%d], SPR16x16[%d], SPR2X[%d] (SPR_SIZE[%d])",
				m_config.vram16k,
				m_config.enable,
				m_config.interruptEnabled,
				m_config.sprites16x16,
				m_config.sprites2x,
				Sprite::GetSize());

			UpdateMode();
			if (!m_config.vram16k)
			{
				LogPrintf(LOG_WARNING, "R1: VRAM 4K not supported");
			}
			if (m_config.sprites2x)
			{
				LogPrintf(LOG_WARNING, "R1: Sprites 2x not supported");
			}
			break;
		case 2:
			m_tables.rawName = m_tempData & 15;
			m_tables.Update(m_mode);
			LogPrintf(LOG_INFO, "Name Table base address               = %04x", m_tables.name);
			break;
		case 3:
			m_tables.rawColor = m_tempData;
			m_tables.Update(m_mode);
			LogPrintf(LOG_INFO, "Color Table base address              = %04x", m_tables.color);
			break;
		case 4:
			m_tables.rawPattern = m_tempData & 7;
			m_tables.Update(m_mode);
			LogPrintf(LOG_INFO, "Pattern Gen Table base address        = %04x", m_tables.pattern);
			break;
		case 5:
			m_tables.rawSpriteAttr = m_tempData & 127;
			m_tables.Update(m_mode);
			LogPrintf(LOG_INFO, "Sprite Attr Table base address        = %04x", m_tables.spriteAttr);
			break;
		case 6:
			m_tables.rawSpritePattern = m_tempData & 7;
			m_tables.Update(m_mode);
			UpdateSpriteData();
			LogPrintf(LOG_INFO, "Sprite Pattern Gen Table base address = %04x", m_tables.spritePattern);
			break;
		case 7:
		{
			m_fgColor = ((m_tempData >> 4) & 0x0F);
			m_bgColor = (m_tempData & 0x0F);
			LogPrintf(LOG_INFO, "FG Color[%d], BG Color[%d]", m_fgColor, m_bgColor);
			break;
		}
		default:
			LogPrintf(LOG_ERROR, "WriteRegister: Invalid register %d", reg);
			break;
		}
	}

	BYTE TMS9918::ReadVRAMData()
	{
		LogPrintf(LOG_DEBUG, "ReadVRAMData, address=%04X", m_currReadAddress);

		m_dataFlipFlop = false;

		BYTE value = m_vram.read(m_currReadAddress++);
		m_currReadAddress &= m_addressMask;
		return value;
	}

	void TMS9918::WriteVRAMData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteVRAMData, address=%04X, value = %02X", m_currWriteAddress, value);

		m_dataFlipFlop = false;

		m_vram.write(m_currWriteAddress++, value);
		m_currWriteAddress &= m_addressMask;
	}

	void TMS9918::UpdateMode()
	{
		VideoMode mode = VideoMode::GRAPH_1;

		// TODO: Check other combinations/modes
		if (m_config.m1)
		{
			mode = VideoMode::TEXT;
		}
		else if (m_config.m2)
		{
			mode = VideoMode::MULTICOLOR;
		}
		else
		{
			mode = m_config.m3 ? VideoMode::GRAPH_2 : VideoMode::GRAPH_1;
		}

		if (mode != m_mode)
		{
			m_mode = mode;

			LogPrintf(LOG_INFO, "UpdateMode: [%s]", GetVideoModeStr(mode));
			m_tables.Update(m_mode);
		}
	}

	void TMS9918::Tables::Reset()
	{
		rawName = 0;
		rawColor = 0;
		rawPattern = 0;
		rawSpriteAttr = 0;
		rawSpritePattern = 0;
	}

	void TMS9918::Tables::Update(VideoMode mode)
	{
		name = rawName * 0x400;
		spriteAttr = rawSpriteAttr * 0x80;
		spritePattern = rawSpritePattern * 0x800;

		switch (mode)
		{
		case VideoMode::GRAPH_1:
			pattern = rawPattern * 0x800;
			patterns[0] = patterns[1] = patterns[2] = pattern;

			color = rawColor * 0x40;
			colors[0] = colors[1] = colors[2] = color;
			break;
		case VideoMode::GRAPH_2:
			pattern = (rawPattern & 0b100) * 0x800;
			patterns[0] = pattern + 0;
			patterns[1] = pattern + 2048;
			patterns[2] = pattern + 4096;

			color = (rawColor & 0b10000000) * 0x40;
			colors[0] = color + 0;
			colors[1] = color + 2048;
			colors[2] = color + 4096;
			break;
		}
	}

	void TMS9918::UpdateSpriteData()
	{
		m_sprites = (Sprite*)(m_vram.getPtr() + m_tables.spriteAttr);
	}

	void TMS9918::DrawGraph()
	{
		if (m_currX % 8 == 0)
		{
			const WORD patterns = m_tables.patterns[m_currName / 256];
			const WORD colors = m_tables.colors[m_currName / 256];

			const BYTE name = m_vram.read(m_tables.name + m_currName++);
			const WORD offset = (name * 8) + (m_currY % 8);
			const BYTE pixels = m_vram.read(patterns + offset);

			const BYTE color = m_vram.read(colors + ((m_mode == VideoMode::GRAPH_1) ? (name / 8) : offset));
			const BYTE fg = (color >> 4) & 0x0F;
			const BYTE bg = color & 0x0F;

			const uint32_t fgColor = GetColor(fg ? fg : m_bgColor);
			const uint32_t bgColor = GetColor(bg ? bg : m_bgColor);

			for (int i = 0; i < 8; ++i)
			{
				bool set = GetBit(pixels, 7 - i);
				m_video->DrawPixel(set ? fgColor : bgColor);
			}
		}
	}

	void TMS9918::DrawSpriteLine()
	{
		SpriteLine& line = m_spritePixels[0];
		line.fill(0);

		for (int i = 0; i < 4; ++i)
		{
			const Sprite* sprite = m_spriteDrawList[i];
			if (!sprite)
			{
				break;
			}
			const uint32_t color = GetColor(sprite->GetColor());
			const int patternLine = m_currY - sprite->GetY();
			const int xStart = sprite->GetX() + LEFT_BORDER;
			const int size = Sprite::GetSize();

			BYTE patternA = *(m_vram.getPtr() + GetSpritePatternBase(sprite->GetName()) + patternLine);
			BYTE patternC = *(m_vram.getPtr() + GetSpritePatternBase(sprite->GetName()) + patternLine + 0x10);
			// TODO: 8/16

			uint32_t* start = &line[xStart];
			for (int i = 0; i < 8; ++i)
			{
				if (GetBit(patternA, 7 - i))
				{
					*start = color;
				}
				++start;
			}
			if (m_config.sprites16x16)
			{
				for (int i = 0; i < 8; ++i)
				{
					if (GetBit(patternC, 7 - i))
					{
						*start = color;
					}
					++start;
				}
			}
		}
	}

	void TMS9918::UpdateSpriteDrawList()
	{
		m_spriteDrawList.fill(nullptr);

		int drawn = 0;
		for (int i = 0; i < 32; ++i)
		{
			const Sprite* sprite = GetSprite(i);
			if (sprite->IsLast())
				break;

			if (sprite->IsVisible(m_currY))
			{
				if (drawn == 4)
				{
					LogPrintf(LOG_DEBUG, ">4 sprites on line %d", m_currY);
					m_status.SetFifthSpriteFlag(i);
					break;
				}
				m_spriteDrawList[drawn++] = sprite;
			}
		}
	}

	void TMS9918::Serialize(json& to)
	{
		to["dataFlipFlop"]= m_dataFlipFlop;
		to["tempData"] = m_tempData;

		to["currReadAddress"] = m_currReadAddress;
		to["currWriteAddress"] = m_currWriteAddress;

		to["cfg.m1"] = m_config.m1;
		to["cfg.m2"] = m_config.m2;
		to["cfg.m3"] = m_config.m3;

		to["cfg.vram16k"] = m_config.vram16k;
		to["cfg.enable"] = m_config.enable;
		to["cfg.interruptEnabled"] = m_config.interruptEnabled;
		to["cfg.sprites16x16"] = m_config.sprites16x16;
		to["cfg.sprites2x"] = m_config.sprites2x;
		to["cfg.spriteSize"] = Sprite::GetSize();

		to["status.interrupt"] = m_status.interrupt;
		to["status.coincidence"] = m_status.coincidence;
		to["status.fifthSpriteFlag"] = m_status.fifthSpriteFlag;
		to["status.fifthSpriteName"] = m_status.fifthSpriteName;

		to["addr.name"] = m_tables.rawName;
		to["addr.color"] = m_tables.rawColor;
		to["addr.pattern"] = m_tables.rawPattern;
		to["addr.spriteAttr"] = m_tables.rawSpriteAttr;
		to["addr.spritePattern"] = m_tables.rawSpritePattern;

		to["fgColor"] = m_fgColor;
		to["bgColor"] = m_bgColor;

		to["currX"] = m_currX;
		to["currY"] = m_currY;
		to["currName"] = m_currName;

		std::string fileName = m_vram.GetId() + ".bin";
		fs::path path = GetSerializationDir() / fileName;
		m_vram.Dump(0, m_vram.GetSize(), path.string().c_str());
		to["vram"] = fileName;
	}
	void TMS9918::Deserialize(const json& from)
	{
		m_dataFlipFlop = from["dataFlipFlop"];
		m_tempData = from["tempData"];

		m_currReadAddress = from["currReadAddress"];
		m_currWriteAddress = from["currWriteAddress"];

		m_config.m1 = from["cfg.m1"];
		m_config.m2 = from["cfg.m2"];
		m_config.m3 = from["cfg.m3"];

		m_config.vram16k = from["cfg.vram16k"];
		m_config.enable = from["cfg.enable"];
		m_config.interruptEnabled = from["cfg.interruptEnabled"];
		m_config.sprites16x16 = from["cfg.sprites16x16"];
		m_config.sprites2x = from["cfg.sprites2x"];
		Sprite::SetSize(from["cfg.spriteSize"]);

		m_status.interrupt = from["status.interrupt"];
		m_status.coincidence = from["status.coincidence"];
		m_status.fifthSpriteFlag = from["status.fifthSpriteFlag"];
		m_status.fifthSpriteName = from["status.fifthSpriteName"];

		m_tables.rawName = from["addr.name"];
		m_tables.rawColor = from["addr.color"];
		m_tables.rawPattern = from["addr.pattern"];
		m_tables.rawSpriteAttr = from["addr.spriteAttr"];
		m_tables.rawSpritePattern = from["addr.spritePattern"];

		m_fgColor = from["fgColor"];
		m_bgColor = from["bgColor"];

		m_currX = from["currX"];
		m_currY = from["currY"];
		m_currName = from["currName"];

		std::string fileName = from["vram"];
		fs::path path = GetSerializationDir() / fileName;
		m_vram.LoadFromFile(path.string().c_str());

		UpdateMode();
		m_tables.Update(m_mode);
		UpdateSpriteData();
	}
}
