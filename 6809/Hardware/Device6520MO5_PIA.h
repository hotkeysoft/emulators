#pragma once
#include "Device6520.h"

namespace kbd { class DeviceKeyboard; }

namespace pia
{
	class Device6520MO5_PIA : public Device6520
	{
	public:
		Device6520MO5_PIA(std::string id = "PIA") : Device6520(id), Logger(id.c_str()) {}

		void Init(kbd::DeviceKeyboard* kbd);

		// Port A Outputs

		// Screen RAM mapping (pixel / attribute data)
		enum class ScreenRAM { PIXEL, ATTR };
		ScreenRAM GetScreenMapping() const { return emul::GetBit(m_portA.GetOutput(), 0) ? ScreenRAM::ATTR : ScreenRAM::PIXEL; }
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

		// CA2
		// CA1

		// CB2
		// CB1

	protected:
		virtual void OnReadPort(PIAPort* src) override;

		kbd::DeviceKeyboard* m_keyboard = nullptr;
	};
}

