#pragma once

#include "Computer.h"
#include "IO/InputEvents.h"

namespace emul
{
	class CPU6502;

	class Computer6502 : public Computer
	{
	public:
		Computer6502();

		virtual std::string_view GetName() const override { return "6502"; };
		virtual std::string_view GetID() const override { return "6502"; };

		virtual void Init(WORD baseRAM) override;
		virtual void Reset() override;

		virtual bool Step() override;

		CPU6502& GetCPU() const { return *((CPU6502*)m_cpu); }

		void DummyOut(BYTE value) {}
		BYTE DummyIn() { return 0xBF; }

	protected:
		emul::MemoryBlock m_baseRAM;
	};
}
