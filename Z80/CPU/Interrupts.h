#pragma once
#include "Common.h"

namespace emul
{
	class Interrupts : public Logger
	{
	public:
		Interrupts() : Logger("INTR") {}
		virtual ~Interrupts() {}

		enum class IntType { NONE, VECTOR, OPCODE };

		virtual bool IsInterrupting() const = 0;
		virtual void Acknowledge() = 0;

		virtual IntType GetType() const = 0;
		virtual BYTE GetOpcode() const { return 0; }
		virtual BYTE GetVector() const { return 0; }
	};
}
