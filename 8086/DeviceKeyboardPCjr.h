#pragma once

#include "Common.h"
#include "Logger.h"

#include "DeviceKeyboard.h"

using emul::BYTE;

namespace kbd
{
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
	};
}
