#pragma once

#include <Common.h>
#include "DeviceFloppy.h"
#include <vector>
#include <deque>

using emul::WORD;
using emul::BYTE;

namespace fdc
{
	class DeviceFloppyPCjr : public DeviceFloppy
	{
	public:
		DeviceFloppyPCjr(WORD baseAddress, size_t clockSpeedHz);

		DeviceFloppyPCjr() = delete;
		DeviceFloppyPCjr(const DeviceFloppyPCjr&) = delete;
		DeviceFloppyPCjr& operator=(const DeviceFloppyPCjr&) = delete;
		DeviceFloppyPCjr(DeviceFloppyPCjr&&) = delete;
		DeviceFloppyPCjr& operator=(DeviceFloppyPCjr&&) = delete;

		void Init();
		void Reset();

		void WriteDigitalOutputReg(BYTE value);

		virtual void Tick() override;

		bool IsWatchdogInterrupt() { return m_dor.wdEnable && (m_wd.counter == 0); }

		// TODO
		virtual bool IsActive(BYTE drive) override { return false; };

	protected:

		enum DOR
		{
			FDC_RESET    = 0x80, // Clear = enter reset mode, Set = normal operation
			WD_TRIGGER   = 0x40, // Start watchdog timer on 1->0 transition (3 seconds)
			WD_ENABLE    = 0x20, // Set to enable Watchdog timer
			// Reserved 0x02-0x10
			DRIVE_ENABLE = 0x01, // Drive motor runs when set
		};

		struct DigitalOutputRegister
		{
			bool wdEnable = false;
			bool wdTrigger = false;

			bool driveEnable = false;
		} m_dor;

		struct Watchdog
		{
			size_t counter = SIZE_MAX;
			bool active = false;
		} m_wd;

		void ResetWatchdog();
		void LaunchWatchdog();
	};
}
