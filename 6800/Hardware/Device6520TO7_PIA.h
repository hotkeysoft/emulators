#pragma once
#include <Hardware/Device6520.h>
#include "DevicePIAThomson.h"

namespace kbd { class DeviceKeyboard; }

namespace pia::thomson
{
	class Device6520TO7_PIA : public Device6520
	{
	public:
		Device6520TO7_PIA(std::string id = "pia") : Device6520(id), Logger(id.c_str()) {}

		void Init(kbd::DeviceKeyboard* kbd);

		void SetPIAEventHandler(EventHandler* handler) { m_piaEventHandler = handler; }
		void SetKeyboard(kbd::DeviceKeyboard* kbd) { assert(kbd); m_keyboard = kbd; };

		// Port A Input: Keyboard row data
		void SetKeyboardRowData(BYTE data) { return m_portA.SetInput(data); }

		// Port B Output: Keyboard column selection
		BYTE GetKeyboardColumnSelect() const { return emul::LogBase2(~m_portB.GetOutput()); }

		// CA1: INIT input
		void SetINIT(bool set) { m_portA.SetC1(set); }

		// CA2: Tape drive motor Output
		bool GetTapeMotorState() const { return !m_portA.GetC2(); }

		// CB1: Light pen Clock
		void TriggerLightpen() { SetLightpenClock(false); SetLightpenClock(true); }
		void SetLightpenClock(bool set) { m_portB.SetC1(set); }

		// CB2: Light pen Latch
		void SetLightpenLatch(bool set) { m_portB.SetC2(set); }

		bool GetFIRQ() const { return GetIRQB(); }

		// emul::Serializable
		virtual void Serialize(json& to) override { Device6520::Serialize(to); }
		virtual void Deserialize(const json& from) override { Device6520::Deserialize(from); }

	protected:
		virtual void OnReadPort(PIAPort* src) override;

		kbd::DeviceKeyboard* m_keyboard = nullptr;

		ScreenRAM m_screenRAM = ScreenRAM::UNINITIALIZED;
		BYTE m_borderRGBP = 0xFF;

		EventHandler* m_piaEventHandler = &s_defaultHandler;
	};
}

