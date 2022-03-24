#pragma once

#include "CPU8086.h"

namespace emul
{
	class CPU80186 : public CPU8086
	{
	public:
		CPU80186(Memory& memory);

		virtual void Init() override;

	protected:
		void PUSHA();
		void POPA();
	};
}
