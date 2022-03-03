#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "../CPU/Memory.h"
#include "Video6845.h"

using emul::WORD;
using emul::BYTE;

namespace video
{
	class VideoMDA : public Video6845
	{
	public:
		VideoMDA(WORD baseAddress);

		VideoMDA() = delete;
		VideoMDA(const VideoMDA&) = delete;
		VideoMDA& operator=(const VideoMDA&) = delete;
		VideoMDA(VideoMDA&&) = delete;
		VideoMDA& operator=(VideoMDA&&) = delete;

		virtual const std::string GetID() const override { return "mda"; }
		virtual const std::string GetDisplayName() const override { return "MDA"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;

		virtual bool IsMonoAdapter() override { return true; }

		emul::MemoryBlock& GetVideoRAM() { return m_screenB000; }

		// crtc::EventHandler
		virtual void OnChangeMode() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

		virtual bool IsEnabled() const override { return m_mode.enableVideo; }

	protected:
		// Mode Control Register
		struct MODEControl
		{
			bool enableVideo = false;
			bool hiResolution = false;
			bool blink = false;
		} m_mode;
		virtual void WriteModeControlRegister(BYTE value);

		ADDRESS GetBaseAddressText() { return 0xB0000 + ((GetCRTC().GetMemoryAddress13() * 2u) & 0x3FFF); }
		uint32_t GetIndexedColor(BYTE index) const { return GetMonitorPalette()[index]; }

		void DrawTextMode();

		// Status Register
		virtual BYTE ReadStatusRegister();

		// 4K screen buffer
		emul::MemoryBlock m_screenB000;

		emul::MemoryBlock m_charROM;
		ADDRESS m_charROMStart = 0;
	};
}
