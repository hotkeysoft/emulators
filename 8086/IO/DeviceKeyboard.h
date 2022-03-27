#pragma once

#include "../Common.h"
#include "../CPU/PortConnector.h"
#include "../Hardware/DevicePPI.h"
#include "../Hardware/Device8259.h"

using emul::BYTE;
using emul::PortConnector;

namespace kbd
{
	class DeviceKeyboard : public PortConnector
	{
	public:
		DeviceKeyboard();
		virtual ~DeviceKeyboard() {} 

		DeviceKeyboard(const DeviceKeyboard&) = delete;
		DeviceKeyboard& operator=(const DeviceKeyboard&) = delete;
		DeviceKeyboard(DeviceKeyboard&&) = delete;
		DeviceKeyboard& operator=(DeviceKeyboard&&) = delete;

		virtual void Init(ppi::DevicePPI* ppi, pic::Device8259* pic);

		virtual void Tick() = 0;

		void InputKey(BYTE ch) { LogPrintf(LOG_DEBUG, "InputKey %02Xh", ch); m_keyBuf[m_keyBufWrite++] = ch; }

	protected:
		ppi::DevicePPI* m_ppi = nullptr;
		pic::Device8259* m_pic = nullptr;

		// Keyboard buffer
		BYTE m_keyBuf[256];
		BYTE m_keyBufRead = 0;
		BYTE m_keyBufWrite = 0;
	};
}
