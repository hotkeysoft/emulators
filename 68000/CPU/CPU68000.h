#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

#define BYTE_REG(src, idx) *(((BYTE*)src) + (idx << 2))
#define WORD_REG(src, idx) *(((WORD*)src) + (idx << 1))

namespace emul::cpu68k
{
	enum class EAMode : WORD
	{
		// Register Direct modes
		DRegDirect = 1 << 0,
		ARegDirect = 1 << 1,

		// Memory Address modes
		ARegIndirect = 1 << 2,
		ARegIndirectPostinc = 1 << 3,
		ARegIndirectPredec = 1 << 4,
		ARegIndirectDisp = 1 << 5,
		ARegIndirectIndex = 1 << 6,

		// Special Address Modes (Mode=111)
		// (need reg# for complete decoding)
		AbsShort = 1 << 7,
		AbsLong = 1 << 8,
		PCDisp = 1 << 9,
		PCIndex = 1 << 10,
		Immediate = 1 << 11,

		Invalid = 0,

		// Sub groups for group modes below
		_RegDirect = DRegDirect | ARegDirect,
		_Displacement = ARegIndirectDisp | ARegIndirectIndex,
		_RegIndirect = ARegIndirect | ARegIndirectPostinc | ARegIndirectPredec | _Displacement,
		_Absolute = AbsShort | AbsLong,
		_PC = PCDisp | PCIndex,

		// Address group modes for validation
		GroupAll = _RegDirect | _RegIndirect | _Absolute | _PC | Immediate,
		GroupData = DRegDirect | _RegIndirect | _Absolute | _PC | Immediate,
		GroupDataNoImm = DRegDirect | _RegIndirect | _Absolute | _PC,
		GroupMemAlt = _RegIndirect | _Absolute,
		GroupDataAlt = DRegDirect | _RegIndirect | _Absolute,
		GroupDataAddrAlt = _RegDirect | _RegIndirect | _Absolute,
		GroupControl = ARegIndirect | _Displacement | _Absolute | _PC,
		GroupControlAlt = ARegIndirect | _Displacement | _Absolute,
		GroupControlAltPredec = GroupControlAlt | ARegIndirectPredec,
		GroupControlAltPostinc = GroupControlAlt | ARegIndirectPostinc,
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
		inline void TICKn(int t) { m_opTicks += t; }

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
			FLAG_CX = FLAG_C | FLAG_X,
			FLAG_VC = FLAG_V | FLAG_C
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

			// Handy aliases
			BYTE& D0b = BYTE_REG(DATA, 0);
			BYTE& D1b = BYTE_REG(DATA, 1);
			BYTE& D2b = BYTE_REG(DATA, 2);
			BYTE& D3b = BYTE_REG(DATA, 3);
			BYTE& D4b = BYTE_REG(DATA, 4);
			BYTE& D5b = BYTE_REG(DATA, 5);
			BYTE& D6b = BYTE_REG(DATA, 6);
			BYTE& D7b = BYTE_REG(DATA, 7);

			BYTE& GetDATAb(int index) { return BYTE_REG(DATA, index); }
			BYTE& GetADDRb(int index) { return BYTE_REG(ADDR, index); }

			WORD& D0w = WORD_REG(DATA, 0); WORD& A0w = WORD_REG(ADDR, 0);
			WORD& D1w = WORD_REG(DATA, 1); WORD& A1w = WORD_REG(ADDR, 1);
			WORD& D2w = WORD_REG(DATA, 2); WORD& A2w = WORD_REG(ADDR, 2);
			WORD& D3w = WORD_REG(DATA, 3); WORD& A3w = WORD_REG(ADDR, 3);
			WORD& D4w = WORD_REG(DATA, 4); WORD& A4w = WORD_REG(ADDR, 4);
			WORD& D5w = WORD_REG(DATA, 5); WORD& A5w = WORD_REG(ADDR, 5);
			WORD& D6w = WORD_REG(DATA, 6); WORD& A6w = WORD_REG(ADDR, 6);
			WORD& D7w = WORD_REG(DATA, 7); WORD& A7w = WORD_REG(ADDR, 7);

			WORD& GetDATAw(int index) { return WORD_REG(DATA, index); }
			WORD& GetADDRw(int index) { return WORD_REG(ADDR, index); }

			DWORD& D0 = DATA[0]; DWORD& A0 = ADDR[0];
			DWORD& D1 = DATA[1]; DWORD& A1 = ADDR[1];
			DWORD& D2 = DATA[2]; DWORD& A2 = ADDR[2];
			DWORD& D3 = DATA[3]; DWORD& A3 = ADDR[3];
			DWORD& D4 = DATA[4]; DWORD& A4 = ADDR[4];
			DWORD& D5 = DATA[5]; DWORD& A5 = ADDR[5];
			DWORD& D6 = DATA[6]; DWORD& A6 = ADDR[6];
			DWORD& D7 = DATA[7]; DWORD& A7 = ADDR[7];

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

		static bool IsWordAligned(ADDRESS addr) { return !GetLSB(addr); }

		void WriteB(ADDRESS dest, BYTE value) { m_memory.Write8(dest, value); }
		void WriteW(ADDRESS dest, WORD value) { Aligned(dest); m_memory.Write16be(dest, value); }
		void WriteL(ADDRESS dest, DWORD value) { Aligned(dest); m_memory.Write32be(dest, value); }
		BYTE ReadB(ADDRESS src) const { return m_memory.Read8(src); }
		WORD ReadW(ADDRESS src) { Aligned(src); return m_memory.Read16be(src); }
		DWORD ReadL(ADDRESS src) { Aligned(src); return m_memory.Read32be(src); }

		void Exec(WORD opcode);
		void Exec(WORD group, WORD subOpcode);
		bool InternalStep();

		// Adjust negative and zero flag
		void AdjustNZ(BYTE val);
		void AdjustNZ(WORD val);
		void AdjustNZ(DWORD val);

		EAMode m_eaMode = EAMode::Invalid;
		static EAMode GetEAMode(WORD opcode);
		// Needs m_eaMode to be set
		void EACheck(EAMode group) { if (!((WORD)m_eaMode & (WORD)group)) IllegalInstruction(); }

		BYTE GetEAByte(EAMode groupCheck);
		WORD GetEAWord(EAMode groupCheck);
		DWORD GetEALong(EAMode groupCheck);

		ADDRESS GetExtensionWordDisp();

		// Used by GetEA(b|w|l) above
		ADDRESS rawGetEA(int size);

		ADDRESS GetEA(int size, EAMode groupCheck)
		{
			m_eaMode = GetEAMode(m_opcode);
			EACheck(groupCheck);

			return rawGetEA(size);
		}

		[[noreturn]] void Exception(VECTOR v);
		void Privileged() { if (!IsSupervisorMode()) Exception(VECTOR::PrivilegeViolation); }
		void Aligned(ADDRESS addr) { if (!IsWordAligned(addr)) Exception(VECTOR::AddressError); }

		// Opcodes

		void LEA(DWORD& dest);

		// Branching
		void BRA(bool cond = true);
		void JMP();

		void DBccw(bool cond);
		void Sccb(bool cond) { NotImplementedOpcode("Scc.b <ea>"); }

		// MOVE

		void MOVEb();
		void MOVEw();
		void MOVEl();

		void MOVEQ();

		void MOVE_w_toSR(WORD src);

		void MOVEMwToEA(WORD regs);
		void MOVEMlToEA(WORD regs);
		void MOVEMwFromEA(WORD regs);
		void MOVEMlFromEA(WORD regs);

		void MOVEPwToReg(WORD& dest) { NotImplementedOpcode("MOVEP.w (<ea> -> reg)"); }
		void MOVEPlToReg(DWORD& dest) { NotImplementedOpcode("MOVEP.l (<ea> -> reg)"); }

		void MOVEPwFromReg(WORD src) { NotImplementedOpcode("MOVEP.w (reg -> <ea>)"); }
		void MOVEPlFromReg(DWORD src) { NotImplementedOpcode("MOVEP.l (reg -> <ea>)"); }

		void EXGl() { NotImplementedOpcode("EXG.l"); }

		// Bit, Logic
		void BTSTimm();
		void BCHGimm() { BitOps(BitOp::CHANGE); }
		void BCLRimm() { BitOps(BitOp::CLEAR); }
		void BSETimm() { BitOps(BitOp::SET); }

		enum class BitOp { SET, CLEAR, CHANGE };
		void BitOps(BitOp bitop);

		void BTST(DWORD src) { NotImplementedOpcode("BTST Dn, <ea>"); }
		void BCHG(DWORD src) { NotImplementedOpcode("BCHG Dn, <ea>"); }
		void BCLR(DWORD src) { NotImplementedOpcode("BCLR Dn, <ea>"); }
		void BSET(DWORD src) { NotImplementedOpcode("BSET Dn, <ea>"); }

		void TSTb();
		void TSTw();
		void TSTl();

		void ANDbToEA(BYTE src);
		void ANDwToEA(WORD src);
		void ANDlToEA(DWORD src);

		void ANDIbToCCR() { NotImplementedOpcode("AND.b #imm, CCR"); }
		void ANDIwToSR() { NotImplementedOpcode("AND.w #imm, SR"); }

		void ANDIb();
		void ANDIw();
		void ANDIl();

		void ANDb(BYTE& dest, BYTE src);
		void ANDw(WORD& dest, WORD src);
		void ANDl(DWORD& dest, DWORD src);

		void EORbToEA(BYTE src);
		void EORwToEA(WORD src);
		void EORlToEA(DWORD src);

		void EORb(BYTE& dest, BYTE src);
		void EORw(WORD& dest, WORD src);
		void EORl(DWORD& dest, DWORD src);

		void SHIFT();

		void SHIFTb(BYTE& dest, int count, bool left, int operation);
		void SHIFTw(WORD& dest, int count, bool left, int operation);
		void SHIFTl(DWORD& dest, int count, bool left, int operation);

		void ASLw()  { NotImplementedOpcode("ADL.w <ea>"); }
		void ASRw()  { NotImplementedOpcode("ASR.w <ea>"); }
		void LSLw()  { NotImplementedOpcode("LSL.w <ea>"); }
		void LSRw()  { NotImplementedOpcode("LSR.w <ea>"); }
		void ROXLw() { NotImplementedOpcode("ROXL.w <ea>"); }
		void ROXRw() { NotImplementedOpcode("ROXR.w <ea>"); }
		void ROLw()  { NotImplementedOpcode("ROL.w <ea>"); }
		void RORw()  { NotImplementedOpcode("ROR.w <ea>"); }

		void ASLw(WORD& dest, int count);
		void ASRw(WORD& dest, int count);
		void LSLw(WORD& dest, int count);
		void LSRw(WORD& dest, int count);
		void ROXLw(WORD& dest, int count);
		void ROXRw(WORD& dest, int count);
		void ROLw(WORD& dest, int count);
		void RORw(WORD& dest, int count);

		void ASLl(DWORD& dest, int count);
		void ASRl(DWORD& dest, int count);
		void LSLl(DWORD& dest, int count);
		void LSRl(DWORD& dest, int count);
		void ROXLl(DWORD& dest, int count);
		void ROXRl(DWORD& dest, int count);
		void ROLl(DWORD& dest, int count);
		void RORl(DWORD& dest, int count);

		// Arithmetic

		void CLRb();
		void CLRw();
		void CLRl();

		void SWAPw();

		void EXTw() { NotImplementedOpcode("EXT.w"); }
		void EXTl() { NotImplementedOpcode("EXT.l"); }

		void ABCDb() { NotImplementedOpcode("ABCD.b"); }

		void ADDXb() { NotImplementedOpcode("ADDX.b"); }
		void ADDXw() { NotImplementedOpcode("ADDX.w"); }
		void ADDXl() { NotImplementedOpcode("ADDX.l"); }

		void ADDQb(BYTE imm);
		void ADDQw(WORD imm);
		void ADDQl(DWORD imm);

		void SUBQb(BYTE imm);
		void SUBQw(WORD imm);
		void SUBQl(DWORD imm);

		void ADDbToEA(BYTE src);
		void ADDwToEA(WORD src);
		void ADDlToEA(DWORD src);

		// dest' <- dest + src
		void ADDb(BYTE& dest, BYTE src);
		void ADDw(WORD& dest, WORD src);
		void ADDl(DWORD& dest, DWORD src);

		// dest' <- dest - src
		void SUBb(BYTE& dest, BYTE src, FLAG carryFlag = FLAG_CX);
		void SUBw(WORD& dest, WORD src, FLAG carryFlag = FLAG_CX);
		void SUBl(DWORD& dest, DWORD src, FLAG carryFlag = FLAG_CX);

		// dest by value so it's not modified
		// (void) <- dest - src
		// (doesn't set X flag)
		void CMPb(BYTE dest, BYTE src) { return SUBb(dest, src, FLAG_C); }
		void CMPw(WORD dest, WORD src) { return SUBw(dest, src, FLAG_C); }
		void CMPl(DWORD dest, DWORD src) { return SUBl(dest, src, FLAG_C); }

		void CMPMb() { NotImplementedOpcode("CMPM.b"); }
		void CMPMw() { NotImplementedOpcode("CMPM.w"); }
		void CMPMl() { NotImplementedOpcode("CMPM.l"); }

		void MULUw(DWORD& dest);

		friend class Monitor68000;
	};
}
