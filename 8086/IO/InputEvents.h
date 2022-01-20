#pragma once

#include "../Common.h"
#include <Logger.h>

#include <SDL.h>

#include <map>
#include <vector>

using emul::BYTE;
namespace kbd { class DeviceKeyboard; }
namespace joy { class DeviceJoystick; }

namespace events
{
	enum class KBDMapping { XT, TANDY };

	class EventHandler
	{
	public:
		virtual bool HandleEvent(SDL_Event& e) = 0;
	};

	class Key
	{
	public:
		Key(BYTE scancode, BYTE prefix = 0, bool repeat = true, bool isToggle = false, bool toggleState = false) :
			m_scancode(scancode),
			m_prefix(prefix),
			m_repeat(repeat),
			m_isToggle(isToggle),
			m_toggleState(toggleState)
		{
		}

		BYTE GetScancode() const { return m_scancode; }
		BYTE GetPrefix() const { return m_prefix; }
		bool IsRepeat() const { return m_repeat; }
		bool IsToggle() const { return m_isToggle; }
		bool GetToggleState() const { return m_toggleState; }

		void Toggle() { m_toggleState = !m_toggleState; }
	private:
		BYTE m_scancode;
		BYTE m_prefix = 0;
		bool m_repeat = true;
		bool m_isToggle = false;
		bool m_toggleState = false;
	};
	class NonRepeatingKey : public Key
	{
	public:
		NonRepeatingKey(BYTE scancode) : Key(scancode, 0, false) {}
	};
	class ExtendedKey : public Key
	{
	public:
		ExtendedKey(BYTE prefix, BYTE scancode, bool repeat = true) : Key(scancode, prefix, repeat) {}
	};
	class ToggleKey : public Key
	{
	public:
		ToggleKey(BYTE scancode, bool state = false) : Key(scancode, 0, false, true, state) {}
	};

	typedef std::map<SDL_Scancode, Key> KeyMap;

	class InputEvents : public Logger
	{
	public:
		InputEvents(size_t clockSpeedHz);
		~InputEvents();

		InputEvents() = delete;
		InputEvents(const InputEvents&) = delete;
		InputEvents& operator=(const InputEvents&) = delete;
		InputEvents(InputEvents&&) = delete;
		InputEvents& operator=(InputEvents&&) = delete;

		void InitKeyboard(kbd::DeviceKeyboard* kbd, KBDMapping mapping = KBDMapping::XT);
		void InitJoystick(joy::DeviceJoystick* joy);

		void AddEventHandler(EventHandler* handler) { m_handlers.push_back(handler); }

		void Tick();

		bool IsQuit() { return m_quit; }

	protected:
		const size_t m_pollRate;
		size_t m_cooldown = m_pollRate;

		void InputKey(SDL_KeyboardEvent& evt);
		void InputControllerButton(uint8_t button, uint8_t state);
		void InputControllerAxis(uint8_t axis, int16_t value);

		bool m_quit = false;

		kbd::DeviceKeyboard* m_keyboard = nullptr;
		KeyMap* m_keyMap = nullptr;

		SDL_GameController* m_gameController = nullptr;
		SDL_JoystickID m_controllerID = -1;

		joy::DeviceJoystick* m_joystick = nullptr;

		std::vector<EventHandler*> m_handlers;
	};

}
