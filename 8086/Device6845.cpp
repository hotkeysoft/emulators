#include "Device6845.h"
#include <assert.h>

namespace crtc
{
	Device6845::Device6845(WORD baseAddress, BYTE charWidth) :
		Logger("crtc"),
		m_baseAddress(baseAddress),
		m_charWidth(charWidth)
	{
		Reset();
	}

	Device6845::~Device6845()
	{
	}

	void Device6845::Reset()
	{
		m_data.hPos = 0;
		m_data.vPos = 0;
	}

	void Device6845::Init()
	{
		// CRTC Register Select
		Connect(m_baseAddress + 0, static_cast<PortConnector::OUTFunction>(&Device6845::SelectCRTCRegister));
		Connect(m_baseAddress + 2, static_cast<PortConnector::OUTFunction>(&Device6845::SelectCRTCRegister));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&Device6845::SelectCRTCRegister));
		Connect(m_baseAddress + 6, static_cast<PortConnector::OUTFunction>(&Device6845::SelectCRTCRegister));

		// CRTC Register Data
		Connect(m_baseAddress + 1, static_cast<PortConnector::OUTFunction>(&Device6845::WriteCRTCData));
		Connect(m_baseAddress + 3, static_cast<PortConnector::OUTFunction>(&Device6845::WriteCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&Device6845::WriteCRTCData));
		Connect(m_baseAddress + 7, static_cast<PortConnector::OUTFunction>(&Device6845::WriteCRTCData));

		Connect(m_baseAddress + 1, static_cast<PortConnector::INFunction>(&Device6845::ReadCRTCData));
		Connect(m_baseAddress + 3, static_cast<PortConnector::INFunction>(&Device6845::ReadCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&Device6845::ReadCRTCData));
		Connect(m_baseAddress + 7, static_cast<PortConnector::INFunction>(&Device6845::ReadCRTCData));
	}

	void Device6845::SelectCRTCRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_DEBUG, "SelectCRTCRegister, reg=%d", value);
		value &= 31;
		m_config.currRegister = (value > _CRT_MAX_REG) ? CRT_INVALID_REG : (CRTRegister)value;
	}

	BYTE Device6845::ReadCRTCData()
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
	void Device6845::WriteCRTCData(BYTE value)
	{
		switch (m_config.currRegister)
		{
		case CRT_H_TOTAL_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             hTotal = %d characters", value);
			m_config.hTotal = value;
			UpdateHVTotals();
			break;
		case CRT_H_DISPLAYED_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hDisplayed = %d characters", value);
			m_config.hDisplayed = value;
			UpdateHVTotals();
			break;
		case CRT_H_SYNC_POS_CHAR:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           hSyncPos = %d characters", value);
			m_config.hSyncPos = value;
			UpdateHVTotals();
			break;
		case CRT_H_SYNC_WIDTH_CHAR:
			m_config.hSyncWidth = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hSyncWidth = %d characters", m_config.hSyncWidth);
			UpdateHVTotals();
			break;

		case CRT_V_TOTAL_ROW:
			m_config.vTotal = value & 127;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             vTotal = %d rows", m_config.vTotal);
			UpdateHVTotals();
			break;
		case CRT_V_TOTAL_ADJ_LINES:
			m_config.vTotalAdjust = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       vTotalAdjust = %d scanlines", m_config.vTotalAdjust);
			UpdateHVTotals();
			break;
		case CRT_V_DISPLAYED_ROW:
			m_config.vTotalDisplayed = value & 127;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:    vTotalDisplayed = %d rows", m_config.vTotalDisplayed);
			UpdateHVTotals();
			break;
		case CRT_V_SYNC_POS_ROW:
			m_config.vSyncPos = value & 127;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           vSyncPos = %d rows", m_config.vSyncPos);
			UpdateHVTotals();
			break;

		case CRT_INTERLACE_MODE:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      interlaceMode = %d", value);
			m_config.interlaceMode = value;
			break;

		case CRT_MAX_SCANLINE_ADDR:
			m_config.maxScanlineAddress = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData: maxScanlineAddress = %d scanlines", m_config.maxScanlineAddress);
			UpdateHVTotals();
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

	void Device6845::UpdateHVTotals()
	{
		m_data.hTotalDisp = m_config.hDisplayed * m_charWidth;
		m_data.hTotal = m_config.hTotal * m_charWidth;

		m_data.vCharHeight = (m_config.maxScanlineAddress + 1);
		m_data.vTotalDisp = m_config.vTotalDisplayed * m_data.vCharHeight;
		m_data.vTotal = m_config.vTotal * m_data.vCharHeight + m_config.vTotalAdjust;

		LogPrintf(LOG_INFO, "UpdateHVTotals: [%d x %d], char width: %d", m_data.hTotalDisp, m_data.vTotalDisp, m_charWidth);
	}

	void Device6845::Tick()
	{
		m_data.hPos += m_charWidth;

		if (m_data.hPos >= m_data.hTotal)
		{
			EndOfRow();
			m_data.hPos = 0;
			++m_data.vPos;
		}

		if (m_data.vPos > m_data.vTotal)
		{
			RenderFrame();

			++m_data.frame;
			m_data.vPos = 0;

			NewFrame();

			if ((m_data.frame % 16) == 0) m_blink16 = !m_blink16;
			if ((m_data.frame % 32) == 0) m_blink32 = !m_blink32;
		}
	}
}
