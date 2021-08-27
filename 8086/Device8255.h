#pragma once

#include "Common.h"
#include "PortConnector.h"

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace ppi
{
	enum class RAMSIZE { RAM_64K, RAM_128K, RAM_192K, RAM_256K };
	enum class DISPLAY { MONO_80x25, COLOR_40x25, COLOR_80x25, NONE };

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

		void SetControlWord(BYTE ctrl);

		// TODO: Should be in child class
		enum CONFIG_SW
		{
			// PB3 == LOW
			SW_POST_LOOP =   0x01, // SW1
			SW_COPROCESSOR = 0x02, // SW2
			SW_RAM_L =       0x04, // SW3
			SW_RAM_H =       0x08, // SW4

			SW_DISPLAY_L =   0x10, // SW5
			SW_DISPLAY_H =   0x20, // SW6
			SW_FLOPPY_L =    0x40, // SW7
			SW_FLOPPY_H =    0x80, // SW8
		};
	
		// Amount of memory on system board
		void SetRAMConfig(RAMSIZE);

		// Display at power up
		void SetDisplayConfig(DISPLAY);

		void SetFloppyCount(BYTE);
		void SetPOSTLoop(bool);
		void SetMathCoprocessor(bool);

		void SetCurrentKeyCode(BYTE keyCode);

	protected:
		BYTE PORTA_IN();
		void PORTA_OUT(BYTE value);

		BYTE PORTB_IN();
		void PORTB_OUT(BYTE value);

		BYTE PORTC_IN();
		void PORTC_OUT(BYTE value);

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

		// Motherboard configuration switches
		BYTE m_switches;

		BYTE m_currentKey;
	};
}
