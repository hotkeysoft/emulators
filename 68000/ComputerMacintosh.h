#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"
#include <Video/VideoNull.h>
#include "Hardware/Device6522Mac.h"
#include "Hardware/IOBlockVIAMac.h"

namespace emul
{
	namespace cpu68k { class CPU68000; };

	class ComputerMacintosh : public ComputerBase
	{
	public:
		ComputerMacintosh();

		virtual std::string_view GetName() const override { return "Macintosh"; };
		virtual std::string_view GetID() const override { return "mac"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		cpu68k::CPU68000& GetCPU() const { return *((cpu68k::CPU68000*)m_cpu); }

		video::VideoNull& GetVideo() { return *((video::VideoNull*)m_video); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;

		via::IOBlockVIAMac m_ioVIA;

		via::Device6522Mac m_via;
	};
}
