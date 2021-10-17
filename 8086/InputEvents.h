#pragma once

#include "Logger.h"
#include "Common.h"

#include <SDL.h>

#include <map>

using emul::BYTE;
namespace kbd { class DeviceKeyboard; }

namespace events
{
	class InputEvents : public Logger
	{
	public:
		InputEvents(size_t clockSpeedHz);

		InputEvents() = delete;
		InputEvents(const InputEvents&) = delete;
		InputEvents& operator=(const InputEvents&) = delete;
		InputEvents(InputEvents&&) = delete;
		InputEvents& operator=(InputEvents&&) = delete;

		void Init(kbd::DeviceKeyboard* kbd);

		void Tick();

		bool IsQuit() { return m_quit; }

	protected:
		const size_t m_pollRate;
		size_t m_cooldown = m_pollRate;

		void InputKey(SDL_KeyboardEvent& evt);

		bool m_quit = false;

		kbd::DeviceKeyboard* m_keyboard = nullptr;
	};

}
