#pragma once

#include "Computer.h"
#include "IO/InputEvents.h"
#include "Video/VideoZX80.h"

namespace emul
{
	class CPUZ80;

	class ComputerZ80 : public Computer
	{
	public:
		ComputerZ80();

		virtual std::string_view GetName() const override { return "Z80"; };
		virtual std::string_view GetID() const override { return "z80"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		CPUZ80& GetCPU() const { return *((CPUZ80*)m_cpu); }

		// TODO: Temporary, put generic video
		video::VideoZX80& GetVideo() { return *((video::VideoZX80*)m_video); }

		void PrintChar(BYTE value);
		void DummyOut(BYTE value) {}
		BYTE DummyIn() { return 0xBF; }

	protected:
		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;
	};
}
