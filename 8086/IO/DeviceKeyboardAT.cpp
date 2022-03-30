#include "stdafx.h"

#include "DeviceKeyboardAT.h"
#include "../Hardware/Device8042AT.h"

namespace kbd
{
	const char* GetCommandName(KBDCommand cmd)
	{
		switch (cmd)
		{
		case KBDCommand::NONE: return "NONE";
		case KBDCommand::RESET: return "RESET";
		case KBDCommand::RESEND: return "RESEND";
		case KBDCommand::SET_DEFAULT: return "SET_DEFAULT";
		case KBDCommand::SET_DEFAULT_DISABLE: return "SET_DEFAULT_DISABLE";
		case KBDCommand::ENABLE: return "ENABLE";
		case KBDCommand::SET_TYPEMATIC: return "SET_TYPEMATIC";
		case KBDCommand::ECHO: return "ECHO";
		case KBDCommand::SET_INDICATORS: return "SET_INDICATORS";

		default: return "NOP";
		}
	}

	DeviceKeyboardAT::DeviceKeyboardAT() : Logger("kbdAT")
	{
	}

	void DeviceKeyboardAT::Init(ppi::DevicePPI* ppi, pic::Device8259* pic)
	{
		assert(ppi);
		assert(dynamic_cast<ppi::Device8042AT*>(ppi));
		assert(pic);
		DeviceKeyboard::Init(ppi, pic);

		// Launch power-on self test
		LaunchCommand(KBDCommand::RESET, 60);
	}

	void DeviceKeyboardAT::PrepareCommand()
	{
		LogPrintf(LOG_DEBUG, "PrepareCommand");

		KBDCommand cmd = (KBDCommand)GetPPI()->ReadInputBuffer();
		if (cmd < KBDCommand::_FIRST_COMMAND)
		{
			LogPrintf(LOG_WARNING, "Invalid Command [%02x]", cmd);
			Reply(KBDReply::RESEND);
		}
		else switch (cmd)
		{
		case KBDCommand::RESET: 
			Reply(KBDReply::ACK);
			LaunchCommand(cmd, 100); break;

		case KBDCommand::RESEND: Reply((KBDReply)m_lastByte); break;
		case KBDCommand::ECHO: Reply(KBDReply::ECHO); break;

		case KBDCommand::SET_DEFAULT:
		case KBDCommand::SET_DEFAULT_DISABLE:
		case KBDCommand::ENABLE:
		case KBDCommand::SET_TYPEMATIC:
		case KBDCommand::SET_INDICATORS:
			Reply(KBDReply::ACK);
			LaunchCommand(cmd);
			break;

		default:
			// NOP Commands
			Reply(KBDReply::ACK);
			break;
		}
	}

	void DeviceKeyboardAT::Reply(KBDReply reply)
	{
		InputKey((BYTE)reply);
	}

	void DeviceKeyboardAT::LaunchCommand(KBDCommand cmd, size_t length)
	{
		assert(length);
		LogPrintf(LOG_WARNING, "LaunchCommand [%s], delay=%d", GetCommandName(cmd), length);
		m_currCommand = cmd;
		m_currCommandDelay = length;
	}

	void DeviceKeyboardAT::ExecCommand()
	{
		if (--m_currCommandDelay)
			return;

		LogPrintf(LOG_WARNING, "Command Completed [%s]", GetCommandName(m_currCommand));

		switch (m_currCommand)
		{
		case KBDCommand::RESET:
			Reply(KBDReply::BAT_COMPLETE);
			break;
		default:
			LogPrintf(LOG_ERROR, "Not implemented");
		}

		m_currCommand = KBDCommand::NONE;
	}

	void DeviceKeyboardAT::Tick()
	{
		static int cooldown = 0;
		static bool keySent = false;

		if (cooldown)
		{
			--cooldown;
			return;
		}

		cooldown = 10000;

		if (GetPPI()->IsKeyboardCommandPending())
		{
			PrepareCommand();
			// If there is an immediate reply, send it on the next cycle
			return;
		}

		if (m_currCommand != KBDCommand::NONE)
		{
			ExecCommand();
		}

		if (keySent)
		{
			keySent = false;
		}
		else if (m_keyBufRead != m_keyBufWrite)
		{
			BYTE currKey = m_keyBuf[m_keyBufRead++];
			if ((KBDReply)currKey != KBDReply::RESEND)
			{
				m_lastByte = (BYTE)currKey;
			}
			m_ppi->SetCurrentKeyCode(currKey);
			keySent = true;
		}

	}
}
