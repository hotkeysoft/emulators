#pragma once
#include "Interrupts.h"

namespace emul
{
	class Interrupts8080 : public Interrupts
	{
		virtual bool IsInterrupting() const override;
		virtual void Acknowledge() override;

		virtual IntType GetType() const override { return IntType::OPCODE; }
	};
}
