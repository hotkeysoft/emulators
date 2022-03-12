#pragma once

#include "../Common.h"
#include "../CPU/MemoryBlock.h"
#include "../CPU/Memory.h"
#include "VideoMDA.h"

using emul::WORD;
using emul::BYTE;

namespace video
{
	// Hercules Graphics Card (HGC)
	//
	// Superset of the Monochrome Display Adapter (MDA)
	//
	// Adds 720x348 graphics mode
	// and some changes to the registers
	class VideoHGC : public VideoMDA
	{
	public:
		VideoHGC(WORD baseAddress);

		VideoHGC() = delete;
		VideoHGC(const VideoHGC&) = delete;
		VideoHGC& operator=(const VideoHGC&) = delete;
		VideoHGC(VideoHGC&&) = delete;
		VideoHGC& operator=(VideoHGC&&) = delete;

		virtual const std::string GetID() const override { return "hgc"; }
		virtual const std::string GetDisplayName() const override { return "Hercules"; }

		virtual void Init(emul::Memory* memory, const char* charROM, bool forceMono = false) override;

		virtual void OnChangeMode() override;

		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		virtual SDL_Rect GetDisplayRect(BYTE border = 0, WORD xMultiplier = 1) const override;

	protected:
		// Mode Control Register (Hercules)
		struct MODEControl
		{
			bool graphics = false;
			bool displayPage = false;
		} m_modeHGC;
		virtual void WriteModeControlRegister(BYTE value) override;

		// Status Register
		virtual BYTE ReadStatusRegister() override;

		ADDRESS GetBaseAddressGraph() { return 0xB0000 + (((m_modeHGC.displayPage * 0x8000) + (GetCRTC().GetData().rowAddress * 0x2000) + (GetCRTC().GetMemoryAddress12() * 2u) & 0xFFFF)); }
	};
}
