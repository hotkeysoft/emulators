#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"

namespace emul
{
	class CPU6809;

	class Computer6809 : public ComputerBase
	{
	public:
		Computer6809();

		virtual std::string_view GetName() const override { return "6809"; };
		virtual std::string_view GetID() const override { return "6809"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6809& GetCPU() const { return *((CPU6809*)m_cpu); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_osROM;
		emul::MemoryBlock m_basicROM;
	};
}
