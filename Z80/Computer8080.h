#pragma once

#include "Computer.h"

namespace emul
{
	class Computer8080 : public Computer
	{
	public:
		Computer8080();

		virtual std::string_view GetName() const override { return "8080"; };
		virtual std::string_view GetID() const override { return "8080"; };

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

	protected:

		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_rom;
	};
}
