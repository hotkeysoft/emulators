#include "UART.h"
#include <conio.h>
#include <fstream>

UART::UART(BYTE baseAddress, int baseClockSpeed) : Logger("UART"), m_baseAddress(baseAddress), m_baseClockSpeed(baseClockSpeed),
													m_currChar(0) 
{
	Reset();
}

void UART::Init()
{
	Connect(m_baseAddress + RBR, static_cast<PortConnector::INFunction>(&UART::RBR_IN));
	Connect(m_baseAddress + THR, static_cast<PortConnector::OUTFunction>(&UART::THR_OUT));

	Connect(m_baseAddress + IER, static_cast<PortConnector::INFunction>(&UART::IER_IN));
	Connect(m_baseAddress + IER, static_cast<PortConnector::OUTFunction>(&UART::IER_OUT));

	Connect(m_baseAddress + IIR, static_cast<PortConnector::INFunction>(&UART::NOP_IN));

	Connect(m_baseAddress + LCR, static_cast<PortConnector::INFunction>(&UART::LCR_IN));
	Connect(m_baseAddress + LCR, static_cast<PortConnector::OUTFunction>(&UART::LCR_OUT));

	Connect(m_baseAddress + MCR, static_cast<PortConnector::INFunction>(&UART::MCR_IN));
	Connect(m_baseAddress + MCR, static_cast<PortConnector::OUTFunction>(&UART::MCR_OUT));

	Connect(m_baseAddress + LSR, static_cast<PortConnector::INFunction>(&UART::LSR_IN));
	Connect(m_baseAddress + LSR, static_cast<PortConnector::OUTFunction>(&UART::NOP_OUT));

	Connect(m_baseAddress + MSR, static_cast<PortConnector::INFunction>(&UART::NOP_IN));
	Connect(m_baseAddress + MSR, static_cast<PortConnector::OUTFunction>(&UART::NOP_OUT));

	Connect(m_baseAddress + SCR, static_cast<PortConnector::INFunction>(&UART::NOP_IN));
	Connect(m_baseAddress + SCR, static_cast<PortConnector::OUTFunction>(&UART::NOP_OUT));
}

void UART::Reset() 
{
	m_ier = 0x00;
	m_iir = 0x01;
	m_lcr = 0x00;
	m_mcr = 0x00;
	m_lsr = 0x60;
	m_msr = 0x00;

	m_dll = 0x00;
	m_dlm = 0x00;

	m_dlab = false;
}

bool UART::IsInterrupting()
{
	return _kbhit();
}

void UART::NOP_OUT(BYTE value) 
{ 
}

BYTE UART::NOP_IN() 
{ 
	return 0; 
}

BYTE UART::LSR_IN()
{
	// Always ready
	return 0x20;
}

// Read a character from the keyboard
BYTE UART::RBR_IN()
{
	// Divisor Latch mode
	if (m_dlab) 
	{
		return DLL_IN();
	}

	if (_kbhit() == 0)
	{
		return 0;
	}
	else
	{
		m_currChar = _getch();
		while (_kbhit()) _getch();
		// echo it back
		THR_OUT(m_currChar);
		return m_currChar;
	}
}

// Outputs a character to the console
void UART::THR_OUT(BYTE value)
{
	// Divisor Latch mode
	if (m_dlab)
	{
		DLL_OUT(value);
		return;
	}

	m_console.OutChar(value);
}

BYTE UART::IER_IN()
{
	// Divisor Latch mode
	if (m_dlab)
	{
		return DLM_IN();
	}

	return m_ier;
}
void UART::IER_OUT(BYTE value)
{
	// Divisor Latch mode
	if (m_dlab)
	{
		DLM_OUT(value);
		return;
	}

	// TODO, implement
	m_ier = value;
}

BYTE UART::LCR_IN()
{
	return m_lcr;
}

void UART::LCR_OUT(BYTE value)
{
	static const char* stop[] = { "1", "1.5", "2" };
	static const char* parity[] = { "NONE", "ODD", "EVEN" };
	m_lcr = value;

	BYTE len = (value & 0x03) + 5;

	bool stopBit = (value & 0x04);
	int stopIdx = 0;
	if (stopBit)
	{
		stopIdx = (len == 5) ? 1 : 2;
	}

	bool parityBit = (value & 0x08);
	int parityIdx = 0;
	if (parityBit)
	{
		parityIdx = (value & 0x10) ? 2 : 1;
	}

	LogPrintf(LOG_INFO, "UART: Set LCR");
	LogPrintf(LOG_INFO, "\tWord Length:   %d", len);
	LogPrintf(LOG_INFO, "\tStop bits:     %s", stop[stopIdx]);
	LogPrintf(LOG_INFO, "\tParity:        %s", parity[parityIdx]);
	// Stick / Break...

	// Divisor Access Bit
	m_dlab = (value & 0x80);
	LogPrintf(LOG_INFO, "\tDivisor Latch: %s", m_dlab ? "ON" : "OFF");
}

BYTE UART::MCR_IN()
{
	return m_mcr;
}

void UART::MCR_OUT(BYTE value)
{
	if (!(m_mcr & 0x1) && (value & 0x01))
	{
//		fprintf(stdout, "[XON]");
	}
	else if ((m_mcr & 0x1) && !(value & 0x01))
	{
//		fprintf(stdout, "[XOFF]");
	}

	if (!(m_mcr & 0x04) && (value & 0x04))
	{
//		fprintf(stdout, "[SOUNDON]");
	}
	else if ((m_mcr & 0x04) && !(value & 0x04))
	{
//		fprintf(stdout, "[SOUNDOFF]");
	}
	m_mcr = value;
}

BYTE UART::DLL_IN()
{
	return m_dll;
}

void UART::DLL_OUT(BYTE value)
{
	LogPrintf(LOG_INFO, "UART: Set DLL");
	m_dll = value;
	printDivisor();
}

BYTE UART::DLM_IN()
{
	return m_dlm;
}

void UART::DLM_OUT(BYTE value)
{
	LogPrintf(LOG_INFO, "UART: Set DLM");
	m_dlm = value;
	printDivisor();
}

void UART::printDivisor()
{
	WORD divisor = m_dll | (m_dlm << 8);
	LogPrintf(LOG_INFO, "\tDivisor:      0x%04X", divisor);
	LogPrintf(LOG_INFO, "\tFrequency =   %0.2fHz", m_baseClockSpeed / (float)16 / (float)divisor);
}