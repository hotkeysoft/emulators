#pragma once

#include "PIAEventsThomson.h"
#include "Serializable.h"

namespace kbd { class DeviceKeyboard; }

namespace pia::thomson
{
	static EventHandler s_defaultHandler;

	class DevicePIAThomson : public emul::Serializable
	{
	public:
		virtual void Reset() = 0;

		virtual void SetPIAEventHandler(EventHandler* handler) { m_piaEventHandler = handler ? handler : &s_defaultHandler; }
		void SetKeyboard(kbd::DeviceKeyboard* kbd) { assert(kbd); m_keyboard = kbd; };

		// Screen RAM mapping (pixel / attribute data)
		virtual ScreenRAM GetScreenMapping() const { return ScreenRAM::UNINITIALIZED; }
		virtual BYTE GetBorderRGBP() const { return 0; }
		virtual bool GetVideoIncrustrationEnable() const { return false; }

		// Video
		virtual void SetVSync(bool set) {}

		// Tape
		virtual bool GetCassetteOut() const { return false; }
		virtual bool GetTapeMotorState() const { return false; }
		virtual void SetCassetteInput(bool set) {}

		// Light pen
		virtual void SetLightPenButtonInput(bool set) {}
		virtual void TriggerLightPenInterrupt() {}

		// Sound
		virtual bool GetBuzzer() const { return false; }

		// Keyboard
		virtual BYTE GetKeyboardColumnSelect() const { return 0; }
		virtual BYTE GetKeyboardRowSelect() const { return 0; }
		virtual void SetSelectedKeyInput(bool set) {};

		// IRQs
		virtual bool GetIRQ() const { return false; }
		virtual bool GetFIRQ() const { return false; }

	protected:
		DevicePIAThomson() {}

		kbd::DeviceKeyboard* m_keyboard = nullptr;

		ScreenRAM m_screenRAM = ScreenRAM::UNINITIALIZED;
		BYTE m_borderRGBP = 0xFF;

		EventHandler* m_piaEventHandler = &s_defaultHandler;
	};
}
