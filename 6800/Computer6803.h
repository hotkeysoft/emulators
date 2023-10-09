#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"

namespace emul
{
	class CPU6803;

	class Computer6803 : public ComputerBase
	{
	public:
		Computer6803();

		virtual std::string_view GetName() const override { return "6800"; };
		virtual std::string_view GetID() const override { return "6800"; };

		virtual void Init(WORD baseRAM) override;
		virtual bool Step() override;

		virtual void Reset() override;

		CPU6803& GetCPU() const { return *((CPU6803*)m_cpu); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		emul::MemoryBlock m_ram;
		emul::MemoryBlock m_rom;
	};
}
