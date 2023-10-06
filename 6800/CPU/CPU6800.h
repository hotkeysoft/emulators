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
	static const size_t CPU6800_ADDRESS_BITS = 16;
	static const char* CPUID_6800 = "6800";

	class CPU6800 : public CPU, public PortConnector
	{
	public:
		CPU6800(Memory& memory);
		virtual ~CPU6800();

		virtual void Init();

		void Dump();

		virtual void Reset();
		virtual void Reset(ADDRESS overrideAddress);

		virtual bool Step();

		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU6800_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		// Interrupts
		//void SetIRQ(bool irq) { m_irq = irq; }
		//void SetNMI(bool nmi) { m_nmi.Set(nmi); }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:

		CPU6800(const char* cpuid, Memory& memory);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		// Use third timing conditional penalty (2nd value not used)
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }
		inline void TICKPAGE() { CPU::TICK(1); } // Page crossing penalty
		inline void TICKINT() { CPU::TICK(m_info.GetMiscTiming(cpuInfo::MiscTiming::TRAP)[0]); }

		// Hardware vectors
		static constexpr ADDRESS ADDR_IRQ = 0xFFF8; // Hardware interrupt vector (IRQ)
		static constexpr ADDRESS ADDR_SWI = 0xFFFA; // Software interrupt vector (SWI)
		static constexpr ADDRESS ADDR_NMI = 0xFFFC; // Non-maskable interrupt vector (NMI)
		static constexpr ADDRESS ADDR_RESET = 0xFFFE; // Reset vector

		using OpcodeTable = std::vector<std::function<void()>>;
		OpcodeTable m_opcodes;
		void UnknownOpcode();

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		BYTE m_opcode = 0;

		// NMI is edge sensitive
		hscommon::EdgeDetectLatch m_nmi;

		// IRQ is level sensitive
		bool m_irq = false;

		virtual void Interrupt();

		enum FLAG : BYTE
		{
			_FLAG_R7 = 128, // 1
			_FLAG_R6 = 64,  // 1
			FLAG_H   = 32,  // 1 on half carry (4 bit)
			FLAG_I   = 16,  // 1 when IRQ line is ignored
			FLAG_N   = 8,   // 1 when result is negative
			FLAG_Z   = 4,   // 1 when result is 0
			FLAG_V   = 2,   // 1 on signed overflow
			FLAG_C   = 1    // 1 on unsigned overflow
		};

		FLAG FLAG_RESERVED_ON = FLAG(_FLAG_R6 | _FLAG_R7);

		ADDRESS m_programCounter = 0;

		// Alias when we need a WORD version of the program counter
		WORD& m_PC = *((WORD*)&m_programCounter);

		struct Registers
		{
			BYTE A; // Accumulator A
			BYTE B; // Accumulator B

			WORD IX; // Index Register
			WORD SP; // Stack Pointer

			BYTE flags = 0;
		} m_reg;

		void ClearFlags(BYTE& flags);
		void SetFlags(BYTE f);

		bool GetFlag(FLAG f) { return (m_reg.flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); };
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		// Misc helpers
		ADDRESS GetDirect() { return FetchByte(); }
		ADDRESS GetIndexed() { return m_reg.IX + FetchByte(); }
		ADDRESS GetExtended() { return FetchWord(); }

		virtual BYTE FetchByte() override;
		virtual WORD FetchWord() override;

		SBYTE FetchSignedByte() { return (SBYTE)FetchByte(); }
		SWORD FetchSignedWord() { return (SWORD)FetchWord(); }

		BYTE GetMemDirectByte() { return m_memory.Read8(GetDirect()); }
		WORD GetMemDirectWord() { return m_memory.Read16be(GetDirect()); }

		BYTE GetMemIndexedByte() { return m_memory.Read8(GetIndexed()); }
		WORD GetMemIndexedWord() { return m_memory.Read16be(GetIndexed()); }

		BYTE GetMemExtendedByte() { return m_memory.Read8(GetExtended()); }
		WORD GetMemExtendedWord() { return m_memory.Read16be(GetExtended()); }

		// Addressing Modes
		// ----------------

		// Adjust negative and zero flag
		void AdjustNZ(BYTE val);
		void AdjustNZ(WORD val);

		// raw push/pop
		void pushB(BYTE value);
		void pushW(WORD value);
		BYTE popB();
		WORD popW();

		void pushAll();
		void popAll();

		// Opcodes

		// Branching
		void BRA(bool condition);

		void JMP(ADDRESS dest) { m_programCounter = dest; }
		void JSR(ADDRESS dest);
		void RTS();

		// Load
		void LD8(BYTE& dest, BYTE src);
		void LD16(WORD& dest, WORD src);

		// Store
		void ST8(ADDRESS dest, BYTE src);
		void ST16(ADDRESS dest, WORD src);

		friend class Monitor6800;
	};
}
