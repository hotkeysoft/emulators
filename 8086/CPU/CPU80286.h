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

		bool IsProtectedMode() const { return GetMSW(MSW_PE); }

		// Machine Status Word
		enum MSW : WORD
		{
			MSW_PE = 0x0001, // Set Protected mode enabled, clear via Reset
			MSW_MP = 0x0002, // Monitor processor extension
			MSW_EM = 0x0004, // Emulate processor extension
			MSW_TS = 0x0008, // Task switched

			MSW_RESET = 0xFFF0,

			MSW_RESERVED_ON = 0xFFF0
		} m_msw = MSW_RESET;

		bool GetMSW(MSW flag) const { return m_msw & flag; }

		BYTE GetIOPL(WORD flags) const
		{
			return (flags >> 12) & 3;
		}

		virtual void SetFlags(WORD flags) override;

		void MultiF0(BYTE op2);
		void MultiF000(BYTE op3);
		void MultiF001(BYTE op3);

		void SGDT(Mem16& dest);
		void SIDT(Mem16& dest);
		void LGDT(const Mem16& source);
		void LIDT(const Mem16& source);
		void SMSW(Mem16& dest);
		void LMSW(const Mem16& source);

		void LAR(BYTE regrm);
		void LSL(BYTE regrm);
		void LOADALL();
		void CLTS();
	};
}
