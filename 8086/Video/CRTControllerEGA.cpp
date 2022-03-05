#include "CRTControllerEGA.h"
#include <assert.h>
#include <algorithm>

namespace crtc_ega
{
	using emul::GetBit;
	using emul::SetBit;
	using emul::SetLByte;
	using emul::SetHByte;

	static EventHandler s_defaultHandler;

	CRTController::CRTController(WORD baseAddress, BYTE charWidth) :
		Logger("crtcEGA"),
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
		ConnectPorts();
	}

	void CRTController::SetBasePort(WORD base)
	{
		DisconnectPorts();
		m_baseAddress = base;
		ConnectPorts();
	}

	void CRTController::DisconnectPorts()
	{
		DisconnectOutput(m_baseAddress + 4);
		DisconnectOutput(m_baseAddress + 5);
		DisconnectInput(m_baseAddress + 5);
	}

	void CRTController::ConnectPorts()
	{
		// CRTC Register Select
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&CRTController::SelectCRTCRegister));

		// CRTC Register Data
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCData));
	}

	void CRTController::SelectCRTCRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "SelectCRTCRegister, reg=%d", value);
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

	WORD GetEndValue(WORD startValue, BYTE endBits, BYTE endMask)
	{
		BYTE currLowBits = ((BYTE)startValue & endMask);
		if (currLowBits >= endBits)
		{
			// Catch the next one
			startValue |= endMask;
			++startValue;
		}
		// Clear low bits
		startValue &= ~((WORD)(endMask));
		return startValue | endBits;
	}

	void CRTController::WriteCRTCData(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteCRTCData, reg[%02x]=%02x", m_config.currRegister, value);

		switch (m_config.currRegister)
		{
		case CRT_H_TOTAL:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             hTotal = %d characters", value);
			m_config.hTotal = value;
			m_configChanged = true;
			break;
		case CRT_H_DISPLAYED:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hDisplayed = %d characters", value);
			m_config.hDisplayed = value;
			m_configChanged = true;
			break;

		case CRT_H_BLANK_START:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        hBlankStart = %d characters", value);
			m_config.hBlankStart = value;
			m_configChanged = true;
			break;
		case CRT_H_BLANK_END:
			m_config.hBlankEnd = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          hBlankEnd = %d characters", m_config.hBlankEnd);
			m_config.displayEnableSkew = (value >> 5) & 3;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:  displayEnableSkew = %d characters", m_config.displayEnableSkew);
			m_configChanged = true;
			break;

		case CRT_H_SYNC_START:
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hSyncStart = %d characters", value);
			m_config.hSyncStart = value;
			m_configChanged = true;
			break;
		case CRT_H_SYNC_END:
			m_config.hSyncEnd = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           hSyncEnd = %d characters", m_config.hSyncEnd);
			m_config.hSyncDelay = (value >> 5) & 3;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         hSyncDelay = %d characters", m_config.hSyncDelay);
			m_config.startOdd = GetBit(value, 7);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           startOdd = %d characters", m_config.startOdd);
			m_configChanged = true;
			break;

		case CRT_V_TOTAL:
			emul::SetLByte(m_config.vTotal, value);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             vTotal = %d", m_config.vTotal);
			m_configChanged = true;
			break;

		case CRT_OVERFLOW:
			// Sets the 9th bit for some parameters
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:               *overflow*");

			SetHByte(m_config.vTotal, GetBit(value, 0));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:            *vTotal = %d", m_config.vTotal);

			SetHByte(m_config.vDisplayed, GetBit(value, 1));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        *vDisplayed = %d", m_config.vDisplayed);

			SetHByte(m_config.vSyncStart, GetBit(value, 2));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        *vSyncStart = %d", m_config.vSyncStart);

			SetHByte(m_config.vBlankStart, GetBit(value, 3));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       *vBlankStart = %d", m_config.vBlankStart);

			SetHByte(m_config.lineCompare, GetBit(value, 4));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       *lineCompare = %d", m_config.lineCompare);

			m_configChanged = true;
			break;

		case CRT_PRESET_ROWSCAN:
			m_config.presetRowScan = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      presetRowScan = %d", m_config.presetRowScan);
			m_configChanged = true;
			break;

		case CRT_MAX_SCANLINE:
			m_config.maxScanlineAddress = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        maxScanline = %d scanlines", m_config.maxScanlineAddress);
			m_configChanged = true;
			break;

		case CRT_CURSOR_START_LINE:
			m_config.cursorStart = (value & 31);
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:        cursorStart = %d scanline", m_config.cursorStart);
			break;
		case CRT_CURSOR_END_LINE:
			m_config.cursorEnd = value & 31;
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:          cursorEnd = %d scanline", m_config.cursorEnd);
			m_config.cursorSkew = (value >> 5) & 3;
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:         cursorSkew = %d", m_config.cursorSkew);
			break;

		case CRT_START_ADDR_HI:
			LogPrintf(Logger::LOG_INFO,  "WriteCRTCData:   startAddress(HI) = %02Xh", value);
			emul::SetHByte(m_config.startAddress, value);
			break;
		case CRT_START_ADDR_LO:
			LogPrintf(Logger::LOG_INFO,  "WriteCRTCData:   startAddress(LO) = %02Xh", value);
			emul::SetLByte(m_config.startAddress, value);
			break;

		case CRT_CURSOR_ADDR_HI:
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:  cursorAddress(HI) = %02Xh", value);
			emul::SetHByte(m_config.cursorAddress, value);
			break;
		case CRT_CURSOR_ADDR_LO:
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:  cursorAddress(LO) = %02Xh", value);
			emul::SetLByte(m_config.cursorAddress, value);
			break;

		case CRT_V_SYNC_START:
			emul::SetLByte(m_config.vSyncStart, value);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         vSyncStart = %d", m_config.vSyncStart);
			m_configChanged = true;
			break;
		case CRT_V_SYNC_END:
			m_config.vSyncEnd = value & 15;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           vSyncEnd = %d", m_config.vSyncEnd);

			if (!GetBit(value, 4))
			{
				LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         *Clear VSync Interrupt*");
				m_interruptPending = false;
			}

			m_config.vSyncInterruptEnable = !GetBit(value, 5);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         vinterrupt = %s", m_config.vSyncInterruptEnable ? "ENABLED" : "DISABLED");
			m_configChanged = true;
			break;

		case CRT_V_DISPLAYED_END:
			emul::SetLByte(m_config.vDisplayed, value);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      vDisplayedEnd = %d", m_config.vDisplayed);
			m_configChanged = true;
			break;

		case CRT_OFFSET:
			m_config.offset = value;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:             offset = %d", m_config.offset);
			m_configChanged = true;
			break;

		case CRT_UNDERLINE_LOCATION:
			//TODO
			m_config.underlineLocation = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          underline = %d", m_config.underlineLocation);
			break;

		case CRT_V_BLANK_START:
			emul::SetLByte(m_config.vBlankStart, value);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        vBlankStart = %d", m_config.vBlankStart);
			m_configChanged = true;
			break;
		case CRT_V_BLANK_END:
			m_config.vBlankEnd = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          vBlankEnd = %d", m_config.vBlankEnd);
			m_configChanged = true;
			break;

		case CRT_MODE_CONTROL:
			m_config.compatibility = !GetBit(value, 0);
			m_config.selectRowScanCounter = GetBit(value, 1);
			m_config.vCounterDiv2 = GetBit(value, 2);
			m_config.countByTwo = GetBit(value, 3);
			m_config.disableOutputControl = GetBit(value, 4);
			m_config.addressWrap = GetBit(value, 5);
			m_config.byteAddressMode = GetBit(value, 6);
			//TODO bit 7:hardware reset
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData: [%cCOMPAT %cROWSCANCNT %cVDIV2 %cCOUNTBY2 %cDISABLEOUT %cADDRESWRAP ADDRESSMODE[%s]] ", 
				m_config.compatibility ? ' ' : '/',
				m_config.selectRowScanCounter ? ' ' : '/',
				m_config.vCounterDiv2 ? ' ' : '/',
				m_config.countByTwo ? ' ' : '/',
				m_config.disableOutputControl ? ' ' : '/',
				m_config.addressWrap ? ' ' : '/',
				m_config.byteAddressMode ? "BYTE" : "WORD");
			m_configChanged = true;

			//TODO
			if (m_config.selectRowScanCounter == false)
			{
				LogPrintf(Logger::LOG_WARNING, "selectRowScanCounter == 0 not implemented");
			}
			if (m_config.vCounterDiv2 == true)
			{
				LogPrintf(Logger::LOG_WARNING, "vCounterDiv2 == 1 not implemented");
			}
			if (m_config.countByTwo == true)
			{
				LogPrintf(Logger::LOG_WARNING, "countByTwo == 1 not implemented");
			}
			break;

		case CRT_LINE_COMPARE:
			emul::SetLByte(m_config.lineCompare, value);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        lineCompare = %d", m_config.lineCompare);
			m_configChanged = true;
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
		m_data.hTotalDisp = (m_config.hDisplayed + 1) * m_charWidth;
		m_data.hTotal = (m_config.hTotal + 2) * m_charWidth;

		m_data.hSyncMin = m_config.hSyncStart * m_charWidth;
		m_data.hSyncMax = (BYTE)GetEndValue(m_config.hSyncStart, m_config.hSyncEnd, 0b11111) * m_charWidth;

		m_data.vCharHeight = m_config.maxScanlineAddress + 1;
		m_data.vTotalDisp = m_config.vDisplayed + 1;
		m_data.vTotal = m_config.vTotal;

		m_data.vSyncMin = m_config.vSyncStart;
		m_data.vSyncMax = GetEndValue(m_config.vSyncStart, m_config.vSyncEnd, 0b1111);

		m_data.hBlankMin = m_config.hBlankStart * m_charWidth;
		m_data.hBlankMax =(BYTE)GetEndValue(m_config.hBlankStart, m_config.hBlankEnd, 0b11111) * m_charWidth;

		m_data.vBlankMin = m_config.vBlankStart;
		m_data.vBlankMax = GetEndValue(m_config.vBlankStart, m_config.vBlankEnd, 0b11111);

		m_data.offset = m_config.offset << 1;

		LogPrintf(LOG_INFO, "UpdateHVTotals: Displayed: [%d x %d], char width: %d", m_data.hTotalDisp, m_data.vTotalDisp, m_charWidth);
		LogPrintf(LOG_INFO, "UpdateHVTotals: Total:     [%d x %d]", m_data.hTotal, m_data.vTotal);
		LogPrintf(LOG_INFO, "UpdateHVTotals: hBlank:    [%d - %d]", m_data.hBlankMin, m_data.hBlankMax);
		LogPrintf(LOG_INFO, "UpdateHVTotals: hSync:     [%d - %d]", m_data.hSyncMin, m_data.hSyncMax);
		LogPrintf(LOG_INFO, "UpdateHVTotals: vBlank:    [%d - %d]", m_data.vBlankMin, m_data.vBlankMax);
		LogPrintf(LOG_INFO, "UpdateHVTotals: vSync:     [%d - %d]", m_data.vSyncMin, m_data.vSyncMax);
		LogPrintf(LOG_INFO, "UpdateHVTotals: offset:    [%d]", m_data.offset);
	}

	void CRTController::Tick()
	{
		m_data.hPos += m_charWidth;
		++m_data.memoryAddress;

		if (m_data.hPos == m_data.hBlankMax)
		{
			m_events->OnEndOfRow();
		}

		if (m_data.hPos >= m_data.hTotal)
		{
			m_data.hPos = 0;
			++m_data.vPos;
			++m_data.rowAddress;
			if (m_data.rowAddress > m_config.maxScanlineAddress)
			{
				m_data.rowAddress = 0;
				m_data.lineStartAddress += m_data.offset;			
			}

			if (m_data.vPos == m_config.lineCompare)
			{
				m_data.lineStartAddress = 0;		
				m_data.rowAddress = 0;
			}
			m_data.memoryAddress = m_data.lineStartAddress;

			if (m_config.vSyncInterruptEnable && (m_data.vPos == m_data.vTotalDisp))
			{
				LogPrintf(LOG_DEBUG, "Set Interrupt Pending");
				m_interruptPending = true;
			}

			if (m_data.vPos == m_data.vSyncMin)
			{
				m_events->OnNewFrame();
			}
			
			if (m_data.vPos >= m_data.vTotal)
			{
				m_events->OnRenderFrame();

				++m_data.frame;
				m_data.vPos = 0;
				m_data.rowAddress = m_config.presetRowScan;
				m_data.lineStartAddress = m_config.startAddress;
				m_data.memoryAddress = m_config.startAddress;

				if (m_configChanged)
				{
					m_configChanged = false;
					UpdateHVTotals();
					m_events->OnChangeMode();
				}

				if ((m_data.frame % 8) == 0) m_blink8 = !m_blink8;
				if ((m_data.frame % 16) == 0) m_blink16 = !m_blink16;
			}
		}
	}

	void CRTController::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "crtcEGA";

		json config;
		config["hTotal"] = m_config.hTotal;
		config["hDisplayed"] = m_config.hDisplayed;
		config["hBlankStart"] = m_config.hBlankStart;
		config["hBlankEnd"] = m_config.hBlankEnd;
		config["displayEnableSkew"] = m_config.displayEnableSkew;
		config["hSyncStart"] = m_config.hSyncStart;
		config["hSyncEnd"] = m_config.hSyncEnd;
		config["hSyncDelay"] = m_config.hSyncDelay;
		config["startOdd"] = m_config.startOdd;
		config["vTotal"] = m_config.vTotal;
		config["vDisplayed"] = m_config.vDisplayed;
		config["vBlankStart"] = m_config.vBlankStart;
		config["vBlankEnd"] = m_config.vBlankEnd;
		config["vSyncStart"] = m_config.vSyncStart;
		config["vSyncEnd"] = m_config.vSyncEnd;
		config["presetRowScan"] = m_config.presetRowScan;
		config["maxScanlineAddress"] = m_config.maxScanlineAddress;
		config["startAddress"] = m_config.startAddress;
		config["cursorAddress"] = m_config.cursorAddress;
		config["cursorStart"] = m_config.cursorStart;
		config["cursorEnd"] = m_config.cursorEnd;
		config["cursorSkew"] = m_config.cursorSkew;
		config["offset"] = m_config.offset;
		config["underlineLocation"] = m_config.underlineLocation;
		config["compatibility"] = m_config.compatibility;
		config["selectRowScanCounter"] = m_config.selectRowScanCounter;
		config["vCounterDiv2"] = m_config.vCounterDiv2;
		config["countByTwo"] = m_config.countByTwo;
		config["disableOutputControl"] = m_config.disableOutputControl;
		config["addressWrap"] = m_config.addressWrap;
		config["byteAddressMode"] = m_config.byteAddressMode;
		config["lineCompare"] = m_config.lineCompare;
		config["vSyncInterruptEnable"] = m_config.vSyncInterruptEnable;
		to["config"] = config;

		json data;
		data["hPos"] = m_data.hPos;
		data["vPos"] = m_data.vPos;
		//data["vPosChar"] = m_data.vPosChar;
		data["rowAddress"] = m_data.rowAddress;
		data["memoryAddress"] = m_data.memoryAddress;
		data["frame"] = m_data.frame;
		to["data"] = data;

		to["interruptPending"] = m_interruptPending;
		to["charWidth"] = m_charWidth;
		to["blink8"] = m_blink8;
		to["blink16"] = m_blink16;
	}

	void CRTController::Deserialize(json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("CRTController: Incompatible baseAddress");
		}

		if (from["id"] != "crtcEGA")
		{
			throw emul::SerializableException("CRTController: Incompatible mode");
		}

		const json& config = from["config"];
		m_config.hTotal = config["hTotal"];
		m_config.hDisplayed = config["hDisplayed"];
		m_config.hBlankStart = config["hBlankStart"];
		m_config.hBlankEnd = config["hBlankEnd"];
		m_config.displayEnableSkew = config["displayEnableSkew"];
		m_config.hSyncStart = config["hSyncStart"];
		m_config.hSyncEnd = config["hSyncEnd"];
		m_config.hSyncDelay = config["hSyncDelay"];
		m_config.startOdd = config["startOdd"];
		m_config.vTotal = config["vTotal"];
		m_config.vDisplayed = config["vDisplayed"];
		m_config.vBlankStart = config["vBlankStart"];
		m_config.vBlankEnd = config["vBlankEnd"];
		m_config.vSyncStart = config["vSyncStart"];
		m_config.vSyncEnd = config["vSyncEnd"];
		m_config.presetRowScan = config["presetRowScan"];
		m_config.maxScanlineAddress = config["maxScanlineAddress"];
		m_config.startAddress = config["startAddress"];
		m_config.cursorAddress = config["cursorAddress"];
		m_config.cursorStart = config["cursorStart"];
		m_config.cursorEnd = config["cursorEnd"];
		m_config.cursorSkew = config["cursorSkew"];
		m_config.offset = config["offset"];
		m_config.underlineLocation = config["underlineLocation"];
		m_config.compatibility = config["compatibility"];
		m_config.selectRowScanCounter = config["selectRowScanCounter"];
		m_config.vCounterDiv2 = config["vCounterDiv2"];
		m_config.countByTwo = config["countByTwo"];
		m_config.disableOutputControl = config["disableOutputControl"];
		m_config.addressWrap = config["addressWrap"];
		m_config.byteAddressMode = config["byteAddressMode"];
		m_config.lineCompare = config["lineCompare"];
		m_config.vSyncInterruptEnable = config["vSyncInterruptEnable"];

		const json& data = from["data"];
		m_data.hPos = data["hPos"];
		m_data.vPos = data["vPos"];
		m_data.rowAddress = data["rowAddress"];
		m_data.memoryAddress = data["memoryAddress"];
		m_data.frame = data["frame"];

		m_interruptPending = from["interruptPending"];
		m_charWidth = from["charWidth"];
		m_blink8 = from["blink8"];
		m_blink16 = from["blink16"];

		UpdateHVTotals();
	}
}
