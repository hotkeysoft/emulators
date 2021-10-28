#pragma once

#include "Computer.h"
#include "Device8250.h"
#include "Device8254.h"
#include "Device8255Tandy.h"
#include "Device8259.h"
#include "DevicePCSpeaker.h"
#include "DeviceSN76489.h"
#include "DeviceFloppyTandy.h"
#include "DeviceKeyboardTandy.h"
#include "VideoTandy.h"
#include "InputEvents.h"

namespace emul
{
	class ComputerTandy : public Computer, public PortConnector
	{
	public:
		ComputerTandy();

		virtual void Init() override;

		virtual bool Step() override;

		Memory& GetMemory() { return m_memory; }
		virtual fdc::DeviceFloppy& GetFloppy() override { return m_floppy; }
		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void SetRAMPage(BYTE value);

		// TODO: Should be dynamic
		emul::MemoryBlock m_base128K;
		emul::MemoryBlock m_ramExtension;
		emul::MemoryBlock m_biosFC00;

		pit::Device8254 m_pit;
		pic::Device8259 m_pic;
		ppi::Device8255Tandy m_ppi;
		fdc::DeviceFloppyTandy m_floppy;
		kbd::DeviceKeyboardTandy m_keyboard;
		uart::Device8250 m_uart;
		video::VideoTandy m_video;
		beeper::DevicePCSpeaker m_pcSpeaker;
		sn76489::DeviceSN76489 m_soundModule;
		events::InputEvents m_inputs;
	};
}
