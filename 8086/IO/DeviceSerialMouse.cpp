#include "stdafx.h"
#include "DeviceSerialMouse.h"

using uart::Device8250;
using emul::SerializationError;

namespace mouse
{
	bool DeviceSerialMouse::MouseState::left = false;
	bool DeviceSerialMouse::MouseState::right = false;

	bool DeviceSerialMouse::MouseDataPacket::Merge(MouseState mergeWith)
	{
		if (!IsLocked())
		{
			int dx = state.dx + mergeWith.dx;
			int dy = state.dy + mergeWith.dy;
			state.dx = std::clamp(dx, -254, 255);
			state.dy = std::clamp(dy, -254, 255);
			return true;
		}
		return false;
	}

	BYTE DeviceSerialMouse::MouseDataPacket::GetNextByte()
	{
		switch (sentBytes++)
		{
		case 0: return
			(state.right << 4) |
			(state.left << 5) |
			((state.dy & 0b11000000) >> 4) |
			((state.dx & 0b11000000) >> 6) |
			(1 << 6);
		case 1: return (state.dx & 0b111111);
		case 2: return (state.dy & 0b111111);
		default: throw std::exception("Invalid index");
		}
	}

	// 1200 bauds 7N1, 3 bytes/message,
	// can send about 40 messsages/s max, so set 
	// refresh rate to 120Hz
	DeviceSerialMouse::DeviceSerialMouse(WORD baseAddress, BYTE irq, size_t clockSpeedHz) :
		Device8250(baseAddress, irq, clockSpeedHz),
		m_pollRate(clockSpeedHz / 120),
		m_cooldown(m_pollRate),
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
			MouseState::left = clicked;
			break;
		case 1:
			MouseState::right = clicked;
			break;
		default:
			LogPrintf(LOG_ERROR, "Invalid button id");
		}
		SendMouseState(MouseState());
	}

	void DeviceSerialMouse::Move(int8_t dx, int8_t dy)
	{
		SendMouseState(MouseState(dx, dy));
	}

	void DeviceSerialMouse::SendMouseState(MouseState state)
	{
		MouseDataPacket packet(state);
		bool addToQueue = true;
		if (!m_queue.empty())
		{
			MouseDataPacket& lastPacket = m_queue.back();
			if (lastPacket.Merge(state))
			{
				LogPrintf(LOG_DEBUG, "Merged mouse packet");
				addToQueue = false;
			}
		}

		if (addToQueue)
		{
			LogPrintf(LOG_DEBUG, "Add packet to queue");
			m_queue.push_back(packet);
		}
	}

	void DeviceSerialMouse::Tick()
	{
		Device8250::Tick();
		if (m_queue.size() && (--m_cooldown == 0))
		{
			MouseDataPacket& packet = m_queue.front();

			BYTE value = packet.GetNextByte();
			LogPrintf(LOG_DEBUG, "Send mouse data: [%02x]", value);
			InputData(value);
			if (!packet.HasNextByte())
			{
				// Done with this packet
				m_queue.pop_front();
			}

			m_cooldown = m_pollRate;
		}
	}

	void DeviceSerialMouse::MouseDataPacket::Serialize(json& to)
	{
		to["state.dx"] = state.dx;
		to["state.dy"] = state.dy;
		to["sentBytes"] = sentBytes;
	}
	void DeviceSerialMouse::Serialize(json& to)
	{
		Device8250::Serialize(to["uart"]);

		to["pollRate"] = m_pollRate;
		to["cooldown"] = m_cooldown;
		to["lastRTS"] = m_lastRTS;

		to["mouseState.left"] = MouseState::left;
		to["mouseState.right"] = MouseState::right;

		json queue = json::array();
		for (auto& item : m_queue)
		{
			json itemJson;
			item.Serialize(itemJson);
			queue.push_back(itemJson);
		}
		to["queue"] = queue;
	}

	void DeviceSerialMouse::MouseDataPacket::Deserialize(const json& from)
	{
		state.dx = from["state.dx"];
		state.dy = from["state.dy"];
		sentBytes = from["sentBytes"];
	}
	void DeviceSerialMouse::Deserialize(const json& from)
	{
		Device8250::Deserialize(from["uart"]);

		size_t pollRate = from["pollRate"];
		if (pollRate != m_pollRate)
		{
			throw emul::SerializableException("Device8250: Incompatible pollRate", SerializationError::COMPAT);
		}

		m_cooldown = from["cooldown"];
		m_lastRTS = from["lastRTS"];

		MouseState::left = from["mouseState.left"];
		MouseState::right = from["mouseState.right"];

		const json& queue = from["queue"];
		for (int i = 0; i < queue.size(); ++i)
		{
			MouseDataPacket packet;
			packet.Deserialize(queue[i]);
			m_queue.push_back(packet);
		}
	}
}