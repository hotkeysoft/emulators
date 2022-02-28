#include "CRTController6845.h"
#include <assert.h>
#include <algorithm>

namespace crtc_6845
{
	static EventHandler s_defaultHandler;

	CRTController::CRTController(WORD baseAddress, BYTE charWidth) :
		Logger("crtc"),
		m_baseAddress(baseAddress),
		m_charWidth(charWidth),
		m_events(&s_defaultHandler)
	{
	}

	CRTController::~CRTController()
	{
	}

	void CRTController::Reset()
	{
		m_data.hPos = 0;
		m_data.vPos = 0;
	}

	void CRTController::Init()
	{
		Reset();

		// CRTC Register Select
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&CRTController::SelectCRTCRegister));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&CRTController::SelectCRTCRegister));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&CRTController::SelectCRTCRegister));
		Connect(m_baseAddress + 6, static_cast<PortConnector::OUTFunction>(&CRTController::SelectCRTCRegister));

		// CRTC Register Data
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCData));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCData));
		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCData));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCData));
		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCData));
		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCData));
	}

	void CRTController::SelectCRTCRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "SelectCRTCRegister, reg=%d", value);
		value &= 31;
		m_config.currRegister = (value > _CRT_MAX_REG) ? CRT_INVALID_REG : (CRTRegister)value;
	}

	BYTE CRTController::ReadCRTCData()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadCRTCData, reg=%d", m_config.currRegister);

		switch (m_config.currRegister)
		{
		case CRT_CURSOR_ADDR_HI:
			return (m_config.cursorAddress >> 8);
		case CRT_CURSOR_ADDR_LO:
			return (BYTE)m_config.cursorAddress;

		case CRT_LIGHT_PEN_HI:
		case CRT_LIGHT_PEN_LO:
		default:
			return 0xFF;
		}
	}
	void CRTController::WriteCRTCData(BYTE value)
	{
		switch (m_config.currRegister)
		{
		case CRT_H_TOTAL_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             hTotal = %d characters", value);
			m_config.hTotal = value;
			m_configChanged = true;
			break;
		case CRT_H_DISPLAYED_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hDisplayed = %d characters", value);
			m_config.hDisplayed = value;
			m_configChanged = true;
			break;
		case CRT_H_SYNC_POS_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           hSyncPos = %d characters", value);
			m_config.hSyncPos = value;
			m_configChanged = true;
			break;
		case CRT_H_SYNC_WIDTH_CHAR:
			m_config.hSyncWidth = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hSyncWidth = %d characters", m_config.hSyncWidth);
			m_configChanged = true;
			break;

		case CRT_V_TOTAL_ROW:
			m_config.vTotal = value & 127;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             vTotal = %d rows", m_config.vTotal);
			m_configChanged = true;
			break;
		case CRT_V_TOTAL_ADJ_LINES:
			m_config.vTotalAdjust = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       vTotalAdjust = %d scanlines", m_config.vTotalAdjust);
			m_configChanged = true;
			break;
		case CRT_V_DISPLAYED_ROW:
			m_config.vTotalDisplayed = value & 127;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:    vTotalDisplayed = %d rows", m_config.vTotalDisplayed);
			m_configChanged = true;
			break;
		case CRT_V_SYNC_POS_ROW:
			m_config.vSyncPos = value & 127;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           vSyncPos = %d rows", m_config.vSyncPos);
			m_configChanged = true;
			break;

		case CRT_INTERLACE_MODE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      interlaceMode = %d", value);
			m_config.interlaceMode = value; // TODO Not implemented
			break;

		case CRT_MAX_SCANLINE_ADDR:
			m_config.maxScanlineAddress = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData: maxScanlineAddress = %d scanlines", m_config.maxScanlineAddress);
			m_configChanged = true;
			break;

		case CRT_CURSOR_START_LINE:
			m_config.cursorStart = (value & 31);
			m_config.cursor = (CRTCConfig::CURSOR)((value >> 5) & 3);
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:        cursorStart = %d scanline, %d", m_config.cursorStart, m_config.cursor);
			break;
		case CRT_CURSOR_END_LINE:
			m_config.cursorEnd = (value & 31);
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:          cursorEnd = %d scanline", m_config.cursorEnd);
			break;

		case CRT_START_ADDR_HI:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:    startAddress(HI) = %02Xh", value);
			emul::SetHByte(m_config.startAddress, value & 63);
			break;
		case CRT_START_ADDR_LO:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:    startAddress(LO) = %02Xh", value);
			emul::SetLByte(m_config.startAddress, value);
			break;

		case CRT_CURSOR_ADDR_HI:
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:  cursorAddress(HI) = %02Xh", value);
			emul::SetHByte(m_config.cursorAddress, value & 63);
			break;
		case CRT_CURSOR_ADDR_LO:
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:  cursorAddress(LO) = %02Xh", value);
			emul::SetLByte(m_config.cursorAddress, value);
			break;

		default:
			LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: Invalid Register, reg=%d", m_config.currRegister);
		}
	}

	void CRTController::SetCharWidth(BYTE charWidth)
	{
		m_charWidth = charWidth;
		UpdateHVTotals();
	}

	void CRTController::UpdateHVTotals()
	{
		m_data.hTotalDisp = m_config.hDisplayed * m_charWidth;
		m_data.hTotal = (m_config.hTotal + 1) * m_charWidth;

		m_data.hSyncMin = m_config.hSyncPos * m_charWidth;
		m_data.hSyncMax = std::min(m_data.hTotal, (WORD)(m_data.hSyncMin + (m_config.hSyncWidth * m_charWidth)));

		m_data.vCharHeight = (m_config.maxScanlineAddress + 1);
		m_data.vTotalDisp = m_config.vTotalDisplayed * m_data.vCharHeight;
		m_data.vTotal = (m_config.vTotal + 1) * m_data.vCharHeight + m_config.vTotalAdjust;

		m_data.vSyncMin = m_config.vSyncPos * m_data.vCharHeight;
		m_data.vSyncMax = std::min(m_data.vTotal, (WORD)(m_data.vSyncMin + 16));

		LogPrintf(LOG_INFO, "UpdateHVTotals: [%d x %d], char width: %d", m_data.hTotalDisp, m_data.vTotalDisp, m_charWidth);
	}

	void CRTController::Tick()
	{
		m_data.hPos += m_charWidth;
		++m_data.memoryAddress;

		if (m_data.hPos == m_data.hSyncMin)
		{
			m_events->OnEndOfRow();
		}

		if (m_data.vPos == m_data.vSyncMin)
		{
			m_events->OnNewFrame();
		}

		if (m_data.hPos >= m_data.hTotal)
		{
			m_data.hPos = 0;
			++m_data.vPos;
			++m_data.rowAddress;
			if (m_data.rowAddress > m_config.maxScanlineAddress)
			{
				m_data.rowAddress = 0;
				++m_data.vPosChar;
			}
			m_data.memoryAddress = m_config.startAddress + (m_data.vPosChar * m_config.hDisplayed);
		}

		if (m_data.vPos >= m_data.vTotal)
		{
			m_events->OnRenderFrame();

			++m_data.frame;
			m_data.vPos = 0;
			m_data.vPosChar = 0;
			m_data.rowAddress = 0;
			m_data.memoryAddress = m_config.startAddress;

			if (m_configChanged)
			{
				m_configChanged = false;
				UpdateHVTotals();
				m_events->OnChangeMode();
			}

			if ((m_data.frame % 16) == 0) m_blink16 = !m_blink16;
			if ((m_data.frame % 32) == 0) m_blink32 = !m_blink32;
		}
	}

	void CRTController::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "6845";

		json config;
		config["hTotal"] = m_config.hTotal;
		config["hDisplayed"] = m_config.hDisplayed;
		config["hSyncPos"] = m_config.hSyncPos;
		config["hSyncWidth"] = m_config.hSyncWidth;
		config["vTotal"] = m_config.vTotal;
		config["vTotalAdjust"] = m_config.vTotalAdjust;
		config["vTotalDisplayed"] = m_config.vTotalDisplayed;
		config["vSyncPos"] = m_config.vSyncPos;
		config["interlaceMode"] = m_config.interlaceMode;
		config["maxScanlineAddress"] = m_config.maxScanlineAddress;
		config["startAddress"] = m_config.startAddress;
		config["cursorAddress"] = m_config.cursorAddress;
		config["cursorStart"] = m_config.cursorStart;
		config["cursorEnd"] = m_config.cursorEnd;
		config["cursor"] = m_config.cursor;
		to["config"] = config;

		json data;
		data["hPos"] = m_data.hPos;
		data["vPos"] = m_data.vPos;
		data["vPosChar"] = m_data.vPosChar;
		data["rowAddress"] = m_data.rowAddress;
		data["memoryAddress"] = m_data.memoryAddress;
		data["frame"] = m_data.frame;
		to["data"] = data;

		to["charWidth"] = m_charWidth;
		to["blink16"] = m_blink16;
		to["blink32"] = m_blink32;
	}

	void CRTController::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("CRTController: Incompatible baseAddress");
		}

		if (from["id"] != "6845")
		{
			throw emul::SerializableException("CRTController: Incompatible mode");
		}

		const json& config = from["config"];
		m_config.hTotal = config["hTotal"];
		m_config.hDisplayed = config["hDisplayed"];
		m_config.hSyncPos = config["hSyncPos"];
		m_config.hSyncWidth = config["hSyncWidth"];
		m_config.vTotal = config["vTotal"];
		m_config.vTotalAdjust = config["vTotalAdjust"];
		m_config.vTotalDisplayed = config["vTotalDisplayed"];
		m_config.vSyncPos = config["vSyncPos"];
		m_config.interlaceMode = config["interlaceMode"];
		m_config.maxScanlineAddress = config["maxScanlineAddress"];
		m_config.startAddress = config["startAddress"];
		m_config.cursorAddress = config["cursorAddress"];
		m_config.cursorStart = config["cursorStart"];
		m_config.cursorEnd = config["cursorEnd"];
		m_config.cursor = config["cursor"];

		const json& data = from["data"];
		m_data.hPos = data["hPos"];
		m_data.vPos = data["vPos"];
		m_data.vPosChar = data["vPosChar"];
		m_data.rowAddress = data["rowAddress"];
		m_data.memoryAddress = data["memoryAddress"];
		m_data.frame = data["frame"];

		m_charWidth = from["charWidth"];
		m_blink16 = from["blink16"];
		m_blink32 = from["blink32"];

		UpdateHVTotals();
	}
}
