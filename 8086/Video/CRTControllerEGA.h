#pragma once

#include <Serializable.h>
#include <CPU/PortConnector.h>
#include <CPU/MemoryBlock.h>
#include <Video/VideoEvents.h>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;
using emul::GetBit;
using emul::SetBit;

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace crtc_ega
{
	// CRT Controller
	enum CRTRegister
	{
		CRT_H_TOTAL = 0x0, // WRITE
		CRT_H_DISPLAYED = 0x1, // WRITE

		CRT_H_BLANK_START = 0x2, // WRITE
		CRT_H_BLANK_END = 0x3, // WRITE

		CRT_H_SYNC_START = 0x4, // WRITE
		CRT_H_SYNC_END = 0x5, // WRITE

		CRT_V_TOTAL = 0x6, // WRITE

		CRT_OVERFLOW = 0x7, // WRITE

		CRT_PRESET_ROWSCAN = 0x8, // WRITE
		CRT_MAX_SCANLINE = 0x9, // WRITE

		CRT_CURSOR_START_LINE = 0xA, // WRITE
		CRT_CURSOR_END_LINE = 0xB, // WRITE

		CRT_START_ADDR_HI = 0xC, // WRITE
		CRT_START_ADDR_LO = 0xD, // WRITE

		CRT_CURSOR_ADDR_HI = 0xE, // READ/WRITE
		CRT_CURSOR_ADDR_LO = 0xF, // READ/WRITE

		CRT_V_SYNC_START = 0x10, // WRITE
		CRT_V_SYNC_END = 0x11, // WRITE

		CRT_LIGHT_PEN_HI = 0x10,// READ
		CRT_LIGHT_PEN_LO = 0x11,// READ

		CRT_V_DISPLAYED_END = 0x12, // WRITE

		CRT_OFFSET = 0x13, // WRITE
		CRT_UNDERLINE_LOCATION = 0x14, // WRITE

		CRT_V_BLANK_START = 0x15, // WRITE
		CRT_V_BLANK_END = 0x16, // WRITE

		CRT_MODE_CONTROL = 0x17, // WRITE
		CRT_LINE_COMPARE = 0x18, // WRITE

		_CRT_MAX_REG = CRT_LINE_COMPARE,
		CRT_INVALID_REG = 0xFF
	};

	struct CRTCConfig
	{
		CRTRegister currRegister = CRT_INVALID_REG;

		BYTE hTotal = 0; // Number of chars -2
		BYTE hDisplayed = 0; // Number of chars -1;

		BYTE hBlankStart = 0;
		BYTE hBlankEnd = 0;

		BYTE displayEnableSkew = 0;

		BYTE hSyncStart = 0;
		BYTE hSyncEnd = 0;

		BYTE hSyncDelay = 0;
		bool startOdd = false;

		WORD vTotal = 0;
		WORD vDisplayed = 0;

		WORD vBlankStart = 0;
		BYTE vBlankEnd = 0;

		WORD vSyncStart = 0;
		BYTE vSyncEnd = 0;

		BYTE presetRowScan = 0;
		BYTE maxScanlineAddress = 0; // -1

		WORD startAddress = 0;
		WORD cursorAddress = 0;

		BYTE cursorStart = 0; // -1
		BYTE cursorEnd = 0;
		BYTE cursorSkew = 0;

		BYTE offset = 0; // Logical line width (WORD or DWORD address)

		BYTE underlineLocation = 0; // -1

		bool compatibility = false;
		bool selectRowScanCounter = false;
		bool vCounterDiv2 = false;
		bool countByTwo = false;
		bool disableOutputControl = false;
		bool addressWrap = false;
		bool byteAddressMode = false;

		WORD lineCompare = 0;

		bool vSyncInterruptEnable = false;

		void Reset();
	};

	struct CRTCData
	{
		BYTE charWidth = 8;

		WORD hPos = 0;
		WORD vPos = 0;

		WORD rowAddress = 0;
		WORD lineStartAddress = 0;
		WORD memoryAddress = 0;

		size_t frame = 0;

		// Computed in UpdateHVTotals
		WORD offset = 0;

		WORD hTotal = 0;
		WORD hTotalDisp = 0;
		WORD hBlankMin = 0;
		WORD hBlankMax = 0;
		WORD hSyncMin = 0;
		WORD hSyncMax = 0;

		WORD vTotal = 0;
		WORD vTotalDisp = 0;
		WORD vCharHeight = 0;
		WORD vBlankMin = 0;
		WORD vBlankMax = 0;
		WORD vSyncMin = 0;
		WORD vSyncMax = 0;

		void Reset();
	};

	class CRTController : public PortConnector, public emul::Serializable, public vid_events::EventSource
	{
	public:
		CRTController(WORD baseAddress, BYTE charWidth = 8);
		~CRTController();

		CRTController() = delete;
		CRTController(const CRTController&) = delete;
		CRTController& operator=(const CRTController&) = delete;
		CRTController(CRTController&&) = delete;
		CRTController& operator=(CRTController&&) = delete;

		virtual void Init();
		virtual void Reset();
		virtual void SetBasePort(WORD base);

		void DisconnectPorts();
		void ConnectPorts();

		virtual void Tick();

		bool IsInit() const { return m_config.hTotal && m_config.vTotal; }

		bool IsHSync() const { return (m_data.hPos >= m_data.hSyncMin) && (m_data.hPos <= m_data.hSyncMax); }
		bool IsVSync() const { return (m_data.vPos >= m_data.vSyncMin) && (m_data.vPos <= m_data.vSyncMax); }

		bool IsHBlank() const { return (m_data.hPos >= m_data.hBlankMin) && (m_data.hPos <= m_data.hBlankMax); }
		bool IsVBlank() const { return (m_data.vPos >= m_data.vBlankMin) && (m_data.vPos <= m_data.vBlankMax); }

		// Add one char width for x pixel panning
		bool IsDisplayArea() const { return (m_data.vPos < m_data.vTotalDisp) && (m_data.hPos < m_data.hTotalDisp + m_data.charWidth); }

		bool IsBlink8() const { return m_blink8; }
		bool IsBlink16() const { return m_blink16; }

		WORD GetMemoryAddress() const {
			WORD address = m_data.memoryAddress;

			if (!m_config.byteAddressMode)
			{
				// word mode
				address <<= 1;

				// Address Wrap: set bit 0 to MA13/MA15
				WORD b0 = GetBit(m_data.memoryAddress, m_config.addressWrap ? 15 : 13);
				address |= b0;
			}

			// In compatibility mode, set MA13 = bit 0 of rowAddress counter
			if (m_config.compatibility)
			{
				SetBit(address, 13, GetBit(m_data.rowAddress, 0));
			}

			// In row scan counter mode, set MA14 = bit 1 of rowAddress counter
			if (m_config.selectRowScanCounter)
			{
				SetBit(address, 14, GetBit(m_data.rowAddress, 1));
			}

			return address;
		}

		const CRTCConfig& GetConfig() const { return m_config; }
		const CRTCData& GetData() const { return m_data; }

		void SetCharWidth(BYTE charWidth);

		virtual void Serialize(json& to);
		virtual void Deserialize(const json& from);

		bool IsInterruptPending() const { return m_interruptPending; }

	protected:
		WORD m_baseAddress;

		bool m_interruptPending = false;

		CRTCConfig m_config;

		void SelectCRTCRegister(BYTE value);
		BYTE ReadCRTCData();
		void WriteCRTCData(BYTE value);

		CRTCData m_data;

		void UpdateHVTotals();

		// Blinky things
		bool m_blink8 = false;
		bool m_blink16 = false;

		bool m_configChanged = false;
	};
}
