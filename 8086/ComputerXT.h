#pragma once

#include "CPU8086.h"
#include "Memory.h"
#include "Device8254.h"
#include "Device8255XT.h"
#include "Device8237.h"
#include "Device8259.h"
#include "DeviceFloppyXT.h"
#include "DevicePCSpeaker.h"
#include "DeviceKeyboardXT.h"
#include "VideoCGA.h"
#include "InputEvents.h"

namespace emul
{
	class ComputerXT : public CPU8086
	{
	public:
		ComputerXT();

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
		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		MemoryMap m_map;

		pit::Device8254 m_pit;
		pic::Device8259 m_pic;
		ppi::Device8255XT m_ppi;
		dma::Device8237 m_dma;
		fdc::DeviceFloppyXT m_floppy;
		video::VideoCGA m_video;
		beeper::DevicePCSpeaker m_pcSpeaker;
		kbd::DeviceKeyboardXT m_keyboard;
		events::InputEvents m_inputs;
	};
}
