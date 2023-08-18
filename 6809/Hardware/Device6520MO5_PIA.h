#pragma once
#include "Device6520.h"
#include "DevicePIAThomson.h"

namespace pia::thomson
{
	class Device6520MO5_PIA : public Device6520, public DevicePIAThomson
	{
	public:
		Device6520MO5_PIA(std::string id = "pia") : Device6520(id), Logger(id.c_str()) {}

		virtual void Reset() override { Device6520::Reset(); }

		void Init(kbd::DeviceKeyboard* kbd);

		// Port A Outputs

		// Screen RAM mapping (pixel / attribute data)
		virtual ScreenRAM GetScreenMapping() const override { return (ScreenRAM)emul::GetBit(m_portA.GetOutput(), 0); }
		virtual BYTE GetBorderRGBP() const override { return (m_portA.GetOutput() >> 1) & 15; }
		virtual bool GetCassetteOut() const override { return emul::GetBit(m_portA.GetOutput(), 6); }

		// Port A Inputs
		virtual void SetLightPenButtonInput(bool set) override { m_portA.SetInputBit(5, set); }
		virtual void SetCassetteInput(bool set) override { m_portA.SetInputBit(7, set); }

		// Port B Outputs
		virtual bool GetBuzzer() const override { return emul::GetBit(m_portB.GetOutput(), 0); }
		virtual BYTE GetKeyboardColumnSelect() const override { return (m_portB.GetOutput() >> 1) & 7; }
		virtual BYTE GetKeyboardRowSelect() const override { return (m_portB.GetOutput() >> 4) & 7; }

		// Port B Inputs
		virtual void SetSelectedKeyInput(bool set) override { m_portB.SetInputBit(7, set); }

		// CA2: Tape drive motor Output
		virtual bool GetTapeMotorState() const override { return !m_portA.GetC2(); }

		// CA1: Light pen interrupt Input
		virtual void TriggerLightPenInterrupt() override { m_portA.SetC1(false); m_portA.SetC1(true); }

		// CB2: Video incrustation enable Output
		virtual bool GetVideoIncrustrationEnable() const override { return m_portB.GetC2(); }

		// CB1: 50Hz vSync Input
		virtual void SetVSync(bool set) override { m_portB.SetC1(set); }

		virtual bool GetIRQ() const override { return GetIRQB(); }
		virtual bool GetFIRQ() const override { return GetIRQA(); }

		// emul::Serializable
		virtual void Serialize(json& to) override { Device6520::Serialize(to); }
		virtual void Deserialize(const json& from) override { Device6520::Deserialize(from); }

	protected:
		virtual void OnReadPort(PIAPort* src) override;
		virtual void OnWritePort(PIAPort* src) override;
	};
}

