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
		virtual ~Device8255() {}

		Device8255() = delete;
		Device8255(const Device8255&) = delete;
		Device8255& operator=(const Device8255&) = delete;
		Device8255(Device8255&&) = delete;
		Device8255& operator=(Device8255&&) = delete;

		void Init();
		void Reset();

		void SetControlWord(BYTE ctrl);

		virtual bool IsSoundON();

		virtual void SetCurrentKeyCode(BYTE keyCode) { m_portAData = keyCode; }

	protected:
		virtual BYTE PORTA_IN() = 0;
		virtual void PORTA_OUT(BYTE value) = 0;

		virtual BYTE PORTB_IN() = 0;
		virtual void PORTB_OUT(BYTE value) = 0;

		virtual BYTE PORTC_IN() = 0;
		virtual void PORTC_OUT(BYTE value) = 0;

		BYTE CONTROL_IN();
		void CONTROL_OUT(BYTE value);

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

		enum class DIRECTION { INPUT, OUTPUT };
		const char* GetPortDirectionStr(DIRECTION d) { return (d == DIRECTION::INPUT) ? "INPUT" : "OUTPUT"; }

		DIRECTION m_portADirection = DIRECTION::INPUT;
		DIRECTION m_portBDirection = DIRECTION::INPUT;
		DIRECTION m_portCHDirection = DIRECTION::INPUT;
		DIRECTION m_portCLDirection = DIRECTION::INPUT;

		BYTE m_portAData = 0;
		BYTE m_portBData = 0;
		BYTE m_portCData = 0;

		BYTE m_controlWord = DEFAULT_CONTROLWORD;
	};
}
