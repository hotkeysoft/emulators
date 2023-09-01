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

		enum class SubOpcodeGroup {
			b0000 = 0,
			b0100,
			b0101,
			b0110,
			b1000,
			b1001,
			b1011,
			b1100,
			b1101,
			misc,
			_MAX,
		};

		void InitGroupB0000(OpcodeTable& table, size_t size);
		void InitGroupB0100(OpcodeTable& table, size_t size);
		void InitGroupB0101(OpcodeTable& table, size_t size);
		void InitGroupB0110(OpcodeTable& table, size_t size);
		void InitGroupB1000(OpcodeTable& table, size_t size);
		void InitGroupB1001(OpcodeTable& table, size_t size);
		void InitGroupB1011(OpcodeTable& table, size_t size);
		void InitGroupB1100(OpcodeTable& table, size_t size);
		void InitGroupB1101(OpcodeTable& table, size_t size);
		void InitGroupMisc(OpcodeTable& table, size_t size);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		inline void TICKn(int t) { m_opTicks += t; }

		// Hardware vectors
		enum class VECTOR : BYTE
		{
			ResetSSP = 0, // Reset, Initial SSP (Supervisor Stack Pointer)
			ResetPC = 1, // Reset, Initial PC (Program Counter)
			BusError = 2,
			AddressError = 3,
			IllegalInstruction = 4,
			ZeroDivide = 5,
			CHK_Instruction = 6,
			TRAPV_Instruction = 7,
			PrivilegeViolation = 8,
			Trace = 9,
			Line1010Emulator = 10,
			Line1111Emulator = 11,

			// (12-14 Reserved)

			UninitializedIntVector = 15,

			// (16-23 Reserved)

			SpuriousInterrupt = 24,

			// 25-31 Level 1-7 Interrupt Autovector
			InterruptBase = 25,

			// 32-47 TRAP Instructions vectors (32 + n)
			TrapBase = 32,

			// 48-63 Reserved
			// 64-255 User Interrupt Vectors
			USER_INT_BASE = 64
		};

		ADDRESS GetVectorAddress(VECTOR v) { return (ADDRESS)v * 4; }
		ADDRESS GetIntVectorAddress(BYTE i) { assert(i <= 7); return ((ADDRESS)VECTOR::InterruptBase + i) * 4; }
		ADDRESS GetTrapVectorAddress(BYTE t) { assert(t <= 16);  return ((ADDRESS)VECTOR::TrapBase + t) * 4; }

		OpcodeTable m_opcodes;
		OpcodeTable m_subOpcodes[(int)SubOpcodeGroup::_MAX];
		[[noreturn]] void IllegalInstruction();
		[[noreturn]] void NotImplementedOpcode(const char* name);

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		WORD m_opcode = 0;
		int GetOpcodeRegisterIndex() const { return m_opcode & 0b111; }

		WORD GetSubopcode6() const { return ((m_opcode >> 6) & 63); }
		WORD GetSubopcodeLow6() const { return m_opcode & 63; }
		WORD GetSubopcode4() const { return ((m_opcode >> 8) & 15); }

		virtual void Interrupt();

		enum FLAG : WORD
		{
			// Supervisor mode flags (R/O in user mode)
			FLAG_T = 0x8000, // 1 Trace mode
			FLAG_R14 = 0x4000, // 0
			FLAG_S = 0x2000, // 1 Supervisor mode
			_FLAG_R12 = 0x1000, // 0
			_FLAG_R11 = 0x0800, // 0
			FLAG_I2 = 0x0400, // I[0..2], interrupt mask
			FLAG_I1 = 0x0200, // ""
			FLAG_I0 = 0x0100, // ""

			// User mode R/W
			_FLAG_R7 = 0x0080, // 0
			_FLAG_R6 = 0x0040, // 0
			_FLAG_R5 = 0x0020, // 0
			FLAG_X = 0x0010, // 1 eXtend
			FLAG_N = 0x0008, // 1 Negative
			FLAG_Z = 0x0004, // 1 Zero
			FLAG_V = 0x0002, // 1 Signed oVerflow
			FLAG_C = 0x0001,  // 1 Carry (unsigned overflow)

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
		bool FlagGE() const {
			return
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

			template <typename SIZE> SIZE& GetDATA(int index);
			template <> BYTE& GetDATA(int index) { return BYTE_REG(DATA, index); }
			template <> WORD& GetDATA(int index) { return WORD_REG(DATA, index); }
			template <> DWORD& GetDATA(int index) { return DATA[index]; }

			template <typename SIZE> SIZE& GetADDR(int index);
			template <> BYTE& GetADDR(int index) { return BYTE_REG(ADDR, index); }
			template <> WORD& GetADDR(int index) { return WORD_REG(ADDR, index); }
			template <> DWORD& GetADDR(int index) { return ADDR[index]; }

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

		template <typename SIZE> SIZE Fetch();
		template<> BYTE Fetch() { return FetchByte(); }
		template<> WORD Fetch() { return FetchWord(); }
		template<> DWORD Fetch() { return FetchLong(); }

		virtual BYTE FetchByte() override { return GetLByte(FetchWord()); }
		virtual WORD FetchWord() override;
		DWORD FetchLong();

		static bool IsWordAligned(ADDRESS addr) { return !GetLSB(addr); }

		template<typename SIZE> void Write(ADDRESS src, SIZE value);
		template<> void Write(ADDRESS dest, BYTE value) { m_memory.Write8(dest, value); }
		template<> void Write(ADDRESS dest, WORD value) { Aligned(dest); m_memory.Write16be(dest, value); }
		template<> void Write(ADDRESS dest, DWORD value) { Aligned(dest); m_memory.Write32be(dest, value); }

		template<typename SIZE> SIZE Read(ADDRESS src);
		template<> BYTE Read(ADDRESS src) { return m_memory.Read8(src); }
		template<> WORD Read(ADDRESS src) { Aligned(src); return m_memory.Read16be(src); }
		template<> DWORD Read(ADDRESS src) { Aligned(src); return m_memory.Read32be(src); }

		void Exec(WORD opcode);
		void Exec(SubOpcodeGroup group, WORD subOpcode);
		bool InternalStep();

		// Adjust negative and zero flag
		template <typename SIZE>
		void AdjustNZ(SIZE val)
		{
			SetFlag(FLAG_N, GetMSB(val));
			SetFlag(FLAG_Z, val == 0);
		}

		EAMode m_eaMode = EAMode::Invalid;
		static EAMode GetEAMode(WORD opcode);
		// Needs m_eaMode to be set
		void EACheck(EAMode group) { if (!((WORD)m_eaMode & (WORD)group)) IllegalInstruction(); }

		template<typename SIZE> SIZE GetEAValue(EAMode groupCheck);

		ADDRESS GetExtensionWordDisp();

		template<typename SIZE> ADDRESS rawGetEA();

		template<typename SIZE>
		ADDRESS GetEA(EAMode groupCheck)
		{
			m_eaMode = GetEAMode(m_opcode);
			EACheck(groupCheck);

			return rawGetEA<SIZE>();
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

		void BSR();
		void JSR();
		void RTS();
		void RTE();

		// Stack
		void PUSHl(DWORD src);
		void PUSHw(WORD src);

		DWORD POPl();
		WORD POPw();

		void PEAl();

		void LINKw(DWORD& dest);
		void UNLK(DWORD& dest);

		// MOVE
		template<typename SIZE> void MOVE();
		template<typename SIZE> void MOVEA();

		void MOVEQ();

		void MOVEwToSR(WORD src);
		void MOVEwFromSR();

		template<typename SIZE> void MOVEMToEA(WORD regs);
		template<typename SIZE> void MOVEMFromEA(WORD regs);

		void MOVEPwToReg(WORD& dest) { NotImplementedOpcode("MOVEP.w (<ea> -> reg)"); }
		void MOVEPlToReg(DWORD& dest) { NotImplementedOpcode("MOVEP.l (<ea> -> reg)"); }

		void MOVEPwFromReg(WORD src);
		void MOVEPlFromReg(DWORD src) { NotImplementedOpcode("MOVEP.l (reg -> <ea>)"); }

		void EXGl();

		// Bit, Logic
		void Sccb(bool cond);

		enum class BitOp { SET, CLEAR, CHANGE };
		void BitOps(BYTE bitNumber, BitOp bitop);
		void BitTst(BYTE bitNumber);

		// Imm source
		void BTSTimm() { BitTst(FetchByte()); }
		void BCHGimm() { BitOps(FetchByte(), BitOp::CHANGE); }
		void BCLRimm() { BitOps(FetchByte(), BitOp::CLEAR); }
		void BSETimm() { BitOps(FetchByte(), BitOp::SET); }

		// Reg source
		void BTST(BYTE src) { BitTst(src); }
		void BCHG(BYTE src) { BitOps(src, BitOp::CHANGE); }
		void BCLR(BYTE src) { BitOps(src, BitOp::CLEAR); }
		void BSET(BYTE src) { BitOps(src, BitOp::SET);; }

		template<typename SIZE> void NOT();
		template<typename SIZE> void TST();

		void ANDIbToCCR() { NotImplementedOpcode("ANDI.b #imm, CCR"); }
		void ANDIwToSR();

		template<typename SIZE> void ANDI();
		template<typename SIZE> void AND(SIZE& dest, SIZE src);
		template<typename SIZE> void ANDToEA(SIZE src);

		void ORIbToCCR();
		void ORIwToSR();

		template<typename SIZE> void ORI();
		template<typename SIZE> void OR(SIZE& dest, SIZE src);
		template<typename SIZE> void ORToEA(SIZE src);

		void EORIbToCCR() { NotImplementedOpcode("EORI.b #imm, CCR"); }
		void EORIwToSR() { NotImplementedOpcode("EORI.w #imm, SR"); }

		template<typename SIZE> void EORI();
		template<typename SIZE> void EOR(SIZE& dest, SIZE src);
		template<typename SIZE> void EORToEA(SIZE src);

		void SHIFT();

		template<typename SIZE> void SHIFT(SIZE& dest, int count, bool left, int operation);

		void SHIFTb(BYTE& dest, int count, bool left, int operation);
		void SHIFTw(WORD& dest, int count, bool left, int operation);
		void SHIFTl(DWORD& dest, int count, bool left, int operation);

		void ASLw() { NotImplementedOpcode("ADL.w <ea>"); }
		void ASRw() { NotImplementedOpcode("ASR.w <ea>"); }
		void LSLw() { NotImplementedOpcode("LSL.w <ea>"); }
		void LSRw() { NotImplementedOpcode("LSR.w <ea>"); }
		void ROXLw() { NotImplementedOpcode("ROXL.w <ea>"); }
		void ROXRw() { NotImplementedOpcode("ROXR.w <ea>"); }
		void ROLw() { NotImplementedOpcode("ROL.w <ea>"); }
		void RORw() { NotImplementedOpcode("ROR.w <ea>"); }

		template<typename SIZE> void ASL(SIZE& dest, int count);
		template<typename SIZE> void ASR(SIZE& dest, int count);

		template<typename SIZE> void LSL(SIZE& dest, int count);
		template<typename SIZE> void LSR(SIZE& dest, int count);

		template<typename SIZE> void ROL(SIZE& dest, int count);
		template<typename SIZE> void ROR(SIZE& dest, int count);

		template<typename SIZE> void ROXL(SIZE& dest, int count);
		template<typename SIZE> void ROXR(SIZE& dest, int count);

		// Arithmetic

		template<class SIZE> void CLR();

		void SWAPw();

		void EXTw();
		void EXTl();

		void ABCDb() { NotImplementedOpcode("ABCD.b"); }
		void SBCDb() { NotImplementedOpcode("SBCD.b"); }

		void ADDXb() { NotImplementedOpcode("ADDX.b"); }
		void ADDXw() { NotImplementedOpcode("ADDX.w"); }
		void ADDXl();

		void SUBXb() { NotImplementedOpcode("SUBX.b"); }
		void SUBXw() { NotImplementedOpcode("SUBX.w"); }
		void SUBXl();

		template<typename SIZE> void ADDQ(SIZE imm);
		template<typename SIZE> void SUBQ(SIZE imm);

		void ADDA(WORD& dest, WORD src) { dest += Widen(src); }
		void ADDA(DWORD& dest, DWORD src) { dest += src; }

		void SUBA(WORD& dest, WORD src) { dest -= Widen(src); }
		void SUBA(DWORD& dest, DWORD src) { dest -= src; }

		// dest' <- dest + src
		template<typename SIZE> void ADD(SIZE& dest, SIZE src, bool carry = false);
		template<typename SIZE> void ADDI();
		template<typename SIZE> void ADDToEA(SIZE src);

		// dest' <- dest - src
		template<typename SIZE> void SUB(SIZE& dest, SIZE src, FLAG carryFlag = FLAG_CX, bool borrow = false);
		template<typename SIZE> void SUBI();
		template<typename SIZE> void SUBToEA(SIZE src);

		// dest by value so it's not modified
		// (void) <- dest - src
		// (doesn't set X flag)
		template<typename SIZE> void CMP(SIZE dest, SIZE src) { return SUB<SIZE>(dest, src, FLAG_C); }

		template<typename SIZE> void CMPM();

		template<typename SIZE> void NEG();

		void MULUw(DWORD& dest);
		void DIVUw(DWORD& dest);
		void DIVSw(DWORD& dest);

		friend class Monitor68000;
	};
}
