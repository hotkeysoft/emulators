#pragma once

#include <Common.h>
#include <CPU/Memory.h>
#include "../CPU/PortConnector.h"
#include "Video6845.h"
#include <array>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoTandy : public Video6845
	{
	public:
		VideoTandy(WORD baseAddress);

		VideoTandy() = delete;
		VideoTandy(const VideoTandy&) = delete;
		VideoTandy& operator=(const VideoTandy&) = delete;
		VideoTandy(VideoTandy&&) = delete;
		VideoTandy& operator=(VideoTandy&&) = delete;

		virtual const std::string GetID() const override { return "tga"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Tick() override;

		void SetRAMBase(ADDRESS base);

		// crtc::EventHandler
		virtual void OnChangeMode() override;
		virtual void OnEndOfRow() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return GetMonitorPalette()[m_mode.borderEnable ? m_mode.borderColor : m_color.color]; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return m_mode.enableVideo; }

	protected:
		const WORD m_baseAddress;

		ADDRESS m_ramBase = 0;

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

			std::array<BYTE, 16> paletteRegister = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
			BYTE paletteMask = 0x0F;

			// Enables the border color register (addr=2). For 0=PC compatibility, 1=PCjr compatibility
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

		ADDRESS GetBaseAddressText() { return m_pageRegister.crtBaseAddress + ((GetCRTC().GetMemoryAddress13() * 2u) & 0x7FFF); }
		ADDRESS GetBaseAddressGraph() { return m_pageRegister.crtBaseAddress + (((GetCRTC().GetData().rowAddress * 0x2000) + (GetCRTC().GetMemoryAddress12() * 2u)) & 0x7FFF); }

		uint32_t GetIndexedColor16(BYTE index) const { return GetMonitorPalette()[m_mode.paletteRegister[(index & m_mode.paletteMask)]]; }
		uint32_t GetIndexedColor4(BYTE index) const { return GetIndexedColor16(m_currGraphPalette[index]); }

		void DrawTextMode();

		emul::MemoryBlock m_charROM;
		ADDRESS m_charROMStart = 0;

		BYTE m_currGraphPalette[4] = { 0, 0, 0, 0 };
	};
}
