#pragma once
#include "Device6846.h"
#include "DevicePIAThomson.h"

namespace pia::thomson
{
	class Device6846TO7_PIA : public Device6846
	{
	public:
		Device6846TO7_PIA(std::string id = "pia") : Device6846(id), Logger(id.c_str()) {}

		void SetPIAEventHandler(EventHandler* handler) { m_piaEventHandler = handler; }

		ScreenRAM GetScreenMapping() const { return (ScreenRAM)emul::GetBit(GetOutput(), 0); }
		void SetLightPenButtonInput(bool set) { SetInputBit(1, set); }
		BYTE GetKeyboardLED() { return emul::GetBit(GetOutput(), 3); }
		BYTE GetBorderRGBP() const { return (GetOutput() >> 4) & 7; }
		void SetCassetteInput(bool set) { SetInputBit(7, set); }
		BYTE GetCassetteOutput() const { return m_timerOutput; }

		// CC2
		bool GetBuzzer() const { return GetCP2(); }

		// emul::Serializable
		virtual void Serialize(json& to) override { Device6846::Serialize(to); }
		virtual void Deserialize(const json& from) override { Device6846::Deserialize(from); }

	protected:
		virtual void OnWritePort() override;

		EventHandler* m_piaEventHandler = nullptr;
		BYTE m_borderRGBP = 0;
		ScreenRAM m_screenRAM = ScreenRAM::PIXEL;
	};
}

