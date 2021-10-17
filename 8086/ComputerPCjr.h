#pragma once

#include "CPU8086.h"
#include "Memory.h"
#include "Device8250.h"
#include "Device8254.h"
#include "Device8255PCjr.h"
#include "Device8259.h"
#include "DevicePCSpeaker.h"
#include "DeviceFloppyPCjr.h"
#include "DeviceKeyboardPCjr.h"
#include "VideoPCjr.h"
#include "CartridgePCjr.h"
#include "InputEvents.h"

namespace emul
{
	class ComputerPCjr : public CPU8086
	{
	public:
		ComputerPCjr();

		void Init();

		virtual bool Step() override;

		bool LoadBinary(const char* file, ADDRESS baseAddress) { return m_memory.LoadBinary(file, baseAddress); }

		Memory& GetMemory() { return m_memory; }
		fdc::DeviceFloppy& GetFloppy() { return m_floppy; }
		kbd::DeviceKeyboard& GetKeyboard() { return m_keyboard; }

	protected:
		Memory m_memory;

		// TODO: Should be dynamic
		emul::MemoryBlock m_base64K;
		emul::MemoryBlock m_ext64K;
		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		cart::CartridgePCjr m_cart1;
		cart::CartridgePCjr m_cart2;

		MemoryMap m_map;

		pit::Device8254 m_pit;
		pic::Device8259 m_pic;
		ppi::Device8255PCjr m_ppi;
		fdc::DeviceFloppyPCjr m_floppy;
		kbd::DeviceKeyboardPCjr m_keyboard;
		uart::Device8250 m_uart;
		video::VideoPCjr m_video;
		beeper::DevicePCSpeaker m_pcSpeaker;
		events::InputEvents m_inputs;
	};
}
