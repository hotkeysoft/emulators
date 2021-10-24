#pragma once

#include "Logger.h"
#include "Common.h"

#include <SDL.h>

#include <map>

using emul::BYTE;
namespace kbd { class DeviceKeyboard; }

namespace events
{
	enum class KBDMapping { XT, TANDY };

	class Key
	{
	public:
		Key(BYTE scancode, BYTE prefix = 0, bool repeat = true) : m_scancode(scancode), m_prefix(prefix), m_repeat(repeat) {}

		BYTE GetScancode() const { return m_scancode; }
		BYTE GetPrefix() const { return m_prefix; }
		bool IsRepeat() const { return m_repeat; }
	private:
		BYTE m_scancode;
		BYTE m_prefix = 0;
		bool m_repeat = true;
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

	typedef std::map<SDL_Scancode, Key> KeyMap;

	class InputEvents : public Logger
	{
	public:
		InputEvents(size_t clockSpeedHz);

		InputEvents() = delete;
		InputEvents(const InputEvents&) = delete;
		InputEvents& operator=(const InputEvents&) = delete;
		InputEvents(InputEvents&&) = delete;
		InputEvents& operator=(InputEvents&&) = delete;

		void Init(kbd::DeviceKeyboard* kbd, KBDMapping mapping = KBDMapping::XT);

		void Tick();

		bool IsQuit() { return m_quit; }

	protected:
		const size_t m_pollRate;
		size_t m_cooldown = m_pollRate;

		void InputKey(SDL_KeyboardEvent& evt);

		bool m_quit = false;

		kbd::DeviceKeyboard* m_keyboard = nullptr;
		const KeyMap* m_keyMap = nullptr;
	};

}
