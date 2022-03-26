#pragma once

#include "CPU80186.h"

namespace emul
{
	static const size_t CPU80286_ADDRESS_BITS = 24;

	class CPU80286 : public CPU80186
	{
	public:
		CPU80286(Memory& memory);

		virtual void Init() override;

		virtual void Reset() override;

		virtual size_t GetAddressBits() const { return CPU80286_ADDRESS_BITS; }

		enum FLAG286 : WORD
		{
			FLAG_IOPL0 = 0x1000,
			FLAG_IOPL1 = 0x2000,

			FLAG_NT = 0x4000,
		};

		void ForceA20Low(bool forceLow);

	protected:

		virtual void CPUExceptionHandler(CPUException e) override;

		BYTE GetIOPL(WORD flags) const
		{
			return (flags >> 12) & 3;
		}

		virtual void SetFlags(WORD flags) override;

	};
}
