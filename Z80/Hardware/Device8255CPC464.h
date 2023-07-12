#pragma once

#include "Device8255.h"

namespace ppi
{
	enum class DistributorID {
		AWA = (4 << 1),
		Schneider = (5 << 1),
		Amstrad = (7 << 1),

		_MASK = ~(7 << 1)
	};

	enum class ScreenRefreshRate {
		REFRESH_50HZ = 1,
		REFRESH_60HZ = 0
	};

	enum class KeyboardLine {
		LINE_0,
		LINE_1,
		LINE_2,
		LINE_3,
		LINE_4,
		LINE_5,
		LINE_6 = 6, JOY_COMMON2 = 6,
		LINE_7,
		LINE_8,
		LINE_9 = 9, JOY_COMMON1 = 9,

		INVALID = -1
	};

	class Device8255CPC464 : public Device8255
	{
	public:
		Device8255CPC464() : Logger("PIO") {}

		Device8255CPC464(const Device8255CPC464&) = delete;
		Device8255CPC464& operator=(const Device8255CPC464&) = delete;
		Device8255CPC464(Device8255CPC464&&) = delete;
		Device8255CPC464& operator=(Device8255CPC464&&) = delete;

		virtual void Reset() override;

		virtual bool IsSoundON() override { return true; }

		virtual void SetCurrentKeyCode(BYTE keyCode) override { }

		// Port B, input
		void SetCassetteInput(bool input) { emul::SetBit(m_portBData, 7, input); }
		void SetPrinterBusy(bool busy) { emul::SetBit(m_portBData, 6, busy); }
		void SetExpansionPortPin(bool exp) { emul::SetBit(m_portBData, 5, !exp); }
		void SetRefreshRate(ScreenRefreshRate rate) { emul::SetBit(m_portBData, 5, (bool)rate); }
		void SetDistributorID(DistributorID id) { m_portBData &= (BYTE)DistributorID::_MASK; m_portBData |= (BYTE)id; }
		void SetVSync(bool vsync) { emul::SetBit(m_portBData, 5, vsync); }

		// PORT C, output
		bool GetBDIR() const { return emul::GetBit(m_portCData, 7); }
		bool GetBC1() const { return emul::GetBit(m_portCData, 6); }
		bool GetCassetteOutput() const { return emul::GetBit(m_portCData, 5); }
		bool GetCassetteMotorOut() const { return !emul::GetBit(m_portCData, 4); }
		KeyboardLine GetKeyboardLine() const { int line = (m_portCData & 15); return (line) > 9 ? KeyboardLine::INVALID : (KeyboardLine)line; }

	protected:
		virtual BYTE PORTA_IN() override;
		virtual void PORTA_OUT(BYTE value) override;

		virtual BYTE PORTB_IN() override;
		virtual void PORTB_OUT(BYTE value) override;

		virtual BYTE PORTC_IN() override;
		virtual void PORTC_OUT(BYTE value) override;
	};
}
