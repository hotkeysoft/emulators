#pragma once
#include "Device6520.h"

namespace kbd
{
	class DeviceKeyboardPET2001;
}

namespace pia
{
	class Device6520PET_PIA1 : public Device6520
	{
	public:
		Device6520PET_PIA1(std::string id = "PIA1") : Device6520(id), Logger(id.c_str()) {}

		void Init(kbd::DeviceKeyboardPET2001* kbd);

		// Port A Inputs
		void SetDiagnosticSenseIn(bool set) { m_portA.SetInputBit(7, !set); }
		void SetIEEE_EOIIn(bool set) { m_portA.SetInputBit(6, !set); }
		void SetCassetteSense2In(bool set) { m_portA.SetInputBit(5, !set); }
		void SetCassetteSense1In(bool set) { m_portA.SetInputBit(4, !set); }

		BYTE GetKeyboardRowSelect() const { return m_portA.GetOutput() & 0b00001111; }

		// CA2
		bool GetBlankScreen() const { return m_portA.GetC2(); }
		// CA1
		void SetCassetteReadLine(bool set) { return m_portA.SetC1(set); }

		// Port B Inputs
		void SetKeyboardRow(BYTE data) { m_portB.SetInput(data); }

		// CB2
		bool GetCassette1Motor() const { return m_portB.GetC2(); }
		// CB1
		void SetScreenRetrace(bool set) { m_portB.SetC1(set); }

	protected:
		virtual void OnReadPort(PIAPort* src) override;

		kbd::DeviceKeyboardPET2001* m_keyboard = nullptr;
	};
}

