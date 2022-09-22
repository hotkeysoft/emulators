#pragma once

#include <Serializable.h>
#include <CPU/CPU.h>
#include <CPU/PortConnector.h>
#include <CPU/CPUInfo.h>

#undef IN
#undef OUT

namespace emul
{
	static const size_t CPU6502_ADDRESS_BITS = 16;

	class CPU6502 : public CPU, public Serializable, public PortConnector
	{
	public:
		CPU6502(Memory& memory);
		virtual ~CPU6502();

		virtual void Init();

		void Dump();

		virtual void Reset();

		virtual bool Step();

		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return m_info.GetId(); };
		virtual size_t GetAddressBits() const override { return CPU6502_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }

		// Pseudo-control lines

		// Set after an IO Operation (in/out port)
		bool IsIORequest() const { return m_ioRequest; }

		// Set after servicing an interrupt
		bool IsInterruptAcknowledge() const { return m_interruptAcknowledge; }

		// emul::Serializable
		virtual void Serialize(json& to) {} // TODO
		virtual void Deserialize(const json& from) {} // TODO

	protected:

		CPU6502(const char* cpuid, Memory& memory);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		// Use third timing conditional penalty (2nd value not used)
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }
		inline void TICKMISC(cpuInfo::MiscTiming misc) { CPU::TICK(m_info.GetMiscTiming(misc)[0]); }

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

		bool m_ioRequest = false;

		bool m_interruptAcknowledge = false;
		bool m_interruptsEnabled = false;
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

		BYTE dummy = 0;

		virtual BYTE FetchByte() override;

		//BYTE ReadMem() const { return m_memory.Read8(GetHL()); }
		//void WriteMem(BYTE value) { m_memory.Write8(GetHL(), value); }

		virtual void AdjustBaseFlags(BYTE val);

		// Opcodes

		friend class Monitor6502;
	};
}
