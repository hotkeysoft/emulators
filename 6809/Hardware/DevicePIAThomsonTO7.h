#pragma once
#include "Device6520TO7_PIA.h"
#include "Device6846TO7_PIA.h"
#include "DevicePIAThomson.h"

namespace pia::thomson
{
	class DevicePIAThomsonTO7 : public DevicePIAThomson
	{
	public:
		DevicePIAThomsonTO7();

		virtual void Reset() override;

		virtual void Tick() override { m_pia6846.Tick(); }

		void Init(kbd::DeviceKeyboard* kbd);

		virtual void SetPIAEventHandler(EventHandler* handler) override { m_pia6846.SetPIAEventHandler(handler); }

		// Screen RAM mapping (pixel / attribute data)
		virtual ScreenRAM GetScreenMapping() const override { return m_pia6846.GetScreenMapping(); }
		virtual BYTE GetBorderRGBP() const override { return m_pia6846.GetBorderRGBP(); }

		// Tape
		virtual bool GetCassetteOut() const override { return m_pia6846.GetCassetteOutput(); }
		virtual bool GetTapeMotorState() const override { return m_pia6520.GetTapeMotorState(); }
		virtual void SetCassetteInput(bool set) override { m_pia6846.SetCassetteInput(set); }

		// Light pen
		virtual void SetLightPenButtonInput(bool set) override { m_pia6846.SetLightPenButtonInput(set); }
		virtual void TriggerLightPenInterrupt() override { m_pia6520.TriggerLightpen(); }

		// Sound
		virtual bool GetBuzzer() const override { return m_pia6846.GetBuzzer(); }

		// Keyboard
		virtual BYTE GetKeyboardColumnSelect() const override { return 0xFF; }
		virtual BYTE GetKeyboardRowSelect() const override { return 0xFF; }
		virtual void SetSelectedKeyInput(bool set) override { }

		// IRQ from 6846 (timer and such), FIRQ from 6520 (light pen)
		virtual bool GetIRQ() const override { return m_pia6846.GetIRQ(); }
		virtual bool GetFIRQ() const override { return m_pia6520.GetFIRQ(); }

		virtual void SetVSync(bool set) override { m_pia6520.SetINIT(!set); }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		Device6846& GetPIA1() { return m_pia6846; };
		Device6520& GetPIA2() { return m_pia6520; };

	protected:
		Device6846TO7_PIA m_pia6846;
		Device6520TO7_PIA m_pia6520;
	};
}

