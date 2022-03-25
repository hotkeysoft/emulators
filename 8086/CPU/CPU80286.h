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

		enum FLAG286 : WORD
		{
			FLAG_IOPL0 = 0x1000,
			FLAG_IOPL1 = 0x2000,

			FLAG_NT = 0x4000,
		};

	protected:

		BYTE GetIOPL(WORD flags) const
		{
			return (flags >> 12) & 3;
		}

		virtual void SetFlags(WORD flags) override;

	};
}
