#pragma once

#include "Computer.h"

namespace emul
{
	class ComputerZ80 : public Computer
	{
	public:
		ComputerZ80();

		virtual std::string_view GetName() const override { return "Z80"; };
		virtual std::string_view GetID() const override { return "z80"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

	protected:
		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;
	};
}
