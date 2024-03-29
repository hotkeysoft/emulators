#pragma once

#include <CPU/CPUCommon.h>
#include "DeviceFloppy.h"
#include <vector>
#include <deque>

using emul::WORD;
using emul::BYTE;

namespace fdc
{
	class DeviceFloppyTandy : public DeviceFloppy
	{
	public:
		DeviceFloppyTandy(WORD baseAddress, size_t clockSpeedHz);

		DeviceFloppyTandy() = delete;
		DeviceFloppyTandy(const DeviceFloppyTandy&) = delete;
		DeviceFloppyTandy& operator=(const DeviceFloppyTandy&) = delete;
		DeviceFloppyTandy(DeviceFloppyTandy&&) = delete;
		DeviceFloppyTandy& operator=(DeviceFloppyTandy&&) = delete;

		void Init();
		void Reset();

		void WriteDigitalOutputReg(BYTE value);

		virtual bool IsActive(BYTE drive) override { return m_dor.motor[drive]; };

	protected:

		virtual void SetDMAPending() override { m_dmaPending = m_dor.irqDMAEnabled; }
		virtual void SetInterruptPending() override { m_interruptPending = m_dor.irqDMAEnabled; }

		enum DOR
		{
			TC    = 0x40, // Set = FDC Terminal Count
			MOTB  = 0x20, // Set to turn drive 1's motor ON
			MOTA  = 0x10, // Set to turn drive 0's motor ON
			IRQ   = 0x08, // Set to enable IRQs and DMA
			RESET = 0x04, // Clear = enter reset mode, Set = normal operation
			DSEL1 = 0x02, // "Select" drive number for next access
			DSEL0 = 0x01,
		};

		struct DigitalOutputRegister
		{
			bool motor[2] = { false, false };
			bool irqDMAEnabled = false;
			BYTE driveSel = 0;
			bool terminalCount = false;
		} m_dor;
	};
}
