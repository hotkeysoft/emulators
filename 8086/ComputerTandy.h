#pragma once

#include "Computer.h"
#include "Hardware/Device8237.h"
#include "Hardware/Device8250.h"
#include "Sound/DeviceSN76489.h"
#include "Storage/DeviceFloppyTandy.h"
#include "IO/InputEvents.h"
#include "IO/DeviceKeyboardTandy.h"

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

		dma::Device8237 m_dma;
		fdc::DeviceFloppyTandy m_floppy;

		kbd::DeviceKeyboardTandy m_keyboard;
		uart::Device8250 m_uart;
		sn76489::DeviceSN76489 m_soundModule;
		events::InputEvents m_inputs;
	};
}
