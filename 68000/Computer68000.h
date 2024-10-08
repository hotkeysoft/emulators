#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"
#include <Video/VideoNull.h>

namespace emul
{
	namespace cpu68k { class CPU68000; }

	class Computer68000 : public ComputerBase
	{
	public:
		Computer68000();

		virtual std::string_view GetName() const override { return "68000"; };
		virtual std::string_view GetID() const override { return "68000"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		cpu68k::CPU68000& GetCPU() const { return *((cpu68k::CPU68000*)m_cpu); }

		// TODO: Temporary, put generic video
		video::VideoNull& GetVideo() { return *((video::VideoNull*)m_video); }

		void PrintChar(BYTE value);
		void DummyOut(BYTE value) {}
		BYTE DummyIn() { return 0xBF; }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;
	};
}
