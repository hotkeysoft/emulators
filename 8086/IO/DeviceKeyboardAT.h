#pragma once

#include "DeviceKeyboard.h"

using emul::BYTE;

namespace ppi { class Device8042AT; }

namespace kbd
{
	enum class KBDCommand : WORD
	{
		NONE = 0xFFFF,

		RESET = 0xFF,
		RESEND = 0xFE,
		SET_DEFAULT = 0xF6,
		SET_DEFAULT_DISABLE = 0xF5,
		ENABLE = 0xF4,
		SET_TYPEMATIC = 0xF3,
		ECHO = 0xEE,
		SET_INDICATORS = 0xED,

		_FIRST_COMMAND = SET_INDICATORS,
		_LAST_COMMAND = RESET,
	};

	enum class KBDReply : BYTE
	{
		RESEND = 0xFE,
		ACK = 0xFA,
		OVERRUN = 0x00,
		DIAG_FAIL = 0xDF,
		BREAK_PREFIX = 0xF0,
		BAT_COMPLETE = 0xAA, // Basic Assurance Test
		ECHO = 0xEE,
	};

	class DeviceKeyboardAT : public DeviceKeyboard
	{
	public:
		DeviceKeyboardAT();

		DeviceKeyboardAT(const DeviceKeyboardAT&) = delete;
		DeviceKeyboardAT& operator=(const DeviceKeyboardAT&) = delete;
		DeviceKeyboardAT(DeviceKeyboardAT&&) = delete;
		DeviceKeyboardAT& operator=(DeviceKeyboardAT&&) = delete;

		virtual void Init(ppi::DevicePPI* ppi, pic::Device8259* pic) override;

		virtual void Tick() override;

		ppi::Device8042AT* GetPPI() { return (ppi::Device8042AT*)m_ppi; }

	protected:
		KBDCommand m_currCommand = KBDCommand::NONE;
		size_t m_currCommandDelay = 0;

		BYTE m_lastByte = 0;

		void Reply(KBDReply reply);

		void PrepareCommand();
		void LaunchCommand(KBDCommand, size_t length = 10);
		void ExecCommand();
	};
}
