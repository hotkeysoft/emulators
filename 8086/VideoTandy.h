#pragma once

#include "Common.h"
#include "Memory.h"
#include "PortConnector.h"
#include "Device6845.h"
#include "Video.h"

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoTandy : public Video
	{
	public:
		VideoTandy(WORD baseAddress);

		VideoTandy() = delete;
		VideoTandy(const VideoTandy&) = delete;
		VideoTandy& operator=(const VideoTandy&) = delete;
		VideoTandy(VideoTandy&&) = delete;
		VideoTandy& operator=(VideoTandy&&) = delete;

		virtual void Init(emul::Memory* memory, const char* charROM, BYTE border, bool forceMono = false) override;
		virtual void Reset() override;
		virtual void Tick() override;

		bool IsVSync() const { return m_crtc.IsVSync(); }

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		void SetRAMBase(ADDRESS base);

		void RenderFrame();
		void NewFrame();
	protected:
		const WORD m_baseAddress;

		virtual bool ConnectTo(emul::PortAggregator& dest) override;

		emul::Memory* m_memory = nullptr;
		ADDRESS m_ramBase = 0;

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
			} addressMode = ADDRESSMODE::ALPHA;

			ADDRESS crtBaseAddress = 0;
			ADDRESS cpuBaseAddress = 0;

		} m_pageRegister;
		void WritePageRegister(BYTE value);

		void UpdatePageRegisters();

		void WriteModeControlRegister(BYTE value);

		// Color Select Register
		struct COLORSelect
		{
			BYTE color = 0; // bits 0-3: bgri border (alpha) / border+bg (320x200) / fg (640x200)

			bool palIntense = false; // bit 4 intensity palette (320x200) and non-blink alpha mode
			bool palSelect = false; // bit 5 graph palette (320x200)
		} m_color;
		void WriteColorSelectRegister(BYTE value);

		enum VideoArrayAddress
		{
			VA_PALETTE_MASK = 0x1,
			VA_BORDER_COLOR = 0x2,
			VA_MODE_CTRL    = 0x3,

			GA_PALETTE      = 0x10, // 0x10-0x1F

			GA_MASK = 0x1F
		};

		BYTE ReadStatusRegister();

		struct ModeControl
		{
			// Mode registers

			// Hi-res dot clock
			// 0: 40 column or low resolution graph modes
			// 1: 80 columns or high resolution graph modes
			bool hiDotClock = false;

			bool graphics = false;
			bool monochrome = false;
			bool enableVideo = false;
			bool hiResolution = false; // true for either 640x200 graphics or 80 columns
			bool blink = false; // alpha mode only, true = blink on attribute 7

			// Video Array registers

			BYTE paletteRegister[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			BYTE paletteMask = 0x0F;

			// Enables the border color register (addr=2). For 0=PC compatibility, 1=PCjr compatibility
			// TODO
			bool borderEnable = false;
			BYTE borderColor = 0;

			// True for the 640x200x4color graphics mode
			bool graph640x200x4 = false;
			bool graph16Colors = false;

		} m_mode;


		VideoArrayAddress m_videoArrayRegisterAddress = (VideoArrayAddress)0;
		void WriteVideoArrayAddress(BYTE value);
		void WriteVideoArrayData(BYTE value);

		void MapB800Window();

		uint32_t GetColor(BYTE index) { return GetMonitorPalette()[m_mode.paletteRegister[(index & m_mode.paletteMask)]]; }

		typedef void(VideoTandy::* DrawFunc)();
		DrawFunc m_drawFunc = &VideoTandy::DrawTextMode;
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

		BYTE m_currGraphPalette[4];

		crtc::Device6845 m_crtc;
	};
}
