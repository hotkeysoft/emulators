#pragma once

#include "CPU.h"
#include "PortConnector.h"
#include "PortAggregator.h"
#include "Interrupts.h"

class CPU8080 : public CPU  
{
public:
	enum InterruptSource {TRAP = 0, RST75, RST65, RST55 /*TODO INTR/INTA not implemented*/ };
	
	CPU8080(Memory &memory, MemoryMap &mmap, Interrupts &interrupts);
	virtual ~CPU8080();

	void AddDevice(PortConnector& ports);

	void Dump();

	virtual void Reset();

	virtual bool Step();

protected:
	PortAggregator m_ports;
	Interrupts &m_interrupts;

	enum FLAG {S_FLAG=128, Z_FLAG=64, AC_FLAG=16, P_FLAG=4, CY_FLAG=1};

	bool m_interruptsEnabled;

	BYTE regA;
	BYTE regB, regC;
	BYTE regD, regE;
	BYTE regH, regL;
	WORD regSP;

	BYTE flags;

	bool getFlag(FLAG f) { return (flags&f)?true:false; };
	void setFlag(FLAG f, bool v) { if (v) flags|=f; else flags&=~f; };

	BYTE dummy;

	// Helper functions
	void interrupt(InterruptSource source);
	
	BYTE &getRegL(BYTE opcode);
	BYTE &getRegR(BYTE opcode);

	WORD getHL() { return getWord(regH, regL); };

	void jumpIF(bool condition, int timeT);
	void callIF(bool condition);
	void retIF(bool condition, int timeT);
	void push(BYTE h, BYTE l);
	void pop(BYTE &h, BYTE &l);
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
};

