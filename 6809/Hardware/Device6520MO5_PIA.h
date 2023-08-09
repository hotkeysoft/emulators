#pragma once
#include "Device6520.h"

namespace kbd { class DeviceKeyboard; }

namespace pia
{
	enum class ScreenRAM { PIXEL = 1, ATTR = 0, UNINITIALIZED = -1 };

	class EventHandler
	{
	public:
		virtual void OnScreenMapChange(ScreenRAM map) {}
		virtual void OnBorderChange(BYTE borderRGBP) {}
	};

	static EventHandler s_defaultHandler;

	class Device6520MO5_PIA : public Device6520
	{
	public:
		Device6520MO5_PIA(std::string id = "PIA") : Device6520(id), Logger(id.c_str()) {}

		void Init(kbd::DeviceKeyboard* kbd);

		void SetPIAEventHandler(EventHandler* handler) { m_piaEventHandler = handler ? handler : &s_defaultHandler; }

		// Port A Outputs

		// Screen RAM mapping (pixel / attribute data)
		ScreenRAM GetScreenMapping() const { return (ScreenRAM)emul::GetBit(m_portA.GetOutput(), 0); }
		BYTE GetBorderRGBP() const { return (m_portA.GetOutput() >> 1) & 15; }
		bool GetCassetteOut() const { return emul::GetBit(m_portA.GetOutput(), 6); }

		// Port A Inputs
		void SetLightPenInput(bool set) { m_portA.SetInputBit(5, set); }
		void SetCassetteInput(bool set) { m_portA.SetInputBit(7, set); }

		// Port B Outputs
		bool GetBuzzer() const { return emul::GetBit(m_portB.GetOutput(), 0); }
		BYTE GetKeyboardColumnSelect() const { return (m_portB.GetOutput() >> 1) & 7; }
		BYTE GetKeyboardRowSelect() const { return (m_portB.GetOutput() >> 4) & 7; }

		// Port B Inputs
		void SetSelectedKeyInput(bool set) { m_portB.SetInputBit(7, set); }

		// CA2: Tape drive motor Output
		bool GetTapeMotorState() const { return m_portA.GetC2(); }

		// CA1: Light pen button Input
		void SetLightPen(bool set) { m_portA.SetC1(set); }

		// CB2: Video incrustation enable Output
		bool GetVideoIncrustrationEnable() const { return m_portB.GetC2(); }

		// CB1: 50Hz vSync Input
		void SetVSync(bool set) { m_portB.SetC1(set); }

	protected:
		virtual void OnReadPort(PIAPort* src) override;
		virtual void OnWritePort(PIAPort* src) override;

		kbd::DeviceKeyboard* m_keyboard = nullptr;

		ScreenRAM m_screenRAM = ScreenRAM::UNINITIALIZED;
		BYTE m_borderRGBP = 0xFF;

		EventHandler* m_piaEventHandler = &s_defaultHandler;
	};
}

