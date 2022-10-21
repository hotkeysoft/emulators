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

		m_config.m1 = false;
		m_config.m2 = false;
		m_config.m3 = false;
		UpdateMode();

		m_config.vram16k = false;
		m_config.enable = false;
		m_config.interruptEnabled = false;
		m_config.sprites16x16 = false;
		m_config.sprites2x = false;

		m_baseAddr.name = 0;
		m_baseAddr.color = 0;
		m_baseAddr.patternGen = 0;
		m_baseAddr.spriteAttr = 0;
		m_baseAddr.spritePattern = 0;

		m_fgColor = GetColor(0);
		m_bgColor = GetColor(0);

		m_currName = m_vram.getPtr() + m_baseAddr.name;
		m_currColor = m_vram.getPtr() + m_baseAddr.color;

		m_currPattern = m_vram.getPtr() + m_baseAddr.patternGen;
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
			m_video->NewLine();
			++m_currY;
			m_currX = -LEFT_BORDER;

			m_currName = m_vram.getPtr() + m_baseAddr.name + ((m_currY / 8) * 32);
		}

		if (m_currY == BOTTOM_BORDER)
		{
			m_video->RenderFrame();
			m_video->BeginFrame();

			m_currY = -LEFT_BORDER;
			m_currX = -TOP_BORDER;

			m_currName = m_vram.getPtr() + m_baseAddr.name;

			m_interrupt = true;
		}

		if (IsDisplay() && IsEnabled())
		{
			DrawMode1(); // TODO
		}
		else
		{
			m_video->DrawPixel(m_bgColor);
		}
	}

	BYTE TMS9918::ReadStatus()
	{
		BYTE value = m_interrupt << 7;

		// Reset interrupt bit
		m_interrupt = false;
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
				LogPrintf(LOG_INFO, "Set Read address = %04X", m_currReadAddress);
				break;
			case 1: // Write address setup
				m_currWriteAddress = MakeWord(value, m_tempData);
				LogPrintf(LOG_INFO, "Set Write address = %04X", m_currWriteAddress);
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
		switch (reg)
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

			LogPrintf(LOG_INFO, "R1: 16K[%d] ENABLE[%d], INT_EN[%d], SPR16x16[%d], SPR2X[%d]",
				m_config.vram16k,
				m_config.enable,
				m_config.interruptEnabled,
				m_config.sprites16x16,
				m_config.sprites2x);
			UpdateMode();
			if (!m_config.vram16k)
			{
				LogPrintf(LOG_WARNING, "R1: VRAM 4K not supported");
			}

			break;
		case 2:
			m_baseAddr.name = (m_tempData & 15) * 0x400;
			LogPrintf(LOG_INFO, "Name Table base address               = %04x", m_baseAddr.name);
			break;
		case 3:
			m_baseAddr.color = (m_tempData) * 0x40;
			LogPrintf(LOG_INFO, "Color Table base address              = %04x", m_baseAddr.color);
			break;
		case 4:
			m_baseAddr.patternGen = (m_tempData & 7) * 0x800;
			LogPrintf(LOG_INFO, "Pattern Gen Table base address        = %04x", m_baseAddr.patternGen);
			break;
		case 5:
			m_baseAddr.spriteAttr = (m_tempData & 127) * 0x80;
			LogPrintf(LOG_INFO, "Sprite Attr Table base address        = %04x", m_baseAddr.spriteAttr);
			break;
		case 6:
			m_baseAddr.spritePattern = (m_tempData & 7) * 0x800;
			LogPrintf(LOG_INFO, "Sprite Pattern Gen Table base address = %04x", m_baseAddr.spritePattern);
			break;
		case 7:
		{
			BYTE fg = GetColor((m_tempData >> 4) & 0x0F);
			BYTE bg = GetColor(m_tempData & 0x0F);
			LogPrintf(LOG_INFO, "FG Color[%d], BG Color[%d]", fg, bg);

			m_fgColor = GetColor(fg);
			m_bgColor = GetColor(bg);
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

		BYTE value = m_vram.read(m_currReadAddress++);
		m_currReadAddress &= m_addressMask;
		return value;
	}

	void TMS9918::WriteVRAMData(BYTE value)
	{
		LogPrintf(LOG_DEBUG, "WriteVRAMData, address=%04X, value = %02X", m_currWriteAddress, value);
		m_vram.write(m_currWriteAddress++, value);
		m_currWriteAddress &= m_addressMask;
	}

	void TMS9918::UpdateMode()
	{
		// TODO: Check other combinations/modes
		if (m_config.m1)
		{
			m_mode = VideoMode::TEXT;
		}
		else if (m_config.m2)
		{
			m_mode = VideoMode::MULTICOLOR;
		}
		else
		{
			m_mode = m_config.m3 ? VideoMode::GRAPH_2 : VideoMode::GRAPH_1;
		}

		LogPrintf(LOG_INFO, "UpdateMode: [%s]", GetVideoModeStr(m_mode));

		if (m_mode != VideoMode::GRAPH_1)
		{
			LogPrintf(LOG_WARNING, "UpdateMode: Not implemented [%s] ", GetVideoModeStr(m_mode));
		}
	}

	void TMS9918::DrawMode1()
	{
		if (m_currX % 8 == 0)
		{
			const BYTE name = *m_currName++;
			const BYTE pixels = m_vram.read(m_baseAddr.patternGen + (name * 8) + (m_currY % 8));
			const BYTE color = m_vram.read(m_baseAddr.color + (name / 8));
			const uint32_t fg = GetColor((color >> 4) & 0x0F);
			const uint32_t bg = GetColor(color & 0x0F);

			for (int i = 0; i < 8; ++i)
			{
				bool set = GetBit(pixels, 7 - i);
				m_video->DrawPixel(set ? fg : bg);
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

		to["addr.name"] = m_baseAddr.name;
		to["addr.color"] = m_baseAddr.color;
		to["addr.patternGen"] = m_baseAddr.patternGen;
		to["addr.spriteAttr"] = m_baseAddr.spriteAttr;
		to["addr.spritePattern"] = m_baseAddr.spritePattern;

		to["fgColor"] = m_fgColor;
		to["bgColor"] = m_bgColor;

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

		m_baseAddr.name = from["addr.name"];
		m_baseAddr.color = from["addr.color"];
		m_baseAddr.patternGen = from["addr.patternGen"];
		m_baseAddr.spriteAttr = from["addr.spriteAttr"];
		m_baseAddr.spritePattern = from["addr.spritePattern"];

		m_fgColor = from["fgColor"];
		m_bgColor = from["bgColor"];

		std::string fileName = from["vram"];
		fs::path path = GetSerializationDir() / fileName;
		m_vram.LoadFromFile(path.string().c_str());
	}
}
