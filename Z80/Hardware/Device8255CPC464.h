#pragma once

#include "Device8255.h"

namespace ppi
{
	class Device8255CPC464 : public Device8255
	{
	public:
		Device8255CPC464() : Logger("PIO") {}

		Device8255CPC464(const Device8255CPC464&) = delete;
		Device8255CPC464& operator=(const Device8255CPC464&) = delete;
		Device8255CPC464(Device8255CPC464&&) = delete;
		Device8255CPC464& operator=(Device8255CPC464&&) = delete;

		virtual bool IsSoundON() override { return true; }

		virtual void SetCurrentKeyCode(BYTE keyCode) override { }

	protected:
		virtual BYTE PORTA_IN() override;
		virtual void PORTA_OUT(BYTE value) override;

		virtual BYTE PORTB_IN() override;
		virtual void PORTB_OUT(BYTE value) override;

		virtual BYTE PORTC_IN() override;
		virtual void PORTC_OUT(BYTE value) override;
	};
}
