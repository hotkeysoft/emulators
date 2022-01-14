#pragma once

#include "Computer.h"
#include "Hardware/Device8237.h"
#include "Storage/DeviceFloppyXT.h"
#include "Sound/DevicePCSpeaker.h"
#include "Sound/DeviceSN76489.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardXT.h"

namespace emul
{
	class ComputerXT : public Computer
	{
	public:
		ComputerXT();

		virtual void Init(WORD baseRAM) override;

		virtual bool Step() override;

		virtual fdc::DeviceFloppy& GetFloppy() override { return m_floppy; }
		virtual kbd::DeviceKeyboard& GetKeyboard() override { return m_keyboard; }

	protected:
		void InitRAM(emul::WORD baseRAM);
		emul::MemoryBlock m_baseRAM;
		emul::MemoryBlock m_biosF000;
		emul::MemoryBlock m_biosF800;

		dma::Device8237 m_dma;
		fdc::DeviceFloppyXT m_floppy;

		sn76489::DeviceSN76489 m_soundModule;
		kbd::DeviceKeyboardXT m_keyboard;
		events::InputEvents m_inputs;
	};
}
