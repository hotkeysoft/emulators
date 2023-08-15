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

		void SetPIAEventHandler(EventHandler* handler) { m_piaEventHandler = handler ? handler : &s_defaultHandler; }
		void SetKeyboard(kbd::DeviceKeyboard* kbd) { assert(kbd); m_keyboard = kbd; };

		// Screen RAM mapping (pixel / attribute data)
		virtual ScreenRAM GetScreenMapping() const = 0;
		virtual BYTE GetBorderRGBP() const = 0;

		// Video
		virtual void SetVSync(bool set) {}

		// Tape
		virtual bool GetCassetteOut() const = 0;
		virtual bool GetTapeMotorState() const = 0;
		virtual void SetCassetteInput(bool set) = 0;

		// Light pen
		virtual void SetLightPenButtonInput(bool set) = 0;
		virtual void TriggerLightPenInterrupt() = 0;

		// sound
		virtual bool GetBuzzer() const = 0;

		// Keyboard
		virtual BYTE GetKeyboardColumnSelect() const = 0;
		virtual BYTE GetKeyboardRowSelect() const = 0;
		virtual void SetSelectedKeyInput(bool set) {};

		// CB2: Video incrustation enable Output
		virtual bool GetVideoIncrustrationEnable() const { return false; };

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
