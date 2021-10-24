#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Device8255.h"

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace ppi
{
	enum class RAMSIZE { RAM_64K, RAM_128K, RAM_192K, RAM_256K };
	enum class DISPLAY { MONO_80x25, COLOR_40x25, COLOR_80x25, NONE };

	class Device8255XT : public Device8255
	{
	public:
		Device8255XT(WORD baseAddress);
		virtual ~Device8255XT() {}

		Device8255XT() = delete;
		Device8255XT(const Device8255XT&) = delete;
		Device8255XT& operator=(const Device8255&) = delete;
		Device8255XT(Device8255XT&&) = delete;
		Device8255XT& operator=(Device8255XT&&) = delete;

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

		bool GetTimer2Gate() const { return m_portBData & 1; }
		void SetTimer2Output(bool value)
		{
			m_timer2Out = value;
		}

	protected:
		virtual BYTE PORTA_IN() override;
		virtual void PORTA_OUT(BYTE value)override;

		virtual BYTE PORTB_IN() override;
		virtual void PORTB_OUT(BYTE value) override;

		virtual BYTE PORTC_IN() override;
		virtual void PORTC_OUT(BYTE value) override;

		bool m_timer2Out = false;

		// Motherboard configuration switches
		BYTE m_switches = 0;
	};
}
