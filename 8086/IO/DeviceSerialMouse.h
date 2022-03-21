#pragma once
#include "../Hardware/Device8250.h"

namespace mouse
{
	class DeviceSerialMouse : public uart::Device8250
	{
	public:
		DeviceSerialMouse(WORD baseAddress, BYTE irq, size_t clockSpeedHz = 1000000);

		virtual void Init() override;

		void Click(BYTE button, bool clicked);
		void Move(int8_t dx, int8_t dy);

		virtual void Tick() override;

		// emul::Serializable
		virtual void Serialize(json& to) override;
		virtual void Deserialize(const json& from) override;

	protected:
		const size_t m_pollRate;
		size_t m_cooldown = m_pollRate;

		virtual void OnRTS(bool state) override;
		virtual void OnDTR(bool state) override;

		bool m_lastRTS = false;

		struct MouseState
		{
			MouseState() {}
			MouseState(int8_t dx, int8_t dy) : dx(dx), dy(dy) {}

			static bool left;
			static bool right;
			int8_t dx = 0;
			int8_t dy = 0;
		};
		void SendMouseState(MouseState state);

		class MouseDataPacket : emul::Serializable
		{
		public:
			MouseDataPacket() {}
			MouseDataPacket(MouseState state) : state(state) {}
			bool IsLocked() const { return sentBytes != 0; }
			bool HasNextByte() const { return sentBytes < 3; }

			bool Merge(MouseState state);
			BYTE GetNextByte();

			// emul::Serializable
			virtual void Serialize(json& to) override;
			virtual void Deserialize(const json& from) override;

		protected:
			MouseState state;
			int sentBytes = 0;
		};

		std::list<MouseDataPacket> m_queue;
	};
}

