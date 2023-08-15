#pragma once
#include "Device6520.h"
#include "DevicePIAThomson.h"

namespace pia::thomson
{
	class DevicePIAThomsonTO7 : public DevicePIAThomson
	{
	public:
		DevicePIAThomsonTO7();

		virtual void Reset() override { m_pia6520.Reset(); }

		void Init(kbd::DeviceKeyboard* kbd);

		// Screen RAM mapping (pixel / attribute data)
		virtual ScreenRAM GetScreenMapping() const override { return ScreenRAM::PIXEL; }
		virtual BYTE GetBorderRGBP() const override { return 0xFF; }

		// Tape
		virtual bool GetCassetteOut() const override { return false; }
		virtual bool GetTapeMotorState() const override { return m_pia6520.GetPortA().GetC2(); }
		virtual void SetCassetteInput(bool set) override { }

		// Light pen
		virtual void SetLightPenButtonInput(bool set) override { }
		virtual void TriggerLightPenInterrupt() override { }

		// Sound
		virtual bool GetBuzzer() const override { return false; }

		// Keyboard
		virtual BYTE GetKeyboardColumnSelect() const override { return 0xFF; }
		virtual BYTE GetKeyboardRowSelect() const override { return 0xFF; }
		virtual void SetSelectedKeyInput(bool set) override { }

		// IRQs
		virtual bool GetIRQ() const override { return false; }
		virtual bool GetFIRQ() const override { return false; }

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

		Device6520& GetPIA2() { return m_pia6520; };

	protected:
		Device6520 m_pia6520;
	};
}

