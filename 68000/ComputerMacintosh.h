#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"
#include "Hardware/Device6522Mac.h"
#include "Hardware/IOBlockVIAMac.h"
#include "Video/VideoMac.h"
#include "Sound/SoundMac.h"

namespace emul
{
	namespace cpu68k { class CPU68000; };

	class ComputerMacintosh : public ComputerBase, public video::mac::EventHandler
	{
	public:
		ComputerMacintosh();

		virtual std::string_view GetName() const override { return "Macintosh"; };
		virtual std::string_view GetID() const override { return "mac"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		cpu68k::CPU68000& GetCPU() const { return *((cpu68k::CPU68000*)m_cpu); }

		video::mac::VideoMac& GetVideo() { return *((video::mac::VideoMac*)m_video); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		// video::mac::EventHandler
		virtual void OnHBlankStart() override;
		virtual void OnVBlankStart() override;

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;

		via::IOBlockVIAMac m_ioVIA;
		via::Device6522Mac m_via;

		sound::mac::SoundMac m_sound;
	};
}
