#pragma once

#include <SDL.h>

#include <map>
#include <vector>
#include "DeviceKeyboardZX80.h"

using emul::BYTE;

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
		InputEvents(size_t clockSpeedHz, size_t pollingHz = 60);
		~InputEvents();

		InputEvents() = delete;
		InputEvents(const InputEvents&) = delete;
		InputEvents& operator=(const InputEvents&) = delete;
		InputEvents(InputEvents&&) = delete;
		InputEvents& operator=(InputEvents&&) = delete;

		void Init();
		void InitKeyboard(kbd::DeviceKeyboardZX80* kbd);

		void AddEventHandler(EventHandler* handler) { m_handlers.push_back(handler); }

		kbd::DeviceKeyboardZX80* GetKeyboard() const { return m_keyboard; }

		void Tick();

		bool IsQuit() { return m_quit; }

	protected:
		const size_t m_pollRate;
		size_t m_cooldown = m_pollRate;

		void InputKey(SDL_KeyboardEvent& evt);
		bool m_quit = false;

		kbd::DeviceKeyboardZX80* m_keyboard = nullptr;
		KeyMap* m_keyMap = nullptr;

		std::vector<EventHandler*> m_handlers;
	};

}
