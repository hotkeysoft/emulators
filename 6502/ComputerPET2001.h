#pragma once

#include "Computer.h"
#include "IO/InputEvents.h"

namespace emul
{
	class CPUZ80;

	class ComputerPET2001 : public Computer
	{
	public:
		ComputerPET2001();

		virtual std::string_view GetName() const override { return "PET2001"; };
		virtual std::string_view GetID() const override { return "pet2001"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		CPU6502& GetCPU() const { return *((CPU6502*)m_cpu); }

	protected:
		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_romC000;
		emul::MemoryBlock m_romD000;
		emul::MemoryBlock m_romE000;
		emul::MemoryBlock m_romF000;

	};
}
