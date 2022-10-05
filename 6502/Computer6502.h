#pragma once

#include <Computer/ComputerBase.h>
#include "IO/InputEvents.h"

namespace emul
{
	class CPU6502;

	class Computer6502 : public ComputerBase
	{
	public:
		Computer6502();

		virtual std::string_view GetName() const override { return "6502"; };
		virtual std::string_view GetID() const override { return "6502"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;
		virtual bool Step() override;

		CPU6502& GetCPU() const { return *((CPU6502*)m_cpu); }

	protected:
		virtual void InitCPU(const char* cpuid) override;

		void InitVideo();

		emul::MemoryBlock m_baseRAM;
	};
}
