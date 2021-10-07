#pragma once

#include "Common.h"
#include "PortConnector.h"
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
		void SetRAMConfig(RAMSIZE);
		void SetKeyboardConnected(bool);
		void SetRAMExpansion(bool);
		void SetDisketteCard(bool);
		void SetModemCard(bool);

		virtual void SetCurrentKeyCode(BYTE keyCode);

	protected:
		virtual BYTE PORTA_IN() override;
		virtual void PORTA_OUT(BYTE value) override;

		virtual BYTE PORTB_IN() override;
		virtual void PORTB_OUT(BYTE value) override;

		virtual BYTE PORTC_IN() override;
		virtual void PORTC_OUT(BYTE value) override;

		BYTE m_currentKey = 0xAA;

		BYTE m_config = 0xFF;
	};
}
