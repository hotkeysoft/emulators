#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Logger.h"
#include "MemoryBlock.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace crtc
{
	class Device6845 : public PortConnector
	{
	public:
		Device6845(WORD baseAddress);
		~Device6845();

		Device6845() = delete;
		Device6845(const Device6845&) = delete;
		Device6845& operator=(const Device6845&) = delete;
		Device6845(Device6845&&) = delete;
		Device6845& operator=(Device6845&&) = delete;

		virtual void Init();
		virtual void Reset();

		virtual void Tick();

		bool IsInit() const { return m_data.hTotal && m_data.vTotal; }

		bool IsHSync() const { return m_data.hPos > m_data.hTotalDisp; }
		bool IsVSync() const { return m_data.vPos > m_data.vTotalDisp; }

		bool IsDisplayArea() const { return (m_data.vPos < m_data.vTotalDisp) && (m_data.hPos < m_data.hTotalDisp); }

		bool IsBlink16() const { return m_blink16; }
		bool IsBlink32() const { return m_blink32; }

	protected:
		const WORD m_baseAddress;

		// CRT Controller
		enum CRTRegister
		{
			CRT_H_TOTAL_CHAR = 0x0, // WRITE
			CRT_H_DISPLAYED_CHAR = 0x1, // WRITE
			CRT_H_SYNC_POS_CHAR = 0x2, // WRITE
			CRT_H_SYNC_WIDTH_CHAR = 0x3, // WRITE
			CRT_V_TOTAL_ROW = 0x4, // WRITE
			CRT_V_TOTAL_ADJ_LINES = 0x5, // WRITE
			CRT_V_DISPLAYED_ROW = 0x6, // WRITE
			CRT_V_SYNC_POS_ROW = 0x7, // WRITE
			CRT_INTERLACE_MODE = 0x8, // WRITE
			CRT_MAX_SCANLINE_ADDR = 0x9, // WRITE
			CRT_CURSOR_START_LINE = 0xA, // WRITE
			CRT_CURSOR_END_LINE = 0xB, // WRITE
			CRT_START_ADDR_HI = 0xC, // WRITE
			CRT_START_ADDR_LO = 0xD, // WRITE
			CRT_CURSOR_ADDR_HI = 0xE, // READ/WRITE
			CRT_CURSOR_ADDR_LO = 0xF, // READ/WRITE
			CRT_LIGHT_PEN_HI = 0x10,// READ
			CRT_LIGHT_PEN_LO = 0x11,// READ

			_CRT_MAX_REG = CRT_LIGHT_PEN_LO,
			CRT_INVALID_REG = 0xFF
		};

		struct CRTCConfig
		{
			CRTRegister currRegister = CRT_INVALID_REG;

			BYTE hTotal = 0;
			BYTE hDisplayed = 0;
			BYTE hSyncPos = 0;
			BYTE hSyncWidth = 0;

			BYTE vTotal = 0;
			BYTE vTotalAdjust = 0;
			BYTE vTotalDisplayed = 0;
			BYTE vSyncPos = 0;

			BYTE interlaceMode = 0;

			BYTE maxScanlineAddress = 0;

			WORD startAddress = 0;

			WORD cursorAddress = 0;
			BYTE cursorStart = 0;
			BYTE cursorEnd = 0;

			enum CURSOR
			{
				CURSOR_NOBLINK = 0,
				CURSOR_NONE = 1,
				CURSOR_BLINK16 = 2,
				CURSOR_BLINK32 = 3
			} cursor;

		} m_config;

		void SelectCRTCRegister(BYTE value);
		BYTE ReadCRTCData();
		void WriteCRTCData(BYTE value);

		struct CRTCData
		{
			WORD hPos = 0;
			WORD hBorder = 0;
			WORD hTotal = 0;
			WORD hTotalDisp = 0;

			WORD vPos = 0;
			WORD vBorder = 0;
			WORD vTotal = 0;
			WORD vTotalDisp = 0;
			WORD vCharHeight = 0;

			size_t frame = 0;
		} m_data;

		void UpdateHVTotals();

		// Blinky things
		bool m_blink16 = false;
		bool m_blink32 = false;

		// Called during Tick
		virtual void RenderFrame() = 0;
		virtual void NewFrame() = 0;
		virtual void EndOfRow() = 0;
	};
}
