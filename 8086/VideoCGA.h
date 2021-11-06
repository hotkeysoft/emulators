#pragma once

#include "Common.h"
#include "MemoryBlock.h"
#include "Memory.h"
#include "Device6845.h"
#include "Video.h"

using emul::WORD;
using emul::BYTE;

namespace video
{
	class VideoCGA : public Video
	{
	public:
		VideoCGA(WORD baseAddress);

		VideoCGA() = delete;
		VideoCGA(const VideoCGA&) = delete;
		VideoCGA& operator=(const VideoCGA&) = delete;
		VideoCGA(VideoCGA&&) = delete;
		VideoCGA& operator=(VideoCGA&&) = delete;

		void Init(emul::Memory& memory, const char* charROM, BYTE border = 10);
		virtual void Reset() override;
		virtual void Tick() override;

		virtual void EnableLog(SEVERITY minSev = LOG_INFO) override;

		emul::MemoryBlock& GetVideoRAM() { return m_screenB800; }

		void RenderFrame();
		void NewFrame();
		void EndOfRow();

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

		// Text mode pointers
		BYTE* m_cursorPos = nullptr;
		BYTE* m_currChar = nullptr;

		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		// Graph mode banks
		BYTE* m_bank0 = nullptr;
		BYTE* m_bank1 = nullptr;

		uint32_t m_currGraphPalette[4];

		crtc::Device6845 m_crtc;
	};
}
