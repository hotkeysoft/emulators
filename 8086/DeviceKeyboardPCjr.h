#pragma once

#include "Common.h"
#include "Logger.h"
#include "DeviceKeyboard.h"

#include <deque>

using emul::BYTE;

namespace kbd
{
	enum class CLK1 { MAIN_CLK, TIMER0_OUT };

	class DeviceKeyboardPCjr : public DeviceKeyboard
	{
	public:
		DeviceKeyboardPCjr(WORD baseAddress);

		DeviceKeyboardPCjr(const DeviceKeyboardPCjr&) = delete;
		DeviceKeyboardPCjr& operator=(const DeviceKeyboardPCjr&) = delete;
		DeviceKeyboardPCjr(DeviceKeyboardPCjr&&) = delete;
		DeviceKeyboardPCjr& operator=(DeviceKeyboardPCjr&&) = delete;

		virtual void Init(ppi::Device8255* ppi, pic::Device8259* pic) override;

		virtual void Tick() override;

		CLK1 GetTimer1Source() { return m_portA0.selectCLK1Input ? CLK1::TIMER0_OUT : CLK1::MAIN_CLK; }

		bool NMIPending();

		bool IsSendingKey() const { return m_keySerialData.size() > 0; }

	protected:
		WORD m_baseAddress;

		bool m_nmiLatch = false;

		struct PortA0
		{
			bool enableNMI = false;       //D7
			bool irTestEnabled = false;   //D6
			bool selectCLK1Input = false; //D5
			bool disableHRQ = false;      //D4
		} m_portA0;

		BYTE ReadPortA0();
		void WritePortA0(BYTE value);

		void LoadKey(BYTE key);
		void PushBit(bool bit);
		void PushStop();
		void SendBit();

		std::deque<bool> m_keySerialData;
	};
}
