#pragma once

#include "Common.h"
#include "DeviceFloppy.h"
#include <vector>
#include <deque>

using emul::WORD;
using emul::BYTE;

namespace fdc
{
	class DeviceFloppyXT : public DeviceFloppy
	{
	public:
		DeviceFloppyXT(WORD baseAddress, size_t clockSpeedHz);

		DeviceFloppyXT() = delete;
		DeviceFloppyXT(const DeviceFloppyXT&) = delete;
		DeviceFloppyXT& operator=(const DeviceFloppyXT&) = delete;
		DeviceFloppyXT(DeviceFloppyXT&&) = delete;
		DeviceFloppyXT& operator=(DeviceFloppyXT&&) = delete;

		void Init();
		void Reset();

		void WriteDigitalOutputReg(BYTE value);

	protected:

		virtual void SetDMAPending() override { m_dmaPending = m_dor.irqDMAEnabled; }
		virtual void SetInterruptPending() override { m_interruptPending = m_dor.irqDMAEnabled; }

		enum DOR
		{
			MOTD  = 0x80, // Set to turn drive 3's motor ON
			MOTC  = 0x40, // Set to turn drive 2's motor ON
			MOTB  = 0x20, // Set to turn drive 1's motor ON
			MOTA  = 0x10, // Set to turn drive 0's motor ON
			IRQ   = 0x08, // Set to enable IRQs and DMA
			RESET = 0x04, // Clear = enter reset mode, Set = normal operation
			DSEL1 = 0x02, // "Select" drive number for next access
			DSEL0 = 0x01,
		};

		struct DigitalOutputRegister
		{
			bool motor[4] = { false, false, false, false };
			bool irqDMAEnabled = false;
			BYTE driveSel = 0;
		} m_dor;
	};
}
