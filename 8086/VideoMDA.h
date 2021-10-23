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
	class VideoMDA : public Video
	{
	public:
		VideoMDA(WORD baseAddress);

		VideoMDA() = delete;
		VideoMDA(const VideoMDA&) = delete;
		VideoMDA& operator=(const VideoMDA&) = delete;
		VideoMDA(VideoMDA&&) = delete;
		VideoMDA& operator=(VideoMDA&&) = delete;

		void Init(emul::Memory& memory, const char* charROM, BYTE border = 10);
		virtual void Reset() override;
		virtual void Tick() override;

		virtual void EnableLog(bool enable, SEVERITY minSev = LOG_INFO) override;

		emul::MemoryBlock& GetVideoRAM() { return m_screenB000; }

		void RenderFrame();
		void NewFrame();

	protected:
		const WORD m_baseAddress;

		virtual bool ConnectTo(emul::PortAggregator& dest) override;

		bool IsCursor() const;

		// Mode Control Register
		struct MODEControl
		{
			bool enableVideo = false;
			bool hiResolution = false;
			bool blink = false;
		} m_mode;
		void WriteModeControlRegister(BYTE value);

		typedef void(VideoMDA::* DrawFunc)();
		DrawFunc m_drawFunc = &VideoMDA::DrawTextMode;
		void DrawTextMode();

		// Status Register
		BYTE ReadStatusRegister();

		// 4K screen buffer
		emul::MemoryBlock m_screenB000;

		// Text mode pointers
		BYTE* m_cursorPos = nullptr;
		BYTE* m_currChar = nullptr;

		emul::MemoryBlock m_charROM;
		BYTE* m_charROMStart;

		const uint32_t* m_alphaPalette = nullptr;

		// dot information (status register)
		bool m_lastDot = 0;

		crtc::Device6845 m_crtc;
	};
}
