#pragma once

#include "Computer.h"
#include "Device8250.h"
#include "Device8254.h"
#include "Device8255PCjr.h"
#include "Device8259.h"
#include "DevicePCSpeaker.h"
#include "DeviceSN76489.h"
#include "DeviceFloppyPCjr.h"
#include "DeviceKeyboardPCjr.h"
#include "VideoPCjr.h"
#include "CartridgePCjr.h"
#include "InputEvents.h"

using emul::WORD;

namespace emul
{
	class ComputerPCjr : public Computer
	{
	public:
		ComputerPCjr();

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		Memory& GetMemory() { return m_memory; }
		virtual fdc::DeviceFloppy& GetFloppy() override { return m_floppy; }
		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void InitRAM(WORD baseRAM);
		emul::MemoryBlock m_base64K;
		emul::MemoryBlock m_ext64K;
		emul::MemoryBlock m_extraRAM;

		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		cart::CartridgePCjr m_cart1;
		cart::CartridgePCjr m_cart2;

		pit::Device8254 m_pit;
		pic::Device8259 m_pic;
		ppi::Device8255PCjr m_ppi;
		fdc::DeviceFloppyPCjr m_floppy;
		kbd::DeviceKeyboardPCjr m_keyboard;
		uart::Device8250 m_uart;
		video::VideoPCjr m_video;
		beeper::DevicePCSpeaker m_pcSpeaker;
		sn76489::DeviceSN76489 m_soundModule;
		events::InputEvents m_inputs;
	};
}
