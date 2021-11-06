#pragma once

#include "Computer.h"
#include "Device8250.h"
#include "DeviceSN76489.h"
#include "DeviceFloppyTandy.h"
#include "DeviceKeyboardTandy.h"
#include "VideoTandy.h"
#include "InputEvents.h"

using emul::WORD;

namespace emul
{
	class ComputerTandy : public Computer, public PortConnector
	{
	public:
		ComputerTandy();

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		Memory& GetMemory() { return m_memory; }
		virtual fdc::DeviceFloppy& GetFloppy() override { return m_floppy; }
		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void SetRAMPage(BYTE value);
		void InitRAM(WORD baseRAM);
		emul::MemoryBlock m_base128K;
		emul::MemoryBlock m_ramExtension;
		emul::MemoryBlock m_biosFC00;

		fdc::DeviceFloppyTandy m_floppy;
		kbd::DeviceKeyboardTandy m_keyboard;
		uart::Device8250 m_uart;
		video::VideoTandy m_video;
		sn76489::DeviceSN76489 m_soundModule;
		events::InputEvents m_inputs;
	};
}
