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

namespace cga
{
	enum class MONITOR { COLOR, GREEN, AMBER, GREY };

	class DeviceCGA : public PortConnector
	{
	public:
		DeviceCGA(WORD baseAddress);

		DeviceCGA() = delete;
		DeviceCGA(const DeviceCGA&) = delete;
		DeviceCGA& operator=(const DeviceCGA&) = delete;
		DeviceCGA(DeviceCGA&&) = delete;
		DeviceCGA& operator=(DeviceCGA&&) = delete;

		void Init(const char* charROM, MONITOR monitor = MONITOR::COLOR, BYTE border = 10);
		void Reset();

		void Tick();

		emul::MemoryBlock& GetVideoRAM() { return m_screenB800; }

	protected:
		const WORD m_baseAddress;

		// CRT Controller
		enum CRTRegister
		{
			CRT_H_TOTAL_CHAR =		0x0, // WRITE
			CRT_H_DISPLAYED_CHAR =	0x1, // WRITE
			CRT_H_SYNC_POS_CHAR =	0x2, // WRITE
			CRT_H_SYNC_WIDTH_CHAR = 0x3, // WRITE
			CRT_V_TOTAL_ROW =		0x4, // WRITE
			CRT_V_TOTAL_ADJ_LINES = 0x5, // WRITE
			CRT_V_DISPLAYED_ROW =	0x6, // WRITE
			CRT_V_SYNC_POS_ROW =	0x7, // WRITE
			CRT_INTERLACE_MODE =	0x8, // WRITE
			CRT_MAX_SCANLINE_ADDR = 0x9, // WRITE
			CRT_CURSOR_START_LINE = 0xA, // WRITE
			CRT_CURSOR_END_LINE =	0xB, // WRITE
			CRT_START_ADDR_HI =		0xC, // WRITE
			CRT_START_ADDR_LO =		0xD, // WRITE
			CRT_CURSOR_ADDR_HI =	0xE, // READ/WRITE
			CRT_CURSOR_ADDR_LO =	0xF, // READ/WRITE
			CRT_LIGHT_PEN_HI =		0x10,// READ
			CRT_LIGHT_PEN_LO =		0x11,// READ
			
			_CRT_MAX_REG = CRT_LIGHT_PEN_LO,
			CRT_INVALID_REG = 0xFF
		};

		struct CRTCData
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

		} m_crtc;

		void SelectCRTCRegister(BYTE value);
		BYTE ReadCRTCData();
		void WriteCRTCData(BYTE value);

		void UpdateHVTotals();
		
		WORD m_hPos = 0;
		WORD m_hBorder = 0;
		WORD m_hTotal = 0;
		WORD m_hTotalDisp = 0;

		WORD m_vPos = 0;
		WORD m_vBorder = 0;
		WORD m_vTotal = 0;
		WORD m_vTotalDisp = 0;
		WORD m_vCharHeight = 0;

		size_t m_frame = 0;

		// Blinky things
		bool m_blink16 = false;
		bool m_blink32 = false;

		bool IsHSync() { return m_hPos > m_hTotalDisp; }
		bool IsVSync() { return m_vPos > m_vTotalDisp; }

		// Mode Control Register
		struct MODEControl
		{
			bool text80Columns = false;
			bool graphics = false;
			bool monochrome = false;
			bool enableVideo = false;
			bool hiResolution = false;
			bool blink = false;
		} m_mode;
		void WriteModeControlRegister(BYTE value);

		// Color Select Register
		BYTE m_colorSelectRegister = 0;
		void WriteColorSelectRegister(BYTE value);

		// Status Register
		BYTE ReadStatusRegister();

		// 16K screen buffer
		emul::MemoryBlock m_screenB800;
		BYTE* m_cursorPos = nullptr;
		BYTE* m_currChar = nullptr;

		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		const uint32_t* m_alphaPalette = nullptr;

		// SDL
		SDL_Window* m_sdlWindow = nullptr;
		SDL_Renderer* m_sdlRenderer = nullptr;

		BYTE m_sdlBorderPixels;
		BYTE m_sdlHBorder;
		BYTE m_sdlVBorder;

		void Render();
	};
}
