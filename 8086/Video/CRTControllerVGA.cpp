#include "stdafx.h"

#include "CRTControllerVGA.h"
#include <algorithm>

namespace crtc_vga
{
	using emul::GetBit;
	using emul::SetBit;
	using emul::SetLByte;
	using emul::SetHByte;

	CRTController::CRTController(WORD baseAddress, BYTE charWidth) :
		Logger("crtcVGA"),
		m_baseAddress(baseAddress)
	{
		m_data.charWidth = charWidth;
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
	}

	void CRTController::SetBasePort(WORD base)
	{
		DisconnectPorts();
		m_baseAddress = base;
		ConnectPorts();
	}

	void CRTController::DisconnectPorts()
	{
		DisconnectInput(m_baseAddress + 4);
		DisconnectOutput(m_baseAddress + 4);

		DisconnectInput(m_baseAddress + 5);
		DisconnectOutput(m_baseAddress + 5);
	}

	void CRTController::ConnectPorts()
	{
		// CRTC Register Select
		Connect(m_baseAddress + 4, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCRegister));
		Connect(m_baseAddress + 4, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCRegister));

		// CRTC Register Data
		Connect(m_baseAddress + 5, static_cast<PortConnector::OUTFunction>(&CRTController::WriteCRTCData));
		Connect(m_baseAddress + 5, static_cast<PortConnector::INFunction>(&CRTController::ReadCRTCData));
	}

	BYTE CRTController::ReadCRTCRegister()
	{
		return m_config.currRegister;
	}

	void CRTController::WriteCRTCRegister(BYTE value)
	{
		LogPrintf(Logger::LOG_TRACE, "WriteCRTCRegister, reg=%d", value);
		value &= 31;
		m_config.currRegister = (value > _CRT_MAX_REG) ? CRT_INVALID_REG : (CRTRegister)value;
	}

	BYTE CRTController::ReadCRTCData()
	{
		LogPrintf(Logger::LOG_DEBUG, "ReadCRTCData, reg=%d", m_config.currRegister);
		if (m_config.currRegister <= _CRT_MAX_REG)
		{
			return m_rawData[m_config.currRegister];
		}
		else
		{
			LogPrintf(LOG_WARNING, "ReadCRTCData, Invalid register %d", m_config.currRegister);
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

		m_rawData[m_config.currRegister] = value;

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
			SetBit(m_config.hBlankEnd, 5, GetBit(value, 7));
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

			SetHByte(m_config.vTotal, 0);
			SetBit(m_config.vTotal, 8, GetBit(value, 0));
			SetBit(m_config.vTotal, 9, GetBit(value, 5));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:            *vTotal = %d", m_config.vTotal);

			SetHByte(m_config.vDisplayed, 0);
			SetBit(m_config.vDisplayed, 8, GetBit(value, 1));
			SetBit(m_config.vDisplayed, 9, GetBit(value, 6));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        *vDisplayed = %d", m_config.vDisplayed);

			SetHByte(m_config.vSyncStart, 0);
			SetBit(m_config.vSyncStart, 8, GetBit(value, 2));
			SetBit(m_config.vSyncStart, 9, GetBit(value, 7));

			SetBit(m_config.vBlankStart, 8, GetBit(value, 3));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       *vBlankStart = %d", m_config.vBlankStart);

			SetBit(m_config.lineCompare, 8, GetBit(value, 4));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       *lineCompare = %d", m_config.lineCompare);

			m_configChanged = true;
			break;

		case CRT_PRESET_ROWSCAN:
			m_config.presetRowScan = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:      presetRowScan = %d", m_config.presetRowScan);
			m_config.bytePanning = (value >> 5) & 3;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       byte panning = %d", m_config.bytePanning);
		
			if (m_config.bytePanning > 0)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: byte panning > 0 not implemented");
			}
			m_configChanged = true;
			break;

		case CRT_MAX_SCANLINE:
			m_config.maxScanlineAddress = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        maxScanline = %d scanlines", m_config.maxScanlineAddress);

			SetBit(m_config.vBlankStart, 9, GetBit(value, 5));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       *vBlankStart = %d", m_config.vBlankStart);

			SetBit(m_config.lineCompare, 9, GetBit(value, 6));
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:       *lineCompare = %d", m_config.lineCompare);

			m_config.doubleScan = GetBit(value, 7);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        double scan = %d", m_config.doubleScan);

			m_configChanged = true;
			break;

		case CRT_CURSOR_START_LINE:
			m_config.cursorStart = (value & 31);
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:        cursorStart = %d scanline", m_config.cursorStart);

			m_config.cursorOff = GetBit(value, 5);
			LogPrintf(Logger::LOG_DEBUG, "WriteCRTCData:          cursorOff = %d", m_config.cursorOff);

			if (m_config.cursorOff)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: cursor off not implemented");
			}
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

			m_config.enableVerticalInterrupt = !GetBit(value, 5);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         vinterrupt = %s", m_config.enableVerticalInterrupt ? "ENABLED" : "DISABLED");

			m_config.select5RefreshCycles = GetBit(value, 6);
			if (m_config.select5RefreshCycles)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: Select 5 Refresh Cycles not implemented");
			}

			m_config.protectRegisters0to7 = GetBit(value, 7);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         protect0-7 = %s", m_config.protectRegisters0to7 ? "ENABLED" : "DISABLED");

			if (m_config.protectRegisters0to7)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: protect0-7 registers not implemented");
			}
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
			m_config.underlineLocation = value & 31;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          underline = %d", m_config.underlineLocation);

			m_config.countByFour = GetBit(value, 5);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:           countBy4 = %d", m_config.countByFour);
			if (m_config.countByFour)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: Count By 4 not implemented");
			}

			m_config.doubleWordAddressMode = GetBit(value, 6);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:         doubleWord = %d", m_config.doubleWordAddressMode);
			break;

		case CRT_V_BLANK_START:
			emul::SetLByte(m_config.vBlankStart, value);
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:        vBlankStart = %d", m_config.vBlankStart);
			m_configChanged = true;
			break;
		case CRT_V_BLANK_END:
			m_config.vBlankEnd = value;
			LogPrintf(Logger::LOG_INFO, "WriteCRTCData:          vBlankEnd = %d", m_config.vBlankEnd);
			m_configChanged = true;
			break;

		case CRT_MODE_CONTROL:
			m_config.compatibility = !GetBit(value, 0);
			m_config.selectRowScanCounter = !GetBit(value, 1);
			m_config.vCounterDiv2 = GetBit(value, 2);
			m_config.countByTwo = GetBit(value, 3);
			
			m_config.addressWrap = GetBit(value, 5);
			m_config.byteAddressMode = GetBit(value, 6);

			// TODO
			m_config.reset = GetBit(value, 7);

			LogPrintf(Logger::LOG_INFO, "WriteCRTCData: [%cCOMPAT %cROWSCANCNT %cVDIV2 %cCOUNTBY2 %cADDRESWRAP ADDRESSMODE[%s]] ", 
				m_config.compatibility ? ' ' : '/',
				m_config.selectRowScanCounter ? ' ' : '/',
				m_config.vCounterDiv2 ? ' ' : '/',
				m_config.countByTwo ? ' ' : '/',
				m_config.addressWrap ? ' ' : '/',
				m_config.byteAddressMode ? "BYTE" : "WORD");
			m_configChanged = true;

			//TODO
			if (m_config.vCounterDiv2 == true)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: vCounterDiv2 == 1 not implemented");
			}
			if (m_config.countByTwo == true)
			{
				LogPrintf(Logger::LOG_WARNING, "WriteCRTCData: countByTwo == 1 not implemented");
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

		if (m_configChanged)
		{
			UpdateHVTotals();
		}
	}

	void CRTController::SetCharWidth(BYTE charWidth)
	{
		m_data.charWidth = charWidth;
		UpdateHVTotals(true);
	}

	void CRTController::UpdateHVTotals(bool log)
	{
		m_data.hTotalDisp = (m_config.hDisplayed + 1) * m_data.charWidth;
		m_data.hTotal = (m_config.hTotal + 5) * m_data.charWidth;

		m_data.hBlankMin = m_config.hBlankStart * m_data.charWidth;
		m_data.hBlankMax = (BYTE)GetEndValue(m_config.hBlankStart, m_config.hBlankEnd, 0b111111) * m_data.charWidth;

		m_data.hSyncMin = m_config.hSyncStart * m_data.charWidth;
		m_data.hSyncMax = (BYTE)GetEndValue(m_config.hSyncStart, m_config.hSyncEnd, 0b11111) * m_data.charWidth;

		m_data.vCharHeight = m_config.maxScanlineAddress + 1;
		m_data.vTotalDisp = m_config.vDisplayed + 1;
		m_data.vTotal = m_config.vTotal + 2;

		m_data.vSyncMin = m_config.vSyncStart;
		m_data.vSyncMax = GetEndValue(m_config.vSyncStart, m_config.vSyncEnd, 0b1111);

		m_data.vBlankMin = m_config.vBlankStart + 1;
		m_data.vBlankMax = GetEndValue(m_config.vBlankStart, m_config.vBlankEnd, 0b11111111);

		m_data.offset = m_config.offset << 1;

		if (log)
		{
			LogPrintf(LOG_INFO, "UpdateHVTotals: Displayed: [%d x %d], char width: %d", m_data.hTotalDisp, m_data.vTotalDisp, m_data.charWidth);
			LogPrintf(LOG_INFO, "UpdateHVTotals: Total:     [%d x %d]", m_data.hTotal, m_data.vTotal);
			LogPrintf(LOG_INFO, "UpdateHVTotals: hBlank:    [%d - %d]", m_data.hBlankMin, m_data.hBlankMax);
			LogPrintf(LOG_INFO, "UpdateHVTotals: hSync:     [%d - %d]", m_data.hSyncMin, m_data.hSyncMax);
			LogPrintf(LOG_INFO, "UpdateHVTotals: vBlank:    [%d - %d]", m_data.vBlankMin, m_data.vBlankMax);
			LogPrintf(LOG_INFO, "UpdateHVTotals: vSync:     [%d - %d]", m_data.vSyncMin, m_data.vSyncMax);
			LogPrintf(LOG_INFO, "UpdateHVTotals: offset:    [%d]", m_data.offset);
		}
	}

	void CRTController::Tick()
	{	
		m_data.hPos += m_data.charWidth;
		++m_data.memoryAddress;

		if (m_data.hPos == m_data.hBlankMax)
		{
			FireEndOfRow();
		}

		if (m_data.hPos >= m_data.hTotal)
		{
			m_data.hPos = 0;

			++m_data.vPos;

			// This is a bit confusing, most detailed info found 
			// in the Chips 82C451 VGA Controller doc:
			// "The vertical parameters in the CRT Controller
			// (even for a split screen) are not affected, 
			// only the CRTC row scan counter and display
			// memory addressing screen refresh are affected."
			if (m_config.doubleScan && !m_data.doubledLine)
			{
				// Do this line one more time
				m_data.doubledLine = !m_data.doubledLine;
			}
			else
			{
				m_data.doubledLine = false;
				++m_data.rowAddress;
			}

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

			if (m_config.enableVerticalInterrupt && (m_data.vPos == m_data.vTotalDisp))
			{
				LogPrintf(LOG_TRACE, "Set Interrupt Pending");
				m_interruptPending = true;
			}

			if (m_data.vPos == m_data.vSyncMin)
			{
				FireNewFrame();
			}
			
			if (m_data.vPos >= m_data.vTotal)
			{
				FireRenderFrame();

				++m_data.frame;
				m_data.doubledLine = false;
				m_data.vPos = 0;
				m_data.rowAddress = m_config.presetRowScan;
				m_data.lineStartAddress = m_config.startAddress;
				m_data.memoryAddress = m_config.startAddress;

				if (m_configChanged)
				{
					m_configChanged = false;
					UpdateHVTotals(true);
					FireChangeMode();
				}

				if ((m_data.frame % 8) == 0) m_blink8 = !m_blink8;
				if ((m_data.frame % 16) == 0) m_blink16 = !m_blink16;
			}
		}
	}

	void CRTController::Serialize(json& to)
	{
		to["baseAddress"] = m_baseAddress;
		to["id"] = "crtcVGA";

		json config;
		config["hTotal"] = m_config.hTotal;
		config["hDisplayed"] = m_config.hDisplayed;
		config["hBlankStart"] = m_config.hBlankStart;
		config["hBlankEnd"] = m_config.hBlankEnd;
		config["displayEnableSkew"] = m_config.displayEnableSkew;
		config["hSyncStart"] = m_config.hSyncStart;
		config["hSyncEnd"] = m_config.hSyncEnd;
		config["hSyncDelay"] = m_config.hSyncDelay;
		config["vTotal"] = m_config.vTotal;
		config["vDisplayed"] = m_config.vDisplayed;
		config["vBlankStart"] = m_config.vBlankStart;
		config["vBlankEnd"] = m_config.vBlankEnd;
		config["vSyncStart"] = m_config.vSyncStart;
		config["vSyncEnd"] = m_config.vSyncEnd;
		config["presetRowScan"] = m_config.presetRowScan;
		config["bytePanning"] = m_config.bytePanning;
		config["maxScanlineAddress"] = m_config.maxScanlineAddress;
		config["doubleScan"] = m_config.doubleScan;
		config["cursorStart"] = m_config.cursorStart;		
		config["cursorOff"] = m_config.cursorOff;
		config["cursorEnd"] = m_config.cursorEnd;
		config["cursorSkew"] = m_config.cursorSkew;
		config["startAddress"] = m_config.startAddress;
		config["cursorAddress"] = m_config.cursorAddress;
		config["enableVerticalInterrupt"] = m_config.enableVerticalInterrupt;
		config["select5RefreshCycles"] = m_config.select5RefreshCycles;
		config["protectRegisters0to7"] = m_config.protectRegisters0to7;
		config["offset"] = m_config.offset;
		config["underlineLocation"] = m_config.underlineLocation;
		config["countByFour"] = m_config.countByFour;
		config["doubleWordAddressMode"] = m_config.doubleWordAddressMode;
		config["compatibility"] = m_config.compatibility;
		config["selectRowScanCounter"] = m_config.selectRowScanCounter;
		config["vCounterDiv2"] = m_config.vCounterDiv2;
		config["countByTwo"] = m_config.countByTwo;
		config["addressWrap"] = m_config.addressWrap;
		config["byteAddressMode"] = m_config.byteAddressMode;
		config["reset"] = m_config.reset;
		config["lineCompare"] = m_config.lineCompare;
		to["config"] = config;

		json data;
		data["charWidth"] = m_data.charWidth;
		data["hPos"] = m_data.hPos;
		data["vPos"] = m_data.vPos;
		data["rowAddress"] = m_data.rowAddress;
		data["memoryAddress"] = m_data.memoryAddress;
		data["frame"] = m_data.frame;
		data["doubledLine"] = m_data.doubledLine;
		to["data"] = data;

		// For debugging, we don't reload these values
		json debug;
		debug["hTotalDisp"] = m_data.hTotalDisp;
		debug["hTotal"] = m_data.hTotal;
		debug["hBlankMin"] = m_data.hBlankMin;
		debug["hBlankMax"] = m_data.hBlankMax;
		debug["hSyncMin"] = m_data.hSyncMin;
		debug["hSyncMax"] = m_data.hSyncMax;
		debug["vCharHeight"] = m_data.vCharHeight;
		debug["vTotalDisp"] = m_data.vTotalDisp;
		debug["vTotal"] = m_data.vTotal;
		debug["vSyncMin"] = m_data.vSyncMin;
		debug["vSyncMax"] = m_data.vSyncMax;
		debug["vBlankMin"] = m_data.vBlankMin;
		debug["vBlankMax"] = m_data.vBlankMax;
		debug["offset"] = m_data.offset;
		to["debug"] = debug;

		to["raw"] = m_rawData;

		to["interruptPending"] = m_interruptPending;
		to["blink8"] = m_blink8;
		to["blink16"] = m_blink16;
	}

	void CRTController::Deserialize(const json& from)
	{
		if (from["baseAddress"] != m_baseAddress)
		{
			throw emul::SerializableException("CRTController: Incompatible baseAddress");
		}

		if (from["id"] != "crtcVGA")
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
		m_config.vTotal = config["vTotal"];
		m_config.vDisplayed = config["vDisplayed"];
		m_config.vBlankStart = config["vBlankStart"];
		m_config.vBlankEnd = config["vBlankEnd"];
		m_config.vSyncStart = config["vSyncStart"];
		m_config.vSyncEnd = config["vSyncEnd"];
		m_config.presetRowScan = config["presetRowScan"];
		m_config.bytePanning = config["bytePanning"];
		m_config.maxScanlineAddress = config["maxScanlineAddress"];
		m_config.doubleScan = config["doubleScan"];
		m_config.cursorStart = config["cursorStart"];
		m_config.cursorOff = config["cursorOff"];
		m_config.cursorEnd = config["cursorEnd"];
		m_config.cursorSkew = config["cursorSkew"];
		m_config.startAddress = config["startAddress"];
		m_config.cursorAddress = config["cursorAddress"];
		m_config.enableVerticalInterrupt = config["enableVerticalInterrupt"];
		m_config.select5RefreshCycles = config["select5RefreshCycles"];
		m_config.protectRegisters0to7 = config["protectRegisters0to7"];
		m_config.offset = config["offset"];
		m_config.underlineLocation = config["underlineLocation"];
		m_config.countByFour = config["countByFour"];
		m_config.doubleWordAddressMode = config["doubleWordAddressMode"];
		m_config.compatibility = config["compatibility"];
		m_config.selectRowScanCounter = config["selectRowScanCounter"];
		m_config.vCounterDiv2 = config["vCounterDiv2"];
		m_config.countByTwo = config["countByTwo"];
		m_config.addressWrap = config["addressWrap"];
		m_config.byteAddressMode = config["byteAddressMode"];
		m_config.reset = config["reset"];
		m_config.lineCompare = config["lineCompare"];

		const json& data = from["data"];
		m_data.charWidth = data["charWidth"];
		m_data.hPos = data["hPos"];
		m_data.vPos = data["vPos"];
		m_data.rowAddress = data["rowAddress"];
		m_data.memoryAddress = data["memoryAddress"];
		m_data.frame = data["frame"];
		m_data.doubledLine = data["doubledLine"];

		m_rawData = from["raw"];

		m_interruptPending = from["interruptPending"];
		m_blink8 = from["blink8"];
		m_blink16 = from["blink16"];

		UpdateHVTotals();
	}
}
