#pragma once

#include "../Common.h"
#include "../CPU/Memory.h"
#include "../CPU/PortConnector.h"
#include "Video6845.h"
#include <array>

using emul::PortConnector;
using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoPCjr : public Video6845
	{
	public:
		VideoPCjr(WORD baseAddress);

		VideoPCjr() = delete;
		VideoPCjr(const VideoPCjr&) = delete;
		VideoPCjr& operator=(const VideoPCjr&) = delete;
		VideoPCjr(VideoPCjr&&) = delete;
		VideoPCjr& operator=(VideoPCjr&&) = delete;

		virtual const std::string GetID() const override { return "pcjr"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Tick() override;

		// crtc::EventHandler
		virtual void OnChangeMode() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return GetMonitorPalette()[m_mode.borderColor]; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return m_mode.enableVideo; }

	protected:
		const WORD m_baseAddress;

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

			std::array<BYTE, 16> paletteRegister = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

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

		ADDRESS GetBaseAddressText() { return m_pageRegister.crtBaseAddress + ((GetCRTC().GetMemoryAddress13() * 2u) & 0x7FFF); }
		ADDRESS GetBaseAddressGraph() { return m_pageRegister.crtBaseAddress + (((GetCRTC().GetData().rowAddress * 0x2000) + (GetCRTC().GetMemoryAddress12() * 2u)) & 0x7FFF); }

		uint32_t GetIndexedColor16(BYTE index) { return GetMonitorPalette()[m_mode.paletteRegister[(index & m_mode.paletteMask)]]; }

		void DrawTextMode();

		emul::MemoryBlock m_charROM;
		ADDRESS m_charROMStart = 0;
	};
}
