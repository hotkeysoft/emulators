#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "../CPU/Memory.h"
#include "Device6845.h"
#include "Video.h"

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoCGA : public Video, public crtc::EventHandler
	{
	public:
		VideoCGA(WORD baseAddress);

		VideoCGA() = delete;
		VideoCGA(const VideoCGA&) = delete;
		VideoCGA& operator=(const VideoCGA&) = delete;
		VideoCGA(VideoCGA&&) = delete;
		VideoCGA& operator=(VideoCGA&&) = delete;

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Reset() override;
		virtual void Tick() override;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		emul::MemoryBlock& GetVideoRAM() { return m_screenB800; }

		// crtc::EventHandler
		virtual void OnChangeMode() override;
		virtual void OnRenderFrame() override;
		virtual void OnNewFrame() override;
		virtual void OnEndOfRow() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return GetMonitorPalette()[m_color.color]; }

	protected:
		const WORD m_baseAddress;

		virtual bool ConnectTo(emul::PortAggregator& dest) override;

		bool IsCursor() const;

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

		typedef void(VideoCGA::* DrawFunc)();
		DrawFunc m_drawFunc = &VideoCGA::DrawTextMode;
		void DrawTextMode();
		void Draw320x200();
		void Draw640x200();

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
		BYTE* m_charROMStart;

		uint32_t m_currGraphPalette[4];

		crtc::Device6845 m_crtc;
	};
}
