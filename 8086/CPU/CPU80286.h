#pragma once

#include "CPU80186.h"

namespace emul
{
	static const size_t CPU80286_ADDRESS_BITS = 24;

	struct Selector
	{
		Selector() : data(0) {}
		Selector(WORD d) : data(d) {}

		BYTE GetRPL() const { return data & 3; }
		bool GetTI() const { return GetBit(data, 0); }
		WORD GetIndex() const { return data >> 3; }

	protected:
		WORD data;
	};

	struct ExplicitRegister
	{
		WORD limit = 0; // 16 bits
		DWORD base = 0; // 32 bits (24 used, 8 undefined)

		const char* ToString() const;
	};

#pragma pack(push, 1)
	struct SegmentDescriptor
	{
		WORD limit = 0;
		WORD baseL = 0;
		BYTE baseH = 0;
		BYTE access = 0;
		WORD reserved = 0;

		DWORD GetBase() const { return (baseH << 16) | baseL; }
	};
#pragma pack(pop)

	struct SegmentTranslationRegister
	{
		Selector selector;
		BYTE access = 0;
		DWORD base = 0;
		WORD size = 0;
	};

	class CPU80286 : public CPU80186
	{
	public:
		CPU80286(Memory& memory);

		virtual void Init() override;

		virtual void Reset() override;

		virtual size_t GetAddressBits() const { return CPU80286_ADDRESS_BITS; }

		virtual ADDRESS GetAddress(SegmentOffset segoff) const override;
		virtual ADDRESS GetCurrentAddress() const override;

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

		// Switch instructions for real/protected mode
		void ProtectedMode();

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

		// System Address Registers
		ExplicitRegister m_gdt; // Global Descriptor Table Register
		ExplicitRegister m_idt; // Interrupt Descriptor Table Register

		SegmentDescriptor LoadSegmentGlobal(Selector selector) const;
		SegmentDescriptor LoadSegmentLocal(Selector selector) const;

		// Segment Address Translation Registers;
		SegmentTranslationRegister m_cs;
		SegmentTranslationRegister m_ds;
		SegmentTranslationRegister m_es;
		SegmentTranslationRegister m_ss;
		void UpdateTranslationRegister(SegmentTranslationRegister& dest, Selector selector, SegmentDescriptor desc);

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
		void LGDT(Mem16& source);
		void LIDT(Mem16& source);
		void SMSW(Mem16& dest);
		void LMSW(Mem16& source);

		void LAR(BYTE regrm);
		void LSL(BYTE regrm);
		void LOADALL();
		void CLTS();
	};
}
