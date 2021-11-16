#pragma once

#include "Computer.h"
#include "Device8237.h"
#include "Device8250.h"
#include "DeviceSN76489.h"
#include "DeviceFloppyTandy.h"
#include "DeviceHardDrive.h"
#include "DeviceKeyboardTandy.h"
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

		dma::Device8237 m_dma;
		fdc::DeviceFloppyTandy m_floppy;
		hdd::DeviceHardDrive m_hardDrive;

		kbd::DeviceKeyboardTandy m_keyboard;
		uart::Device8250 m_uart;
		sn76489::DeviceSN76489 m_soundModule;
		events::InputEvents m_inputs;
	};
}
