#include "stdafx.h"

#include "AttributeControllerVGA.h"

using emul::GetBit;

namespace attr_vga
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

	uint32_t MONOtoARGB32(BYTE value)
	{
		// Bit3 = mono
		// Bit4 = intensity
		BYTE a = 0xFF;
		BYTE m = MakeColor8(GetBit(value, 3), GetBit(value, 4));

		return (a << 24) | (m << 16) | (m << 8) | m;
	}

	AttrController::AttrController(WORD baseAddress) :
		Logger("attrVGA"),
		m_baseAddress(baseAddress)
	{
	}

	void AttrController::Reset()
	{
	}

	void AttrController::Init()
	{
		Reset();
	}

	void AttrController::ConnectPorts()
	{
		Connect(m_baseAddress + 0x0, static_cast<PortConnector::OUTFunction>(&AttrController::WriteData));
		Connect(m_baseAddress + 0x1, static_cast<PortConnector::OUTFunction>(&AttrController::WriteData));
	}

	void AttrController::DisconnectPorts()
	{
		DisconnectOutput(m_baseAddress + 0x0);
		DisconnectOutput(m_baseAddress + 0x1);
	}

	uint32_t AttrController::GetRGB32Color(BYTE value)
	{
		if (m_data.monochrome)
		{
			return MONOtoARGB32(value);
		}
		else
		{
			return (m_colorMode == ColorMode::RGB4) ? RGB4toARGB32(value) : RGB6toARGB32(value);
		}
	}

	void AttrController::WriteData(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteData, value=%02x", value);
		if (m_currMode == RegisterMode::ADDRESS)
		{
			//ATTR_PALETTE_MAX
			value &= 31;
			m_currRegister = ((AttrControllerAddress)value > AttrControllerAddress::_ATTR_MAX) ? AttrControllerAddress::ATTR_INVALID : (AttrControllerAddress)value;

			m_currMode = RegisterMode::DATA;

			m_data.paletteSource = GetBit(value, 5) ? PaletteSource::VIDEO : PaletteSource::CPU;

		}
		else // DATA
		{
			if (m_currRegister <= AttrControllerAddress::ATTR_PALETTE_MAX)
			{
				if (m_data.paletteSource == PaletteSource::CPU)
				{
					BYTE index = (BYTE)m_currRegister;
					LogPrintf(LOG_INFO, "WriteData, Palette[%d]=%d", index, value);
					m_data.palette[index] = GetRGB32Color(value);
				}
				else
				{
					LogPrintf(LOG_WARNING, "WriteData, Trying to set palette with source=VIDEO");
				}
			}
			else switch (m_currRegister)
			{
			case AttrControllerAddress::ATTR_MODE_CONTROL:
				LogPrintf(LOG_DEBUG, "WriteData, Mode Control %d", value);

				m_data.graphics = GetBit(value, 0);
				m_data.monochrome = GetBit(value, 1);
				m_data.extend8to9 = GetBit(value, 2);
				m_data.blink = GetBit(value, 3);

				LogPrintf(Logger::LOG_INFO, "WriteData Mode control [%cGRAPH %cMONO %cEXTEND8TO9, %cBLINK]",
					m_data.graphics ? ' ' : '/',
					m_data.monochrome ? ' ' : '/',
					m_data.extend8to9 ? ' ' : '/',
					m_data.blink ? ' ' : '/');
				break;
			case AttrControllerAddress::ATTR_OVERSCAN_COLOR:
				LogPrintf(LOG_DEBUG, "WriteData, Overscan Color %d", value);
				m_data.overscanColor = GetRGB32Color(value);
				break;
			case AttrControllerAddress::ATTR_COLOR_PLANE_EN:
				LogPrintf(LOG_DEBUG, "WriteData, Color Plane Enable %d", value);

				m_data.colorPlaneEnable = value & 15;
				LogPrintf(LOG_INFO, "WriteData, Color Plane Enable %02x", m_data.colorPlaneEnable);

				m_data.videoStatusMux = (value >> 4) & 3;
				LogPrintf(LOG_INFO, "WriteData, Video Status Mux %02x", m_data.videoStatusMux);
				break;
			case AttrControllerAddress::ATTR_H_PEL_PANNING:
				m_data.hPelPanning = value & 15;
				LogPrintf(LOG_INFO, "WriteData, Horizontal Pel Panning %d", m_data.hPelPanning);
				break;
			default:
				LogPrintf(LOG_ERROR, "WriteData, Invalid Address %d", m_currRegister);
			}

			m_currMode = RegisterMode::ADDRESS;
		}
	}

	void AttrController::Serialize(json& to)
	{
		to["currMode"] = m_currMode;
		to["currRegister"] = m_currRegister;

		to["paletteSource"] = m_data.paletteSource;
		to["palette"] = m_data.palette;
		to["graphics"] = m_data.graphics;
		to["monochrome"] = m_data.monochrome;
		to["extend8to9"] = m_data.extend8to9;
		to["blink"] = m_data.blink;
		to["overscanColor"] = m_data.overscanColor;
		to["colorPlaneEnable"] = m_data.colorPlaneEnable;
		to["videoStatusMux"] = m_data.videoStatusMux;
		to["hPelPanning"] = m_data.hPelPanning;
	}

	void AttrController::Deserialize(const json& from)
	{
		m_currMode = from["currMode"];
		m_currRegister = from["currRegister"];

		m_data.paletteSource = from["paletteSource"];
		m_data.palette = from["palette"];
		m_data.graphics = from["graphics"];
		m_data.monochrome = from["monochrome"];
		m_data.extend8to9 = from["extend8to9"];
		m_data.blink = from["blink"];
		m_data.overscanColor = from["overscanColor"];
		m_data.colorPlaneEnable = from["colorPlaneEnable"];
		m_data.videoStatusMux = from["videoStatusMux"];
		m_data.hPelPanning = from["hPelPanning"];
	}


}
