#pragma once

#include "CPU.h"
#include "../Serializable.h"
#include "PortConnector.h"
#include "CPUInfo.h"
#include "Interrupts.h"

#undef IN
#undef OUT

namespace emul
{
	static const size_t CPU8080_ADDRESS_BITS = 16;

	class CPU8080 : public CPU, public Serializable, public PortConnector
	{
	public:
		enum InterruptSource { TRAP = 0, RST75, RST65, RST55 /*TODO INTR/INTA not implemented*/ };

		CPU8080(Memory& memory, Interrupts& interrupts);
		virtual ~CPU8080();

		virtual void Init();

		void Dump();

		virtual void Reset();

		virtual bool Step();

		virtual void Exec(BYTE opcode) override;

		virtual const std::string GetID() const override { return "8080"; };
		virtual size_t GetAddressBits() const override { return CPU8080_ADDRESS_BITS; };
		virtual ADDRESS GetCurrentAddress() const override { return m_programCounter; }

		const cpuInfo::CPUInfo& GetInfo() const { return m_info; }


		// emul::Serializable
		virtual void Serialize(json& to) {} // TODO
		virtual void Deserialize(const json& from) {} // TODO

	protected:
		CPU8080(cpuInfo::CPUType type, Memory& memory, Interrupts& interrupts);

		inline void TICK() { m_opTicks += (*m_currTiming)[(int)cpuInfo::OpcodeTimingType::BASE]; };
		// Use third timing conditional penalty (2nd value not used)
		inline void TICKT3() { CPU::TICK((*m_currTiming)[(int)cpuInfo::OpcodeTimingType::T3]); }

		std::vector<std::function<void()>> m_opcodes;
		void UnknownOpcode();

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;
		BYTE m_opcode = 0;

		Interrupts& m_interrupts;
		bool m_interruptsEnabled = false;

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

		BYTE regA = 0;
		BYTE regB = 0;
		BYTE regC = 0;
		BYTE regD = 0; 
		BYTE regE = 0;
		BYTE regH = 0; 
		BYTE regL = 0;
		WORD regSP = 0;

		BYTE flags = 0;

		void ClearFlags();
		void SetFlags(BYTE f);

		bool GetFlag(FLAG f) { return (flags & f) ? true : false; };
		void SetFlag(FLAG f, bool v) { SetBitMask(flags, f, v); };

		BYTE dummy = 0;

		// Helper functions
		BYTE FetchByte();
		WORD FetchWord();

		void interrupt(InterruptSource source);

		ADDRESS GetHL() const { return MakeWord(regH, regL); };
		BYTE ReadMem() const { return m_memory.Read8(GetHL()); }
		void WriteMem(BYTE value) { m_memory.Write8(GetHL(), value); }

		void jumpIF(bool condition);
		void callIF(bool condition);
		void retIF(bool condition);
		void push(BYTE h, BYTE l);
		void pop(BYTE& h, BYTE& l);
		void add(BYTE src, bool carry = false);
		void sub(BYTE src, bool borrow = false);
		void dad(WORD value);
		void ana(BYTE src);
		void xra(BYTE src);
		void ora(BYTE src);
		void cmp(BYTE src);

		void adjustParity(BYTE data);
		void adjustSign(BYTE data);
		void adjustZero(BYTE data);

		// Opcodes

		// Move, load and store
		void LXIb();
		void LXId();
		void LXIh();
		void STAXb();
		void STAXd();
		void LDAXb();
		void LDAXd();
		void STA();
		void LDA();
		void SHLD();
		void LHLD();
		void XCHG();

		// Stack ops
		void XTHL();
		void SPHL();
		void LXIsp();

		// Restart
		void RST(BYTE rst);

		// Increment and Decrement
		void INRr(BYTE& reg);
		void DCRr(BYTE& reg);
		void INRm();
		void DCRm();
		void INX(BYTE& h, BYTE& l);
		void DCX(BYTE& h, BYTE& l);

		// Rotate
		void RLC();
		void RRC();
		void RAL();
		void RAR();

		// Specials
		void DAA();

		// Control
		void EI();
		void NOP();
		void HLT();

		friend class Monitor8080;
	};
}
