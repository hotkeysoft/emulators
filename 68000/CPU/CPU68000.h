#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

namespace emul::cpu68k
{
	enum class EAMode
	{
		// Register Direct modes
		DataRegDirect = 0b000000,
		AddrRegDirect = 0b001000,

		// Memory Address modes
		AddrRegIndirect = 0b010000,
		AddrRegIndirectPostIncrement = 0b011000,
		AddrRegIndirectPreDecrement = 0b100000,
		AddrRegIndirectDisplacement = 0b101000,
		AddrRegIndirectIndex = 0b110000,

		// Special Address Modes (Mode=111)
		// (need reg# for complete decoding)
		AbsoluteShort = 0b111000,
		AbsoluteLong = 0b111001,
		ProgramCounterDisplacement = 0b111010,
		ProgramCounterIndex = 0b111011,
		Immediate = 0b111100,

		Invalid = 0b111111
	};

	static const size_t CPU68000_ADDRESS_BITS = 24;
	static const char* CPUID_68000 = "68000";

	class CPU68000 : public CPU, public PortConnector
	{
	public:
		constexpr static ADDRESS ADDRESS_MASK = 0xFFFFFF;
		constexpr static int OP_BYTE = 1;
		constexpr static int OP_WORD = 2;
		constexpr static int OP_LONG = 4;

		CPU68000(Memory& memory);
		virtual ~CPU68000();

		virtual void Init();

		void Dump();

		virtual void Reset();
		virtual void Reset(ADDRESS overrideAddress);

		virtual bool Step() override;
		// Not used, 16 bit opcode
		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU68000_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter & ADDRESS_MASK; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		CPU68000(const char* cpuid, Memory& memory);

		using OpcodeTable = std::vector<std::function<void()>>;

		void InitTable(OpcodeTable& table, size_t size);

		void InitGroup0(OpcodeTable& table, size_t size);
		void InitGroup4(OpcodeTable& table, size_t size);
		void InitGroup5(OpcodeTable& table, size_t size);
		void InitGroup6(OpcodeTable& table, size_t size);
		void InitGroup8(OpcodeTable& table, size_t size);
		void InitGroup9(OpcodeTable& table, size_t size);
		void InitGroup11(OpcodeTable& table, size_t size);
		void InitGroup12(OpcodeTable& table, size_t size);
		void InitGroup13(OpcodeTable& table, size_t size);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };

		// Hardware vectors
		enum class VECTOR : BYTE
		{
			ResetSSP               = 0, // Reset, Initial SSP (Supervisor Stack Pointer)
			ResetPC                = 1, // Reset, Initial PC (Program Counter)
			BusError               = 2,
			AddressError           = 3,
			IllegalInstruction     = 4,
			ZeroDivide             = 5,
			CHK_Instruction        = 6,
			TRAPV_Instruction      = 7,
			PrivilegeViolation     = 8,
			Trace                  = 9,
			Line1010Emulator       = 10,
			Line1111Emulator       = 11,

			// (12-14 Reserved)

			UninitializedIntVector = 15,

			// (16-23 Reserved)

			SpuriousInterrupt      = 24,

			// 25-31 Level 1-7 Interrupt Autovector
			InterruptBase          = 25,

			// 32-47 TRAP Instructions vectors (32 + n)
			TrapBase               = 32,

			// 48-63 Reserved
			// 64-255 User Interrupt Vectors
			USER_INT_BASE          = 64
		};

		ADDRESS GetVectorAddress(VECTOR v) { return (ADDRESS)v * 4; }
		ADDRESS GetIntVectorAddress(BYTE i) { assert(i <= 7); return ((ADDRESS)VECTOR::InterruptBase + i) * 4; }
		ADDRESS GetTrapVectorAddress(BYTE t) { assert(t <= 16);  return ((ADDRESS)VECTOR::TrapBase + t) * 4; }

		OpcodeTable m_opcodes;
		OpcodeTable m_subOpcodes[16];
		[[noreturn]] void IllegalInstruction();
		[[noreturn]] void NotImplementedOpcode(const char* name);

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		WORD m_opcode = 0;
		int GetOpcodeRegisterIndex() const { return m_opcode & 0b111; }

		WORD GetSubopcode6() const { return ((m_opcode >> 6) & 63); }
		WORD GetSubopcode4() const { return ((m_opcode >> 8) & 15); }

		virtual void Interrupt();

		enum FLAG : WORD
		{
			// Supervisor mode flags (R/O in user mode)
			FLAG_T		= 0x8000, // 1 Trace mode
			FLAG_R14	= 0x4000, // 0
			FLAG_S		= 0x2000, // 1 Supervisor mode
			_FLAG_R12	= 0x1000, // 0
			_FLAG_R11	= 0x0800, // 0
			FLAG_I2 	= 0x0400, // I[0..2], interrupt mask
			FLAG_I1		= 0x0200, // ""
			FLAG_I0		= 0x0100, // ""

			// User mode R/W
			_FLAG_R7	= 0x0080, // 0
			_FLAG_R6	= 0x0040, // 0
			_FLAG_R5	= 0x0020, // 0
			FLAG_X		= 0x0010, // 1 eXtend
			FLAG_N      = 0x0008, // 1 Negative
			FLAG_Z		= 0x0004, // 1 Zero
			FLAG_V		= 0x0002, // 1 Signed oVerflow
			FLAG_C		= 0x0001,  // 1 Carry (unsigned overflow)

			// Pseudo-flags
			FLAG_CX = FLAG_C | FLAG_X
		};

		FLAG FLAG_RESERVED_OFF = FLAG(
			_FLAG_R5 | _FLAG_R6 | _FLAG_R7 |
			_FLAG_R11 | FLAG_R14);

		bool FlagHI() const { return !GetFlag(FLAG_C) && !GetFlag(FLAG_Z); }
		bool FlagLS() const { return !FlagHI(); }
		bool FlagCC() const { return !GetFlag(FLAG_C); }
		bool FlagCS() const { return GetFlag(FLAG_C); }
		bool FlagNE() const { return !GetFlag(FLAG_Z); }
		bool FlagEQ() const { return GetFlag(FLAG_Z); }
		bool FlagVC() const { return !GetFlag(FLAG_V); }
		bool FlagVS() const { return GetFlag(FLAG_V); }
		bool FlagPL() const { return !GetFlag(FLAG_N); }
		bool FlagMI() const { return GetFlag(FLAG_N); }
		bool FlagGE() const { return
			(GetFlag(FLAG_N) && GetFlag(FLAG_V)) ||
			(!GetFlag(FLAG_N) && !GetFlag(FLAG_V));
		}
		bool FlagLT() const { return !FlagGE(); }
		bool FlagGT() const { return !FlagEQ() && FlagGE(); }
		bool FlagLE() const { return FlagEQ() || FlagLT(); };

		ADDRESS m_programCounter = 0;

		struct Registers
		{
			std::array<DWORD, 16> DataAddress; // D0..D7, A0..A7

			// Alias for DATA and ADDRESS registers
			DWORD* const DATA = &DataAddress[0];
			DWORD* const ADDR = &DataAddress[8];

			// Alias stack pointer to A7
			DWORD& SP = ADDR[7];

			// Stack pointer is aliased to A7 but user and supervisor
			// modes keep separate values.
			// These are updated only then supervisor mode changes so to
			// get a "fresh" value use GetUSP()/GetSSP()

			DWORD USP; // User Stack Pointer
			DWORD SSP; // Supervisor Stack Pointer

			WORD flags;
		} m_reg;

		DWORD GetSP() const { return m_reg.SP; }
		DWORD GetUSP() const { return IsSupervisorMode() ? m_reg.USP : m_reg.SP; }
		DWORD GetSSP() const { return IsSupervisorMode() ? m_reg.SP : m_reg.SSP; }

		DWORD& GetSP() { return m_reg.SP; }
		DWORD& GetUSP() { return IsSupervisorMode() ? m_reg.USP : m_reg.SP; }
		DWORD& GetSSP() { return IsSupervisorMode() ? m_reg.SP : m_reg.SSP; }

		void ClearFlags(WORD& flags);
		void SetFlags(WORD f);

		// Flag Helpers
		bool GetFlag(FLAG f) const { return (m_reg.flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); };
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		bool IsSupervisorMode() const { return GetFlag(FLAG_S); }
		void SetSupervisorMode(bool set);

		bool IsTrace() const { return GetFlag(FLAG_T); }
		void SetTrace(bool set) { SetFlag(FLAG_T, set); }

		BYTE GetInterruptMask() const { return (m_reg.flags >> 8) | 7; }
		void SetInterruptMask(BYTE mask) {
			assert(mask < 8);
			SetFlag(FLAG_I0, GetBit(mask, 0));
			SetFlag(FLAG_I1, GetBit(mask, 1));
			SetFlag(FLAG_I2, GetBit(mask, 2));
		}

		virtual BYTE FetchByte() override { return GetLByte(FetchWord()); }
		virtual WORD FetchWord() override;
		DWORD FetchLong();

		void Exec(WORD opcode);
		void Exec(WORD group, WORD subOpcode);
		bool InternalStep();

		// Adjust negative and zero flag
		void AdjustNZ(BYTE val);
		void AdjustNZ(WORD val);
		void AdjustNZ(DWORD val);

		EAMode m_eaMode;
		static EAMode GetEAMode(WORD opcode);
		BYTE GetEAByte();
		WORD GetEAWord();
		DWORD GetEALong();
		ADDRESS GetEA(int size);

		[[noreturn]] void Exception(VECTOR v);
		void Privileged() { if (!IsSupervisorMode()) Exception(VECTOR::PrivilegeViolation); }

		// Opcodes

		void LEA(DWORD& dest);

		// Branching
		void BRA(bool cond = true);

		void MOVE_b() { NotImplementedOpcode("MOVE.b"); }
		void MOVE_l() { NotImplementedOpcode("MOVE.l"); }
		void MOVE_w() { NotImplementedOpcode("MOVE.w"); }
		void MOVEQ() { NotImplementedOpcode("MOVEQ"); }
		void SHIFT() { NotImplementedOpcode("Shift ops"); }

		void MOVE_w_toSR(WORD src);

		void MOVEM_w_toMem(WORD regs) { NotImplementedOpcode("MOVEM.w (regs->mem)"); }
		void MOVEM_l_toMem(WORD regs) { NotImplementedOpcode("MOVEM.l (regs->mem)"); }
		void MOVEM_w_fromMem(WORD regs);
		void MOVEM_l_fromMem(WORD regs);

		void EXT_w() { NotImplementedOpcode("EXT.w"); }
		void EXT_l() { NotImplementedOpcode("EXT.l"); }

		// dest' <- dest + src
		void ADD_b(BYTE& dest, BYTE src);
		void ADD_w(WORD& dest, WORD src);
		void ADD_l(DWORD& dest, DWORD src);

		// dest' <- dest - src
		void SUB_b(BYTE& dest, BYTE src, FLAG carryFlag = FLAG_CX);
		void SUB_w(WORD& dest, WORD src, FLAG carryFlag = FLAG_CX);
		void SUB_l(DWORD& dest, DWORD src, FLAG carryFlag = FLAG_CX);

		// dest by value so it's not modified
		// (void) <- dest - src
		// (doesn't set X flag)
		void CMP_b(BYTE dest, BYTE src) { return SUB_b(dest, src, FLAG_C); }
		void CMP_w(WORD dest, WORD src) { return SUB_w(dest, src, FLAG_C); }
		void CMP_l(DWORD dest, DWORD src) { return SUB_l(dest, src, FLAG_C); }

		friend class Monitor68000;
	};
}
