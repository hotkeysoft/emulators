#pragma once

#include "Common.h"
#include "PortConnector.h"


namespace emul
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

	protected:
		const WORD m_baseAddress;

	};
}
