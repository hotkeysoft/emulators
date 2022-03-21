#include "stdafx.h"
#include "DeviceSerialMouse.h"

using uart::Device8250;

namespace mouse
{
	DeviceSerialMouse::DeviceSerialMouse(WORD baseAddress, size_t clockSpeedHz) :
		Device8250(baseAddress, clockSpeedHz),
		Logger("serial.mouse")
	{

	}

	void DeviceSerialMouse::Init()
	{
		Device8250::Init();
	}

	void DeviceSerialMouse::OnRTS(bool state)
	{
		LogPrintf(LOG_INFO, "OnRTS[%d]", state);

		if (!GetDTR())
		{
			return;
		}

		// Reset sequence on RTS 0->1
		if (!m_lastRTS && state)
		{
			LogPrintf(LOG_INFO, "Mouse RESET");
			// After reset, mouse replies 'M' to computer
			InputData('M');
		}
		m_lastRTS = state;
	}

	void DeviceSerialMouse::OnDTR(bool state)
	{
		LogPrintf(LOG_INFO, "OnDTR[%d]", state);
	}

	void DeviceSerialMouse::Click(BYTE button, bool clicked)
	{
		switch (button)
		{
		case 0:
			m_state.left = clicked;
			break;
		case 1:
			m_state.right = clicked;
			break;
		default:
			LogPrintf(LOG_ERROR, "Invalid button id");
		}
		SendMouseState();
	}

	void DeviceSerialMouse::Move(int8_t dx, int8_t dy)
	{
		m_state.dx = dx;
		m_state.dy = dy;
		SendMouseState();
	}

	void DeviceSerialMouse::SendMouseState()
	{
		// TODO

		BYTE b1 =
			(m_state.right << 4) |
			(m_state.left << 5) |
			((m_state.dy & 0b11000000) >> 4) |
			((m_state.dx & 0b11000000) >> 6) |
			(1 << 6);

		m_queue.push_back(b1);
		m_queue.push_back(m_state.dx & 0b111111);
		m_queue.push_back(m_state.dy & 0b111111);
	}

	void DeviceSerialMouse::Tick()
	{
		static int cooldown = 10000;
		Device8250::Tick();
		if (m_queue.size() && (--cooldown == 0))
		{
			BYTE value = m_queue.front();
			LogPrintf(LOG_DEBUG, "Send mouse data: [%02x]", value);
			InputData(value);
			m_queue.pop_front();
			cooldown = 50000;
		}
	}
}