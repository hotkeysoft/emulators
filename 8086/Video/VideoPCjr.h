#pragma once

#include "../Common.h"
#include "../CPU/Memory.h"
#include "../CPU/PortConnector.h"
#include "Device6845.h"
#include "Video.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoPCjr : public Video
	{
	public:
		VideoPCjr(WORD baseAddress);

		VideoPCjr() = delete;
		VideoPCjr(const VideoPCjr&) = delete;
		VideoPCjr& operator=(const VideoPCjr&) = delete;
		VideoPCjr(VideoPCjr&&) = delete;
		VideoPCjr& operator=(VideoPCjr&&) = delete;

		virtual void Init(emul::Memory* memory, const char* charROM, BYTE border, bool forceMono = false) override;
		virtual void Reset() override;
		virtual void Tick() override;

		bool IsVSync() const { return m_crtc.IsVSync(); }

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		void RenderFrame();
		void NewFrame();
	protected:
		const WORD m_baseAddress;

		virtual bool ConnectTo(emul::PortAggregator& dest) override;

		emul::Memory* m_memory = nullptr;

		bool IsCursor() const;

		struct PageRegister
		{
			BYTE crtPage = 0;
			BYTE cpuPage = 0;
			BYTE videoAddressMode = 0;

			enum class ADDRESSMODE {
				ALPHA = 0,
				GRAPH_LOW = 1,
				GRAPH_HI = 2,
				RESERVED = 3
			} addressMode;

			ADDRESS crtBaseAddress = 0;
			ADDRESS cpuBaseAddress = 0;

		} m_pageRegister;
		void WritePageRegister(BYTE value);

		enum GateArrayAddress
		{
			GA_MODE_CTRL_1  = 0x0,
			GA_PALETTE_MASK = 0x1,
			GA_BORDER_COLOR = 0x2,
			GA_MODE_CTRL_2  = 0x3,
			GA_RESET        = 0x4,

			GA_PALETTE      = 0x10, // 0x10-0x1F

			GA_MASK = 0x1F
		};

		struct GateArrayRegister
		{
			GateArrayAddress currRegister = (GateArrayAddress)0;

			BYTE paletteRegister[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

			BYTE borderColor = 0;
			BYTE paletteMask = 0x0F;

			bool hiBandwidth = false; // Require 128K memory: 80x24 alpha, 640x200x4colors, 320x200x16colors
			bool graphics = false;
			bool monochrome = false;
			bool enableVideo = false;
			bool graph16Colors = false;
			bool blink = false;
			bool graph2Colors = false;

			// false = address, true = data
			bool addressDataFlipFlop = false;
		} m_mode;

		void WriteGateArrayRegister(BYTE value);
		BYTE ReadStatusRegister();

		void MapB800Window();

		uint32_t GetColor(BYTE index) { return GetMonitorPalette()[m_mode.paletteRegister[(index & m_mode.paletteMask)]]; }

		typedef void(VideoPCjr::* DrawFunc)();
		DrawFunc m_drawFunc = &VideoPCjr::DrawTextMode;
		void DrawTextMode();
		void Draw160x200x16();
		void Draw320x200x4();
		void Draw320x200x16();
		void Draw640x200x2();
		void Draw640x200x4();

		int m_xAxisDivider = 1;

		// Text mode pointers
		BYTE* m_cursorPos = nullptr;
		BYTE* m_currChar = nullptr;

		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		// Graph mode banks
		BYTE* m_banks[4] = { 0, 0, 0, 0 };

		// Diagnostics: dot information (status register)
		// Only works in alpha modes for the moment
		BYTE m_lastDot = 0; 

		crtc::Device6845 m_crtc;
	};
}
