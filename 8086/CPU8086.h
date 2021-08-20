#pragma once

#include "CPU.h"
#include "PortConnector.h"
#include "PortAggregator.h"

namespace emul
{
	static const size_t CPU8086_ADDRESS_BITS = 20;

	#pragma pack(push, 1)
	union Register
	{
		WORD x;
		struct {
			BYTE l;
			BYTE h;
		} hl;
	};
	#pragma pack(pop)

	inline ADDRESS S2A(WORD segment, WORD offset = 0)
	{
		return (segment << 4) + offset;
	}

	class CPU8086 : public CPU
	{
	public:
		CPU8086(Memory& memory, MemoryMap& mmap);
		virtual ~CPU8086();

		virtual size_t GetAddressBits() const { return CPU8086_ADDRESS_BITS; }
		virtual ADDRESS GetCurrentAddress() const { return S2A(regCS, regIP); }
		virtual void Exec(BYTE opcode);

		void AddDevice(PortConnector& ports);

		void Dump();

		virtual void Reset();

	protected:
		PortAggregator m_ports;

		// General Registers
		Register regA; // Accumulator
		Register regB; // Base
		Register regC; // Count
		Register regD; // Data

		WORD regSP; // Stack Pointer
		WORD regBP; // Base Pointer
		WORD regSI; // Source Index
		WORD regDI; // Destination Index

		// Segment Registers
		WORD regCS; // Code Segment
		WORD regDS; // Data Segment
		WORD regSS; // Stack Segment
		WORD regES; // Extra Segment

		WORD regIP; // Instruction Pointer

		enum FLAG : WORD
		{
			FLAG_C = 0x0001, // Carry
			FLAG_P = 0x0004, // Parity
			FLAG_A = 0x0010, // Auxiliary Carry
			FLAG_Z = 0x0040, // Zero
			FLAG_S = 0x0080, // Sign
			FLAG_T = 0x0100, // Trap
			FLAG_I = 0x0200, // Interrupt Enable
			FLAG_D = 0x0400, // Direction
			FLAG_O = 0x0800, // Overflow

			FLAG_R1 = 0x0002,
			FLAG_R3 = 0x0008,
			FLAG_R5 = 0x0020,
			FLAG_R12 = 0x1000, // IOPL, Always 1 on 8086/186
			FLAG_R13 = 0x2000, // IOPL, Always 1 on 8086/186
			FLAG_R14 = 0x4000, // NT, Always 1 on 8086/186
			FLAG_R15 = 0x8000, // Always 1 on 8086/186
		};

		WORD flags;

		void ClearFlags();
		bool GetFlag(FLAG f) { return (flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { if (v) flags |= f; else flags &= ~f; };

		// Pseudo flags
		// ----------
		bool GetFlagNotAbove() { return GetFlag(FLAG_C) || GetFlag(FLAG_Z); }
		bool GetFlagNotLess() { return GetFlag(FLAG_S) == GetFlag(FLAG_O); }
		bool GetFlagGreater() { return (GetFlag(FLAG_S) == GetFlag(FLAG_O)) && GetFlag(FLAG_Z); }

		// Helper functions

		void AdjustParity(BYTE data);
		void AdjustSign(BYTE data);
		void AdjustZero(BYTE data);

		void AdjustParity(WORD data);
		void AdjustSign(WORD data);
		void AdjustZero(WORD data);

		BYTE FetchByte();
		WORD FetchWord();

		BYTE& GetModRM8(BYTE modrm);
		BYTE& GetReg8(BYTE reg);
		const char* GetReg8Str(BYTE reg); // For logging

		//WORD& GetModRM16(BYTE modrm);

		// Opcodes
		void JMPfar();

		void CLC();
		void CMC();
		void STC();
		void CLD();
		void STD();
		void CLI();
		void STI();
		void HLT();

		void INC16(WORD&);
		void DEC16(WORD&);

		void MOV8(BYTE& d, BYTE s);
		void MOV16(WORD& d, WORD s);

		void SAHF();
		void LAHF();

		void JMPif(bool cond);

		void SHIFTROT8(BYTE op2, BYTE count);
		void SHIFTROT16(BYTE op2, BYTE count);

		void NotImplemented(BYTE);
	};
}
