#pragma once

#include <SDL.h>
#include <map>
#include <vector>

#include <IO/DeviceMouse.h>

using emul::BYTE;

namespace kbd { class DeviceKeyboard; }
namespace joy { class DeviceJoystick; }

namespace events
{
	class EventHandler;

	class Key
	{
	public:
		Key(BYTE row, BYTE col) :
			m_row(row),
			m_col(col)
		{
		}

		BYTE GetRow() const { return m_row; }
		BYTE GetCol() const { return m_col; }

	private:
		BYTE m_row;
		BYTE m_col;
	};

	typedef std::map<SDL_Scancode, Key> KeyMap;

	class InputEvents : public Logger
	{
	public:
		InputEvents(size_t clockSpeedHz, size_t pollInterval);
		~InputEvents();

		InputEvents() = delete;
		InputEvents(const InputEvents&) = delete;
		InputEvents& operator=(const InputEvents&) = delete;
		InputEvents(InputEvents&&) = delete;
		InputEvents& operator=(InputEvents&&) = delete;

		void Init();
		void InitKeyboard(kbd::DeviceKeyboard* kbd);
		void InitJoystick(joy::DeviceJoystick* joy);
		void InitMouse(mouse::DeviceMouse* mouse);

		void AddEventHandler(EventHandler* handler) { m_handlers.push_back(handler); }

		kbd::DeviceKeyboard* GetKeyboard() const { return m_keyboard; }
		joy::DeviceJoystick* GetJoystick() const { return m_joystick; }

		void Tick();

		bool IsQuit() { return m_quit; }

	protected:
		const size_t m_clockSpeedHz;
		const size_t m_pollInterval;
		size_t m_cooldown;

		void InputKey(SDL_KeyboardEvent& evt);
		void InputControllerButton(uint8_t button, uint8_t state);
		void InputControllerAxis(uint8_t axis, int16_t value);
		bool m_quit = false;

		kbd::DeviceKeyboard* m_keyboard = nullptr;
		KeyMap* m_keyMap = nullptr;

		SDL_GameController* m_gameController = nullptr;
		SDL_JoystickID m_controllerID = -1;

		joy::DeviceJoystick* m_joystick = nullptr;

		mouse::DeviceMouse* m_mouse = nullptr;
		mouse::DeviceMouse m_nullMouse;

		std::vector<EventHandler*> m_handlers;
	};

}
