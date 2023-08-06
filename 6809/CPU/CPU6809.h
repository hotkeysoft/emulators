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
	static const size_t CPU6809_ADDRESS_BITS = 16;
	static const char* CPUID_6809 = "6809";

	class CPU6809 : public CPU, public PortConnector
	{
	public:
		CPU6809(Memory& memory);
		virtual ~CPU6809();

		virtual void Init();

		void Dump();

		virtual void Reset();
		virtual void Reset(ADDRESS overrideAddress);

		virtual bool Step();

		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU6809_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		// Interrupts
		void SetNMI(bool nmi) { m_nmi.Set(nmi); }
		void SetFIRQ(bool firq) { m_firq = firq; }
		void SetIRQ(bool irq) { m_irq = irq; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:

		CPU6809(const char* cpuid, Memory& memory);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		// Use third timing conditional penalty (2nd value not used)
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }
		inline void TICKINT() { CPU::TICK(m_info.GetMiscTiming(cpuInfo::MiscTiming::TRAP)[0]); }

		// Vectors
		static const ADDRESS _ADDR_RSV  = 0xFFF0; // Reserved by Motorola
		static const ADDRESS ADDR_SWI3  = 0xFFF2; // SWI3 instruction interrupt vector
		static const ADDRESS ADDR_SWI2  = 0xFFF4; // SWI2 instruction interrupt vector
		static const ADDRESS ADDR_FIRQ  = 0xFFF6; // Fast hardware interrupt vector (FIRQ)
		static const ADDRESS ADDR_IRQ   = 0xFFF8; // Hardware interrupt vector (IRQ)
		static const ADDRESS ADDR_SWI   = 0xFFFA; // SWI instruction interrupt vector
		static const ADDRESS ADDR_NMI   = 0xFFFC; // Non-maskable interrupt vector (NMI)
		static const ADDRESS ADDR_RESET = 0xFFFE; // Reset vector

		using OpcodeTable = std::vector<std::function<void()>>;
		OpcodeTable m_opcodes;
		void UnknownOpcode();

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		BYTE m_opcode = 0;

		// NMI is edge sensitive
		hscommon::EdgeDetectLatch m_nmi;

		// NMI is disabled on Reset until the S stack pointer is set
		bool m_nmiEnabled = false;

		// IRQ and FIRQ are level sensitive
		bool m_firq = false;
		bool m_irq = false;

		virtual void Interrupt();

		enum FLAG : BYTE
		{
			FLAG_E		= 128, // 1 when Entire machine state was stacked
			FLAG_F		= 64,  // 1 with FIRQ line is ignored
			FLAG_H		= 32,  // 1 on half carry (4 bit)
			FLAG_I		= 16,  // 1 when IRQ line is ignored
			FLAG_N		= 8,   // 1 when result is negative
			FLAG_Z		= 4,   // 1 when result is 0
			FLAG_V		= 2,   // 1 on signed overflow
			FLAG_C		= 1    // 1 on unsigned overflow
		};

		ADDRESS m_programCounter = 0;

		struct Registers
		{
#pragma pack(push, 1)
			union
			{
				WORD D = 0;	// Accumulators
				struct {
					BYTE A;
					BYTE B;
				} ab;
			};
#pragma pack(pop)

			WORD S = 0; // Hardware Stack Pointer
			WORD U = 0; // User Stack Pointer
			WORD X = 0; // Index Register
			WORD Y = 0; // Index Register

			BYTE DP = 0; // Direct Page Register
			BYTE flags = 0;

		} m_reg;

		void ClearFlags(BYTE& flags) { flags = 0; }
		void SetFlags(BYTE f) { m_reg.flags = f; }

		bool GetFlag(FLAG f) { return (m_reg.flags & f) ? true : false; }
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); }
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		virtual BYTE FetchByte() override;

		// Addressing Modes
		// ----------------


		// Opcodes


		friend class Monitor6809;
	};
}
