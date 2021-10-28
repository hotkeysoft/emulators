#pragma once

#include "Common.h"
#include "PortConnector.h"
#include "Device8255.h"

using emul::PortConnector;
using emul::BYTE;
using emul::WORD;

namespace ppi
{
	class Device8255Tandy : public Device8255
	{
	public:
		Device8255Tandy(WORD baseAddress);
		virtual ~Device8255Tandy() {}

		Device8255Tandy() = delete;
		Device8255Tandy(const Device8255Tandy&) = delete;
		Device8255Tandy& operator=(const Device8255&) = delete;
		Device8255Tandy(Device8255Tandy&&) = delete;
		Device8255Tandy& operator=(Device8255Tandy&&) = delete;

		bool GetTimer2Gate() const { return m_portBData & 1; }
		void SetTimer2Output(bool value) 
		{ 
			m_timer2Out = value; 
		}

		bool IsKeyboardBusy() const { return emul::GetBit(m_portBData, 7); }

	protected:
		virtual BYTE PORTA_IN() override;
		virtual void PORTA_OUT(BYTE value) override;

		virtual BYTE PORTB_IN() override;
		virtual void PORTB_OUT(BYTE value) override;

		virtual BYTE PORTC_IN() override;
		virtual void PORTC_OUT(BYTE value) override;

		bool m_timer2Out = false;

		BYTE m_config = 0xFF;
	};
}
