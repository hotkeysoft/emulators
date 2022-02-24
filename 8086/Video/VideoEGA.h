#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "Video.h"

using emul::WORD;
using emul::BYTE;
using emul::ADDRESS;

namespace video
{
	class VideoEGA : public Video
	{
	public:
		enum RAMSIZE { EGA_64K, EGA_128K, EGA_256K };

		VideoEGA(RAMSIZE ramsize, WORD baseAddressMisc, WORD baseAddressMono, WORD baseAddressColor);

		VideoEGA() = delete;
		VideoEGA(const VideoEGA&) = delete;
		VideoEGA& operator=(const VideoEGA&) = delete;
		VideoEGA(VideoEGA&&) = delete;
		VideoEGA& operator=(VideoEGA&&) = delete;

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;
		virtual void Tick() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(json& from) override;

		virtual uint32_t GetBackgroundColor() const override { return GetMonitorPalette()[0]; }
		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;
		virtual bool IsEnabled() const override { return true; }

		virtual bool IsHSync() const override { return false; }
		virtual bool IsVSync() const override { return false; }

	protected:
		RAMSIZE m_ramSize;
		WORD m_baseAddressMisc;
		WORD m_baseAddressMono;
		WORD m_baseAddressColor;

		emul::MemoryBlock m_egaROM;

		ADDRESS GetBaseAddress() { return 0; }

		uint32_t GetIndexedColor(BYTE index) const { return index; }

		void DrawTextMode();
	};
}
