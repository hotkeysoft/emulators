#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>
#include <EdgeDetectLatch.h>

#undef IN
#undef OUT

namespace emul
{
	static const size_t CPU6502_ADDRESS_BITS = 16;
	static const char* CPUID_6502 = "6502";

	class CPU6502 : public CPU, public PortConnector
	{
	public:
		CPU6502(Memory& memory);
		virtual ~CPU6502();

		virtual void Init();

		void Dump();

		virtual void Reset();
		virtual void Reset(ADDRESS overrideAddress);

		virtual bool Step();

		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU6502_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		// Interrupts
		void SetIRQ(bool irq) { m_irq = irq; }
		void SetNMI(bool nmi) { m_nmi.Set(nmi); }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:

		CPU6502(const char* cpuid, Memory& memory);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		// Use third timing conditional penalty (2nd value not used)
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }
		inline void TICKPAGE() { CPU::TICK(1); } // Page crossing penalty
		inline void TICKINT() { CPU::TICK(m_info.GetMiscTiming(cpuInfo::MiscTiming::TRAP)[0]); }

		// Hardware vectors
		const ADDRESS ADDR_NMI = 0xFFFA;
		const ADDRESS ADDR_RESET = 0xFFFC;
		const ADDRESS ADDR_IRQ = 0xFFFE;

		using OpcodeTable = std::vector<std::function<void()>>;
		OpcodeTable m_opcodes;
		void UnknownOpcode();

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		BYTE m_opcode = 0;

		// IRQ is level sensitive
		bool m_irq = false;
		// NMI is edge sensitive
		hscommon::EdgeDetectLatch m_nmi;
		virtual void Interrupt();

		enum FLAG : BYTE
		{
			FLAG_N		= 128, // 1 when result is negative
			FLAG_V		= 64,  // 1 on signed overflow
			_FLAG_R5	= 32,  // 1
			FLAG_B		= 16,  // 1 when interrupt was caused by a BRK
			FLAG_D		= 8,   // 1 when CPU is in BCD mode
			FLAG_I		= 4,   // 1 when IRQ is disabled
			FLAG_Z		= 2,   // 1 when result is 0
			FLAG_C		= 1    // 1 on unsigned overflow
		};

		FLAG FLAG_RESERVED_ON = FLAG(_FLAG_R5);

		ADDRESS m_programCounter = 0;

		struct Registers
		{
			BYTE A = 0;
			BYTE flags = 0;

			BYTE X = 0;
			BYTE Y = 0;

			BYTE SP = 0;
		} m_reg;

		void ClearFlags(BYTE& flags);
		void SetFlags(BYTE f);

		bool GetFlag(FLAG f) { return (m_reg.flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); };
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		WORD GetSP() const { return m_reg.SP | 0x0100; }

		BYTE GetPage(ADDRESS addr) const { return emul::GetHByte(addr); }

		// Memory operation Read-Modify-Write
		void MEMopRMW(std::function<void(CPU6502*, BYTE&)> func, ADDRESS dest);

		// Memory operation Read
		void MEMopR(std::function<void(CPU6502*, BYTE)> func, ADDRESS oper);

		virtual BYTE FetchByte() override;

		// Addressing Modes
		// ----------------

		// Zero Page
		ADDRESS GetZP() { return FetchByte(); }
		ADDRESS GetZPX() { return (BYTE)(FetchByte() + m_reg.X); }
		ADDRESS GetZPY() { return (BYTE)(FetchByte() + m_reg.Y); }

		// Absolute
		ADDRESS GetABS() { return FetchWord(); }
		// TODO: 1 cycle penalty for boundary crossing
		ADDRESS GetABSX() { return (WORD)(FetchWord() + m_reg.X); }
		ADDRESS GetABSY() { return (WORD)(FetchWord() + m_reg.Y); }

		// Indirect
		ADDRESS GetIND();
		ADDRESS GetINDX();
		ADDRESS GetINDY();

		void AdjustNZ(BYTE val);

		// Opcodes
		void PUSH(BYTE val);
		BYTE POP();

		void LD(BYTE& dest, BYTE src);
		void LDmem(BYTE& dest, ADDRESS src);
		void STmem(ADDRESS dest, BYTE src);

		void PLA();
		void PHA();
		void PLP();
		void PHP();

		void BRANCHif(bool cond);
		void JSR(ADDRESS dest);
		void RTS();
		void JMP(ADDRESS dest);
		void BIT(ADDRESS src);
		void BRK();
		void RTI();

		void ORA(BYTE oper);
		void AND(BYTE oper);
		void EOR(BYTE oper);
		BYTE addSubBinary(BYTE oper, bool carry);
		BYTE addBCD(BYTE oper, bool carry);
		BYTE subBCD(BYTE oper, bool carry);
		void ADC(BYTE oper);
		void SBC(BYTE oper);

		void cmp(BYTE reg, BYTE oper);
		void CMPA(BYTE oper) { cmp(m_reg.A, oper); }
		void CMPX(BYTE oper) { cmp(m_reg.X, oper); }
		void CMPY(BYTE oper) { cmp(m_reg.Y, oper); }

		void INC(BYTE& dest);
		void DEC(BYTE& dest);
		void ASL(BYTE& dest);
		void ROL(BYTE& dest);
		void LSR(BYTE& dest);
		void ROR(BYTE& dest);

		void SLO(BYTE& dest);
		void ISC(BYTE& dest);

		friend class Monitor6502;
	};
}
