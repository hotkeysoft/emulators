#pragma once

#include "../Common.h"
#include "../CPU/PortConnector.h"
#include "Device8255.h"

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace ppi
{
	class Device8255PCjr : public Device8255
	{
	public:
		Device8255PCjr(WORD baseAddress);
		virtual ~Device8255PCjr() {}

		Device8255PCjr() = delete;
		Device8255PCjr(const Device8255PCjr&) = delete;
		Device8255PCjr& operator=(const Device8255&) = delete;
		Device8255PCjr(Device8255PCjr&&) = delete;
		Device8255PCjr& operator=(Device8255PCjr&&) = delete;

		// Amount of memory on system board
		enum class RAMSIZE { RAM_64K, RAM_128K };
		void SetKeyboardConnected(bool);
		void SetRAMExpansion(bool);
		void SetDisketteCard(bool);
		void SetModemCard(bool);

		void SetNMILatch(bool value) { m_nmiLatch = value; }
		void SetKeyboardDataBit(bool value) { m_keyboardDataBit = value; }
		void SetCassetteDataBit(bool value) { m_cassetteDataBit = value; }

		bool GetCassetteMotorRelay() const { return (m_portBData & 0x18) == 0; }

		bool GetTimer2Gate() const { return m_portBData & 1; }
		void SetTimer2Output(bool value) 
		{ 
			m_timer2Out = value; 
			// When the cassette motor relay is off,
			// timer 2 output is looped back to cassette data bit
			if (!GetCassetteMotorRelay())
			{
				SetCassetteDataBit(value);
			}
		}

	protected:
		virtual BYTE PORTA_IN() override;
		virtual void PORTA_OUT(BYTE value) override;

		virtual BYTE PORTB_IN() override;
		virtual void PORTB_OUT(BYTE value) override;

		virtual BYTE PORTC_IN() override;
		virtual void PORTC_OUT(BYTE value) override;

		bool m_timer2Out = false;
		bool m_nmiLatch = false;
		bool m_keyboardDataBit = false;
		bool m_cassetteDataBit = false;

		BYTE m_config = 0xFF;
	};
}
