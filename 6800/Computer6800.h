#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"

namespace emul
{
	class CPU6800;

	class Computer6800 : public ComputerBase
	{
	public:
		Computer6800();

		virtual std::string_view GetName() const override { return "6800"; };
		virtual std::string_view GetID() const override { return "6800"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6800& GetCPU() const { return *((CPU6800*)m_cpu); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		emul::MemoryBlock m_ram;
		emul::MemoryBlock m_rom;
	};
}
