#pragma once
#include "Device6846.h"
#include "DevicePIAThomson.h"

namespace pia::thomson
{
	class Device6846TO7_PIA : public Device6846, public DevicePIAThomson
	{
	public:
		Device6846TO7_PIA(std::string id = "pia") : Device6846(id), Logger(id.c_str()) {}

		virtual void Reset() override { Device6846::Reset(); }

		void Init(kbd::DeviceKeyboard* kbd);

		virtual ScreenRAM GetScreenMapping() const override { return (ScreenRAM)emul::GetBit(GetOutput(), 0); }
		virtual void SetLightPenButtonInput(bool set) override { SetInputBit(1, set); }
		virtual BYTE GetKeyboardLED() { return emul::GetBit(GetOutput(), 3); }
		virtual BYTE GetBorderRGBP() const override { return (GetOutput() >> 4) & 7; }
		virtual void SetCassetteInput(bool set) override { SetInputBit(7, set); }

		// CC2
		virtual bool GetBuzzer() const override { return false; }

		virtual bool GetIRQ() const override { return false; }

		// emul::Serializable
		virtual void Serialize(json& to) override { Device6846::Serialize(to); }
		virtual void Deserialize(const json& from) override { Device6846::Deserialize(from); }

	protected:
		virtual void OnWritePort() override;
	};
}

