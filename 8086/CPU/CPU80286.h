#pragma once

#include "CPU80186.h"

namespace emul
{
	class CPU80286 : public CPU80186
	{
	public:
		CPU80286(Memory& memory);

		virtual void Init() override;

		virtual void Reset() override;

	protected:
	};
}
