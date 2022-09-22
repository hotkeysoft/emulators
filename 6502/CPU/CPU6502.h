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
			FLAG_S		= 128,
			FLAG_Z		= 64,
			_FLAG_R5	= 32,
			FLAG_AC		= 16,
			_FLAG_R3	= 8,
			FLAG_P		= 4,
			_FLAG_R1	= 2,
			FLAG_CY		= 1
		};

		FLAG FLAG_RESERVED_ON = FLAG(_FLAG_R1);
		FLAG FLAG_RESERVED_OFF = FLAG(_FLAG_R3 | _FLAG_R5);

		ADDRESS m_programCounter = 0;

		struct Registers
		{
			BYTE A = 0;
			BYTE flags = 0;

			BYTE B = 0;
			BYTE C = 0;

			BYTE D = 0;
			BYTE E = 0;

			BYTE H = 0;
			BYTE L = 0;
		} m_reg;

		WORD m_regSP = 0;

		void ClearFlags(BYTE& flags);
		void SetFlags(BYTE f);

		bool GetFlag(FLAG f) { return (m_reg.flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(m_reg.flags, f, v); };
		void ComplementFlag(FLAG f) { m_reg.flags ^= f; }

		BYTE dummy = 0;

		virtual BYTE FetchByte() override;

		WORD GetBC() const { return MakeWord(m_reg.B, m_reg.C); };
		WORD GetDE() const { return MakeWord(m_reg.D, m_reg.E); };
		WORD GetHL() const { return MakeWord(m_reg.H, m_reg.L); };

		void SetBC(WORD val) { m_reg.B = GetHByte(val); m_reg.C = GetLByte(val); }
		void SetDE(WORD val) { m_reg.D = GetHByte(val); m_reg.E = GetLByte(val); }
		void SetHL(WORD val) { m_reg.H = GetHByte(val); m_reg.L = GetLByte(val); }

		BYTE ReadMem() const { return m_memory.Read8(GetHL()); }
		void WriteMem(BYTE value) { m_memory.Write8(GetHL(), value); }

		virtual void AdjustBaseFlags(BYTE val);

		// Opcodes

		friend class Monitor6502;
	};
}
