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
		bool IsNull() const { return data == 0; }

		const char* ToString() const;

		operator WORD() const { return data; };

	protected:
		WORD data;
	};

	struct ExplicitRegister
	{
		WORD limit = 0; // 16 bits
		DWORD base = 0; // 32 bits (24 used, 8 undefined)

		const char* ToString() const;
	};


	struct AccessRights
	{
		AccessRights() {}
		AccessRights(BYTE a) : access(a) {}
		BYTE access = 0;

		// Applicable to all types
		bool IsValid() const { return (access & 0b00001111) == 0; }
		bool IsPresent() const { return GetBit(access, 7); }
		BYTE GetDPL() const { return (access >> 5) & 3; }

		bool IsCodeOrData() const { return GetBit(access, 4); }
		bool IsControl() const { return !GetBit(access, 4); }

		// Applies to Code/Data descriptor
		bool IsExecutable() const { return GetBit(access, 3); }
		bool IsAccessed() const { return GetBit(access, 0); }
		bool IsReadable() const { return !IsExecutable() || (IsExecutable() && GetBit(access, 1)); }

		// Applies to Code
		bool IsConforming() const { return IsExecutable() && GetBit(access, 2); }

		// Applies to Data
		bool IsExpandDown() const { return !IsExecutable() && GetBit(access, 2); }
		bool IsWritable() const { return !IsExecutable() && GetBit(access, 1); }

		// Applies to Control descriptor
		bool IsLDT() const      { return (access & 0b00011111) == 0b00000010; }
		bool IsTask() const     { return (access & 0b00011101) == 0b00000001; }
		bool IsTaskBusy() const { return (access & 0b00011111) == 0b00000011; }

		const char* GetTypeStr() const;

		const char* ToString() const;
	};

	struct SegmentDescriptor
	{
		WORD limit = 0;
		DWORD base = 0;
		AccessRights access;

		const char* ToString() const;
	};

	struct SegmentTranslationRegister
	{
		Selector selector;
		AccessRights access;
		DWORD base = 0;
		WORD size = 0;
	};

	struct InterruptDescriptor
	{
		WORD offset = 0;
		Selector selector;
		BYTE flags = 0;

		bool IsPresent() const { return GetBit(flags, 7); }
		BYTE GetDPL() const { return (flags >> 5) & 3; }
		enum class GateType { INTERRUPT = 0b0110, TRAP = 0b0111 };
		bool IsGateTypeValid() const { return (flags & 0b1110) == 0b0110; }
		GateType GetGateType() const { return (GateType)(flags & 0b1111); }

		const char* ToString() const;
	};

	class CPU80286 : public CPU80186
	{
	public:
		CPU80286(Memory& memory);

		virtual void Init() override;

		virtual void Reset() override;

		virtual size_t GetAddressBits() const { return CPU80286_ADDRESS_BITS; }

		virtual ADDRESS GetAddress(SegmentOffset segoff, MemAccess access = MemAccess::NONE) const override;
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

		std::map<BYTE, std::function<void()>> m_replacedOpcodes;
		void ReplaceOpcode(BYTE, std::function<void()>);
		void RestoreOpcodes();

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

		// Local Descriptor Table Register
		SegmentTranslationRegister m_ldtr;

		// Task Register
		SegmentTranslationRegister m_task;

		void UpdateTranslationRegister(SegmentTranslationRegister& dest, Selector selector, SegmentDescriptor desc);

		BYTE m_iopl = 0;

		virtual void SetFlags(WORD flags) override;

		void ARPL(BYTE regrm);

		void MultiF0(BYTE op2);
		void MultiF000(BYTE op3);
		void MultiF001(BYTE op3);

		void SLDT(Mem16& dest);
		void STR(Mem16& dest);
		void LLDT(Mem16& source);
		void LTR(Mem16& source);
		void VERR(Mem16& source);
		void VERW(Mem16& source);

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

		InterruptDescriptor GetInterruptDescriptor(BYTE interrupt) const;

		virtual void INT(BYTE interrupt) override;
		virtual void LoadPTR(SEGREG dest, SourceDest16 modRegRm) override;

		void MOVSegReg(SourceDest16 sd);
		void POPSegReg(SEGREG segreg);

	};
}
