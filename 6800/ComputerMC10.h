#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"
#include "Hardware/IOBlockMC10.h"

namespace emul
{
	class CPU6803;

	class ComputerMC10 : public ComputerBase
	{
	public:
		ComputerMC10();

		virtual std::string_view GetName() const override { return "TRS-80"; };
		virtual std::string_view GetModel() const override { return "MC-10"; };
		virtual std::string_view GetID() const override { return "mc10"; };

		virtual void Init(WORD baseRAM) override;
		virtual bool Step() override;

		virtual void Reset() override;

		CPU6803& GetCPU() const { return *((CPU6803*)m_cpu); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitROM();
		void InitRAM(WORD baseRAM);
		void InitIO();
		void InitKeyboard();
		void InitVideo();

		static constexpr DWORD   MAIN_CLOCK = 3579545;
		static constexpr DWORD   CPU_CLOCK = MAIN_CLOCK / 4;
		static constexpr DWORD   INPUT_REFRESH_HZ = 60;
		static constexpr DWORD   INPUT_REFRESH_PERIOD = CPU_CLOCK / INPUT_REFRESH_HZ;

		static constexpr WORD    RAM_SIZE = 0x1000;
		static constexpr ADDRESS RAM_BASE = 0x4000;

		static constexpr DWORD   ROM_SIZE = 0x2000;
		static constexpr ADDRESS ROM_BASE = 0xC000;

		static constexpr ADDRESS IO_BASE =  0x8000;

		emul::MemoryBlock m_ram;
		emul::MemoryBlock m_rom;

		bool m_soundData = false;

		io::mc10::IOBlockMC10 m_io;
	};
}
