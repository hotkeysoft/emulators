#pragma once
#include "../Hardware/Device8250.h"

namespace mouse
{
	class DeviceSerialMouse : public uart::Device8250
	{
	public:
		DeviceSerialMouse(WORD baseAddress, size_t clockSpeedHz = 1000000);

		virtual void Init() override;

		void Click(BYTE button, bool clicked);
		void Move(int8_t dx, int8_t dy);

		virtual void Tick() override;

	protected:
		virtual void OnRTS(bool state) override;
		virtual void OnDTR(bool state) override;

		bool m_lastRTS = false;

		void SendMouseState();

		struct MouseState
		{
			bool left = false;
			bool right = false;
			int8_t dx = 0;
			int8_t dy = 0;
		} m_state;

		std::list<BYTE> m_queue;
	};
}

