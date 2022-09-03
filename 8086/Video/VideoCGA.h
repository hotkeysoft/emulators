#pragma once

#include <CPU/CPUCommon.h>
#include <CPU/MemoryBlock.h>
#include "Video6845.h"

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoCGA : public Video6845
	{
	public:
		VideoCGA(WORD baseAddress);

		VideoCGA() = delete;
		VideoCGA(const VideoCGA&) = delete;
		VideoCGA& operator=(const VideoCGA&) = delete;
		VideoCGA(VideoCGA&&) = delete;
		VideoCGA& operator=(VideoCGA&&) = delete;

		virtual const std::string GetID() const override { return "cga"; }
		virtual const std::string GetDisplayName() const override { return "CGA"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Tick() override;

		emul::MemoryBlock& GetVideoRAM() { return m_screenB800; }

		// crtc::EventHandler
		virtual void OnChangeMode() override;
		virtual void OnEndOfRow() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return GetMonitorPalette()[m_color.color]; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return m_mode.enableVideo; }

	protected:
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

		ADDRESS GetBaseAddressText() { return 0xB8000 + ((GetCRTC().GetMemoryAddress13() * 2u) & 0x3FFF); }
		ADDRESS GetBaseAddressGraph() { return 0xB8000 + (((GetCRTC().GetData().rowAddress * 0x2000) + (GetCRTC().GetMemoryAddress12() * 2u)) & 0x3FFF); }

		uint32_t GetIndexedColor2(BYTE index) const { return GetMonitorPalette()[index ? m_color.color : 0]; }
		uint32_t GetIndexedColor4(BYTE index) const { return GetMonitorPalette()[m_currGraphPalette[index]]; }
		uint32_t GetIndexedColor16(BYTE index) const { return GetMonitorPalette()[index]; }

		void DrawTextMode();

		// Color Select Register
		struct COLORSelect
		{
			BYTE color = 0; // bits 0-3: bgri border (alpha) / border+bg (320x200) / fg (640x200)

			bool palIntense = false; // bit 4 intensity palette (320x200)
			bool palSelect = false; // bit 5 graph palette (320x200)
		} m_color;
		void WriteColorSelectRegister(BYTE value);

		// Status Register
		BYTE ReadStatusRegister();

		// 16K screen buffer
		emul::MemoryBlock m_screenB800;

		emul::MemoryBlock m_charROM;
		ADDRESS m_charROMStart = 0;

		BYTE m_currGraphPalette[4] = { 0, 0, 0, 0 };
	};
}
