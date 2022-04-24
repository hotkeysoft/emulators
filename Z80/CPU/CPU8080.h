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

		typedef void (CPU::* OPCodeFunction)(BYTE);
		void AddOpcode(BYTE, OPCodeFunction);
		std::array<OPCodeFunction, 256> m_opcodesTable;

		void UnknownOpcode(BYTE);
		void DumpUnassignedOpcodes();

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

		cpuInfo::CPUInfo m_info;
		const cpuInfo::OpcodeTiming* m_currTiming = nullptr;

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

		BYTE& getRegL(BYTE opcode);
		BYTE& getRegR(BYTE opcode);

		ADDRESS getHL() { return MakeWord(regH, regL); };

		void jumpIF(bool condition);
		void callIF(bool condition);
		void retIF(bool condition);
		void push(BYTE h, BYTE l);
		void pop(BYTE& h, BYTE& l);
		void add(BYTE src, bool carry = false);
		void sub(BYTE src, bool borrow = false);
		void dad(WORD value);
		void cmp(BYTE src);

		void adjustParity(BYTE data);
		void adjustSign(BYTE data);
		void adjustZero(BYTE data);

		// Opcodes

		// Move, load and store
		void MOVrr(BYTE opcode);
		void MOVmr(BYTE opcode);
		void MOVrm(BYTE opcode);
		void MVIr(BYTE opcode);
		void MVIm(BYTE opcode);
		void LXIb(BYTE opcode);
		void LXId(BYTE opcode);
		void LXIh(BYTE opcode);
		void STAXb(BYTE opcode);
		void STAXd(BYTE opcode);
		void LDAXb(BYTE opcode);
		void LDAXd(BYTE opcode);
		void STA(BYTE opcode);
		void LDA(BYTE opcode);
		void SHLD(BYTE opcode);
		void LHLD(BYTE opcode);
		void XCHG(BYTE opcode);

		// Stack ops
		void PUSHb(BYTE opcode);
		void PUSHd(BYTE opcode);
		void PUSHh(BYTE opcode);
		void PUSHpsw(BYTE opcode);
		void POPb(BYTE opcode);
		void POPd(BYTE opcode);
		void POPh(BYTE opcode);
		void POPpsw(BYTE opcode);
		void XTHL(BYTE opcode);
		void SPHL(BYTE opcode);
		void LXIsp(BYTE opcode);
		void INXsp(BYTE opcode);
		void DCXsp(BYTE opcode);

		// Jump
		void JMP(BYTE opcode);
		void JC(BYTE opcode);
		void JNC(BYTE opcode);
		void JZ(BYTE opcode);
		void JNZ(BYTE opcode);
		void JP(BYTE opcode);
		void JM(BYTE opcode);
		void JPE(BYTE opcode);
		void JPO(BYTE opcode);
		void PCHL(BYTE opcode);

		// Call
		void CALL(BYTE opcode);
		void CC(BYTE opcode);
		void CNC(BYTE opcode);
		void CZ(BYTE opcode);
		void CNZ(BYTE opcode);
		void CP(BYTE opcode);
		void CM(BYTE opcode);
		void CPE(BYTE opcode);
		void CPO(BYTE opcode);

		// Return
		void RET(BYTE opcode);
		void RC(BYTE opcode);
		void RNC(BYTE opcode);
		void RZ(BYTE opcode);
		void RNZ(BYTE opcode);
		void RP(BYTE opcode);
		void RM(BYTE opcode);
		void RPE(BYTE opcode);
		void RPO(BYTE opcode);

		// Restart
		void RST(BYTE opcode);

		// Increment and Decrement
		void INRr(BYTE opcode);
		void DCRr(BYTE opcode);
		void INRm(BYTE opcode);
		void DCRm(BYTE opcode);
		void INXb(BYTE opcode);
		void INXd(BYTE opcode);
		void INXh(BYTE opcode);
		void DCXb(BYTE opcode);
		void DCXd(BYTE opcode);
		void DCXh(BYTE opcode);

		// Add
		void ADDr(BYTE opcode);
		void ADCr(BYTE opcode);
		void ADDm(BYTE opcode);
		void ADCm(BYTE opcode);
		void ADI(BYTE opcode);
		void ACI(BYTE opcode);
		void DADb(BYTE opcode);
		void DADd(BYTE opcode);
		void DADh(BYTE opcode);
		void DADsp(BYTE opcode);

		// Substract
		void SUBr(BYTE opcode);
		void SBBr(BYTE opcode);
		void SUBm(BYTE opcode);
		void SBBm(BYTE opcode);
		void SUI(BYTE opcode);
		void SBI(BYTE opcode);

		// Logical
		void ANAr(BYTE opcode);
		void XRAr(BYTE opcode);
		void ORAr(BYTE opcode);
		void CMPr(BYTE opcode);
		void ANAm(BYTE opcode);
		void XRAm(BYTE opcode);
		void ORAm(BYTE opcode);
		void CMPm(BYTE opcode);
		void ANI(BYTE opcode);
		void XRI(BYTE opcode);
		void ORI(BYTE opcode);
		void CPI(BYTE opcode);

		// Rotate
		void RLC(BYTE opcode);
		void RRC(BYTE opcode);
		void RAL(BYTE opcode);
		void RAR(BYTE opcode);

		// Specials
		void CMA(BYTE opcode);
		void STC(BYTE opcode);
		void CMC(BYTE opcode);
		void DAA(BYTE opcode);

		// Input/Output
		void IN(BYTE opcode);
		void OUT(BYTE opcode);

		// Control
		void EI(BYTE opcode);
		void DI(BYTE opcode);
		void NOP(BYTE opcode);
		void HLT(BYTE opcode);

		friend class Monitor8080;
	};
}
