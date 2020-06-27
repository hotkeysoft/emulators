#pragma once

#include "Common.h"
#include "Console.h"
#include "PortConnector.h"
#include "InterruptSource.h"

class UART : public PortConnector, public InterruptSource
{
public:
	enum Regs {
		RBR = 0, THR = 0,
		IER, IIR, LCR, MCR, LSR, MSR, SCR,
		DLL = 0, DLM = 1
	};

	UART(BYTE baseAddress, int baseClockSpeed = 2000000);

	bool IsEscape() { return m_currChar == 27; }

	void Init();
	void Reset();

	// Inherited via InterruptSource
	virtual bool IsInterrupting() override;

	// Various registers

	void NOP_OUT(BYTE value);
	BYTE NOP_IN();
	
	// RXTX, can forward to Divisor Latch depending on flag
	BYTE RBR_IN();
	void THR_OUT(BYTE value);

	// Interrupt Enable Register
	BYTE IER_IN();
	void IER_OUT(BYTE value);

	// Line Control Register
	BYTE LCR_IN();
	void LCR_OUT(BYTE value);

	// MODEM Control Register
	BYTE MCR_IN();
	void MCR_OUT(BYTE value);

	// Line Status Register
	BYTE LSR_IN();

	// Divisor Latch
	BYTE DLL_IN();
	void DLL_OUT(BYTE value);

	BYTE DLM_IN();
	void DLM_OUT(BYTE value);

private:
	UART();

	void printDivisor();

	Console m_console;
	BYTE m_currChar;

	// Internal Registers
	BYTE m_ier;
	BYTE m_iir;
	BYTE m_lcr;
	BYTE m_mcr;
	BYTE m_lsr;
	BYTE m_msr;

	BYTE m_dll;
	BYTE m_dlm;

	bool m_dlab;

	const int m_baseClockSpeed;
	const BYTE m_baseAddress;
};
