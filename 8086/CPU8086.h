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
			FLAG_CF = 0x0001, // Carry
			FLAG_PF = 0x0004, // Parity
			FLAG_AF = 0x0010, // Auxiliary Carry
			FLAG_ZF = 0x0040, // Zero
			FLAG_SF = 0x0080, // Sign
			FLAG_TF = 0x0100, // Trap
			FLAG_IF = 0x0200, // Interrupt Enable
			FLAG_DF = 0x0400, // Direction
			FLAG_OF = 0x0800, // Overflow

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

		// Helper functions
		BYTE fetchByte();
		WORD fetchWord();

		// Opcodes
		void JMPfar(BYTE);

		void CLC(BYTE);
		void CMC(BYTE);
		void STC(BYTE);
		void CLD(BYTE);
		void STD(BYTE);
		void CLI(BYTE);
		void STI(BYTE);
		void HLT(BYTE);

		void NotImplemented(BYTE);
	};
}
