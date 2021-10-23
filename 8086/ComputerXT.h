#pragma once

#include "Computer.h"
#include "Device8254.h"
#include "Device8255XT.h"
#include "Device8237.h"
#include "Device8259.h"
#include "DeviceFloppyXT.h"
#include "DevicePCSpeaker.h"
#include "DeviceSN76489.h"
#include "DeviceKeyboardXT.h"
#include "VideoCGA.h"
#include "VideoMDA.h"
#include "InputEvents.h"

namespace emul
{
	class ComputerXT : public Computer
	{
	public:
		ComputerXT();

		virtual void Init() override;

		virtual bool Step() override;

		virtual fdc::DeviceFloppy& GetFloppy() override { return m_floppy; }
		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		// TODO: Should be dynamic
		emul::MemoryBlock m_base64K;
		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		pit::Device8254 m_pit;
		pic::Device8259 m_pic;
		ppi::Device8255XT m_ppi;
		dma::Device8237 m_dma;
		fdc::DeviceFloppyXT m_floppy;

		video::VideoCGA m_videoCGA;
		video::VideoMDA m_videoMDA;

		beeper::DevicePCSpeaker m_pcSpeaker;
		sn76489::DeviceSN76489 m_soundModule;
		kbd::DeviceKeyboardXT m_keyboard;
		events::InputEvents m_inputs;
	};
}
