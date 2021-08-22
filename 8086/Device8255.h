#pragma once

#include "Common.h"
#include "PortConnector.h"

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace ppi
{
	class Device8255 : public PortConnector
	{
	public:
		Device8255(WORD baseAddress);

		Device8255() = delete;
		Device8255(const Device8255&) = delete;
		Device8255& operator=(const Device8255&) = delete;
		Device8255(Device8255&&) = delete;
		Device8255& operator=(Device8255&&) = delete;

		void Init();
		void Reset();

		BYTE PORTA_IN();
		void PORTA_OUT(BYTE value);

		BYTE PORTB_IN();
		void PORTB_OUT(BYTE value);

		BYTE PORTC_IN();
		void PORTC_OUT(BYTE value);

		BYTE CONTROL_IN();
		void CONTROL_OUT(BYTE value);

		void SetControlWord(BYTE ctrl);

	protected:
		const WORD m_baseAddress;

		enum CTRL {
			CTRL_MODESETFLAG = 128,
			CTRL_GA_MODE2    = 64,
			CTRL_GA_MODE1    = 32,
			CTRL_GA_A_DIR    = 16,
			CTRL_GA_C_DIR_H  = 8,
			CTRL_GB_MODE1    = 4,
			CTRL_GB_B_DIR    = 2,
			CTRL_GB_C_DIR_L  = 1,
		};

		const BYTE DEFAULT_CONTROLWORD = (
			CTRL_MODESETFLAG | 
			CTRL_GA_A_DIR | 
			CTRL_GA_C_DIR_H | 
			CTRL_GB_B_DIR | 
			CTRL_GB_C_DIR_L
		);

		enum DIRECTION { INPUT, OUTPUT };
		const char* GetPortDirectionStr(DIRECTION d) { return (d == INPUT) ? "INPUT" : "OUTPUT"; }

		DIRECTION m_portADirection;
		DIRECTION m_portBDirection;
		DIRECTION m_portCHDirection;
		DIRECTION m_portCLDirection;

		BYTE m_portAData;
		BYTE m_portBData;
		BYTE m_portCData;

		BYTE m_controlWord;
	};
}
