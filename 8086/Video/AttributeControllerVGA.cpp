#include "stdafx.h"

#include "AttributeControllerVGA.h"

using emul::GetBit;

namespace attr_vga
{
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
		Connect(m_baseAddress + 0x0, static_cast<PortConnector::INFunction>(&AttrController::ReadAddress));
		Connect(m_baseAddress + 0x1, static_cast<PortConnector::INFunction>(&AttrController::ReadData));

		// For write, A0 doesn't care
		Connect(m_baseAddress + 0x0, static_cast<PortConnector::OUTFunction>(&AttrController::WriteData));
		Connect(m_baseAddress + 0x1, static_cast<PortConnector::OUTFunction>(&AttrController::WriteData));
	}

	void AttrController::DisconnectPorts()
	{
		DisconnectInput(m_baseAddress + 0x0);
		DisconnectInput(m_baseAddress + 0x1);

		DisconnectOutput(m_baseAddress + 0x0);
		DisconnectOutput(m_baseAddress + 0x1);
	}

	BYTE AttrController::ReadAddress()
	{
		return (BYTE)m_currRegister;
	}

	BYTE AttrController::ReadData()
	{
		LogPrintf(Logger::LOG_TRACE, "ReadData, address=%02x", m_currRegister);
		if (m_currRegister <= AttrControllerAddress::ATTR_PALETTE_MAX)
		{
			BYTE index = (BYTE)m_currRegister;
			return m_data.palette[index];
		}
		else switch (m_currRegister)
		{
		case AttrControllerAddress::ATTR_MODE_CONTROL:
			return (m_data.graphics << 0) |
				(m_data.monochrome << 1) |
				(m_data.extend8to9 << 2) |
				(m_data.blink << 3) |
				(m_data.pelPanCompatibility << 5) |
				(m_data.pelWidth << 6) |
				(m_data.p4p5Select << 7);
		case AttrControllerAddress::ATTR_OVERSCAN_COLOR:
			return m_data.overscanColor;
		case AttrControllerAddress::ATTR_COLOR_PLANE_EN:
			return m_data.colorPlaneEnable;
		case AttrControllerAddress::ATTR_H_PEL_PANNING:
			return m_data.hPelPanning;
		case AttrControllerAddress::ATTR_COLOR_SELECT:
			return m_data.colorSelect;
		default:
			LogPrintf(LOG_ERROR, "WriteData, Invalid Address %d", m_currRegister);
			return 0xFF;
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
					value &= 63;
					m_data.palette[index] = value;
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
				m_data.pelPanCompatibility = GetBit(value, 5);
				m_data.pelWidth = GetBit(value, 6);
				m_data.p4p5Select = GetBit(value, 7);

				LogPrintf(Logger::LOG_INFO, "WriteData, Mode control [%cGRAPH %cMONO %cEXTEND8TO9 %cBLINK %cPELCOMPAT %cPELWIDE %cP4P5SEL]",
					m_data.graphics ? ' ' : '/',
					m_data.monochrome ? ' ' : '/',
					m_data.extend8to9 ? ' ' : '/',
					m_data.blink ? ' ' : '/',
					m_data.pelPanCompatibility ? ' ' : '/',
					m_data.pelWidth ? ' ' : '/',
					m_data.p4p5Select ? ' ' : '/');

				if (m_data.pelPanCompatibility)
				{
					LogPrintf(LOG_WARNING, "WriteData, Pel Pan Compatibility not implemented");
				}
				if (m_data.pelWidth)
				{
					LogPrintf(LOG_WARNING, "WriteData, Pel Width not implemented");
				}
				if (m_data.p4p5Select)
				{
					LogPrintf(LOG_WARNING, "WriteData, P5, P4 Select not implemented");
				}
				break;
			case AttrControllerAddress::ATTR_OVERSCAN_COLOR:
				LogPrintf(LOG_DEBUG, "WriteData, Overscan Color %d", value);
				m_data.overscanColor = value;
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
				if (m_data.hPelPanning > 7)
				{
					m_data.hPelPanning = 0;
					LogPrintf(LOG_WARNING, "Pel Panning > 7 not implemented");
				}
				break;
			case AttrControllerAddress::ATTR_COLOR_SELECT:
				m_data.colorSelect = value & 15;
				LogPrintf(LOG_INFO, "WriteData, Color Select %d", m_data.colorSelect);
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
		to["lastDot"] = m_lastDot;

		to["paletteSource"] = m_data.paletteSource;
		to["palette"] = m_data.palette;
		to["graphics"] = m_data.graphics;
		to["monochrome"] = m_data.monochrome;
		to["extend8to9"] = m_data.extend8to9;
		to["blink"] = m_data.blink;
		to["pelPanCompatibility"] = m_data.pelPanCompatibility;
		to["pelWidth"] = m_data.pelWidth;
		to["p4p5Select"] = m_data.p4p5Select;
		to["overscanColor"] = m_data.overscanColor;
		to["colorPlaneEnable"] = m_data.colorPlaneEnable;
		to["videoStatusMux"] = m_data.videoStatusMux;
		to["hPelPanning"] = m_data.hPelPanning;
		to["colorSelect"] = m_data.colorSelect;
	}

	void AttrController::Deserialize(const json& from)
	{
		m_currMode = from["currMode"];
		m_currRegister = from["currRegister"];
		m_lastDot = from["lastDot"];

		m_data.paletteSource = from["paletteSource"];
		m_data.palette = from["palette"];
		m_data.graphics = from["graphics"];
		m_data.monochrome = from["monochrome"];
		m_data.extend8to9 = from["extend8to9"];
		m_data.blink = from["blink"];
		m_data.pelPanCompatibility = from["pelPanCompatibility"];
		m_data.pelWidth = from["pelWidth"];
		m_data.p4p5Select = from["p4p5Select"];
		m_data.overscanColor = from["overscanColor"];
		m_data.colorPlaneEnable = from["colorPlaneEnable"];
		m_data.videoStatusMux = from["videoStatusMux"];
		m_data.hPelPanning = from["hPelPanning"];
		m_data.colorSelect = from["colorSelect"];
	}


}
